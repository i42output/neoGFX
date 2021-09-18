// opengl_rendering_context.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <neogfx/neogfx.hpp>
#include <boost/math/constants/constants.hpp>
#include <neolib/core/thread_local.hpp>
#include <neolib/app/i_power.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gfx/text/i_emoji_atlas.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/text/i_glyph_texture.hpp>
#include <neogfx/gfx/shapes.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/game/rectangle.hpp>
#include <neogfx/game/text_mesh.hpp>
#include <neogfx/game/ecs_helpers.hpp>
#include <neogfx/hid/i_native_surface.hpp>
#include "i_native_texture.hpp"
#include "../text/native/i_native_font_face.hpp"
#include "opengl_rendering_context.hpp"

namespace neogfx
{
    namespace 
    {
        template <typename T>
        class skip_iterator
        {
        public:
            typedef T value_type;
        public:
            skip_iterator(const value_type* aBegin, const value_type* aEnd, std::size_t aSkipAmount = 2u, std::size_t aPasses = 1) :
                iBegin{ aBegin }, iEnd{ aEnd }, iSkipAmount{ std::max<std::size_t>(1u, std::min<std::size_t>(aEnd - aBegin, aSkipAmount)) }, iPasses{ aPasses }, iNext{ aBegin }, iSkipPass{ 1u }, iPass{ 1u }
            {
            }
            skip_iterator() = delete;
        public:
            std::size_t skip_amount() const
            {
                return iSkipAmount;
            }
            std::size_t pass() const
            {
                return iPass;
            }
        public:
            skip_iterator& operator++()
            {
                if (static_cast<std::size_t>(iEnd - iNext) <= iSkipAmount)
                {
                    if (iSkipPass < iSkipAmount)
                        iNext = iBegin + iSkipPass;
                    else
                    {
                        if (iPass >= iPasses)
                            iNext = iEnd;
                        else
                        {
                            ++iPass;
                            iSkipPass = 0u;
                            iNext = iBegin;
                        }
                    }
                    ++iSkipPass;
                }
                else
                    iNext += iSkipAmount;
                return *this;
            }
            value_type const& operator*() const
            {
                return *iNext;
            }
            bool operator==(const graphics_operation::operation* aTest) const
            {
                return iNext == aTest;
            }
            bool operator!=(const graphics_operation::operation* aTest) const
            {
                return !operator==(aTest);
            }
        private:
            const value_type* iBegin;
            const value_type* iEnd;
            std::size_t iSkipAmount;
            std::size_t iPasses;
            const value_type* iNext;
            std::size_t iSkipPass;
            std::size_t iPass;
        };

        inline vertices line_loop_to_lines(const vertices& aLineLoop, bool aClosed = true)
        {
            vertices result;
            result.reserve(aLineLoop.size() * 2);
            for (auto v = aLineLoop.begin(); v != aLineLoop.end(); ++v)
            {
                result.push_back(*v);
                if (v != aLineLoop.begin() && (aClosed || v != std::prev(aLineLoop.end())))
                    result.push_back(*v);
            }
            if (aClosed)
                result.push_back(*aLineLoop.begin());
            return result;
        }

        inline vertices line_strip_to_lines(const vertices& aLineStrip, bool aClosed = true)
        {
            return line_loop_to_lines(aLineStrip, false);
        }

        inline quad line_to_quad(const vec3& aStart, const vec3& aEnd, double aLineWidth)
        {
            auto const vecLine = aEnd - aStart;
            auto const length = vecLine.magnitude();
            auto const halfWidth = aLineWidth / 2.0;
            auto const v1 = vec3{ -halfWidth, -halfWidth, 0.0 };
            auto const v2 = vec3{ -halfWidth, halfWidth, 0.0 };
            auto const v3 = vec3{ length + halfWidth, halfWidth, 0.0 };
            auto const v4 = vec3{ length + halfWidth, -halfWidth, 0.0 };
            auto const r = rotation_matrix(vec3{ 1.0, 0.0, 0.0 }, vecLine);
            return quad{ aStart + r * v1, aStart + r * v2, aStart + r * v3, aStart + r * v4 };
        }

        template <typename VerticesIn, typename VerticesOut>
        inline void lines_to_quads(const VerticesIn& aLines, double aLineWidth, VerticesOut& aQuads)
        {
            for (auto v = aLines.begin(); v != aLines.end(); v += 2)
            {
                quad const q = line_to_quad(v[0], v[1], aLineWidth);
                aQuads.insert(aQuads.end(), q.begin(), q.end());
            }
        }

        template <typename VerticesIn, typename VerticesOut>
        inline void quads_to_triangles(const VerticesIn& aQuads, VerticesOut& aTriangles)
        {
            for (auto v = aQuads.begin(); v != aQuads.end(); v += 4)
            {
                aTriangles.push_back(v[0]);
                aTriangles.push_back(v[1]);
                aTriangles.push_back(v[2]);
                aTriangles.push_back(v[0]);
                aTriangles.push_back(v[3]);
                aTriangles.push_back(v[2]);
            }
        }

        inline GLenum path_shape_to_gl_mode(path_shape aShape)
        {
            switch (aShape)
            {
            case path_shape::Quads:
                return GL_QUADS;
            case path_shape::Lines:
                return GL_LINES;
            case path_shape::LineLoop:
                return GL_LINE_LOOP;
            case path_shape::LineStrip:
                return GL_LINE_STRIP;
            case path_shape::ConvexPolygon:
                return GL_TRIANGLE_FAN;
            default:
                return GL_POINTS;
            }
        }

        inline vertices path_vertices(const path& aPath, const path::sub_path_type& aSubPath, double aLineWidth, GLenum& aMode)
        {
            aMode = path_shape_to_gl_mode(aPath.shape());
            neogfx::vertices vertices = aPath.to_vertices(aSubPath);
            if (aMode == GL_LINE_LOOP)
            {
                aMode = GL_LINES;
                vertices = line_loop_to_lines(vertices);
            }
            else if (aMode == GL_LINE_STRIP)
            {
                aMode = GL_LINES;
                vertices = line_strip_to_lines(vertices);
            }
            if (aMode == GL_LINES)
            {
                aMode = GL_QUADS;
                lines_to_quads(neogfx::vertices{ std::move(vertices) }, aLineWidth, vertices);
            }
            if (aMode == GL_QUADS)
            {
                aMode = GL_TRIANGLES;
                quads_to_triangles(neogfx::vertices{ std::move(vertices) }, vertices);
            }
            return vertices;
        }

        void emit_any_stipple(i_rendering_context& aContext, use_vertex_arrays& aInstance, bool aLoop = false)
        {
            // assumes vertices are quads (as two triangles) created with quads_to_triangles above.
            auto& stippleShader = aContext.rendering_engine().default_shader_program().stipple_shader();
            if (stippleShader.stipple_active())
            {
                auto start = midpoint(aInstance.begin()->xyz, std::next(aInstance.begin())->xyz);
                auto end = midpoint(std::next(aInstance.begin(), 4)->xyz, std::next(aInstance.begin(), 2)->xyz);
                stippleShader.start(aContext, start);
                aInstance.draw(6u);
                auto positionOffset = 0.0;
                while (!aInstance.empty())
                {
                    positionOffset += start.distance(end);
                    start = midpoint(aInstance.begin()->xyz, std::next(aInstance.begin())->xyz);
                    if (aLoop)
                        positionOffset += start.distance(end);
                    end = midpoint(std::next(aInstance.begin(), 4)->xyz, std::next(aInstance.begin(), 2)->xyz);
                    stippleShader.next(aContext, start, positionOffset);
                    aInstance.draw(6u);
                }
            }
        }

        template <typename ColorContainer, typename T>
        inline vec4f to_function(ColorContainer const& aColor, T const& aValue)
        {
            if (std::holds_alternative<gradient>(aColor))
            {
                auto const& value = static_variant_cast<const gradient&>(aColor).bounding_box();
                if (value != std::nullopt)
                    return value->as<float>().to_vec4();
            }
            return aValue.as<float>().to_vec4();
        }
    }

    opengl_rendering_context::opengl_rendering_context(const i_render_target& aTarget, neogfx::blending_mode aBlendingMode) :
        iRenderingEngine{ service<i_rendering_engine>() },
        iTarget{ aTarget }, 
        iWidget{ nullptr },
        iInFlush{ false },
        iMultisample{ true },
        iOpacity{ 1.0 },
        iSubpixelRendering{ rendering_engine().is_subpixel_rendering_on() },
        iSnapToPixel{ false },
        iUseDefaultShaderProgram{ *this, rendering_engine().default_shader_program() }
    {
        set_blending_mode(aBlendingMode);
        set_smoothing_mode(neogfx::smoothing_mode::AntiAlias);
        iSink += render_target().target_deactivating([&]() 
        { 
            flush(); 
            glCheck(glDisable(GL_SCISSOR_TEST));
        });
        iSink += render_target().target_activating([&]()
        {
            apply_scissor();
        });
    }

    opengl_rendering_context::opengl_rendering_context(const i_render_target& aTarget, const i_widget& aWidget, neogfx::blending_mode aBlendingMode) :
        iRenderingEngine{ service<i_rendering_engine>() },
        iTarget{ aTarget },
        iWidget{ &aWidget },
        iInFlush{ false },
        iLogicalCoordinateSystem{ aWidget.logical_coordinate_system() },
        iMultisample{ true },
        iOpacity{ 1.0 },
        iSubpixelRendering{ rendering_engine().is_subpixel_rendering_on() },
        iSnapToPixel{ false },
        iUseDefaultShaderProgram{ *this, rendering_engine().default_shader_program() }
    {
        set_blending_mode(aBlendingMode);
        set_smoothing_mode(neogfx::smoothing_mode::AntiAlias);
        iSink += render_target().target_deactivating([&]()
        {
            flush();
            glCheck(glDisable(GL_SCISSOR_TEST));
        });
        iSink += render_target().target_activating([&]()
        {
            apply_scissor();
        });
    }

    opengl_rendering_context::opengl_rendering_context(const opengl_rendering_context& aOther) :
        iRenderingEngine{ aOther.iRenderingEngine },
        iTarget{ aOther.iTarget },
        iWidget{ aOther.iWidget },
        iInFlush{ false },
        iLogicalCoordinateSystem{ aOther.iLogicalCoordinateSystem },
        iLogicalCoordinates{ aOther.iLogicalCoordinates },
        iMultisample{ true },
        iOpacity{ 1.0 },
        iSubpixelRendering{ aOther.iSubpixelRendering },
        iSnapToPixel{ false },
        iUseDefaultShaderProgram{ *this, rendering_engine().default_shader_program() }
    {
        set_blending_mode(aOther.blending_mode());
        set_smoothing_mode(aOther.smoothing_mode());
        iSink += render_target().target_deactivating([&]()
        {
            flush();
            glCheck(glDisable(GL_SCISSOR_TEST));
        });
        iSink += render_target().target_activating([&]()
        {
            apply_scissor();
        });
    }

    opengl_rendering_context::~opengl_rendering_context()
    {
    }

    std::unique_ptr<i_rendering_context> opengl_rendering_context::clone() const
    {
        return std::unique_ptr<i_rendering_context>(new opengl_rendering_context(*this));
    }

    i_rendering_engine& opengl_rendering_context::rendering_engine() const
    {
        return iRenderingEngine;
    }

    const i_render_target& opengl_rendering_context::render_target() const
    {
        return iTarget;
    }

    rect opengl_rendering_context::rendering_area(bool aConsiderScissor) const
    {
        if (scissor_rect() == std::nullopt || !aConsiderScissor)
            return rect{ render_target().target_origin(), render_target().target_extents() };
        else
            return *scissor_rect();
    }

    neogfx::logical_coordinate_system opengl_rendering_context::logical_coordinate_system() const
    {
        if (iLogicalCoordinateSystem != std::nullopt)
            return *iLogicalCoordinateSystem;
        return render_target().logical_coordinate_system();
    }

    void opengl_rendering_context::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
    {
        iLogicalCoordinateSystem = aSystem;
    }

    logical_coordinates opengl_rendering_context::logical_coordinates() const
    {
        if (iLogicalCoordinates != std::nullopt)
            return *iLogicalCoordinates;
        auto result = render_target().logical_coordinates();
        if (logical_coordinate_system() != render_target().logical_coordinate_system())
        {
            switch (logical_coordinate_system())
            {
            case neogfx::logical_coordinate_system::Specified:
                break;
            case neogfx::logical_coordinate_system::AutomaticGame:
                if (render_target().logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui)
                    std::swap(result.bottomLeft.y, result.topRight.y);
                break;
            case neogfx::logical_coordinate_system::AutomaticGui:
                std::swap(result.bottomLeft.y, result.topRight.y);
                break;
            }
        }
        return result;
    }

    void opengl_rendering_context::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates)
    {
        iLogicalCoordinates = aCoordinates;
    }

    point opengl_rendering_context::origin() const
    {
        return iOrigin;
    }

    void opengl_rendering_context::set_origin(const point& aOrigin)
    {
        iOrigin = aOrigin;
    }

    vec2 opengl_rendering_context::offset() const
    {
        return (iOffset != std::nullopt ? *iOffset : vec2{}) + (snap_to_pixel() ? 0.5 : 0.0);
    }

    void opengl_rendering_context::set_offset(const optional_vec2& aOffset)
    {
        iOffset = aOffset;
    }

    bool opengl_rendering_context::gradient_set() const
    {
        return !!iGradient;
    }

    void opengl_rendering_context::apply_gradient(i_gradient_shader& aShader)
    {
        aShader.set_gradient(*this, *iGradient, iOpacity);
    }

    bool opengl_rendering_context::snap_to_pixel() const
    {
        return iSnapToPixel;
    }

    void opengl_rendering_context::set_snap_to_pixel(bool aSnapToPixel)
    {
        iSnapToPixel = true;
    }

    const graphics_operation::queue& opengl_rendering_context::queue() const
    {
        return iQueue;
    }

    graphics_operation::queue& opengl_rendering_context::queue()
    {
        return const_cast<graphics_operation::queue&>(to_const(*this).queue());
    }

    void opengl_rendering_context::enqueue(const graphics_operation::operation& aOperation)
    {
        queue().push_back(aOperation);
    }

    void opengl_rendering_context::flush()
    {
        if (iInFlush)
            return;

        neolib::scoped_flag sf{ iInFlush };

        if (queue().empty())
            return;

        scoped_render_target srt{ render_target() };
        set_blending_mode(blending_mode());
        apply_scissor();

        for (auto batchStart = queue().begin(); batchStart != queue().end();)
        {
            auto batchEnd = std::next(batchStart);
            while (batchEnd != queue().end() && graphics_operation::batchable(*batchStart, *batchEnd))
                ++batchEnd;
            graphics_operation::batch const opBatch{ &*batchStart, &*batchStart + (batchEnd - batchStart) };
            batchStart = batchEnd;
            switch (opBatch.first->index())
            {
            case graphics_operation::operation_type::SetLogicalCoordinateSystem:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    set_logical_coordinate_system(static_variant_cast<const graphics_operation::set_logical_coordinate_system&>(*op).system);
                break;
            case graphics_operation::operation_type::SetLogicalCoordinates:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    set_logical_coordinates(static_variant_cast<const graphics_operation::set_logical_coordinates&>(*op).coordinates);
                break;
            case graphics_operation::operation_type::SetOrigin:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    set_origin(static_variant_cast<const graphics_operation::set_origin&>(*op).origin);
                break;
            case graphics_operation::operation_type::SetViewport:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& setViewport = static_variant_cast<const graphics_operation::set_viewport&>(*op);
                    if (setViewport.rect)
                        render_target().set_viewport(setViewport.rect->as<int32_t>());
                    else
                        render_target().set_viewport(rect{ render_target().target_origin(), render_target().extents() }.as<int32_t>());
                }
                break;
            case graphics_operation::operation_type::ScissorOn:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    scissor_on(static_variant_cast<const graphics_operation::scissor_on&>(*op).rect);
                break;
            case graphics_operation::operation_type::ScissorOff:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    (void)op;
                    scissor_off();
                }
                break;
            case graphics_operation::operation_type::SnapToPixelOn:
                set_snap_to_pixel(true);
                break;
            case graphics_operation::operation_type::SnapToPixelOff:
                set_snap_to_pixel(false);
                break;
            case graphics_operation::operation_type::SetOpacity:
                set_opacity(static_variant_cast<const graphics_operation::set_opacity&>(*(std::prev(opBatch.second))).opacity);
                break;
            case graphics_operation::operation_type::SetBlendingMode:
                set_blending_mode(static_variant_cast<const graphics_operation::set_blending_mode&>(*(std::prev(opBatch.second))).blendingMode);
                break;
            case graphics_operation::operation_type::SetSmoothingMode:
                set_smoothing_mode(static_variant_cast<const graphics_operation::set_smoothing_mode&>(*(std::prev(opBatch.second))).smoothingMode);
                break;
            case graphics_operation::operation_type::PushLogicalOperation:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    push_logical_operation(static_variant_cast<const graphics_operation::push_logical_operation&>(*op).logicalOperation);
                break;
            case graphics_operation::operation_type::PopLogicalOperation:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    (void)op;
                    pop_logical_operation();
                }
                break;
            case graphics_operation::operation_type::LineStippleOn:
                {
                    auto const& lso = static_variant_cast<const graphics_operation::line_stipple_on&>(*(std::prev(opBatch.second)));
                    line_stipple_on(lso.factor, lso.pattern, lso.position);
                }
                break;
            case graphics_operation::operation_type::LineStippleOff:
                line_stipple_off();
                break;
            case graphics_operation::operation_type::SubpixelRenderingOn:
                subpixel_rendering_on();
                break;
            case graphics_operation::operation_type::SubpixelRenderingOff:
                subpixel_rendering_off();
                break;
            case graphics_operation::operation_type::Clear:
                clear(static_variant_cast<const graphics_operation::clear&>(*(std::prev(opBatch.second))).color);
                break;
            case graphics_operation::operation_type::ClearDepthBuffer:
                clear_depth_buffer();
                break;
            case graphics_operation::operation_type::ClearStencilBuffer:
                clear_stencil_buffer();
                break;
            case graphics_operation::operation_type::SetGradient:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    set_gradient(static_variant_cast<const graphics_operation::set_gradient&>(*op).gradient);
                break;
            case graphics_operation::operation_type::ClearGradient:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    clear_gradient();
                break;
            case graphics_operation::operation_type::SetPixel:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    set_pixel(static_variant_cast<const graphics_operation::set_pixel&>(*op).point, static_variant_cast<const graphics_operation::set_pixel&>(*op).color);
                break;
            case graphics_operation::operation_type::DrawPixel:
                draw_pixels(opBatch);
                break;
            case graphics_operation::operation_type::DrawLine:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_line&>(*op);
                    draw_line(args.from, args.to, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawRect:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_rect&>(*op);
                    draw_rect(args.rect, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawRoundedRect:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_rounded_rect&>(*op);
                    draw_rounded_rect(args.rect, args.radius, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawCircle:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_circle&>(*op);
                    draw_circle(args.center, args.radius, args.pen, args.startAngle);
                }
                break;
            case graphics_operation::operation_type::DrawArc:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_arc&>(*op);
                    draw_arc(args.center, args.radius, args.startAngle, args.endAngle, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawCubicBezier:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_cubic_bezier&>(*op);
                    draw_cubic_bezier(args.p0, args.p1, args.p2, args.p3, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawPath:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_path&>(*op);
                    draw_path(args.path, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawShape:
                // todo: batch
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_shape&>(*op);
                    draw_shape(args.mesh, args.position, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawEntities:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_entities&>(*op);
                    draw_entities(args.ecs, args.layer, args.transformation);
                }
                break;
            case graphics_operation::operation_type::FillRect:
                fill_rects(opBatch);
                break;
            case graphics_operation::operation_type::FillRoundedRect:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::fill_rounded_rect&>(*op);
                    fill_rounded_rect(args.rect, args.radius, args.fill);
                }
                break;
            case graphics_operation::operation_type::FillCheckerRect:
                fill_checker_rects(opBatch);
                break;
            case graphics_operation::operation_type::FillCircle:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::fill_circle&>(*op);
                    fill_circle(args.center, args.radius, args.fill);
                }
                break;
            case graphics_operation::operation_type::FillArc:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::fill_arc&>(*op);
                    fill_arc(args.center, args.radius, args.startAngle, args.endAngle, args.fill);
                }
                break;
            case graphics_operation::operation_type::FillPath:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    fill_path(static_variant_cast<const graphics_operation::fill_path&>(*op).path, static_variant_cast<const graphics_operation::fill_path&>(*op).fill);
                break;
            case graphics_operation::operation_type::FillShape:
                fill_shapes(opBatch);
                break;
            case graphics_operation::operation_type::DrawGlyph:
                draw_glyphs(opBatch);
                break;
            case graphics_operation::operation_type::DrawMesh:
                // todo: use draw_meshes
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_mesh&>(*op);
                    draw_mesh(args.mesh, args.material, args.transformation, args.filter);
                }
                break;
            }
        }
        queue().clear();
    }

    void opengl_rendering_context::scissor_on(const rect& aRect)
    {
        iScissorRects.push_back(aRect);
        iScissorRect = std::nullopt;
        apply_scissor();
    }

    void opengl_rendering_context::scissor_off()
    {
        if (!iScissorRects.empty())
            iScissorRects.pop_back();
        iScissorRect = std::nullopt;
        apply_scissor();
    }

    const optional_rect& opengl_rendering_context::scissor_rect() const
    {
        if (iScissorRect == std::nullopt && !iScissorRects.empty())
        {
            for (auto const& rect : iScissorRects)
                if (iScissorRect != std::nullopt)
                    iScissorRect = iScissorRect->intersection(rect);
                else
                    iScissorRect = rect;
        }
        return iScissorRect;
    }

    void opengl_rendering_context::apply_scissor()
    {
        auto sr = scissor_rect();
        if (sr != std::nullopt)
        {
            glCheck(glEnable(GL_SCISSOR_TEST));
            GLint x = static_cast<GLint>(std::ceil(sr->x));
            GLint y = static_cast<GLint>(logical_coordinates().is_gui_orientation() ? std::ceil(rendering_area(false).cy - sr->cy - sr->y) : sr->y);
            GLsizei cx = static_cast<GLsizei>(std::ceil(sr->cx));
            GLsizei cy = static_cast<GLsizei>(std::ceil(sr->cy));
            glCheck(glScissor(x, y, cx, cy));
        }
        else
        {
            glCheck(glDisable(GL_SCISSOR_TEST));
        }
    }

    bool opengl_rendering_context::multisample() const
    {
        return iMultisample;
    }
    
    void opengl_rendering_context::set_multisample(bool aMultisample)
    {
        if (iMultisample != aMultisample)
        {
            iMultisample = aMultisample;
            if (multisample())
            {
                glCheck(glEnable(GL_MULTISAMPLE));
            }
            else
            {
                glCheck(glDisable(GL_MULTISAMPLE));
            }
        }
    }

    void opengl_rendering_context::enable_sample_shading(double aSampleShadingRate)
    {
        if (iSampleShadingRate == std::nullopt || iSampleShadingRate != aSampleShadingRate)
        {
            iSampleShadingRate = aSampleShadingRate;
            glCheck(glEnable(GL_SAMPLE_SHADING));
            glCheck(glMinSampleShading(1.0));
        }
    }

    void opengl_rendering_context::disable_sample_shading()
    {
        if (iSampleShadingRate != std::nullopt)
        {
            iSampleShadingRate = std::nullopt;
            glCheck(glDisable(GL_SAMPLE_SHADING));
        }
    }
    
    void opengl_rendering_context::set_opacity(double aOpacity)
    {
        iOpacity = aOpacity;
    }

    neogfx::blending_mode opengl_rendering_context::blending_mode() const
    {
        return *iBlendingMode;
    }

    void opengl_rendering_context::set_blending_mode(neogfx::blending_mode aBlendingMode)
    {
        if (iBlendingMode == std::nullopt || *iBlendingMode != aBlendingMode)
        {
            iBlendingMode = aBlendingMode;
            switch (*iBlendingMode)
            {
            case neogfx::blending_mode::None:
                glCheck(glDisable(GL_BLEND));
                break;
            case neogfx::blending_mode::Blit:
                glCheck(glEnable(GL_BLEND));
                glCheck(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
                break;
            case neogfx::blending_mode::Default:
                glCheck(glEnable(GL_BLEND));
                glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                break;
            }
        }
    }

    smoothing_mode opengl_rendering_context::smoothing_mode() const
    {
        return *iSmoothingMode;
    }

    void opengl_rendering_context::set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode)
    {
        if (iSmoothingMode == std::nullopt || *iSmoothingMode != aSmoothingMode)
        {
            iSmoothingMode = aSmoothingMode;
            if (*iSmoothingMode == neogfx::smoothing_mode::AntiAlias)
            {
                glCheck(glEnable(GL_LINE_SMOOTH));
                glCheck(glEnable(GL_POLYGON_SMOOTH));
            }
            else
            {
                glCheck(glDisable(GL_LINE_SMOOTH));
                glCheck(glDisable(GL_POLYGON_SMOOTH));
            }
        }
    }

    void opengl_rendering_context::push_logical_operation(logical_operation aLogicalOperation)
    {
        iLogicalOperationStack.push_back(aLogicalOperation);
        apply_logical_operation();
    }

    void opengl_rendering_context::pop_logical_operation()
    {
        if (!iLogicalOperationStack.empty())
            iLogicalOperationStack.pop_back();
        apply_logical_operation();
    }

    void opengl_rendering_context::apply_logical_operation()
    {
        if (iLogicalOperationStack.empty() || iLogicalOperationStack.back() == logical_operation::None)
        {
            glCheck(glDisable(GL_COLOR_LOGIC_OP));
        }
        else
        {
            glCheck(glEnable(GL_COLOR_LOGIC_OP));
            switch (iLogicalOperationStack.back())
            {
            case logical_operation::Xor:
                glCheck(glLogicOp(GL_XOR));
                break;
            }
        }    
    }

    void opengl_rendering_context::line_stipple_on(scalar aFactor, uint16_t aPattern, scalar aPosition)
    {
        rendering_engine().default_shader_program().stipple_shader().set_stipple(aFactor, aPattern, aPosition);
    }

    void opengl_rendering_context::line_stipple_off()
    {
        rendering_engine().default_shader_program().stipple_shader().clear_stipple();
    }

    void opengl_rendering_context::set_gradient(const gradient& aGradient)
    {
        iGradient = aGradient;
    }

    void opengl_rendering_context::clear_gradient()
    {
        iGradient = std::nullopt;
    }

    bool opengl_rendering_context::is_subpixel_rendering_on() const
    {
        return iSubpixelRendering;
    }

    void opengl_rendering_context::subpixel_rendering_on()
    {
        iSubpixelRendering = true;
    }

    void opengl_rendering_context::subpixel_rendering_off()
    {
        iSubpixelRendering = false;
    }

    void opengl_rendering_context::clear(const color& aColor)
    {
        glCheck(glClearColor(aColor.red<GLclampf>(), aColor.green<GLclampf>(), aColor.blue<GLclampf>(), aColor.alpha<GLclampf>()));
        glCheck(glClear(GL_COLOR_BUFFER_BIT));
    }

    void opengl_rendering_context::clear_depth_buffer()
    {
        glCheck(glClearDepth(1.0));
        glCheck(glClear(GL_DEPTH_BUFFER_BIT));
    }

    void opengl_rendering_context::clear_stencil_buffer()
    {
        glCheck(glStencilMask(static_cast<GLuint>(-1)));
        glCheck(glClearStencil(0xFF));
        glCheck(glClear(GL_STENCIL_BUFFER_BIT));
    }

    void opengl_rendering_context::set_pixel(const point& aPoint, const color& aColor)
    {
        /* todo: faster alternative to this... */
        disable_anti_alias daa{ *this };
        draw_pixel(aPoint, aColor.with_alpha(1.0));
    }

    void opengl_rendering_context::draw_pixel(const point& aPoint, const color& aColor)
    {
        graphics_operation::operation op{ graphics_operation::draw_pixel{ aPoint, aColor } };
        draw_pixels(graphics_operation::batch{ &op, &op + 1 });
    }

    void opengl_rendering_context::draw_pixels(const graphics_operation::batch& aDrawPixelOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };
        scoped_anti_alias saa{ *this, smoothing_mode::None };
        disable_multisample disableMultisample{ *this };

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 3u * (aDrawPixelOps.second - aDrawPixelOps.first)) };

            for (auto op = aDrawPixelOps.first; op != aDrawPixelOps.second; ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_pixel&>(*op);
                auto rectVertices = rect_vertices(rect{ drawOp.point, size{1.0, 1.0} }, mesh_type::Triangles, 0);
                for (auto const& v : rectVertices)
                    vertexArrays.push_back({ v,
                            vec4f{{
                                drawOp.color.red<float>(),
                                drawOp.color.green<float>(),
                                drawOp.color.blue<float>(),
                                drawOp.color.alpha<float>() * static_cast<float>(iOpacity)}} });
            }
        }
    }

    void opengl_rendering_context::draw_line(const point& aFrom, const point& aTo, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.color()), iOpacity);

        auto v1 = aFrom.to_vec3();
        auto v2 = aTo.to_vec3();
        if (snap_to_pixel() && static_cast<int32_t>(aPen.width()) % 2 == 0)
        {
            v1 -= vec3{ 0.5, 0.5, 0.0 };
            v2 -= vec3{ 0.5, 0.5, 0.0 };
        }

        vec3_array<2> line = { v1, v2 };
        vec3_array<4> quad;
        lines_to_quads(line, aPen.width(), quad);
        vec3_array<6> triangles;
        quads_to_triangles(quad, triangles);

        use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, triangles.size() };

        auto const function = to_function(aPen.color(), basic_rect<float>{ aFrom, aTo });

        for (auto const& v : triangles)
            vertexArrays.push_back({ v, std::holds_alternative<color>(aPen.color()) ?
                vec4f{{
                    static_variant_cast<color>(aPen.color()).red<float>(),
                    static_variant_cast<color>(aPen.color()).green<float>(),
                    static_variant_cast<color>(aPen.color()).blue<float>(),
                    static_variant_cast<color>(aPen.color()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{},
                {},
                function });

        emit_any_stipple(*this, vertexArrays);
    }

    void opengl_rendering_context::draw_rect(const rect& aRect, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        scoped_anti_alias saa{ *this, smoothing_mode::None };
        std::optional<disable_multisample> disableMultisample;

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.color()), iOpacity);

        auto adjustedRect = aRect;
        if (snap_to_pixel())
        {
            disableMultisample.emplace(*this);
            bool const oddWidth = static_cast<int32_t>(aPen.width()) % 2 == 1;
            adjustedRect.position() -= size{ oddWidth ? 0.0 : 0.5 };
            if (oddWidth)
                adjustedRect = adjustedRect.with_epsilon(size{ 1.0 });
        }
        else
            adjustedRect.inflate(size{ aPen.width() / 2.0 }.floor());

        vec3_array<8> lines = rect_vertices(adjustedRect, mesh_type::Outline, 0.0);
        lines[1].x -= (aPen.width() + rect::default_epsilon);
        lines[3].y -= (aPen.width() + rect::default_epsilon);
        lines[5].x += (aPen.width() + rect::default_epsilon);
        lines[7].y += (aPen.width() + rect::default_epsilon);
        vec3_array<4 * 4> quads;
        lines_to_quads(lines, aPen.width(), quads);
        vec3_array<4 * 6> triangles;
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, triangles.size() };

        auto const function = to_function(aPen.color(), aRect);

        for (auto const& v : triangles)
            vertexArrays.push_back({ v, std::holds_alternative<color>(aPen.color()) ?
                vec4f{{
                    static_variant_cast<color>(aPen.color()).red<float>(),
                    static_variant_cast<color>(aPen.color()).green<float>(),
                    static_variant_cast<color>(aPen.color()).blue<float>(),
                    static_variant_cast<color>(aPen.color()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{},
                {},
                function });

        emit_any_stipple(*this, vertexArrays);
    }

    void opengl_rendering_context::draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.color()), iOpacity);

        auto adjustedRect = aRect;
        if (snap_to_pixel())
        {
            bool const oddWidth = static_cast<int32_t>(aPen.width()) % 2 == 1;
            adjustedRect.position() -= size{ oddWidth ? 0.0 : 0.5 };
            if (oddWidth)
                adjustedRect = adjustedRect.with_epsilon(size{ 1.0 });
        }
        else
            adjustedRect.inflate(size{ aPen.width() / 2.0 }.floor());

        auto vertices = rounded_rect_vertices(adjustedRect, aRadius, mesh_type::Outline);

        auto lines = line_loop_to_lines(vertices);
        thread_local vec3_list quads;
        quads.clear();
        lines_to_quads(lines, aPen.width(), quads);
        thread_local vec3_list triangles;
        triangles.clear();
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, triangles.size() };

        auto const function = to_function(aPen.color(), aRect);

        for (auto const& v : triangles)
            vertexArrays.push_back({ v, std::holds_alternative<color>(aPen.color()) ?
                vec4f{{
                    static_variant_cast<color>(aPen.color()).red<float>(),
                    static_variant_cast<color>(aPen.color()).green<float>(),
                    static_variant_cast<color>(aPen.color()).blue<float>(),
                    static_variant_cast<color>(aPen.color()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{},
                {},
                function });

        emit_any_stipple(*this, vertexArrays);
    }

    void opengl_rendering_context::draw_circle(const point& aCenter, dimension aRadius, const pen& aPen, angle aStartAngle)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.color()), iOpacity);

        auto vertices = circle_vertices(aCenter, aRadius, aStartAngle, mesh_type::Outline);

        auto lines = line_loop_to_lines(vertices);
        thread_local vec3_list quads;
        quads.clear();
        lines_to_quads(lines, aPen.width(), quads);
        thread_local vec3_list triangles;
        triangles.clear();
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, triangles.size() };

        auto const function = to_function(aPen.color(), basic_rect<float>{ aCenter - size{ aRadius, aRadius }, size{ aRadius * 2.0, aRadius * 2.0 } });

        for (auto const& v : triangles)
            vertexArrays.push_back({ v, std::holds_alternative<color>(aPen.color()) ?
                vec4f{{
                    static_variant_cast<color>(aPen.color()).red<float>(),
                    static_variant_cast<color>(aPen.color()).green<float>(),
                    static_variant_cast<color>(aPen.color()).blue<float>(),
                    static_variant_cast<color>(aPen.color()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{},
                {},
                function });

        emit_any_stipple(*this, vertexArrays);
    }

    void opengl_rendering_context::draw_arc(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.color()), iOpacity);

        auto vertices = arc_vertices(aCenter, aRadius, aStartAngle, aEndAngle, aCenter, mesh_type::Outline);

        auto lines = line_loop_to_lines(vertices, false);
        thread_local vec3_list quads;
        quads.clear();
        lines_to_quads(lines, aPen.width(), quads);
        thread_local vec3_list triangles;
        triangles.clear();
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, triangles.size() };

        auto const function = to_function(aPen.color(), basic_rect<float>{ aCenter - size{ aRadius, aRadius }, size{ aRadius * 2.0, aRadius * 2.0 } });

        for (auto const& v : triangles)
            vertexArrays.push_back({ v, std::holds_alternative<color>(aPen.color()) ?
                vec4f{{
                    static_variant_cast<color>(aPen.color()).red<float>(),
                    static_variant_cast<color>(aPen.color()).green<float>(),
                    static_variant_cast<color>(aPen.color()).blue<float>(),
                    static_variant_cast<color>(aPen.color()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{},
                {},
                function });

        emit_any_stipple(*this, vertexArrays);
    }

    void opengl_rendering_context::draw_cubic_bezier(const point& aP0, const point& aP1, const point& aP2, const point& aP3, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        rendering_engine().default_shader_program().shape_shader().set_cubic_bezier(aP0.to_vec2(), aP1.to_vec2(), aP2.to_vec2(), aP3.to_vec2(), aPen.width());

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aPen.color()), iOpacity);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 3u) };

            auto r = rect{ aP0.min(aP1.min(aP2.min(aP3))), aP0.max(aP1.max(aP2.max(aP3))) }.inflated(aPen.width());
            auto const function = to_function(aPen.color(), r);
            auto rectVertices = rect_vertices(r, mesh_type::Triangles, 0.0);
            for (auto const& v : rectVertices)
                vertexArrays.push_back({ v,
                    std::holds_alternative<color>(aPen.color()) ?
                        vec4f{{
                            static_variant_cast<const color&>(aPen.color()).red<float>(),
                            static_variant_cast<const color&>(aPen.color()).green<float>(),
                            static_variant_cast<const color&>(aPen.color()).blue<float>(),
                            static_variant_cast<const color&>(aPen.color()).alpha<float>() * static_cast<float>(iOpacity)}} :
                        vec4f{},
                        {},
                        function });
        }
    }

    void opengl_rendering_context::draw_path(const path& aPath, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.color()), iOpacity);

        auto const function = to_function(aPen.color(), aPath.bounding_rect());

        for (auto const& subPath : aPath.sub_paths())
        {
            if (subPath.size() >= 2)
            {
                GLenum mode;
                auto vertices = path_vertices(aPath, subPath, aPen.width(), mode);

                {
                    use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, mode, vertices.size() };
                    for (auto const& v : vertices)
                        vertexArrays.push_back({ v, std::holds_alternative<color>(aPen.color()) ?
                            vec4f{{
                                static_variant_cast<color>(aPen.color()).red<float>(),
                                static_variant_cast<color>(aPen.color()).green<float>(),
                                static_variant_cast<color>(aPen.color()).blue<float>(),
                                static_variant_cast<color>(aPen.color()).alpha<float>() * static_cast<float>(iOpacity)}} :
                            vec4f{},
                            {},
                            function });
                }
            }
        }
    }

    void opengl_rendering_context::draw_shape(const game::mesh& aMesh, const vec3& aPosition, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.color()), iOpacity);

        auto const& vertices = aMesh.vertices;

        auto lines = line_loop_to_lines(vertices);
        thread_local vec3_list quads;
        quads.clear();
        lines_to_quads(lines, aPen.width(), quads);
        thread_local vec3_list triangles;
        triangles.clear();
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, triangles.size() };

        auto const function = to_function(aPen.color(), bounding_rect(aMesh));

        for (auto const& v : triangles)
            vertexArrays.push_back({ v + aPosition, std::holds_alternative<color>(aPen.color()) ?
                vec4f{{
                    static_variant_cast<color>(aPen.color()).red<float>(),
                    static_variant_cast<color>(aPen.color()).green<float>(),
                    static_variant_cast<color>(aPen.color()).blue<float>(),
                    static_variant_cast<color>(aPen.color()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{},
                {},
                function });
    }

    void opengl_rendering_context::draw_entities(game::i_ecs& aEcs, int32_t aLayer, const mat44& aTransformation)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        thread_local std::vector<std::vector<mesh_drawable>> drawables;
        thread_local int32_t maxLayer = 0;
        thread_local optional_ecs_render_lock lock;

        if (drawables.size() <= aLayer)
            drawables.resize(aLayer + 1);

        if (aLayer == 0)
        {
            for (auto& d : drawables)
                d.clear();
            lock.emplace(aEcs);
            auto const& rigidBodies = aEcs.component<game::rigid_body>();
            auto const& animatedMeshFilters = aEcs.component<game::animation_filter>();
            auto const& infos = aEcs.component<game::entity_info>();
            auto const& meshRenderers = aEcs.component<game::mesh_renderer>();
            auto const& meshFilters = aEcs.component<game::mesh_filter>();
            auto const& cache = aEcs.component<game::mesh_render_cache>();
            for (auto entity : meshRenderers.entities())
            {
#ifdef NEOGFX_DEBUG
                if (infos.entity_record(entity).debug::layoutItem)
                    service<debug::logger>() << "Rendering debug::layoutItem entity..." << endl;
#endif // NEOGFX_DEBUG
                auto const& info = infos.entity_record_no_lock(entity);
                if (info.destroyed)
                    continue;
                auto const& meshRenderer = meshRenderers.entity_record_no_lock(entity);
                maxLayer = std::max(maxLayer, meshRenderer.layer);
                if (drawables.size() <= maxLayer)
                    drawables.resize(maxLayer + 1);
                auto const& meshFilter = meshFilters.has_entity_record_no_lock(entity) ?
                    meshFilters.entity_record_no_lock(entity) :
                    game::current_animation_frame(animatedMeshFilters.entity_record_no_lock(entity));
                drawables[meshRenderer.layer].emplace_back(
                    meshFilter,
                    meshRenderer,
                    optional_mat44{},
                    entity);
                if (!game::is_render_cache_clean_no_lock(cache, entity))
                {
                    auto const& rigidBodyTransformation = (rigidBodies.has_entity_record_no_lock(entity) ?
                        to_transformation_matrix(rigidBodies.entity_record_no_lock(entity)) : mat44::identity());
                    auto const& meshFilterTransformation = (meshFilter.transformation ?
                        *meshFilter.transformation : mat44::identity());
                    auto const& animationMeshFilterTransformation = (animatedMeshFilters.has_entity_record_no_lock(entity) ?
                        to_transformation_matrix(animatedMeshFilters.entity_record_no_lock(entity)) : mat44::identity());
                    auto const& transformation = rigidBodyTransformation * meshFilterTransformation * animationMeshFilterTransformation;
                    drawables[meshRenderer.layer].back().transformation = transformation;
                }
            }
        }
        if (!drawables[aLayer].empty())
            draw_meshes(lock, dynamic_cast<i_vertex_provider&>(aEcs), &*drawables[aLayer].begin(), &*drawables[aLayer].begin() + drawables[aLayer].size(), aTransformation);
        if (aLayer >= maxLayer)
        {
            maxLayer = 0;
            for (auto& d : drawables)
                d.clear();
            lock.reset();
        }
    }

    void opengl_rendering_context::fill_rect(const rect& aRect, const brush& aFill, scalar aZpos)
    {
        graphics_operation::operation op{ graphics_operation::fill_rect{ aRect, aFill, aZpos } };
        fill_rects(graphics_operation::batch{ &op, &op + 1 });
    }

    void opengl_rendering_context::fill_rects(const graphics_operation::batch& aFillRectOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };
        scoped_anti_alias saa{ *this, smoothing_mode::None };
        disable_multisample disableMultisample{ *this };

        auto& firstOp = static_variant_cast<const graphics_operation::fill_rect&>(*aFillRectOps.first);

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill), iOpacity);
        
        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 3u * (aFillRectOps.second - aFillRectOps.first))};

            for (auto op = aFillRectOps.first; op != aFillRectOps.second; ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::fill_rect&>(*op);
                auto const function = to_function(drawOp.fill, drawOp.rect);
                auto rectVertices = rect_vertices(drawOp.rect, mesh_type::Triangles, drawOp.zpos);
                for (auto const& v : rectVertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            vec4f{{
                                static_variant_cast<const color&>(drawOp.fill).red<float>(),
                                static_variant_cast<const color&>(drawOp.fill).green<float>(),
                                static_variant_cast<const color&>(drawOp.fill).blue<float>(),
                                static_variant_cast<const color&>(drawOp.fill).alpha<float>() * static_cast<float>(iOpacity)}} :
                            vec4f{},
                            {},
                            function });
            }
        }
    }

    void opengl_rendering_context::fill_rounded_rect(const rect& aRect, dimension aRadius, const brush& aFill)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        if (aRect.empty())
            return;

        if (std::holds_alternative<gradient>(aFill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill), iOpacity);

        auto vertices = rounded_rect_vertices(aRect, aRadius, mesh_type::TriangleFan);
        
        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLE_FAN, vertices.size() };

            auto const function = to_function(aFill, aRect);

            for (auto const& v : vertices)
            {
                vertexArrays.push_back({v, std::holds_alternative<color>(aFill) ?
                    vec4f{{
                        static_variant_cast<const color&>(aFill).red<float>(),
                        static_variant_cast<const color&>(aFill).green<float>(),
                        static_variant_cast<const color&>(aFill).blue<float>(),
                        static_variant_cast<const color&>(aFill).alpha<float>() * static_cast<float>(iOpacity)}} :
                    vec4f{},
                    {},
                    function});
            }
        }
    }

    void opengl_rendering_context::fill_checker_rects(const graphics_operation::batch& aFillCheckerRectOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };
        scoped_anti_alias saa{ *this, smoothing_mode::None };
        disable_multisample disableMultisample{ *this };

        // todo: add a shader-based primitive for this operation

        for (int32_t step = 0; step <= 1; ++step)
        {
            auto& firstOp = static_variant_cast<const graphics_operation::fill_checker_rect&>(*aFillCheckerRectOps.first);

            if (std::holds_alternative<gradient>(step == 0 ? firstOp.fill1 : firstOp.fill2))
                rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(step == 0 ? firstOp.fill1 : firstOp.fill2), iOpacity);

            {
                for (auto op = aFillCheckerRectOps.first; op != aFillCheckerRectOps.second; ++op)
                {
                    auto& drawOp = static_variant_cast<const graphics_operation::fill_checker_rect&>(*op);

                    auto const squareCount = static_cast<uint32_t>(
                        std::ceil(std::ceil(drawOp.rect.extents().cx / drawOp.squareSize.cx) * std::ceil(std::ceil(drawOp.rect.extents().cy / drawOp.squareSize.cy)) / 2.0));
                    use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 3u * squareCount) };

                    for (coordinate x = 0; x < drawOp.rect.cx; x += drawOp.squareSize.cx)
                    {
                        bool alt = ((static_cast<int32_t>(x / drawOp.squareSize.cx) % 2) == step);

                        for (coordinate y = 0; y < drawOp.rect.cy; y += drawOp.squareSize.cy)
                        {
                            if (alt)
                            {
                                auto const square = rect{ drawOp.rect.top_left() + point{ x, y }, drawOp.squareSize };
                                auto const& fill = (step == 0 ? drawOp.fill1 : drawOp.fill2);
                                auto const function = to_function(fill, square);
                                auto rectVertices = rect_vertices(square, mesh_type::Triangles, drawOp.zpos);
                                for (auto const& v : rectVertices)
                                    vertexArrays.push_back({ v,
                                        std::holds_alternative<color>(fill) ?
                                            vec4f{{
                                                static_variant_cast<const color&>(fill).red<float>(),
                                                static_variant_cast<const color&>(fill).green<float>(),
                                                static_variant_cast<const color&>(fill).blue<float>(),
                                                static_variant_cast<const color&>(fill).alpha<float>() * static_cast<float>(iOpacity)}} :
                                            vec4f{},
                                            {},
                                            function });
                            }
                            alt = !alt;
                        }
                    }
                }
            }
        }
    }

    void opengl_rendering_context::fill_circle(const point& aCenter, dimension aRadius, const brush& aFill)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        if (std::holds_alternative<gradient>(aFill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill), iOpacity);

        auto vertices = circle_vertices(aCenter, aRadius, 0.0, mesh_type::TriangleFan);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLE_FAN, vertices.size() };

            auto const function = to_function(aFill, rect{ aCenter - point{ aRadius, aRadius }, size{ aRadius * 2.0 } });

            for (auto const& v : vertices)
            {
                vertexArrays.push_back({v, std::holds_alternative<color>(aFill) ?
                    vec4f{{
                        static_variant_cast<const color&>(aFill).red<float>(),
                        static_variant_cast<const color&>(aFill).green<float>(),
                        static_variant_cast<const color&>(aFill).blue<float>(),
                        static_variant_cast<const color&>(aFill).alpha<float>() * static_cast<float>(iOpacity)}} :
                    vec4f{},
                    {},
                    function });
            }
        }
    }

    void opengl_rendering_context::fill_arc(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        if (std::holds_alternative<gradient>(aFill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill), iOpacity);

        auto vertices = arc_vertices(aCenter, aRadius, aStartAngle, aEndAngle, aCenter, mesh_type::TriangleFan);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLE_FAN, vertices.size() };

            auto const function = to_function(aFill, rect{ aCenter - point{ aRadius, aRadius }, size{ aRadius * 2.0 } });

            for (auto const& v : vertices)
            {
                vertexArrays.push_back({v, std::holds_alternative<color>(aFill) ?
                    vec4f{{
                        static_variant_cast<const color&>(aFill).red<float>(),
                        static_variant_cast<const color&>(aFill).green<float>(),
                        static_variant_cast<const color&>(aFill).blue<float>(),
                        static_variant_cast<const color&>(aFill).alpha<float>() * static_cast<float>(iOpacity)}} :
                    vec4f{},
                    {},
                    function });
            }
        }
    }

    void opengl_rendering_context::fill_path(const path& aPath, const brush& aFill)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        for (auto const& subPath : aPath.sub_paths())
        {
            if (subPath.size() > 2)
            {
                if (std::holds_alternative<gradient>(aFill))
                    rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill), iOpacity);

                GLenum mode;
                auto vertices = path_vertices(aPath, subPath, 0.0, mode);

                {
                    use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, mode, vertices.size() };

                    auto const function = to_function(aFill, aPath.bounding_rect());

                    for (auto const& v : vertices)
                    {
                        vertexArrays.push_back({v, std::holds_alternative<color>(aFill) ?
                            vec4f{{
                                static_variant_cast<const color&>(aFill).red<float>(),
                                static_variant_cast<const color&>(aFill).green<float>(),
                                static_variant_cast<const color&>(aFill).blue<float>(),
                                static_variant_cast<const color&>(aFill).alpha<float>() * static_cast<float>(iOpacity)}} :
                            vec4f{},
                            {},
                            function });
                    }
                }
            }
        }
    }

    void opengl_rendering_context::fill_shapes(const graphics_operation::batch& aFillShapeOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::fill_shape&>(*aFillShapeOps.first);

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill), iOpacity);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES };

            for (auto op = aFillShapeOps.first; op != aFillShapeOps.second; ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::fill_shape&>(*op);
                auto const& vertices = drawOp.mesh.vertices;
                auto const& uv = drawOp.mesh.uv;
                vec3 min, max;
                if (std::holds_alternative<gradient>(drawOp.fill))
                {
                    min = vertices[0].xyz;
                    max = min;
                    for (auto const& v : vertices)
                    {
                        min.x = std::min(min.x, v.x + firstOp.position.x);
                        max.x = std::max(max.x, v.x + firstOp.position.x);
                        min.y = std::min(min.y, v.y + firstOp.position.y);
                        max.y = std::max(max.y, v.y + firstOp.position.y);
                    }
                }
                auto const function = to_function(drawOp.fill, rect{ point{ min.x, min.y }, size{ max.x - min.x, max.y - min.y } });
                if (!vertexArrays.room_for(drawOp.mesh.faces.size() * 3u))
                    vertexArrays.draw_and_execute();
                for (auto const& f : drawOp.mesh.faces)
                {
                    for (auto vi : f)
                    {
                        auto const& v = vertices[vi];
                        vertexArrays.push_back({
                            v + drawOp.position,
                            std::holds_alternative<color>(drawOp.fill) ?
                                vec4f{{
                                    static_variant_cast<const color&>(drawOp.fill).red<float>(),
                                    static_variant_cast<const color&>(drawOp.fill).green<float>(),
                                    static_variant_cast<const color&>(drawOp.fill).blue<float>(),
                                    static_variant_cast<const color&>(drawOp.fill).alpha<float>() * static_cast<float>(iOpacity)}} :
                                vec4f{},
                            uv[vi],
                            function });
                    }
                }
            }
        }
    }

    subpixel_format opengl_rendering_context::subpixel_format() const
    {
        if (render_target().target_type() == render_target_type::Texture)
            return neogfx::subpixel_format::None;
        else if (iWidget != nullptr)
            return service<i_surface_manager>().display(iWidget->surface()).subpixel_format();
        // todo: might not be monitor 0!
        return service<i_basic_services>().display(0).subpixel_format();
    }

    void opengl_rendering_context::draw_glyphs(const graphics_operation::batch& aDrawGlyphOps)
    {
        thread_local neolib::variable_stack<std::vector<draw_glyph>> glyphCacheStack;
        neolib::variable_stack_context<std::vector<draw_glyph>> context{ glyphCacheStack };

        auto& drawGlyphCache = glyphCacheStack.current();
        drawGlyphCache.clear();

        for (auto op = aDrawGlyphOps.first; op != aDrawGlyphOps.second; ++op)
        {
            auto& drawOp = static_variant_cast<const graphics_operation::draw_glyphs&>(*op);
            vec3 pos = drawOp.point;
            for (auto g = drawOp.begin; g != drawOp.end; ++g)
            {
                auto& glyph = *g;
                drawGlyphCache.emplace_back(pos, &drawOp.glyphText.content(), &glyph, &drawOp.appearance, drawOp.showMnemonics );
                pos.x += advance(glyph).cx;
            }
        }

        if (drawGlyphCache.empty())
            return;

        auto start = drawGlyphCache.begin();
        for (auto next = std::next(start); next != drawGlyphCache.end(); ++next)
        {
            if (!graphics_operation::batchable(*start->glyphText, *next->glyphText, *start->glyph, *next->glyph))
            {
                draw_glyphs(&*start, &*next);
                start = next;
            }
        }
        if (start != drawGlyphCache.end())
            draw_glyphs(&*start, std::next(&*std::prev(drawGlyphCache.end())));
    }

    void opengl_rendering_context::draw_glyphs(const draw_glyph* aBegin, const draw_glyph* aEnd)
    {
        disable_anti_alias daa{ *this };
        neolib::scoped_flag snap{ iSnapToPixel, false };

        thread_local std::vector<game::mesh_filter> meshFilters;
        thread_local std::vector<game::mesh_renderer> meshRenderers;
        thread_local std::vector<mesh_drawable> drawables;

        auto draw = [&]()
        {
            for (std::size_t i = 0; i < meshFilters.size(); ++i)
                drawables.emplace_back(meshFilters[i], meshRenderers[i]);
            optional_ecs_render_lock ignore;
            if (!drawables.empty())
                draw_meshes(ignore, as_vertex_provider(), &*drawables.begin(), &*drawables.begin() + drawables.size(), mat44::identity());
            meshFilters.clear();
            meshRenderers.clear();
            drawables.clear();
        };

        std::size_t normalGlyphCount = 0;

        optional_rect filterRegion;

        for (int32_t pass = 1; pass <= 6; ++pass)
        {
            switch (pass)
            {
            case 1: // Paper (glyph background)
                for (auto op = aBegin; op != aEnd; ++op)
                {
                    auto& drawOp = *op;
                    auto& glyphText = *drawOp.glyphText;
                    auto& glyph = *drawOp.glyph;

                    if (!is_whitespace(glyph) && !is_emoji(glyph))
                        ++normalGlyphCount;

                    if (drawOp.appearance->effect() != std::nullopt && !drawOp.appearance->being_filtered() &&
                        (drawOp.appearance->effect()->type() == text_effect_type::Glow || drawOp.appearance->effect()->type() == text_effect_type::Shadow))
                    {
                        font const& glyphFont = glyphText.glyph_font(glyph);
                        rect const glyphRect{ point{ drawOp.point } + glyph.offset.as<scalar>(), size{ advance(glyph).cx, glyphFont.height() } };

                        if (!filterRegion)
                            filterRegion.emplace(glyphRect);
                        else
                            filterRegion->combine(glyphRect);
                    }

                    if (drawOp.appearance->paper() != std::nullopt)
                    {
                        font const& glyphFont = glyphText.glyph_font(glyph);
                        rect const glyphRect{ point{ drawOp.point } + glyph.offset.as<scalar>(), size{ advance(glyph).cx, glyphFont.height() } };

                        auto const& mesh = to_ecs_component(
                            glyphRect,
                            mesh_type::Triangles,
                            drawOp.point.z);

                        meshFilters.push_back(game::mesh_filter{ {}, mesh });
                        meshRenderers.push_back(
                            game::mesh_renderer{
                                game::material{
                                    std::holds_alternative<color>(*drawOp.appearance->paper()) ?
                                        to_ecs_component(std::get<color>(*drawOp.appearance->paper())) : std::optional<game::color>{},
                                    std::holds_alternative<gradient>(*drawOp.appearance->paper()) ?
                                        to_ecs_component(std::get<gradient>(*drawOp.appearance->paper())) : std::optional<game::gradient>{} } });
                    }
                }
                break;
            case 2: // Special effects
                if (filterRegion)
                {
                    std::optional<scoped_filter<blur_filter>> filter;
                    for (auto op = aBegin; op != aEnd; ++op)
                    {
                        auto& drawOp = *op;
                        auto& glyphText = *drawOp.glyphText;
                        auto& glyph = *drawOp.glyph;

                        if (is_whitespace(glyph))
                            continue;

                        if (drawOp.appearance->being_filtered())
                            continue;

                        bool const renderEffects = !drawOp.appearance->only_calculate_effect() && drawOp.appearance->effect();
                        if (!renderEffects)
                            continue;

                        if (is_emoji(glyph) && drawOp.appearance->effect()->ignore_emoji())
                            continue;

                        if (!filter)
                            filter.emplace(*this, blur_filter{ *filterRegion, drawOp.appearance->effect()->width() });

                        filter->front_buffer().draw_glyph(
                            drawOp.point + drawOp.appearance->effect()->offset(),
                            glyphText,
                            glyph,
                            drawOp.appearance->as_being_filtered());
                    }
                }
                break;
            case 3: // Emoji render (final pass)
                for (auto op = aBegin; op != aEnd; ++op)
                {
                    auto& drawOp = *op;
                    auto& glyphText = *drawOp.glyphText;
                    auto& glyph = *drawOp.glyph;

                    if (is_whitespace(glyph) || !is_emoji(glyph))
                        continue;

                    font const& glyphFont = glyphText.glyph_font(glyph);

                    rect const outputRect = { point{ drawOp.point } + glyph.offset.as<scalar>(), size{ advance(glyph).cx, glyphFont.height() } };

                    auto const& mesh = logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui ?
                        to_ecs_component(
                            outputRect,
                            mesh_type::Triangles,
                            drawOp.point.z) :
                        to_ecs_component(
                            game_rect{ outputRect },
                            mesh_type::Triangles,
                            drawOp.point.z);

                    auto const& emojiAtlas = rendering_engine().font_manager().emoji_atlas();
                    auto const& emojiTexture = emojiAtlas.emoji_texture(glyph.value).as_sub_texture();
                    meshFilters.push_back(game::mesh_filter{ game::shared<game::mesh>{}, mesh });
                    auto const& ink = !drawOp.appearance->effect() || !drawOp.appearance->being_filtered() ?
                        (drawOp.appearance->ignore_emoji() ? neolib::none : drawOp.appearance->ink()) : 
                        (drawOp.appearance->effect()->ignore_emoji() ? neolib::none : drawOp.appearance->effect()->color());
                    meshRenderers.push_back(game::mesh_renderer
                        {
                            game::material
                            {
                                std::holds_alternative<color>(ink) ? to_ecs_component(static_variant_cast<const color&>(ink)) : std::optional<game::color>{},
                                std::holds_alternative<gradient>(ink) ? to_ecs_component(static_variant_cast<const gradient&>(ink).with_bounding_box_if_none(outputRect)) : std::optional<game::gradient>{},
                                {}, 
                                to_ecs_component(emojiTexture),
                                !drawOp.appearance->being_filtered() ?
                                    drawOp.appearance->ignore_emoji() ? 
                                        shader_effect::None : shader_effect::Colorize : 
                                    !drawOp.appearance->effect() || drawOp.appearance->effect()->ignore_emoji() ?
                                        shader_effect::None : to_ecs_component(drawOp.appearance->effect()->type())
                            }
                        });
                }
                break;
            case 4: // Glyph render (outline pass)
            case 5: // Glyph render (final pass)
                {
                    bool updateGlyphShader = true;

                    for (auto op = aBegin; op != aEnd; ++op)
                    {
                        auto& drawOp = *op;
                        auto& glyphText = *drawOp.glyphText;
                        auto& glyph = *drawOp.glyph;

                        if (is_whitespace(glyph) || is_emoji(glyph))
                            continue;

                        auto const& glyphTexture = glyphText.glyph_texture(glyph);

                        if (updateGlyphShader)
                        {
                            updateGlyphShader = false;
                            rendering_engine().default_shader_program().glyph_shader().set_first_glyph(*this, glyphText, glyph);
                        }

                        bool const subpixelRender = subpixel(glyph) && glyphTexture.subpixel();

                        auto const& glyphFont = glyphText.glyph_font(glyph);

                        auto glyphOrigin2D = point{
                            drawOp.point.x + glyphTexture.placement().x,
                            logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGame ?
                                drawOp.point.y + (glyphTexture.placement().y + -glyphFont.descender()) :
                                drawOp.point.y + glyphFont.height() - (glyphTexture.placement().y + -glyphFont.descender()) - glyphTexture.texture().extents().cy
                        } + glyph.offset.as<scalar>();

                        vec3 const glyphOrigin{ glyphOrigin2D.x, glyphOrigin2D.y, drawOp.point.z };

                        auto const xTransformCoefficient = logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui ? -1.0 : 1.0;
                        auto const transformation = ((glyphFont.style() & font_style::EmulatedItalic) != font_style::EmulatedItalic) ?
                            optional_mat44{} :
                            mat44{ 
                                { 1.0, 0.0, 0.0, 0.0 }, 
                                { xTransformCoefficient * 0.25, 1.0, 0.0, 0.0 },
                                { 0.0, 0.0, 1.0, 0.0 }, 
                                { 0.0, 0.0, 0.0, 1.0 } };

                        if (pass == 4)
                        {
                            if (drawOp.appearance->effect() && drawOp.appearance->effect()->type() == text_effect_type::Outline)
                            {
                                auto const scanlineOffsets = static_cast<uint32_t>(drawOp.appearance->effect()->width()) * 2u + 1u;
                                auto const offsets = scanlineOffsets * scanlineOffsets;
                                point const offsetOrigin = drawOp.appearance->effect()->offset();
                                for (uint32_t offset = 0; offset < offsets; ++offset)
                                {
                                    rect const outputRect = {
                                            point{ glyphOrigin } + offsetOrigin + point{ static_cast<coordinate>(offset % scanlineOffsets), static_cast<coordinate>(offset / scanlineOffsets) },
                                            glyphTexture.texture().extents() };
                                    auto mesh = logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui ?
                                        to_ecs_component(
                                            outputRect,
                                            mesh_type::Triangles,
                                            drawOp.point.z,
                                            transformation) :
                                        to_ecs_component(
                                            game_rect{ outputRect },
                                            mesh_type::Triangles,
                                            drawOp.point.z, 
                                            transformation);
                                    meshFilters.push_back(game::mesh_filter{ {}, mesh });
                                    auto const& ink = drawOp.appearance->effect()->color();
                                    meshRenderers.push_back(
                                        game::mesh_renderer{
                                            game::material{
                                                std::holds_alternative<color>(ink) ? to_ecs_component(static_variant_cast<const color&>(ink)) : std::optional<game::color>{},
                                                std::holds_alternative<gradient>(ink) ? to_ecs_component(static_variant_cast<const gradient&>(ink).with_bounding_box_if_none(outputRect)) : std::optional<game::gradient>{},
                                                {},
                                                to_ecs_component(glyphTexture.texture()),
                                                shader_effect::Ignore
                                            },
                                            {},
                                            0,
                                            {}, subpixelRender });
                                }
                            }
                            continue;
                        }

                        rect const outputRect = { point{ glyphOrigin }, glyphTexture.texture().extents() };
                        auto mesh = logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui ?
                            to_ecs_component(
                                outputRect,
                                mesh_type::Triangles,
                                drawOp.point.z,
                                transformation) :
                            to_ecs_component(
                                game_rect{ outputRect },
                                mesh_type::Triangles,
                                drawOp.point.z,
                                transformation);
                        meshFilters.push_back(game::mesh_filter{ {}, mesh });
                        auto const& ink = !drawOp.appearance->effect() || !drawOp.appearance->being_filtered() ?
                            drawOp.appearance->ink() : drawOp.appearance->effect()->color();
                        meshRenderers.push_back(
                            game::mesh_renderer{
                                game::material{
                                    std::holds_alternative<color>(ink) ? to_ecs_component(static_variant_cast<const color&>(ink)) : std::optional<game::color>{},
                                    std::holds_alternative<gradient>(ink) ? to_ecs_component(static_variant_cast<const gradient&>(ink).with_bounding_box_if_none(outputRect)) : std::optional<game::gradient>{},
                                    {},
                                    to_ecs_component(glyphTexture.texture()),
                                    shader_effect::Ignore
                                },
                                {},
                                0,
                                {}, subpixelRender });
                    }
                }
                break;
            case 6: // adornments
                {
                    struct y_underline_metrics
                    {
                        scalar ypos;
                        scalar yUnderline;
                        scalar cyUnderline;
                    };
                    thread_local std::vector<y_underline_metrics> yUnderlineMetrics;
                    yUnderlineMetrics.clear();
                    for (int32_t adornmentPass = 1; adornmentPass <=2; ++adornmentPass)
                    {
                        auto yUnderlineMetricsIter = yUnderlineMetrics.begin();
                        for (auto op = aBegin; op != aEnd; ++op)
                        {
                            auto& drawOp = *op;
                            auto& glyphText = *drawOp.glyphText;
                            auto& glyph = *drawOp.glyph;
                            auto const& glyphFont = glyphText.glyph_font(glyph);
                            auto const& ink = !drawOp.appearance->effect() || !drawOp.appearance->being_filtered() ?
                                drawOp.appearance->ink() : drawOp.appearance->effect()->color();
                            if (underline(glyph) || (drawOp.showMnemonics && neogfx::mnemonic(glyph)))
                            {
                                if (adornmentPass == 1)
                                {
                                    auto const descender = glyphFont.descender();
                                    auto const underlinePosition = glyphFont.native_font_face().underline_position();
                                    auto const dy = descender - underlinePosition;
                                    auto const yLine = (logical_coordinates().is_gui_orientation() ? glyphFont.height() - 1 + dy : -dy) + glyph.offset.as<scalar>().y;
                                    if (yUnderlineMetrics.empty() || yUnderlineMetrics.back().ypos != drawOp.point.y)
                                        yUnderlineMetrics.emplace_back(drawOp.point.y, yLine + drawOp.point.y, glyphFont.native_font_face().underline_thickness());
                                    else
                                    {
                                        yUnderlineMetrics.back().yUnderline = std::max(yLine + drawOp.point.y, yUnderlineMetrics.back().yUnderline);
                                        yUnderlineMetrics.back().cyUnderline = std::max(glyphFont.native_font_face().underline_thickness(), yUnderlineMetrics.back().cyUnderline);
                                    }
                                }
                                else
                                {
                                    if (yUnderlineMetricsIter->ypos != drawOp.point.y)
                                        ++yUnderlineMetricsIter;
                                    draw_line(
                                        vec3{ drawOp.point.x, yUnderlineMetricsIter->yUnderline },
                                        vec3{ drawOp.point.x + (drawOp.showMnemonics && neogfx::mnemonic(glyph) ? glyphText.extents(glyph).cx : advance(glyph).cx), yUnderlineMetricsIter->yUnderline },
                                        pen{ ink, yUnderlineMetricsIter->cyUnderline });
                                }
                            }
                        }
                    }
                }
                break;
            }
            draw();
        }
    }

    void opengl_rendering_context::draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const mat44& aTransformation, const std::optional<game::filter>& aFilter)
    {
        draw_mesh(game::mesh_filter{ { &aMesh }, {}, {} }, game::mesh_renderer{ aMaterial, {}, 0, aFilter }, aTransformation);
    }
    
    void opengl_rendering_context::draw_mesh(const game::mesh_filter& aMeshFilter, const game::mesh_renderer& aMeshRenderer, const mat44& aTransformation)
    {
        mesh_drawable drawable
        {
            aMeshFilter,
            aMeshRenderer
        };
        optional_ecs_render_lock ignore;
        draw_meshes(ignore, as_vertex_provider(), &drawable, &drawable + 1, aTransformation);
    }

    void opengl_rendering_context::draw_meshes(optional_ecs_render_lock& aLock, i_vertex_provider& aVertexProvider, mesh_drawable* aFirst, mesh_drawable* aLast, const mat44& aTransformation)
    {
        auto const logicalCoordinates = logical_coordinates();

        thread_local patch_drawable patchDrawable = {};
        patchDrawable.provider = &aVertexProvider;
        patchDrawable.items.clear();

        auto cache = aVertexProvider.cacheable() ? &aVertexProvider.cache() : nullptr;

        std::size_t vertexCount = 0;
        std::size_t cachedVertexCount = 0;
        for (auto md = aFirst; md != aLast; ++md)
        {
            auto& meshDrawable = *md;
            auto& meshRenderer = *meshDrawable.renderer;
            auto& meshFilter = *meshDrawable.filter;
            bool const cached = meshDrawable.entity != null_entity &&
                game::is_render_cache_valid_no_lock(*cache, meshDrawable.entity);
            auto& mesh = (meshFilter.mesh != std::nullopt ? *meshFilter.mesh : *meshFilter.sharedMesh.ptr);
            auto const& faces = mesh.faces;
            vertexCount += faces.size() * 3;
            if (cached)
                cachedVertexCount += faces.size() * 3;
            for (auto const& meshPatch : meshRenderer.patches)
            {
                vertexCount += meshPatch.faces.size() * 3;
                if (cached)
                    cachedVertexCount += meshPatch.faces.size() * 3;
            }
        }

        auto& vertexBuffer = static_cast<opengl_vertex_buffer<>&>(service<i_rendering_engine>().vertex_buffer(aVertexProvider));
        auto& vertices = vertexBuffer.vertices();
        if (!vertices.room_for(vertexCount - cachedVertexCount))
        {
            vertexBuffer.execute();
            vertices.clear();
            for (auto md = aFirst; md != aLast; ++md)
            {
                auto& meshDrawable = *md;
                if (meshDrawable.entity != null_entity)
                    game::set_render_cache_invalid_no_lock(*cache, meshDrawable.entity);
            }
        }

        for (auto md = aFirst; md != aLast; ++md)
        {
            auto& meshDrawable = *md;
            auto& meshFilter = *meshDrawable.filter;
            auto& meshRenderer = *meshDrawable.renderer;
            thread_local game::mesh_render_cache ignore;
            ignore = {};
            auto const& meshRenderCache = (meshDrawable.entity != null_entity ? cache->entity_record_no_lock(meshDrawable.entity, true) : ignore);
            auto& mesh = (meshFilter.mesh != std::nullopt ? *meshFilter.mesh : *meshFilter.sharedMesh.ptr);
            auto const& transformation = meshDrawable.transformation && meshFilter.transformation ?
                *meshDrawable.transformation * *meshFilter.transformation :
                meshDrawable.transformation ? *meshDrawable.transformation :
                meshFilter.transformation ? *meshFilter.transformation :
                optional_mat44{};
            auto const& faces = mesh.faces;
            auto const& material = meshRenderer.material;
            vec2 textureStorageExtents;
            vec2 uvFixupCoefficient;
            vec2 uvFixupOffset;
            std::optional<neolib::cookie> textureId;
            auto add_item = [&](vec2u32& cacheIndices, auto const& mesh, auto const& material, auto const& faces)
            {
                auto const function = material.gradient != std::nullopt && material.gradient->boundingBox ?
                    vec4{
                        material.gradient->boundingBox->min.x, material.gradient->boundingBox->min.y,
                        material.gradient->boundingBox->max.x, material.gradient->boundingBox->max.y }.as<float>() :
                    meshRenderer.filter != std::nullopt && meshRenderer.filter->boundingBox ?
                        vec4{
                            meshRenderer.filter->boundingBox->min.x, meshRenderer.filter->boundingBox->min.y,
                            meshRenderer.filter->boundingBox->max.x, meshRenderer.filter->boundingBox->max.y }.as<float>() :
                        vec4f{};
                if (meshRenderCache.state != game::cache_state::Clean)
                {
                    std::optional<float> uvGui;
                    if (patch_drawable::has_texture(meshRenderer, material))
                    {
                        auto const& materialTexture = patch_drawable::texture(meshRenderer, material);
                        auto nextTextureId = materialTexture.id.cookie();
                        if (textureId == std::nullopt || *textureId != nextTextureId)
                        {
                            textureId = nextTextureId;
                            auto const& texture = *service<i_texture_manager>().find_texture(nextTextureId);
                            textureStorageExtents = texture.storage_extents().to_vec2();
                            uvFixupCoefficient = materialTexture.extents;
                            if (materialTexture.type == texture_type::Texture)
                                uvFixupOffset = vec2{ 1.0, 1.0 };
                            else if (materialTexture.subTexture == std::nullopt)
                                uvFixupOffset = texture.as_sub_texture().atlas_location().top_left().to_vec2() + vec2{ 1.0, 1.0 };
                            else
                                uvFixupOffset = materialTexture.subTexture->min + vec2{ 1.0, 1.0 };
                            if (texture.is_render_target() && texture.as_render_target().logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui)
                                uvGui = static_cast<float>(texture.extents().to_vec2().y / textureStorageExtents.y);
                        }
                    }
                    // todo: check vertex count is same as in cache
                    auto const vertexStartIndex = (meshRenderCache.state != game::cache_state::Invalid ? cacheIndices[0] : vertices.find_space_for(faces.size() * 3));
                    auto nextIndex = vertexStartIndex;
                    for (auto const& face : faces)
                    {
                        for (auto faceVertexIndex : face)
                        {
                            auto const& xyz = (transformation? *transformation * mesh.vertices[faceVertexIndex] : mesh.vertices[faceVertexIndex]);
                            auto const& rgba = (material.color != std::nullopt ? material.color->rgba.as<float>() : vec4f{ 1.0f, 1.0f, 1.0f, 1.0f });
                            auto const& uv = (patch_drawable::has_texture(meshRenderer, material) ?
                                (mesh.uv[faceVertexIndex].scale(uvFixupCoefficient) + uvFixupOffset).scale(1.0 / textureStorageExtents) : vec2{});
                            auto const& xyzw = function;
                            if (nextIndex == vertices.size())
                                vertices.emplace_back(xyz, rgba, uv, xyzw );
                            else
                                vertices[nextIndex] = { xyz, rgba, uv, xyzw };
                            if (uvGui)
                                vertices[nextIndex].st.y = *uvGui - vertices[nextIndex].st.y;
                            vertices[nextIndex].rgba[3] *= static_cast<float>(iOpacity);
                            ++nextIndex;
                        }
                    }
                    cacheIndices[0] = static_cast<uint32_t>(vertexStartIndex);
                    cacheIndices[1] = static_cast<uint32_t>(nextIndex);
                }
                patchDrawable.items.emplace_back(meshDrawable, cacheIndices[0], cacheIndices[1], material, faces);
            };
#ifdef NEOGFX_DEBUG
            if (meshDrawable.entity != game::null_entity &&
                dynamic_cast<game::i_ecs&>(aVertexProvider).component<game::entity_info>().entity_record(meshDrawable.entity).debug::layoutItem)
                service<debug::logger>() << "Adding debug::layoutItem entity drawable..." << endl;
#endif // NEOGFX_DEBUG
            if (!faces.empty())
                add_item(meshRenderCache.meshVertexArrayIndices, mesh, material, faces);
            auto const patchCount = meshRenderer.patches.size();
            meshRenderCache.patchVertexArrayIndices.resize(patchCount);
            for (std::size_t patchIndex = 0; patchIndex < patchCount; ++patchIndex)
            {
                auto& patch = meshRenderer.patches[patchIndex];
                add_item(meshRenderCache.patchVertexArrayIndices[patchIndex], mesh, patch.material, patch.faces);
            }
            meshRenderCache.state = game::cache_state::Clean;
        }

        draw_patch(patchDrawable, aTransformation);
    }

    void opengl_rendering_context::draw_patch(patch_drawable& aPatch, const mat44& aTransformation)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };
        neolib::scoped_flag snap{ iSnapToPixel, false };

        std::optional<use_vertex_arrays> vertexArrayUsage;

        auto const logicalCoordinates = logical_coordinates();

        auto& vertexBuffer = static_cast<opengl_vertex_buffer<>&>(service<i_rendering_engine>().vertex_buffer(*aPatch.provider));
        auto& vertices = vertexBuffer.vertices();

        for (auto item = aPatch.items.begin(); item != aPatch.items.end();)
        {
            std::optional<GLint> previousTexture;

            auto const& batchRenderer = *item->meshDrawable->renderer;
            auto const& batchMaterial = *item->material;

            auto calc_bounding_rect = [&vertices, &aPatch](const patch_drawable::item& aItem) -> rect
            {
                return game::bounding_rect(vertices, *aItem.faces, mat44f::identity(), aItem.vertexArrayIndexStart);
            };

            auto calc_sampling = [&aPatch, &calc_bounding_rect](const patch_drawable::item& aItem) -> texture_sampling
            {
                if (!aItem.has_texture())
                    return texture_sampling::Normal;
                auto const& texture = *service<i_texture_manager>().find_texture(aItem.texture().id.cookie());
                auto sampling = (aItem.texture().sampling != std::nullopt ? *aItem.texture().sampling : texture.sampling());
                if (sampling == texture_sampling::Scaled)
                {
                    auto const extents = size_u32{ texture.extents() };
                    auto const outputRect = calc_bounding_rect(aItem);
                    if (extents / 2u * 2u == extents && (outputRect.cx > extents.cx || outputRect.cy > extents.cy))
                        sampling = texture_sampling::Nearest;
                    else
                        sampling = texture_sampling::Normal;
                }
                return sampling;
            };

            std::size_t faceCount = item->faces->size();
            auto sampling = calc_sampling(*item);
            auto next = std::next(item);

            while (next != aPatch.items.end() &&
                std::prev(next)->vertexArrayIndexEnd == next->vertexArrayIndexStart &&
                game::batchable(*item->material, *next->material) && 
                sampling == calc_sampling(*next))
            {   
                faceCount += next->faces->size();
                ++next;
            }

            if (item->material->gradient)
                rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, *item->material->gradient, iOpacity);
            else if (iGradient)
                rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, *iGradient, iOpacity);
            else
                rendering_engine().default_shader_program().gradient_shader().clear_gradient();

            if (item->meshDrawable->renderer->filter)
            {
                auto const& filter = *item->meshDrawable->renderer->filter;
                rendering_engine().default_shader_program().filter_shader().set_filter(filter.type, filter.arg1, filter.arg2, filter.arg3, filter.arg4);
            }
            else
                rendering_engine().default_shader_program().filter_shader().clear_filter();

            if (item->has_texture())
            {
                auto const& texture = *service<i_texture_manager>().find_texture(item->texture().id.cookie());

                glCheck(glActiveTexture(sampling != texture_sampling::Multisample ? GL_TEXTURE1 : GL_TEXTURE2));

                previousTexture.emplace(0);
                glCheck(glGetIntegerv(sampling != texture_sampling::Multisample ? GL_TEXTURE_BINDING_2D : GL_TEXTURE_BINDING_2D_MULTISAMPLE, &*previousTexture));
                glCheck(glBindTexture(sampling != texture_sampling::Multisample ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(texture.native_handle())));
                if (sampling != texture_sampling::Multisample)
                {
                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampling != texture_sampling::Nearest && sampling != texture_sampling::Data ? 
                        GL_LINEAR : 
                        GL_NEAREST));
                    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampling == texture_sampling::NormalMipmap ? 
                        GL_LINEAR_MIPMAP_LINEAR : 
                        sampling != texture_sampling::Nearest && sampling != texture_sampling::Data ? 
                            GL_LINEAR : 
                            GL_NEAREST));
                }

                rendering_engine().default_shader_program().texture_shader().set_texture(texture);
                rendering_engine().default_shader_program().texture_shader().set_effect(batchMaterial.shaderEffect != std::nullopt ?
                    *batchMaterial.shaderEffect : shader_effect::None);
                if (texture.sampling() == texture_sampling::Multisample && render_target().target_texture().sampling() == texture_sampling::Multisample)
                    enable_sample_shading(1.0);

                if (vertexArrayUsage == std::nullopt || !vertexArrayUsage->with_textures())
                    vertexArrayUsage.emplace(*aPatch.provider, *this, GL_TRIANGLES, aTransformation, with_textures, 0, batchRenderer.barrier);

#ifdef NEOGFX_DEBUG
                if (item->meshDrawable->entity != game::null_entity &&
                    dynamic_cast<game::i_ecs&>(*aPatch.provider).component<game::entity_info>().entity_record(item->meshDrawable->entity).debug::layoutItem)
                    service<debug::logger>() << "Drawing debug::layoutItem entity (texture)..." << endl;

#endif // NEOGFX_DEBUG
                vertexArrayUsage->draw(item->vertexArrayIndexStart, faceCount * 3);
            }
            else
            {
                rendering_engine().default_shader_program().texture_shader().clear_texture();

                if (vertexArrayUsage == std::nullopt || vertexArrayUsage->with_textures())
                    vertexArrayUsage.emplace(*aPatch.provider, *this, GL_TRIANGLES, aTransformation, 0, batchRenderer.barrier);

#ifdef NEOGFX_DEBUG
                if (item->meshDrawable->entity != game::null_entity &&
                    dynamic_cast<game::i_ecs&>(*aPatch.provider).component<game::entity_info>().entity_record(item->meshDrawable->entity).debug::layoutItem)
                    service<debug::logger>() << "Drawing debug::layoutItem entity (non-texture)..." << endl;

#endif // NEOGFX_DEBUG
                vertexArrayUsage->draw(item->vertexArrayIndexStart, faceCount * 3);
            }

            item = next;

            if (previousTexture != std::nullopt)
                glCheck(glBindTexture(sampling != texture_sampling::Multisample ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(*previousTexture)));

            disable_sample_shading();
        }
    }

    void opengl_rendering_context::draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect, const optional_color& aColor, shader_effect aShaderEffect)
    {
        auto mesh = to_ecs_component(aRect);
        for (auto& uv : mesh.uv)
            uv = (aTextureRect.top_left() / aTexture.extents()).to_vec2() + uv.scale((aTextureRect.extents() / aTexture.extents()).to_vec2());
        draw_mesh(
            mesh,
            game::material
            {
                aColor != std::nullopt ? game::color{ *aColor } : std::optional<game::color>{},
                {},
                {},
                to_ecs_component(aTexture),
                aShaderEffect
            },
            mat44::identity());
    }
}