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

#include <neolib/core/thread_local.hpp>
#include <neolib/app/i_power.hpp>

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gfx/text/glyph_text.hpp>
#include <neogfx/gfx/text/i_emoji_atlas.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/text/i_glyph.hpp>
#include <neogfx/gfx/shapes.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/game/rectangle.hpp>
#include <neogfx/game/text_mesh.hpp>
#include <neogfx/game/ecs_helpers.hpp>
#include <neogfx/hid/i_native_surface.hpp>
#include "../i_native_texture.hpp"
#include "../../text/native/i_native_font_face.hpp"
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

        inline game::vertices line_loop_to_lines(const game::vertices& aLineLoop, bool aClosed = true)
        {
            game::vertices result;
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

        inline game::vertices line_strip_to_lines(const game::vertices& aLineStrip, bool aClosed = true)
        {
            return line_loop_to_lines(aLineStrip, false);
        }

        inline quadf line_to_quad(const vec3f& aStart, const vec3f& aEnd, float aLineWidth)
        {
            auto const vecLine = aEnd - aStart;
            auto const length = vecLine.magnitude();
            auto const halfWidth = aLineWidth / 2.0f;
            auto const v1 = vec3f{ -halfWidth, -halfWidth, 0.0f };
            auto const v2 = vec3f{ -halfWidth, halfWidth, 0.0f };
            auto const v3 = vec3f{ length + halfWidth, halfWidth, 0.0f };
            auto const v4 = vec3f{ length + halfWidth, -halfWidth, 0.0f };
            auto const r = rotation_matrix(vec3f{ 1.0f, 0.0f, 0.0f }, vecLine);
            return quadf{ aStart + r * v1, aStart + r * v2, aStart + r * v3, aStart + r * v4 };
        }

        template <typename VerticesIn, typename VerticesOut>
        inline void lines_to_quads(const VerticesIn& aLines, float aLineWidth, VerticesOut& aQuads)
        {
            for (auto v = aLines.begin(); v != aLines.end(); v += 2)
            {
                quadf const q = line_to_quad(v[0], v[1], aLineWidth);
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

        inline game::vertices path_vertices(const path& aPath, const path::sub_path_type& aSubPath, float aLineWidth, GLenum& aMode)
        {
            aMode = path_shape_to_gl_mode(aPath.shape());
            thread_local neogfx::game::vertices vertices;
            aPath.to_vertices(aSubPath, vertices);
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
                lines_to_quads(neogfx::game::vertices{ std::move(vertices) }, aLineWidth, vertices);
            }
            if (aMode == GL_QUADS)
            {
                aMode = GL_TRIANGLES;
                quads_to_triangles(neogfx::game::vertices{ std::move(vertices) }, vertices);
            }
            return vertices;
        }

        bool emit_any_stipple(i_rendering_context& aContext, use_vertex_arrays& aInstance, bool aLoop = false)
        {
            // assumes vertices are quads (as two triangles) created with quads_to_triangles above.
            auto& stippleShader = aContext.rendering_engine().default_shader_program().stipple_shader();
            if (stippleShader.stipple_active())
            {
                auto& shapeShader = aContext.rendering_engine().default_shader_program().shape_shader();
                if (!shapeShader.shape_active())
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
                else
                {
                    auto start = midpoint(aInstance.begin()->xyz, std::next(aInstance.begin())->xyz);
                    stippleShader.start(aContext, start);
                    aInstance.draw();
                }
                return true;
            }
            return false;
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
        iSnapToPixelUsesOffset{ true },
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
        iSnapToPixelUsesOffset{ true },
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
        iSnapToPixelUsesOffset{ true },
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
        return iOffset.value_or(vec2{}) + (snap_to_pixel() && iSnapToPixelUsesOffset ? 0.5 : 0.0);
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
        aShader.set_gradient(*this, *iGradient);
    }

    bool opengl_rendering_context::snap_to_pixel() const
    {
        return iSnapToPixel;
    }

    void opengl_rendering_context::set_snap_to_pixel(bool aSnapToPixel)
    {
        iSnapToPixel = aSnapToPixel;
    }

    graphics_operation::queue& opengl_rendering_context::queue() const
    {
        return static_cast<graphics_operation::queue&>(iTarget.graphics_operation_queue());
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
            switch (opBatch.cbegin()->index())
            {
            case graphics_operation::operation_type::SetLogicalCoordinateSystem:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    set_logical_coordinate_system(static_variant_cast<const graphics_operation::set_logical_coordinate_system&>(*op).system);
                break;
            case graphics_operation::operation_type::SetLogicalCoordinates:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    set_logical_coordinates(static_variant_cast<const graphics_operation::set_logical_coordinates&>(*op).coordinates);
                break;
            case graphics_operation::operation_type::SetOrigin:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    set_origin(static_variant_cast<const graphics_operation::set_origin&>(*op).origin);
                break;
            case graphics_operation::operation_type::SetViewport:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    auto const& setViewport = static_variant_cast<const graphics_operation::set_viewport&>(*op);
                    if (setViewport.viewport)
                        render_target().set_viewport(setViewport.viewport.value().as<std::int32_t>());
                    else
                        render_target().set_viewport(rect{ render_target().target_origin(), render_target().extents() }.as<std::int32_t>());
                }
                break;
            case graphics_operation::operation_type::ScissorOn:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    scissor_on(static_variant_cast<const graphics_operation::scissor_on&>(*op).rect);
                break;
            case graphics_operation::operation_type::ScissorOff:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
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
                set_opacity(static_variant_cast<const graphics_operation::set_opacity&>(*(std::prev(opBatch.cend()))).opacity);
                break;
            case graphics_operation::operation_type::SetBlendingMode:
                set_blending_mode(static_variant_cast<const graphics_operation::set_blending_mode&>(*(std::prev(opBatch.cend()))).blendingMode);
                break;
            case graphics_operation::operation_type::SetSmoothingMode:
                set_smoothing_mode(static_variant_cast<const graphics_operation::set_smoothing_mode&>(*(std::prev(opBatch.cend()))).smoothingMode);
                break;
            case graphics_operation::operation_type::PushLogicalOperation:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    push_logical_operation(static_variant_cast<const graphics_operation::push_logical_operation&>(*op).logicalOperation);
                break;
            case graphics_operation::operation_type::PopLogicalOperation:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    (void)op;
                    pop_logical_operation();
                }
                break;
            case graphics_operation::operation_type::LineStippleOn:
                {
                    auto const& lso = static_variant_cast<const graphics_operation::line_stipple_on&>(*(std::prev(opBatch.cend())));
                    line_stipple_on(lso.stipple);
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
                clear(static_variant_cast<const graphics_operation::clear&>(*(std::prev(opBatch.cend()))).color);
                break;
            case graphics_operation::operation_type::ClearDepthBuffer:
                clear_depth_buffer();
                break;
            case graphics_operation::operation_type::ClearStencilBuffer:
                clear_stencil_buffer();
                break;
            case graphics_operation::operation_type::SetGradient:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    set_gradient(static_variant_cast<const graphics_operation::set_gradient&>(*op).gradient);
                break;
            case graphics_operation::operation_type::ClearGradient:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    clear_gradient();
                break;
            case graphics_operation::operation_type::SetPixel:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    set_pixel(static_variant_cast<const graphics_operation::set_pixel&>(*op).point, static_variant_cast<const graphics_operation::set_pixel&>(*op).color);
                break;
            case graphics_operation::operation_type::DrawPixel:
                draw_pixels(opBatch);
                break;
            case graphics_operation::operation_type::DrawLine:
                draw_lines(opBatch);
                break;
            case graphics_operation::operation_type::DrawTriangle:
                draw_triangles(opBatch);
                break;
            case graphics_operation::operation_type::DrawRect:
                draw_rects(opBatch);
                break;
            case graphics_operation::operation_type::DrawRoundedRect:
                draw_rounded_rects(opBatch);
                break;
            case graphics_operation::operation_type::DrawEllipseRect:
                draw_ellipse_rects(opBatch);
                break;
            case graphics_operation::operation_type::DrawCheckerboard:
                draw_checkerboards(opBatch);
                break;
            case graphics_operation::operation_type::DrawCircle:
                draw_circles(opBatch);
                break;
            case graphics_operation::operation_type::DrawEllipse:
                draw_ellipses(opBatch);
                break;
            case graphics_operation::operation_type::DrawPie:
                draw_pies(opBatch);
                break;
            case graphics_operation::operation_type::DrawArc:
                draw_arcs(opBatch);
                break;
            case graphics_operation::operation_type::DrawCubicBezier:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_cubic_bezier&>(*op);
                    draw_cubic_bezier(args.p0, args.p1, args.p2, args.p3, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawPath:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_path&>(*op);
                    draw_path(args.path, args.shape, args.boundingRect, args.pen, args.fill);
                }
                break;
            case graphics_operation::operation_type::DrawShape:
                draw_shapes(opBatch);
                break;
            case graphics_operation::operation_type::DrawEntities:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    auto const& args = static_variant_cast<const graphics_operation::draw_entities&>(*op);
                    draw_entities(args.ecs, args.layer, args.transformation);
                }
                break;
            case graphics_operation::operation_type::DrawGlyph:
                draw_glyphs(opBatch);
                break;
            case graphics_operation::operation_type::DrawMesh:
                // todo: use draw_meshes
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
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
                glCheck(glBlendEquation(GL_FUNC_ADD));
                glCheck(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
                break;
            case neogfx::blending_mode::Default:
                glCheck(glEnable(GL_BLEND));
                glCheck(glBlendEquation(GL_FUNC_ADD));
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

    bool opengl_rendering_context::logical_operation_active() const
    {
        return !iLogicalOperationStack.empty() && iLogicalOperationStack.back() != logical_operation::None;
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
        auto const currentBlendingMode = iBlendingMode;

        if (!logical_operation_active())
        {
            iBlendingMode = std::nullopt;
            if (currentBlendingMode != std::nullopt)
                set_blending_mode(currentBlendingMode.value());
        }
        else
        {
            switch (iLogicalOperationStack.back())
            {
            case logical_operation::Xor:
                glCheck(glEnable(GL_BLEND));
                glCheck(glBlendEquation(GL_FUNC_ADD));
                glCheck(glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR));
                break;
            default:
                iBlendingMode = std::nullopt;
                if (currentBlendingMode != std::nullopt)
                    set_blending_mode(currentBlendingMode.value());
                break;
            }
        }    
    }

    void opengl_rendering_context::line_stipple_on(stipple const& aStipple)
    {
        rendering_engine().default_shader_program().stipple_shader().set_stipple(aStipple);
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
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };
        scoped_anti_alias saa{ *this, smoothing_mode::None };
        disable_multisample disableMultisample{ *this };

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 3u * (aDrawPixelOps.cend() - aDrawPixelOps.cbegin()))};

            for (auto op = aDrawPixelOps.cbegin(); op != aDrawPixelOps.cend(); ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_pixel&>(*op);
                auto rectVertices = rect_vertices<vec3f>(rect{ drawOp.point, size{1.0, 1.0} }, mesh_type::Triangles);
                for (auto const& v : rectVertices)
                    vertexArrays.push_back({ v,
                            vec4f{{
                                drawOp.color.red<float>(),
                                drawOp.color.green<float>(),
                                drawOp.color.blue<float>(),
                                drawOp.color.alpha<float>() }} });
            }
        }
    }

    void opengl_rendering_context::draw_line(const point& aFrom, const point& aTo, const pen& aPen)
    {
        graphics_operation::operation op{ graphics_operation::draw_line{ aFrom, aTo, aPen } };
        draw_lines(graphics_operation::batch{ &op, &op + 1 });
    }

    void opengl_rendering_context::draw_lines(const graphics_operation::batch& aDrawLineOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Line);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 2u * 3u * (aDrawLineOps.cend() - aDrawLineOps.cbegin())) };

            for (auto op = aDrawLineOps.cbegin(); op != aDrawLineOps.cend(); ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_line&>(*op);
                auto boundingRect = rect{ drawOp.from.min(drawOp.to), drawOp.from.max(drawOp.to) }.inflated(drawOp.pen.width());
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ drawOp.from.x, drawOp.from.y, drawOp.to.x, drawOp.to.y }.as<float>(),
                        vec4{},
                        vec4{ 
                            0.0,
                            !logical_operation_active() && !snap_to_pixel() ?
                                drawOp.pen.anti_aliased() ?
                                    0.5 : 0.0 :
                                0.0,
                            0.0, 
                            drawOp.pen.width() }.as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_triangles(const graphics_operation::batch& aDrawTriangleOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_triangle&>(*aDrawTriangleOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Triangle);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 2u * 3u * (aDrawTriangleOps.cend() - aDrawTriangleOps.cbegin()))};

            for (auto op = aDrawTriangleOps.cbegin(); op != aDrawTriangleOps.cend(); ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_triangle&>(*op);
                auto boundingRect = rect{ drawOp.p0.min(drawOp.p1.min(drawOp.p2)), drawOp.p0.max(drawOp.p1.max(drawOp.p2)) }.inflated(drawOp.pen.width());
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ drawOp.p0.x, drawOp.p0.y, drawOp.p1.x, drawOp.p1.y }.as<float>(),
                        vec4{ drawOp.p2.x, drawOp.p2.y }.as<float>(),
                        vec4{
                            drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                            !logical_operation_active() ?
                                drawOp.pen.anti_aliased() ? 
                                    0.5 : 0.0 :
                                0.0,
                            0.0,
                            drawOp.pen.width() }.as<float>(),
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).as<float>() });
            }
        }
    }

    namespace
    {
    }

    void opengl_rendering_context::draw_rects(const graphics_operation::batch& aDrawRectOps)
    {
        std::optional<use_shader_program> usp;
        std::optional<neolib::scoped_flag> snap;

        {
            std::optional<use_vertex_arrays> maybeVertexArrays;

            for (auto op = aDrawRectOps.cbegin(); op != aDrawRectOps.cend(); ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_rect&>(*op);

                if (drawOp.rect.top_left().z == 0.0 && drawOp.rect.bottom_right().z == 0.0 &&
                    ((std::holds_alternative<color>(drawOp.fill) && 
                        static_variant_cast<color>(drawOp.fill).alpha() == 0xFF) ||
                        std::holds_alternative<std::monostate>(drawOp.fill)))
                {
                    bool optimise = false;
                    if (!logical_operation_active())
                    {
                        auto const penWidth = drawOp.pen.width();
                        if (penWidth == 0.0)
                            optimise = true;
                        else if (scissor_rect() != std::nullopt)
                        {
                            auto const& tl = scissor_rect().value().top_left() - drawOp.rect.top_left();
                            auto const& br = drawOp.rect.bottom_right() - scissor_rect().value().bottom_right();
                            if (tl.x > penWidth && tl.y > penWidth && br.x > penWidth && br.y > penWidth)
                                optimise = true;
                        }
                    }
                    if (optimise)
                    {
                        if (std::holds_alternative<color>(drawOp.fill))
                        {
                            scissor_on(drawOp.rect);
                            clear(static_variant_cast<color>(drawOp.fill));
                            scissor_off();
                        }
                        continue;
                    }
                }

                if (usp == std::nullopt)
                {
                    usp.emplace(*this, rendering_engine().default_shader_program(), iOpacity);
                    snap.emplace(iSnapToPixelUsesOffset, false);

                    auto& firstOp = static_variant_cast<const graphics_operation::draw_rect&>(*aDrawRectOps.cbegin());

                    if (std::holds_alternative<gradient>(firstOp.fill))
                        rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
                    else if (std::holds_alternative<gradient>(firstOp.pen.color()))
                        rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

                    rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Rect);

                    maybeVertexArrays.emplace(as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 2u * 3u * (aDrawRectOps.cend() - aDrawRectOps.cbegin())));
                }

                auto& vertexArrays = maybeVertexArrays.value();

                auto const sdfRect = snap_to_pixel() ? drawOp.rect.deflated(drawOp.pen.width() / 2.0) : drawOp.rect;
                auto const boundingRect = drawOp.rect.inflated(drawOp.pen.width() / 2.0);
                auto const vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(drawOp.fill, boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ sdfRect.center().x, sdfRect.center().y, sdfRect.width(), sdfRect.height() }.as<float>(),
                        vec4{},
                        vec4{
                            drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                            !logical_operation_active() && !snap_to_pixel() ?
                                drawOp.pen.anti_aliased() ? 
                                    0.5 : 0.0 :
                                0.0,
                            0.0,
                            drawOp.pen.width() }.as<float>(),
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_rounded_rects(const graphics_operation::batch& aDrawRoundedRectOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixelUsesOffset, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_rounded_rect&>(*aDrawRoundedRectOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::RoundedRect);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 2u * 3u * (aDrawRoundedRectOps.cend() - aDrawRoundedRectOps.cbegin()))};

            for (auto op = aDrawRoundedRectOps.cbegin(); op != aDrawRoundedRectOps.cend(); ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_rounded_rect&>(*op);
                auto const sdfRect = snap_to_pixel() ? drawOp.rect.deflated(drawOp.pen.width() / 2.0) : drawOp.rect;
                auto const boundingRect = drawOp.rect.inflated(drawOp.pen.width() / 2.0);
                auto const vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(drawOp.fill, boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ sdfRect.center().x, sdfRect.center().y, sdfRect.width(), sdfRect.height() }.as<float>(),
                        drawOp.radius.as<float>(),
                        vec4{
                            drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                            !logical_operation_active() && !snap_to_pixel() ?
                                drawOp.pen.anti_aliased() ? 
                                    0.5 : 0.0 :
                                0.0,
                            0.0,
                            drawOp.pen.width() }.as<float>(),
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_ellipse_rects(const graphics_operation::batch& aDrawEllpseRectOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixelUsesOffset, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_ellipse_rect&>(*aDrawEllpseRectOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::EllipseRect);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 2u * 3u * (aDrawEllpseRectOps.cend() - aDrawEllpseRectOps.cbegin())) };

            for (auto op = aDrawEllpseRectOps.cbegin(); op != aDrawEllpseRectOps.cend(); ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_ellipse_rect&>(*op);
                auto const sdfRect = snap_to_pixel() ? drawOp.rect.deflated(drawOp.pen.width() / 2.0) : drawOp.rect;
                auto const boundingRect = drawOp.rect.inflated(drawOp.pen.width() / 2.0);
                auto const vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(drawOp.fill, boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ sdfRect.center().x, sdfRect.center().y, sdfRect.width(), sdfRect.height() }.as<float>(),
                        drawOp.radiusX.as<float>(),
                        vec4{
                            drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                            !logical_operation_active() ?
                                drawOp.pen.anti_aliased() ? 
                                    0.5 : 0.0 :
                                0.0,
                            0.0,
                            drawOp.pen.width() }.as<float>(),
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).as<float>(),
                        drawOp.radiusY.as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_checkerboards(const graphics_operation::batch& aDrawCheckerboardOps)
    {
        std::optional<use_shader_program> usp;
        std::optional<neolib::scoped_flag> snap;

        {
            std::optional<use_vertex_arrays> maybeVertexArrays;

            for (auto op = aDrawCheckerboardOps.cbegin(); op != aDrawCheckerboardOps.cend(); ++op)
            {
                for (std::uint32_t pass = 0u; pass <= 1u; ++pass)
                {
                    usp.emplace(*this, rendering_engine().default_shader_program(), iOpacity);
                    snap.emplace(iSnapToPixelUsesOffset, false);

                    auto& drawOp = static_variant_cast<const graphics_operation::draw_checkerboard&>(*op);
                    auto& fill = (pass == 0u ? drawOp.fill1 : drawOp.fill2);

                    if (std::holds_alternative<gradient>(fill))
                        rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(fill));
                    else if (std::holds_alternative<gradient>(drawOp.pen.color()))
                        rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(drawOp.pen.color()));

                    rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Checkerboard);

                    maybeVertexArrays.emplace(as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 2u * 3u));

                    auto& vertexArrays = maybeVertexArrays.value();

                    auto const sdfRect = snap_to_pixel() ? drawOp.rect.deflated(drawOp.pen.width() / 2.0) : drawOp.rect;
                    auto const boundingRect = drawOp.rect.inflated(drawOp.pen.width() / 2.0);
                    auto const vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                    auto const function = to_function(fill, boundingRect);

                    for (auto const& v : vertices)
                        vertexArrays.push_back({ v,
                            std::holds_alternative<color>(fill) ?
                                static_variant_cast<color>(fill).as<float>() :
                                vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(fill) ? 0.0f : 1.0f },
                            {},
                            function,
                            vec4{ sdfRect.center().x, sdfRect.center().y, sdfRect.width(), sdfRect.height() }.as<float>(),
                            vec4{ drawOp.squareSize.cx, drawOp.squareSize.cy, static_cast<double>(pass) }.as<float>(),
                            vec4{
                                drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                                !logical_operation_active() && !snap_to_pixel() ?
                                    drawOp.pen.anti_aliased() ?
                                        0.5 : 0.0 :
                                    0.0,
                                0.0,
                                drawOp.pen.width() }.as<float>(),
                            std::holds_alternative<color>(drawOp.pen.color()) ?
                                static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                                vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                            drawOp.pen.secondary_color().value_or(vec4{}).as<float>() });

                    maybeVertexArrays = std::nullopt;
                    usp = std::nullopt;
                }
            }
        }
    }

    void opengl_rendering_context::draw_ellipses(const graphics_operation::batch& aDrawEllipseOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_ellipse&>(*aDrawEllipseOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Ellipse);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 2u * 3u * (aDrawEllipseOps.cend() - aDrawEllipseOps.cbegin()))};

            for (auto op = aDrawEllipseOps.cbegin(); op != aDrawEllipseOps.cend(); ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_ellipse&>(*op);
                auto boundingRect = rect{ drawOp.center - point{ std::max(drawOp.radiusA, drawOp.radiusB), std::max(drawOp.radiusA, drawOp.radiusB) }, size{ std::max(drawOp.radiusA, drawOp.radiusB) * 2.0 } }.inflated(drawOp.pen.width() / 2.0);
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ drawOp.center.x, drawOp.center.y, drawOp.radiusA, drawOp.radiusB }.as<float>(),
                        {},
                        vec4{
                            drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                            !logical_operation_active() ?
                                drawOp.pen.anti_aliased() ?
                                    0.5 : 0.0 :
                                0.0,
                            0.0,
                            drawOp.pen.width() }.as<float>(),
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_circles(const graphics_operation::batch& aDrawCircleOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_circle&>(*aDrawCircleOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Circle);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 2u * 3u * (aDrawCircleOps.cend() - aDrawCircleOps.cbegin()))};

            for (auto op = aDrawCircleOps.cbegin(); op != aDrawCircleOps.cend(); ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_circle&>(*op);
                auto boundingRect = rect{ drawOp.center - point{ drawOp.radius, drawOp.radius }, size{ drawOp.radius * 2.0 } }.inflated(drawOp.pen.width() / 2.0);
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ drawOp.center.x, drawOp.center.y, drawOp.radius }.as<float>(),
                        {},
                        vec4{
                            drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                            !logical_operation_active() ?
                                drawOp.pen.anti_aliased() ? 
                                    0.5 : 0.0 :
                                0.0,
                            0.0,
                            drawOp.pen.width() }.as<float>(),
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_pies(const graphics_operation::batch& aDrawPieOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_pie&>(*aDrawPieOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Pie);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 2u * 3u * (aDrawPieOps.cend() - aDrawPieOps.cbegin()))};

            for (auto op = aDrawPieOps.cbegin(); op != aDrawPieOps.cend(); ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_pie&>(*op);
                auto boundingRect = rect{ drawOp.center - point{ drawOp.radius, drawOp.radius }, size{ drawOp.radius * 2.0 } }.inflated(drawOp.pen.width() / 2.0);
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ drawOp.center.x, drawOp.center.y, drawOp.radius, drawOp.startAngle }.as<float>(),
                        vec4{ drawOp.endAngle }.as<float>(),
                        vec4{
                            drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                            !logical_operation_active() ?
                                drawOp.pen.anti_aliased() ? 
                                    0.5 : 0.0 :
                                0.0,
                            0.0,
                            drawOp.pen.width() }.as<float>(),
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_arcs(const graphics_operation::batch& aDrawArcOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_arc&>(*aDrawArcOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Arc);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 2u * 3u * (aDrawArcOps.cend() - aDrawArcOps.cbegin()))};

            for (auto op = aDrawArcOps.cbegin(); op != aDrawArcOps.cend(); ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::draw_arc&>(*op);
                auto boundingRect = rect{ drawOp.center - point{ drawOp.radius, drawOp.radius }, size{ drawOp.radius * 2.0 } }.inflated(drawOp.pen.width() / 2.0);
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ drawOp.center.x, drawOp.center.y, drawOp.radius, drawOp.startAngle }.as<float>(),
                        vec4{ drawOp.endAngle }.as<float>(),
                        vec4{
                            drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                            !logical_operation_active() ?
                                drawOp.pen.anti_aliased() ? 
                                    0.5 : 0.0 :
                                0.0,
                            0.0,
                            drawOp.pen.width() }.as<float>(),
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_cubic_bezier(const point& aP0, const point& aP1, const point& aP2, const point& aP3, const pen& aPen)
    {
        if (!aPen.width())
            return;

        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::CubicBezier);

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aPen.color()));

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 3u) };

            auto r = rect{ aP0.min(aP1.min(aP2.min(aP3))), aP0.max(aP1.max(aP2.max(aP3))) }.inflated(aPen.width());
            auto const function = to_function(aPen.color(), r);
            auto rectVertices = rect_vertices<vec3f>(r, mesh_type::Triangles);
            if (aPen.width())
                for (auto const& v : rectVertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(aPen.color()) ?
                            static_variant_cast<color>(aPen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(aPen.color()) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ aP0.x, aP0.y, aP1.x, aP1.y }.as<float>(),
                        vec4{ aP2.x, aP2.y, aP3.x, aP3.y }.as<float>(),
                        vec4{ 0.0, 0.0, 0.0, aPen.width() }.as<float>() });
        }
    }

    void opengl_rendering_context::draw_path(const ssbo_range& aPath, path_shape aPathShape, const rect aBoundingRect, const pen& aPen, const brush& aFill)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixelUsesOffset, false };

        switch (aPathShape)
        {
        case path_shape::ConvexPolygon:
            {
                std::optional<point> previousPoint;

                if (std::holds_alternative<gradient>(aFill))
                    rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill));
                else if (std::holds_alternative<gradient>(aPen.color()))
                    rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aPen.color()));

                rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Polygon);

                {
                    use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 3u) };

                    auto boundingRect = aBoundingRect.inflated(aPen.width() * 2.0);
                    auto boundingRectVertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                    auto const function = to_function(aPen.color(), boundingRect);

                    for (auto const& v : boundingRectVertices)
                        vertexArrays.push_back({ v,
                            std::holds_alternative<color>(aFill) ?
                                static_variant_cast<color>(aFill).as<float>() :
                                vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(aFill) ? 0.0f : 1.0f },
                            {},
                            function,
                            vec4u32{ aPath.first, aPath.last }.as<float>(),
                            vec4f{},
                            vec4{
                                aPen.width() ? aPen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                                !logical_operation_active() ?
                                    aPen.anti_aliased() ?
                                        0.5 : 0.0 :
                                    0.0,
                                0.0,
                                aPen.width() }.as<float>(),
                            std::holds_alternative<color>(aPen.color()) ?
                                static_variant_cast<color>(aPen.color()).as<float>() :
                                vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(aPen.color()) ? 0.0f : 1.0f },
                            aPen.secondary_color().value_or(vec4{}).as<float>() });
                }
            }
            break;
        default:
            throw std::logic_error("opengl_rendering_context::draw_path: path shape not yet implemented");
        }
    }

    void opengl_rendering_context::draw_shapes(const graphics_operation::batch& aDrawShapeOps)
    {
        for (auto const& op : aDrawShapeOps)
        {
            auto const& shapeOp = static_variant_cast<const graphics_operation::draw_shape&>(op);
            fill_shape(shapeOp.mesh, shapeOp.position, shapeOp.fill);
            draw_shape(shapeOp.mesh, shapeOp.position, shapeOp.pen);
        }
    }
        
    void opengl_rendering_context::draw_shape(const game::mesh& aMesh, const vec3& aPosition, const pen& aPen)
    {
        if (!aPen.width())
            return;

        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.color()));

        auto const& vertices = aMesh.vertices;

        auto lines = line_loop_to_lines(vertices);
        thread_local vec3f_list quads;
        quads.clear();
        lines_to_quads(lines, static_cast<float>(aPen.width()), quads);
        thread_local vec3f_list triangles;
        triangles.clear();
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES, triangles.size() };

        auto const function = to_function(aPen.color(), bounding_rect(aMesh));

        for (auto const& v : triangles)
            vertexArrays.push_back({ v + aPosition.as<float>(),
                std::holds_alternative<color>(aPen.color()) ? 
                    static_variant_cast<color>(aPen.color()).as<float>() : 
                    vec4f{ 0.0f, 0.0f, 0.0f, 1.0f },
                {},
                function });
    }

    void opengl_rendering_context::draw_entities(game::i_ecs& aEcs, game::scene_layer aLayer, const mat44& aTransformation)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        thread_local std::vector<std::vector<mesh_drawable>> drawables;
        thread_local game::scene_layer maxLayer = 0;
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
#if defined(NEOGFX_DEBUG) && !defined(NDEBUG)
                if (infos.entity_record(entity).debug)
                    service<debug::logger>() << neolib::logger::severity::Debug << "Rendering debug::layoutItem entity..." << std::endl;
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
                    optional_mat44f{},
                    entity);
                if (!game::is_render_cache_clean_no_lock(cache, entity))
                {
                    auto const& rigidBodyTransformation = (rigidBodies.has_entity_record_no_lock(entity) ?
                        to_transformation_matrix(rigidBodies.entity_record_no_lock(entity)) : mat44f::identity());
                    auto const& meshFilterTransformation = (meshFilter.transformation ?
                        *meshFilter.transformation : mat44f::identity());
                    auto const& animationMeshFilterTransformation = (animatedMeshFilters.has_entity_record_no_lock(entity) ?
                        to_transformation_matrix(animatedMeshFilters.entity_record_no_lock(entity)) : mat44f::identity());
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

    void opengl_rendering_context::fill_shape(const game::mesh& aMesh, const vec3& aPosition, const brush& aFill)
    {
        if (std::holds_alternative<std::monostate>(aFill))
            return;

        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        if (std::holds_alternative<gradient>(aFill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill));

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, GL_TRIANGLES };

            auto const& vertices = aMesh.vertices;
            auto const& uv = aMesh.uv;
            vec3f min, max;
            if (std::holds_alternative<gradient>(aFill))
            {
                min = vertices[0].xyz;
                max = min;
                for (auto const& v : vertices)
                {
                    min.x = std::min(min.x, v.x + aPosition.as<float>().x);
                    max.x = std::max(max.x, v.x + aPosition.as<float>().x);
                    min.y = std::min(min.y, v.y + aPosition.as<float>().y);
                    max.y = std::max(max.y, v.y + aPosition.as<float>().y);
                }
            }
            auto const function = to_function(aFill, rect{ point{ min.x, min.y }, size{ max.x - min.x, max.y - min.y } });
            if (!vertexArrays.room_for(aMesh.faces.size() * 3u))
                vertexArrays.draw_and_execute();
            for (auto const& f : aMesh.faces)
            {
                for (auto vi : f)
                {
                    auto const& v = vertices[vi];
                    vertexArrays.push_back({
                        v + aPosition.as<float>(),
                        std::holds_alternative<color>(aFill) ? 
                            static_variant_cast<color>(aFill).as<float>() : 
                            vec4f{ 0.0f, 0.0f, 0.0f, 1.0f },
                        uv[vi],
                        function });
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

        for (auto op = aDrawGlyphOps.cbegin(); op != aDrawGlyphOps.cend(); ++op)
        {
            auto& drawOp = static_variant_cast<const graphics_operation::draw_glyphs&>(*op);
            auto a = drawOp.attributes.begin();
            for (auto g = drawOp.begin; g != drawOp.end; ++g)
            {
                while (a != drawOp.attributes.end() && (g - drawOp.begin) >= a->end)
                    ++a;
                auto& glyphChar = *g;
                drawGlyphCache.emplace_back(drawOp.point, &drawOp.glyphText.content(), &glyphChar, a != drawOp.attributes.end() && (g - drawOp.begin) >= a->start ? &a->attributes : nullptr, drawOp.showMnemonics);
            }
        }

        if (drawGlyphCache.empty())
            return;

        auto start = drawGlyphCache.begin();
        for (auto next = std::next(start); next != drawGlyphCache.end(); ++next)
        {
            if (!graphics_operation::batchable(*start->glyphText, *next->glyphText, *start->glyphChar, *next->glyphChar) ||
                !graphics_operation::batchable(*start->appearance, *next->appearance))
            {
                draw_glyphs(&*start, &*next);
                start = next;
            }
        }
        if (start != drawGlyphCache.end())
            draw_glyphs(&*start, std::next(&*std::prev(drawGlyphCache.end())));
    }

    namespace
    {
        enum class draw_glyphs_stage : std::int32_t
        {
            Paper,
            SpecialEffects,
            EmojiFinal,
            GlyphOutline,
            GlyphFinal,
            Adornments
        };
    }

    void opengl_rendering_context::draw_glyphs(const draw_glyph* aBegin, const draw_glyph* aEnd)
    {
        // Ensure texture shader enabled as glyph shader depends on it...
        rendering_engine().default_shader_program().texture_shader().clear_texture();

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

        auto bounding_rect = [&]()
        {
            optional_rect result;
            for (auto const& drawOp : std::ranges::subrange(aBegin, aEnd))
            {
                auto const& glyphChar = *drawOp.glyphChar;
                // todo: union of AABB of cell and shape quads(, and transform?)
                auto const aabb = to_aabb_2d(glyphChar.cell.begin(), glyphChar.cell.end());
                rect glyphRect{ aabb };
                glyphRect.translate(point{ drawOp.point });
                if (result == std::nullopt)
                    result = glyphRect;
                else
                    result->combine(glyphRect);
            }
            return result;
        };

        auto shape_quad = [&](font const& glyphFont, glyph_char const& glyphChar, bool outline = false)
        {
            static optional_mat44f const italicTransformGui = mat44f{
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { -0.25f, 1.0f, 0.0f, 0.0f },
                    { 0.0f, 0.0f, 1.0f, 0.0f },
                    { 0.0f, 0.0f, 0.0f, 1.0f } };
            static optional_mat44f const italicTransformGame = mat44f{
                    { 1.0f, 0.0f, 0.0f, 0.0f },
                    { 0.25f, 1.0f, 0.0f, 0.0f },
                    { 0.0f, 0.0f, 1.0f, 0.0f },
                    { 0.0f, 0.0f, 0.0f, 1.0f } };
            auto const& italicTransform = ((glyphFont.style() & font_style::EmulatedItalic) != font_style::EmulatedItalic) ?
                optional_mat44f{} :
                logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui ?
                italicTransformGui : italicTransformGame;

            if (!italicTransform)
                return !outline ? glyphChar.shape : glyphChar.outlineShape.value();

            thread_local quadf_2d transformedQuad;
            vec2f centeringTranslation;
            transformedQuad = center_quad(!outline ? glyphChar.shape : glyphChar.outlineShape.value(), centeringTranslation);
            for (auto& v : transformedQuad)
                v = (*italicTransform * vec3f{ v } + -vec3f{ centeringTranslation }).xy;

            return transformedQuad;
        };

        std::size_t normalGlyphCount = 0;

        optional_rect filterRegion;

        for (auto stage : { 
            draw_glyphs_stage::Paper, 
            draw_glyphs_stage::SpecialEffects, 
            draw_glyphs_stage::EmojiFinal, 
            draw_glyphs_stage::GlyphOutline, 
            draw_glyphs_stage::GlyphFinal, 
            draw_glyphs_stage::Adornments })
        {
            switch (stage)
            {
            case draw_glyphs_stage::Paper:
                for (auto const& drawOp : std::ranges::subrange(aBegin, aEnd))
                {
                    auto const& glyphChar = *drawOp.glyphChar;

                    if (!is_whitespace(glyphChar) && !is_emoji(glyphChar))
                        ++normalGlyphCount;

                    if (drawOp.appearance->effect() != std::nullopt && !drawOp.appearance->being_filtered() &&
                        (drawOp.appearance->effect()->type() == text_effect_type::Glow || drawOp.appearance->effect()->type() == text_effect_type::Shadow))
                    {
                        if (filterRegion == std::nullopt)
                            filterRegion = bounding_rect();
                    }
                        
                    if (drawOp.appearance->paper() != std::nullopt && !drawOp.appearance->being_filtered())
                    {
                        auto const& mesh = to_ecs_component(drawOp.point.as<float>() + quadf{glyphChar.cell[0], glyphChar.cell[1], glyphChar.cell[2], glyphChar.cell[3]}, mesh_type::Triangles);

                        meshFilters.push_back(game::mesh_filter{ {}, mesh });
                        meshRenderers.push_back(
                            game::mesh_renderer{
                                game::material{
                                    std::holds_alternative<color>(*drawOp.appearance->paper()) ?
                                        to_ecs_component(std::get<color>(*drawOp.appearance->paper())) : std::optional<game::color>{},
                                    std::holds_alternative<gradient>(*drawOp.appearance->paper()) ?
                                        to_ecs_component(std::get<gradient>(*drawOp.appearance->paper()).with_bounding_box_if_none(bounding_rect())) : std::optional<game::gradient>{} } });
                    }
                }
                break;
            case draw_glyphs_stage::SpecialEffects:
                if (filterRegion)
                {
                    std::optional<scoped_filter<blur_filter>> filter;
                    for (auto const& drawOp : std::ranges::subrange(aBegin, aEnd))
                    {
                        auto& glyphText = *drawOp.glyphText;
                        auto const& glyphChar = *drawOp.glyphChar;

                        if (is_whitespace(glyphChar))
                            continue;

                        if (drawOp.appearance->being_filtered())
                            continue;

                        bool const renderEffects = !drawOp.appearance->only_calculate_effect() && drawOp.appearance->effect();
                        if (!renderEffects)
                            continue;

                        if (is_emoji(glyphChar) && drawOp.appearance->effect()->ignore_emoji())
                            continue;

                        if (!filter)
                            filter.emplace(*this, blur_filter{ *filterRegion, drawOp.appearance->effect()->width() });

                        filter->front_buffer().draw_glyph(
                            drawOp.point.as<scalar>() + drawOp.appearance->effect()->offset(),
                            glyphText,
                            glyphChar,
                            drawOp.appearance->as_being_filtered());
                    }
                }
                break;
            case draw_glyphs_stage::EmojiFinal:
                for (auto const& drawOp : std::ranges::subrange(aBegin, aEnd))
                {
                    auto& glyphChar = *drawOp.glyphChar;

                    if (is_whitespace(glyphChar) || !is_emoji(glyphChar))
                        continue;

                    auto const& glyphQuad = quadf_2d{
                        (glyphChar.cell[0] + glyphChar.shape[0]).round(),
                        (glyphChar.cell[0] + glyphChar.shape[1]).round(),
                        (glyphChar.cell[0] + glyphChar.shape[2]).round(),
                        (glyphChar.cell[0] + glyphChar.shape[3]).round() } + ~drawOp.point.as<float>().xy;

                    auto const& mesh = to_ecs_component(glyphQuad, mesh_type::Triangles);

                    auto const& emojiAtlas = rendering_engine().font_manager().emoji_atlas();
                    auto const& emojiTexture = emojiAtlas.emoji_texture(glyphChar.value).as_sub_texture();
                    meshFilters.push_back(game::mesh_filter{ game::shared<game::mesh>{}, mesh });
                    auto const& ink = !drawOp.appearance->effect() || !drawOp.appearance->being_filtered() ?
                        (drawOp.appearance->ignore_emoji() ? neolib::none : drawOp.appearance->ink()) : 
                        (drawOp.appearance->effect()->ignore_emoji() ? neolib::none : drawOp.appearance->effect()->color());
                    meshRenderers.push_back(game::mesh_renderer
                        {
                            game::material
                            {
                                std::holds_alternative<color>(ink) ? to_ecs_component(static_variant_cast<const color&>(ink)) : std::optional<game::color>{},
                                std::holds_alternative<gradient>(ink) ? to_ecs_component(static_variant_cast<const gradient&>(ink).with_bounding_box_if_none(rect{ to_aabb_2d(glyphQuad.begin(), glyphQuad.end()) })) : std::optional<game::gradient>{},
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
            case draw_glyphs_stage::GlyphOutline:
            case draw_glyphs_stage::GlyphFinal:
                {
                    bool updateGlyphShader = true;

                    for (auto const& drawOp : std::ranges::subrange(aBegin, aEnd))
                    {
                        auto& glyphText = *drawOp.glyphText;
                        auto& glyphChar = *drawOp.glyphChar;

                        if (is_whitespace(glyphChar) || is_emoji(glyphChar))
                            continue;

                        auto const& theGlyph = glyphText.glyph(glyphChar);
                        auto const& glyphFont = glyphText.glyph_font(glyphChar);

                        if (updateGlyphShader)
                        {
                            updateGlyphShader = false;
                            rendering_engine().default_shader_program().glyph_shader().set_first_glyph(*this, glyphText, glyphChar);
                        }

                        bool const subpixelRender = subpixel(glyphChar) && theGlyph.subpixel();

                        if (stage == draw_glyphs_stage::GlyphOutline)
                        {
                            if (theGlyph.has_outline_texture() && drawOp.appearance->effect() && drawOp.appearance->effect()->type() == text_effect_type::Outline)
                            {
                                auto const& shapeQuad = shape_quad(glyphFont, glyphChar, true);

                                auto const& glyphQuad = quadf_2d{
                                    (glyphChar.cell[0] + shapeQuad[0]).round(),
                                    (glyphChar.cell[0] + shapeQuad[1]).round(),
                                    (glyphChar.cell[0] + shapeQuad[2]).round(),
                                    (glyphChar.cell[0] + shapeQuad[3]).round() } + ~drawOp.point.as<float>().xy;

                                auto const& mesh = to_ecs_component(glyphQuad, mesh_type::Triangles);
                                meshFilters.push_back(game::mesh_filter{ {}, mesh });
                                auto const& ink = drawOp.appearance->effect()->color();
                                meshRenderers.push_back(
                                    game::mesh_renderer{
                                        game::material{
                                            std::holds_alternative<color>(ink) ? to_ecs_component(static_variant_cast<const color&>(ink)) : std::optional<game::color>{},
                                            std::holds_alternative<gradient>(ink) ? to_ecs_component(static_variant_cast<const gradient&>(ink).with_bounding_box_if_none(to_aabb_2d(glyphQuad.begin(), glyphQuad.end()))) : std::optional<game::gradient>{},
                                            {},
                                            to_ecs_component(theGlyph.outline_texture()),
                                            shader_effect::Ignore
                                        },
                                        {},
                                        0,
                                        {}, subpixelRender });
                            }
                            continue;
                        }

                        auto const& shapeQuad = shape_quad(glyphFont, glyphChar);

                        auto const& glyphQuad = quadf_2d{
                            (glyphChar.cell[0] + shapeQuad[0]).round(),
                            (glyphChar.cell[0] + shapeQuad[1]).round(),
                            (glyphChar.cell[0] + shapeQuad[2]).round(),
                            (glyphChar.cell[0] + shapeQuad[3]).round() } + ~drawOp.point.as<float>().xy;

                        auto const& mesh = to_ecs_component(glyphQuad, mesh_type::Triangles);
                        meshFilters.push_back(game::mesh_filter{ {}, mesh });
                        auto const& ink = !drawOp.appearance->effect() || !drawOp.appearance->being_filtered() ?
                            drawOp.appearance->ink() : drawOp.appearance->effect()->color();
                        meshRenderers.push_back(
                            game::mesh_renderer{
                                game::material{
                                    std::holds_alternative<color>(ink) ? to_ecs_component(static_variant_cast<const color&>(ink)) : std::optional<game::color>{},
                                    std::holds_alternative<gradient>(ink) ? to_ecs_component(static_variant_cast<const gradient&>(ink).with_bounding_box_if_none(to_aabb_2d(glyphQuad.begin(), glyphQuad.end()))) : std::optional<game::gradient>{},
                                    {},
                                    to_ecs_component(theGlyph.texture()),
                                    shader_effect::Ignore
                                },
                                {},
                                0,
                                {}, subpixelRender });
                    }
                }
                break;
            case draw_glyphs_stage::Adornments:
                {
                    for (auto const& drawOp : std::ranges::subrange(aBegin, aEnd))
                    {
                        auto& glyphText = *drawOp.glyphText;
                        auto& glyphChar = *drawOp.glyphChar;
                        auto const baseline = static_cast<float>(glyphText.baseline());

                        if (underline(glyphChar) || (drawOp.showMnemonics && neogfx::mnemonic(glyphChar)))
                        {
                            auto const& ink = !drawOp.appearance->effect() || !drawOp.appearance->being_filtered() ?
                                drawOp.appearance->ink() : drawOp.appearance->effect()->color();

                            auto const& glyphFont = glyphText.glyph_font(glyphChar);
                            auto const& shapeQuad = shape_quad(glyphFont, glyphChar);

                            auto const& majorFont = glyphText.major_font();
                            auto const yUnderline = static_cast<float>(std::round(majorFont.native_font_face().underline_position()));
                            auto const cyUnderline = static_cast<float>(std::ceil(majorFont.native_font_face().underline_thickness()));

                            if (drawOp.appearance->smart_underline() &&
                                !is_whitespace(glyphChar) && !is_emoji(glyphChar) &&
                                (glyphFont.style() & font_style::EmulatedItalic) != font_style::EmulatedItalic &&
                                logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui)
                            {
                                auto const& theGlyph = glyphText.glyph(glyphChar);

                                auto const& testLine = texture_line_segment{
                                    { -shapeQuad[0].x, shapeQuad[0].y - (baseline - yUnderline) },
                                    { -shapeQuad[0].x + glyphChar.cell[1].x - glyphChar.cell[0].x, shapeQuad[0].y - (baseline - yUnderline)}};

                                thread_local vector<texture_line_segment> lineSegments;
                                lineSegments = theGlyph.texture().intersection(testLine, rect{ point{}, theGlyph.texture().extents() },
                                    vec2{ std::max<scalar>( 3.0_dip, drawOp.appearance->effect() ? drawOp.appearance->effect().value().width() : 0.0), 1.0});
                                for (auto& segment : lineSegments)
                                {
                                    segment.v1.x += (shapeQuad[0].x + glyphChar.cell[0].x);
                                    segment.v2.x += (shapeQuad[0].x + glyphChar.cell[0].x);
                                    /// @todo apply any transformation to the line segment
                                }
                                if (lineSegments.empty())
                                    draw_line(
                                        drawOp.point + vec3f{ glyphChar.cell[0].x, glyphChar.cell[0].y + baseline - yUnderline },
                                        drawOp.point + vec3f{ glyphChar.cell[1].x, glyphChar.cell[1].y + baseline - yUnderline },
                                        pen{ ink, cyUnderline });
                                else
                                {
                                    auto lineSegment = lineSegments.begin();
                                    for (float x = glyphChar.cell[0].x; lineSegment != lineSegments.end() && x < glyphChar.cell[1].x;)
                                    {
                                        draw_line(
                                            drawOp.point + vec3f{ x, glyphChar.cell[0].y + baseline - yUnderline },
                                            drawOp.point + vec3f{ static_cast<float>(lineSegment->v1.x), glyphChar.cell[1].y + baseline - yUnderline },
                                            pen{ ink, cyUnderline });
                                        x = static_cast<float>(lineSegment->v2.x);
                                        if (++lineSegment == lineSegments.end())
                                        {
                                            draw_line(
                                                drawOp.point + vec3f{ x, glyphChar.cell[0].y + baseline - yUnderline },
                                                drawOp.point + vec3f{ glyphChar.cell[1].x, glyphChar.cell[1].y + baseline - yUnderline },
                                                pen{ ink, cyUnderline });
                                        }
                                    }
                                }
                            }
                            else
                            {
                                draw_line(
                                    drawOp.point + vec3f{ glyphChar.cell[0].x, glyphChar.cell[0].y + baseline - yUnderline },
                                    drawOp.point + vec3f{ glyphChar.cell[1].x, glyphChar.cell[1].y + baseline - yUnderline },
                                    pen{ ink, cyUnderline });
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

        std::optional<neolib::cookie> textureId;
        std::optional<float> uvGui;
        vec2f textureStorageExtents;
        vec2f uvFixupCoefficient;
        vec2f uvFixupOffset;

        for (auto md = aFirst; md != aLast; ++md)
        {
            auto& meshDrawable = *md;
            auto& meshFilter = *meshDrawable.filter;
            auto& meshRenderer = *meshDrawable.renderer;
            thread_local game::mesh_render_cache ignore;
            ignore = {};
            auto const& meshRenderCache = (meshDrawable.entity != null_entity ? cache->entity_record_no_lock(meshDrawable.entity, true) : ignore);
            auto& mesh = (meshFilter.mesh != std::nullopt ? *meshFilter.mesh : *meshFilter.sharedMesh.ptr);
            auto const& transformation = meshDrawable.transformation;
            auto const& faces = mesh.faces;
            auto const& material = meshRenderer.material;
            vec4f const defaultColor{ 1.0f, 1.0f, 1.0f, 1.0f };
            auto add_item = [&](vec2u32& cacheIndices, auto const& mesh, auto const& material, auto const& faces)
            {
                auto const function = material.gradient != std::nullopt && material.gradient->boundingBox ?
                    vec4f{
                        material.gradient->boundingBox->min.x, material.gradient->boundingBox->min.y,
                        material.gradient->boundingBox->max.x, material.gradient->boundingBox->max.y } :
                    meshRenderer.filter != std::nullopt && meshRenderer.filter->boundingBox ?
                        vec4f{
                            meshRenderer.filter->boundingBox->min.x, meshRenderer.filter->boundingBox->min.y,
                            meshRenderer.filter->boundingBox->max.x, meshRenderer.filter->boundingBox->max.y } :
                        vec4f{};
                if (meshRenderCache.state != game::cache_state::Clean)
                {
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
                                uvFixupOffset = vec2f{ 1.0f, 1.0f };
                            else if (materialTexture.subTexture == std::nullopt)
                                uvFixupOffset = texture.as_sub_texture().atlas_location().top_left().to_vec2().as<float>() + vec2f{1.0f, 1.0f};
                            else
                                uvFixupOffset = materialTexture.subTexture->min + vec2f{ 1.0f, 1.0f };
                            if (texture.is_render_target() && texture.as_render_target().logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui)
                                uvGui = static_cast<float>(texture.extents().to_vec2().as<float>().y / textureStorageExtents.y);
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
                            auto const& rgba = (material.color != std::nullopt ? material.color->rgba : defaultColor);
                            auto const& uv = (patch_drawable::has_texture(meshRenderer, material) ?
                                (mesh.uv[faceVertexIndex].scale(uvFixupCoefficient) + uvFixupOffset).scale(1.0f / textureStorageExtents) : vec2f{});
                            auto const& xyzw = function;
                            if (nextIndex == vertices.size())
                                vertices.emplace_back(xyz, rgba, uv, xyzw);
                            else
                            {
                                auto& vertex = vertices[nextIndex];
                                vertex.xyz = xyz;
                                vertex.rgba = rgba;
                                vertex.st = uv;
                                vertex.xyzw = xyzw;
                            }
                            if (uvGui)
                                vertices[nextIndex].st.y = *uvGui - vertices[nextIndex].st.y;
                            ++nextIndex;
                        }
                    }
                    cacheIndices[0] = static_cast<std::uint32_t>(vertexStartIndex);
                    cacheIndices[1] = static_cast<std::uint32_t>(nextIndex);
                }
                patchDrawable.items.emplace_back(meshDrawable, cacheIndices[0], cacheIndices[1], material, faces);
            };
#if defined(NEOGFX_DEBUG) && !defined(NDEBUG)
            if (meshDrawable.entity != game::null_entity &&
                dynamic_cast<game::i_ecs&>(aVertexProvider).component<game::entity_info>().entity_record(meshDrawable.entity).debug)
                service<debug::logger>() << neolib::logger::severity::Debug << "Adding debug::layoutItem entity drawable..." << std::endl;
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
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iOpacity };
        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto const transformation = aTransformation.as<float>();

        std::optional<use_vertex_arrays> vertexArrayUsage;

        auto const logicalCoordinates = logical_coordinates();

        for (auto item = aPatch.items.begin(); item != aPatch.items.end();)
        {
            auto& vertexBuffer = static_cast<opengl_vertex_buffer<>&>(service<i_rendering_engine>().vertex_buffer(*aPatch.provider));
            auto& vertices = vertexBuffer.vertices();

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
                rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, *item->material->gradient);
            else if (iGradient)
                rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, *iGradient);
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
                    vertexArrayUsage.emplace(*aPatch.provider, *this, GL_TRIANGLES, transformation, with_textures, 0, batchRenderer.barrier);

#if defined(NEOGFX_DEBUG) && !defined(NDEBUG)
                if (item->meshDrawable->entity != game::null_entity &&
                    dynamic_cast<game::i_ecs&>(*aPatch.provider).component<game::entity_info>().entity_record(item->meshDrawable->entity).debug)
                    service<debug::logger>() << neolib::logger::severity::Debug << "Drawing debug::layoutItem entity (texture)..." << std::endl;

#endif // NEOGFX_DEBUG
                vertexArrayUsage->draw(item->vertexArrayIndexStart, faceCount * 3);
            }
            else
            {
                rendering_engine().default_shader_program().texture_shader().clear_texture();

                if (vertexArrayUsage == std::nullopt || vertexArrayUsage->with_textures())
                    vertexArrayUsage.emplace(*aPatch.provider, *this, GL_TRIANGLES, transformation, 0, batchRenderer.barrier);

#if defined(NEOGFX_DEBUG) && !defined(NDEBUG)
                if (item->meshDrawable->entity != game::null_entity &&
                    dynamic_cast<game::i_ecs&>(*aPatch.provider).component<game::entity_info>().entity_record(item->meshDrawable->entity).debug)
                    service<debug::logger>() << neolib::logger::severity::Debug << "Drawing debug::layoutItem entity (non-texture)..." << std::endl;

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
            uv = (aTextureRect.top_left() / aTexture.extents()).to_vec2().as<float>() + uv.scale((aTextureRect.extents() / aTexture.extents()).to_vec2().as<float>());
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