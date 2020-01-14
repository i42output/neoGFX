// opengl_rendering_context.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/game/entity_info.hpp>
#include "../../hid/native/i_native_surface.hpp"
#include "i_native_texture.hpp"
#include "../text/native/i_native_font_face.hpp"
#include "opengl_rendering_context.hpp"
#include "opengl_vertex_arrays.hpp"

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
            skip_iterator() :
                iBegin{ aBegin }, iEnd{ aEnd }, iSkipAmount{ 2u }, iPasses{ 1u }, iNext{ aBegin }, iSkipPass{ 1u }, iPass{ 1u }
            {
            }
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
            const value_type& operator*() const
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

        inline vertices line_loop_to_lines(const vertices& aLineLoop)
        {
            vertices result;
            result.reserve(aLineLoop.size() * 2);
            for (auto v = aLineLoop.begin(); v != aLineLoop.end(); ++v)
            {
                result.push_back(*v);
                if (v != aLineLoop.begin() && v != aLineLoop.end() - 1)
                    result.push_back(*v);
            }
            return result;
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

        void emit_any_stipple(i_rendering_context& aContext, use_vertex_arrays_instance& aInstance, scalar aDiscardFor = 0.0)
        {
            // assumes vertices are quads (as two triangles) created with quads_to_triangles above.
            auto& stippleShader = aContext.rendering_engine().default_shader_program().stipple_shader();
            if (stippleShader.stipple_active())
            {
                auto start = midpoint(aInstance.begin()->xyz, std::next(aInstance.begin())->xyz);
                auto end = midpoint(std::next(aInstance.begin(), 4)->xyz, std::next(aInstance.begin(), 2)->xyz);
                stippleShader.start(aContext, start);
                aInstance.draw(6u);
                while (!aInstance.empty())
                {
                    auto const counterOffset = start.distance(end) - aDiscardFor;
                    start = midpoint(aInstance.begin()->xyz, std::next(aInstance.begin())->xyz);
                    end = midpoint(std::next(aInstance.begin(), 4)->xyz, std::next(aInstance.begin(), 2)->xyz);
                    stippleShader.next(aContext, start, counterOffset, aDiscardFor);
                    aInstance.draw(6u);
                }
            }
        }
    }

    opengl_rendering_context::opengl_rendering_context(const i_render_target& aTarget, neogfx::blending_mode aBlendingMode) :
        iRenderingEngine{ service<i_rendering_engine>() },
        iTarget{aTarget}, 
        iWidget{ nullptr },
        iMultisample{ true },
        iOpacity{ 1.0 },
        iSubpixelRendering{ rendering_engine().is_subpixel_rendering_on() },
        iClipCounter{ 0 },
        iSrt{ iTarget },
        iUseDefaultShaderProgram{ *this, rendering_engine().default_shader_program() },
        iSnapToPixel{ false }
    {
        set_blending_mode(aBlendingMode);
        set_smoothing_mode(neogfx::smoothing_mode::AntiAlias);
        iSink += render_target().target_deactivating([this]() 
        { 
            flush(); 
        });
    }

    opengl_rendering_context::opengl_rendering_context(const i_render_target& aTarget, const i_widget& aWidget, neogfx::blending_mode aBlendingMode) :
        iRenderingEngine{ service<i_rendering_engine>() },
        iTarget{ aTarget },
        iWidget{ &aWidget },
        iLogicalCoordinateSystem{ aWidget.logical_coordinate_system() },        
        iMultisample{ true },
        iOpacity{ 1.0 },
        iSubpixelRendering{ rendering_engine().is_subpixel_rendering_on() },
        iClipCounter{ 0 },
        iSrt{ iTarget },
        iUseDefaultShaderProgram{ *this, rendering_engine().default_shader_program() },
        iSnapToPixel{ false }
    {
        set_blending_mode(aBlendingMode);
        set_smoothing_mode(neogfx::smoothing_mode::AntiAlias);
        iSink += render_target().target_deactivating([this]()
        {
            flush();
        });
    }

    opengl_rendering_context::opengl_rendering_context(const opengl_rendering_context& aOther) :
        iRenderingEngine{ aOther.iRenderingEngine },
        iTarget{ aOther.iTarget },
        iWidget{ aOther.iWidget },
        iLogicalCoordinateSystem{ aOther.iLogicalCoordinateSystem },
        iLogicalCoordinates{ aOther.iLogicalCoordinates },
        iMultisample{ true },
        iOpacity{ 1.0 },
        iSubpixelRendering{ aOther.iSubpixelRendering },
        iClipCounter{ 0 },
        iSrt{ iTarget },
        iUseDefaultShaderProgram{ *this, rendering_engine().default_shader_program() },
        iSnapToPixel{ false }
    {
        set_blending_mode(aOther.blending_mode());
        set_smoothing_mode(aOther.smoothing_mode());
        iSink += render_target().target_deactivating([this]()
        {
            flush();
        });
    }

    opengl_rendering_context::~opengl_rendering_context()
    {
    }

    std::unique_ptr<i_rendering_context> opengl_rendering_context::clone() const
    {
        return std::unique_ptr<i_rendering_context>(new opengl_rendering_context(*this));
    }

    i_rendering_engine& opengl_rendering_context::rendering_engine()
    {
        return iRenderingEngine;
    }

    const i_render_target& opengl_rendering_context::render_target() const
    {
        return iTarget;
    }

    const i_render_target& opengl_rendering_context::render_target()
    {
        return iTarget;
    }

    rect opengl_rendering_context::rendering_area(bool aConsiderScissor) const
    {
        if (scissor_rect() == std::nullopt || !aConsiderScissor)
            return rect{ point{}, render_target().target_extents() };
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

    vec2 opengl_rendering_context::offset() const
    {
        return (iOffset != std::nullopt ? *iOffset : vec2{}) + (snap_to_pixel() ? 0.5 : 0.0);
    }

    void opengl_rendering_context::set_offset(const optional_vec2& aOffset)
    {
        iOffset = aOffset;
    }

    bool opengl_rendering_context::snap_to_pixel() const
    {
        return iSnapToPixel;
    }

    void opengl_rendering_context::set_snap_to_pixel(bool aSnapToPixel)
    {
        iSnapToPixel = true;
    }

    void opengl_rendering_context::enqueue(const graphics_operation::operation& aOperation)
    {
        scoped_render_target srt{ render_target() };

        if (iQueue.second.empty())
            iQueue.second.push_back(0);
        bool sameBatch = (iQueue.first.empty() || graphics_operation::batchable(iQueue.first.back(), aOperation)) && iQueue.first.size() - iQueue.second.back() < max_operations(aOperation);
        if (!sameBatch)
            iQueue.second.push_back(iQueue.first.size());
        iQueue.first.push_back(aOperation);
    }

    void opengl_rendering_context::flush()
    {
        if (iQueue.first.empty())
            return;

        apply_clip();
        apply_scissor();
        set_blending_mode(blending_mode());

        iQueue.second.push_back(iQueue.first.size());
        auto endIndex = std::prev(iQueue.second.end());
        for (auto startIndex = iQueue.second.begin(); startIndex != endIndex; ++startIndex)
        {
            graphics_operation::batch opBatch{ &*iQueue.first.begin() + *startIndex, &*iQueue.first.begin() + *std::next(startIndex) };
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
            case graphics_operation::operation_type::ClipToRect:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    clip_to(static_variant_cast<const graphics_operation::clip_to_rect&>(*op).rect);
                break;
            case graphics_operation::operation_type::ClipToPath:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    clip_to(static_variant_cast<const graphics_operation::clip_to_path&>(*op).path, static_variant_cast<const graphics_operation::clip_to_path&>(*op).pathOutline);
                break;
            case graphics_operation::operation_type::ResetClip:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    (void)op;
                    reset_clip();
                }
                break;
            case graphics_operation::operation_type::SnapToPixelOn:
                set_snap_to_pixel(true);
                break;
            case graphics_operation::operation_type::SnapToPixelOff:
                set_snap_to_pixel(false);
                break;
            case graphics_operation::operation_type::SetOpacity:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    set_opacity(static_variant_cast<const graphics_operation::set_opacity&>(*op).opacity);
                break;
            case graphics_operation::operation_type::SetBlendingMode:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    set_blending_mode(static_variant_cast<const graphics_operation::set_blending_mode&>(*op).blendingMode);
                break;
            case graphics_operation::operation_type::SetSmoothingMode:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    set_smoothing_mode(static_variant_cast<const graphics_operation::set_smoothing_mode&>(*op).smoothingMode);
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
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    line_stipple_on(static_variant_cast<const graphics_operation::line_stipple_on&>(*op).factor, static_variant_cast<const graphics_operation::line_stipple_on&>(*op).pattern);
                break;
            case graphics_operation::operation_type::LineStippleOff:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    (void)op;
                    line_stipple_off();
                }
                break;
            case graphics_operation::operation_type::SubpixelRenderingOn:
                subpixel_rendering_on();
                break;
            case graphics_operation::operation_type::SubpixelRenderingOff:
                subpixel_rendering_off();
                break;
            case graphics_operation::operation_type::Clear:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    clear(static_variant_cast<const graphics_operation::clear&>(*op).colour);
                break;
            case graphics_operation::operation_type::ClearDepthBuffer:
                clear_depth_buffer();
                break;
            case graphics_operation::operation_type::ClearStencilBuffer:
                clear_stencil_buffer();
                break;
            case graphics_operation::operation_type::SetPixel:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    set_pixel(static_variant_cast<const graphics_operation::set_pixel&>(*op).point, static_variant_cast<const graphics_operation::set_pixel&>(*op).colour);
                break;
            case graphics_operation::operation_type::DrawPixel:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    draw_pixel(static_variant_cast<const graphics_operation::draw_pixel&>(*op).point, static_variant_cast<const graphics_operation::draw_pixel&>(*op).colour);
                break;
            case graphics_operation::operation_type::DrawLine:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::draw_line&>(*op);
                    draw_line(args.from, args.to, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawRect:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::draw_rect&>(*op);
                    draw_rect(args.rect, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawRoundedRect:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::draw_rounded_rect&>(*op);
                    draw_rounded_rect(args.rect, args.radius, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawCircle:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::draw_circle&>(*op);
                    draw_circle(args.centre, args.radius, args.pen, args.startAngle);
                }
                break;
            case graphics_operation::operation_type::DrawArc:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::draw_arc&>(*op);
                    draw_arc(args.centre, args.radius, args.startAngle, args.endAngle, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawPath:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::draw_path&>(*op);
                    draw_path(args.path, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawShape:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::draw_shape&>(*op);
                    draw_shape(args.mesh, args.pen);
                }
                break;
            case graphics_operation::operation_type::DrawEntities:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::draw_entities&>(*op);
                    draw_entities(args.ecs, args.transformation);
                }
                break;
            case graphics_operation::operation_type::FillRect:
                fill_rect(opBatch);
                break;
            case graphics_operation::operation_type::FillRoundedRect:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::fill_rounded_rect&>(*op);
                    fill_rounded_rect(args.rect, args.radius, args.fill);
                }
                break;
            case graphics_operation::operation_type::FillCircle:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::fill_circle&>(*op);
                    fill_circle(args.centre, args.radius, args.fill);
                }
                break;
            case graphics_operation::operation_type::FillArc:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::fill_arc&>(*op);
                    fill_arc(args.centre, args.radius, args.startAngle, args.endAngle, args.fill);
                }
                break;
            case graphics_operation::operation_type::FillPath:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                    fill_path(static_variant_cast<const graphics_operation::fill_path&>(*op).path, static_variant_cast<const graphics_operation::fill_path&>(*op).fill);
                break;
            case graphics_operation::operation_type::FillShape:
                fill_shape(opBatch);
                break;
            case graphics_operation::operation_type::DrawGlyph:
                draw_glyph(opBatch);
                break;
            case graphics_operation::operation_type::DrawMesh:
                for (auto op = opBatch.first; op != opBatch.second; ++op)
                {
                    const auto& args = static_variant_cast<const graphics_operation::draw_mesh&>(*op);
                    draw_mesh(args.mesh, args.material, args.transformation);
                }
                break;
            }
        }
        iQueue.first.clear();
        iQueue.second.clear();

        use_vertex_arrays uva{ *this, GL_TRIANGLES };
        uva.instance().execute();
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

    void opengl_rendering_context::clip_to(const rect& aRect)
    {
        if (iClipCounter++ == 0)
        {
            glCheck(glClear(GL_STENCIL_BUFFER_BIT));
        }
        apply_clip();
        glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
        glCheck(glDepthMask(GL_FALSE));
        glCheck(glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP));  // draw 1s on test fail (always)
        glCheck(glStencilMask(static_cast<GLuint>(-1)));
        glCheck(glStencilFunc(GL_NEVER, 0, static_cast<GLuint>(-1)));
        fill_rect(rendering_area(), colour::White);
        glCheck(glStencilFunc(GL_NEVER, 1, static_cast<GLuint>(-1)));
        fill_rect(aRect, colour::White);
        glCheck(glStencilFunc(GL_NEVER, 1, static_cast<GLuint>(-1)));
        glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
        glCheck(glDepthMask(GL_TRUE));
        glCheck(glStencilMask(0x00));
        // draw only where stencil's value is 1
        glCheck(glStencilFunc(GL_EQUAL, 1, static_cast<GLuint>(-1)));
    }

    void opengl_rendering_context::clip_to(const path& aPath, dimension aPathOutline)
    {
        neolib::scoped_flag snap{ iSnapToPixel, false };

        if (iClipCounter++ == 0)
        {
            glCheck(glClear(GL_STENCIL_BUFFER_BIT));
        }
        apply_clip();
        glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
        glCheck(glDepthMask(GL_FALSE));
        glCheck(glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP));  // draw 1s on test fail (always)
        glCheck(glStencilMask(static_cast<GLuint>(-1)));
        glCheck(glStencilFunc(GL_NEVER, 0, static_cast<GLuint>(-1)));
        fill_rect(rendering_area(), colour::White);
        glCheck(glStencilFunc(GL_EQUAL, 1, static_cast<GLuint>(-1)));
        for (auto const& subPath : aPath.sub_paths())
        {
            if (subPath.size() > 2)
            {
                GLenum mode;
                auto vertices = path_vertices(aPath, subPath, aPathOutline, mode);

                use_vertex_arrays vertexArrays{ *this, mode, vertices.size() };
                for (const auto& v : vertices)
                    vertexArrays.instance().push_back(opengl_standard_vertex_arrays::vertex{v, vec4f{{1.0, 1.0, 1.0, 1.0}}});
            }
        }
        if (aPathOutline != 0)
        {
            glCheck(glStencilFunc(GL_NEVER, 0, static_cast<GLuint>(-1)));
            path innerPath = aPath;
            innerPath.deflate(delta{ aPathOutline });
            for (auto const& innerSubPath : innerPath.sub_paths())
            {
                if (innerSubPath.size() > 2)
                {
                    GLenum mode;
                    auto vertices = path_vertices(aPath, innerSubPath, 0.0, mode);

                    use_vertex_arrays vertexArrays{ *this, mode, vertices.size() };
                    for (const auto& v : vertices)
                        vertexArrays.instance().push_back(opengl_standard_vertex_arrays::vertex{v, vec4f{{1.0, 1.0, 1.0, 1.0}}});
                }
            }
        }
        glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
        glCheck(glDepthMask(GL_TRUE));
        glCheck(glStencilMask(0x00));
        // draw only where stencil's value is 1
        glCheck(glStencilFunc(GL_EQUAL, 1, static_cast<GLuint>(-1)));
    }

    void opengl_rendering_context::reset_clip()
    {
        --iClipCounter;
        apply_clip();
    }

    void opengl_rendering_context::apply_clip()
    {
        if (iClipCounter > 0)
        {
            glCheck(glEnable(GL_STENCIL_TEST));
        }
        else
        {
            glCheck(glDisable(GL_STENCIL_TEST));
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

    void opengl_rendering_context::line_stipple_on(uint32_t aFactor, uint16_t aPattern)
    {
        rendering_engine().default_shader_program().stipple_shader().set_stipple(aFactor, aPattern);
    }

    void opengl_rendering_context::line_stipple_off()
    {
        rendering_engine().default_shader_program().stipple_shader().clear_stipple();
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

    void opengl_rendering_context::clear(const colour& aColour)
    {
        glCheck(glClearColor(aColour.red<GLclampf>(), aColour.green<GLclampf>(), aColour.blue<GLclampf>(), aColour.alpha<GLclampf>()));
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

    void opengl_rendering_context::set_pixel(const point& aPoint, const colour& aColour)
    {
        /* todo: faster alternative to this... */
        disable_anti_alias daa{ *this };
        draw_pixel(aPoint, aColour.with_alpha(0xFF));
    }

    void opengl_rendering_context::draw_pixel(const point& aPoint, const colour& aColour)
    {
        /* todo: faster alternative to this... */
        fill_rect(rect{ aPoint, size{1.0, 1.0} }, aColour);
    }

    void opengl_rendering_context::draw_line(const point& aFrom, const point& aTo, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        if (std::holds_alternative<gradient>(aPen.colour()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.colour()), rect{ aFrom, aTo });

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

        use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES, triangles.size() };

        for (auto const& v : triangles)
            vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
                vec4f{{
                    static_variant_cast<colour>(aPen.colour()).red<float>(),
                    static_variant_cast<colour>(aPen.colour()).green<float>(),
                    static_variant_cast<colour>(aPen.colour()).blue<float>(),
                    static_variant_cast<colour>(aPen.colour()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{} });

        emit_any_stipple(*this, vertexArrays.instance());
    }

    void opengl_rendering_context::draw_rect(const rect& aRect, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        scoped_anti_alias saa{ *this, smoothing_mode::None };
        std::optional<disable_multisample> disableMultisample;

        if (std::holds_alternative<gradient>(aPen.colour()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.colour()), aRect);

        auto adjustedRect = aRect;
        if (snap_to_pixel())
        {
            disableMultisample.emplace(*this);
            adjustedRect.position() -= size{ static_cast<int32_t>(aPen.width()) % 2 == 1 ? 0.0 : 0.5 };
            adjustedRect = adjustedRect.with_epsilon(size{ 1.0, 1.0 });
        }

        vec3_array<8> lines = rect_vertices(adjustedRect, mesh_type::Outline, 0.0);
        vec3_array<4 * 4> quads;
        lines_to_quads(lines, aPen.width(), quads);
        vec3_array<4 * 6> triangles;
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES, triangles.size() };

        for (auto const& v : triangles)
            vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
                vec4f{{
                    static_variant_cast<colour>(aPen.colour()).red<float>(),
                    static_variant_cast<colour>(aPen.colour()).green<float>(),
                    static_variant_cast<colour>(aPen.colour()).blue<float>(),
                    static_variant_cast<colour>(aPen.colour()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{} });

        emit_any_stipple(*this, vertexArrays.instance(), aPen.width());
    }

    void opengl_rendering_context::draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        if (std::holds_alternative<gradient>(aPen.colour()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.colour()), aRect);

        auto adjustedRect = aRect;
        if (snap_to_pixel())
        {
            adjustedRect.position() -= size{ static_cast<int32_t>(aPen.width()) % 2 == 1 ? 0.0 : 0.5 };
            adjustedRect = adjustedRect.with_epsilon(size{ 1.0, 1.0 });
        }

        auto vertices = rounded_rect_vertices(adjustedRect, aRadius, mesh_type::Outline);

        auto lines = line_loop_to_lines(vertices);
        vec3_list quads;
        lines_to_quads(lines, aPen.width(), quads);
        vec3_list triangles;
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES, triangles.size() };

        for (auto const& v : triangles)
            vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
                vec4f{{
                    static_variant_cast<colour>(aPen.colour()).red<float>(),
                    static_variant_cast<colour>(aPen.colour()).green<float>(),
                    static_variant_cast<colour>(aPen.colour()).blue<float>(),
                    static_variant_cast<colour>(aPen.colour()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{} });

        emit_any_stipple(*this, vertexArrays.instance());
    }

    void opengl_rendering_context::draw_circle(const point& aCentre, dimension aRadius, const pen& aPen, angle aStartAngle)
    {
        neolib::scoped_flag snap{ iSnapToPixel, false };

        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        if (std::holds_alternative<gradient>(aPen.colour()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.colour()), 
                rect{ aCentre - size{aRadius, aRadius}, size{aRadius * 2.0, aRadius * 2.0 } });

        auto vertices = circle_vertices(aCentre, aRadius, aStartAngle, mesh_type::Outline);

        auto lines = line_loop_to_lines(vertices);
        vec3_list quads;
        lines_to_quads(lines, aPen.width(), quads);
        vec3_list triangles;
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES, triangles.size() };

        for (auto const& v : triangles)
            vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
                vec4f{{
                    static_variant_cast<colour>(aPen.colour()).red<float>(),
                    static_variant_cast<colour>(aPen.colour()).green<float>(),
                    static_variant_cast<colour>(aPen.colour()).blue<float>(),
                    static_variant_cast<colour>(aPen.colour()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{} });

        emit_any_stipple(*this, vertexArrays.instance());
    }

    void opengl_rendering_context::draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen)
    {
        neolib::scoped_flag snap{ iSnapToPixel, false };

        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        if (std::holds_alternative<gradient>(aPen.colour()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.colour()),
                rect{ aCentre - size{aRadius, aRadius}, size{aRadius * 2.0, aRadius * 2.0 } });

        auto vertices = arc_vertices(aCentre, aRadius, aStartAngle, aEndAngle, aCentre, mesh_type::Outline);

        auto lines = line_loop_to_lines(vertices);
        vec3_list quads;
        lines_to_quads(lines, aPen.width(), quads);
        vec3_list triangles;
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES, triangles.size() };

        for (auto const& v : triangles)
            vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
                vec4f{{
                    static_variant_cast<colour>(aPen.colour()).red<float>(),
                    static_variant_cast<colour>(aPen.colour()).green<float>(),
                    static_variant_cast<colour>(aPen.colour()).blue<float>(),
                    static_variant_cast<colour>(aPen.colour()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{} });

        emit_any_stipple(*this, vertexArrays.instance());
    }

    void opengl_rendering_context::draw_path(const path& aPath, const pen& aPen)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        if (std::holds_alternative<gradient>(aPen.colour()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.colour()), aPath.bounding_rect());

        for (auto const& subPath : aPath.sub_paths())
        {
            if (subPath.size() > 2)
            {
                if (aPath.shape() == path_shape::ConvexPolygon)
                    clip_to(aPath, aPen.width());

                GLenum mode;
                auto vertices = path_vertices(aPath, subPath, aPen.width(), mode);

                {
                    use_vertex_arrays vertexArrays{ *this, mode, vertices.size() };
                    for (auto const& v : vertices)
                        vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
                            vec4f{{
                                static_variant_cast<colour>(aPen.colour()).red<float>(),
                                static_variant_cast<colour>(aPen.colour()).green<float>(),
                                static_variant_cast<colour>(aPen.colour()).blue<float>(),
                                static_variant_cast<colour>(aPen.colour()).alpha<float>() * static_cast<float>(iOpacity)}} :
                            vec4f{} });
                }
                if (aPath.shape() == path_shape::ConvexPolygon)
                    reset_clip();
            }
        }
    }

    void opengl_rendering_context::draw_shape(const game::mesh& aMesh, const pen& aPen)
    {
        neolib::scoped_flag snap{ iSnapToPixel, false };

        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        if (std::holds_alternative<gradient>(aPen.colour()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const neogfx::gradient&>(aPen.colour()), bounding_rect(aMesh));

        auto const& vertices = aMesh.vertices;

        auto lines = line_loop_to_lines(vertices);
        vec3_list quads;
        lines_to_quads(lines, aPen.width(), quads);
        vec3_list triangles;
        quads_to_triangles(quads, triangles);

        use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES, triangles.size() };

        for (auto const& v : triangles)
            vertexArrays.instance().push_back({ v, std::holds_alternative<colour>(aPen.colour()) ?
                vec4f{{
                    static_variant_cast<colour>(aPen.colour()).red<float>(),
                    static_variant_cast<colour>(aPen.colour()).green<float>(),
                    static_variant_cast<colour>(aPen.colour()).blue<float>(),
                    static_variant_cast<colour>(aPen.colour()).alpha<float>() * static_cast<float>(iOpacity)}} :
                vec4f{} });
    }

    void opengl_rendering_context::draw_entities(game::i_ecs& aEcs, const mat44& aTransformation)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        iRenderingEngine.want_game_mode();
        aEcs.component<game::rigid_body>().take_snapshot();
        auto rigidBodiesSnapshot = aEcs.component<game::rigid_body>().snapshot();
        auto const& rigidBodies = rigidBodiesSnapshot.data();
        thread_local std::vector<mesh_drawable> drawables;
        for (auto entity : aEcs.component<game::mesh_renderer>().entities())
        {
            if (entity == game::null_entity)
                continue; // todo: sort/remove and/or create skipping iterator
            #ifndef NDEBUG
            if (aEcs.component<game::entity_info>().entity_record(entity).debug)
                std::cerr << "Rendering debug entity..." << std::endl;
            #endif
            drawables.emplace_back(
                aEcs.component<game::mesh_filter>().entity_record(entity), 
                aEcs.component<game::mesh_renderer>().entity_record(entity),
                rigidBodies.has_entity_record(entity) ? 
                    to_transformation_matrix(rigidBodies.entity_record(entity)) : mat44::identity(),
                entity);
        }
        draw_meshes(&*drawables.begin(), &*drawables.begin() + drawables.size(), aTransformation);
        for (auto const& d : drawables)
            if (!d.drawn && d.renderer->destroyOnFustrumCull)
                aEcs.destroy_entity(d.entity);
        drawables.clear();
    }

    void opengl_rendering_context::fill_rect(const rect& aRect, const brush& aFill, scalar aZpos)
    {
        graphics_operation::operation op{ graphics_operation::fill_rect{ aRect, aFill, aZpos } };
        fill_rect(graphics_operation::batch{ &op, &op + 1 });
    }

    void opengl_rendering_context::fill_rect(const graphics_operation::batch& aFillRectOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        neolib::scoped_flag snap{ iSnapToPixel, false };
        scoped_anti_alias saa{ *this, smoothing_mode::None };
        disable_multisample disableMultisample{ *this };

        auto& firstOp = static_variant_cast<const graphics_operation::fill_rect&>(*aFillRectOps.first);

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill), firstOp.rect);
        {
            use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES, static_cast<std::size_t>(2u * 3u * (aFillRectOps.second - aFillRectOps.first))};

            for (auto op = aFillRectOps.first; op != aFillRectOps.second; ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::fill_rect&>(*op);
                auto rectVertices = rect_vertices(drawOp.rect, mesh_type::Triangles, drawOp.zpos);
                for (const auto& v : rectVertices)
                    vertexArrays.instance().push_back({ v,
                        std::holds_alternative<colour>(drawOp.fill) ?
                            vec4f{{
                                static_variant_cast<const colour&>(drawOp.fill).red<float>(),
                                static_variant_cast<const colour&>(drawOp.fill).green<float>(),
                                static_variant_cast<const colour&>(drawOp.fill).blue<float>(),
                                static_variant_cast<const colour&>(drawOp.fill).alpha<float>() * static_cast<float>(iOpacity)}} :
                            vec4f{} });
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
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill), aRect);

        auto vertices = rounded_rect_vertices(aRect, aRadius, mesh_type::Triangles);
        
        {
            use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES, vertices.size() };

            for (const auto& v : vertices)
            {
                vertexArrays.instance().push_back({v, std::holds_alternative<colour>(aFill) ?
                    vec4f{{
                        static_variant_cast<const colour&>(aFill).red<float>(),
                        static_variant_cast<const colour&>(aFill).green<float>(),
                        static_variant_cast<const colour&>(aFill).blue<float>(),
                        static_variant_cast<const colour&>(aFill).alpha<float>() * static_cast<float>(iOpacity)}} :
                    vec4f{}}); 
            }
        }
    }

    void opengl_rendering_context::fill_circle(const point& aCentre, dimension aRadius, const brush& aFill)
    {
        neolib::scoped_flag snap{ iSnapToPixel, false };

        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        if (std::holds_alternative<gradient>(aFill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill), 
                rect{ aCentre - point{ aRadius, aRadius }, size{ aRadius * 2.0 } });

        auto vertices = circle_vertices(aCentre, aRadius, 0.0, mesh_type::TriangleFan);

        {
            use_vertex_arrays vertexArrays{ *this, GL_TRIANGLE_FAN, vertices.size() };
            for (const auto& v : vertices)
            {
                vertexArrays.instance().push_back({v, std::holds_alternative<colour>(aFill) ?
                    vec4f{{
                        static_variant_cast<const colour&>(aFill).red<float>(),
                        static_variant_cast<const colour&>(aFill).green<float>(),
                        static_variant_cast<const colour&>(aFill).blue<float>(),
                        static_variant_cast<const colour&>(aFill).alpha<float>() * static_cast<float>(iOpacity)}} :
                    vec4f{}});
            }
        }
    }

    void opengl_rendering_context::fill_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill)
    {
        neolib::scoped_flag snap{ iSnapToPixel, false };

        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        if (std::holds_alternative<gradient>(aFill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill), 
                rect{ aCentre - point{ aRadius, aRadius }, size{ aRadius * 2.0 } });

        auto vertices = arc_vertices(aCentre, aRadius, aStartAngle, aEndAngle, aCentre, mesh_type::TriangleFan);

        {
            use_vertex_arrays vertexArrays{ *this, GL_TRIANGLE_FAN, vertices.size() };
            for (const auto& v : vertices)
            {
                vertexArrays.instance().push_back({v, std::holds_alternative<colour>(aFill) ?
                    vec4f{{
                        static_variant_cast<const colour&>(aFill).red<float>(),
                        static_variant_cast<const colour&>(aFill).green<float>(),
                        static_variant_cast<const colour&>(aFill).blue<float>(),
                        static_variant_cast<const colour&>(aFill).alpha<float>() * static_cast<float>(iOpacity)}} :
                    vec4f{}});
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
                clip_to(aPath, 0.0);
                point min = subPath[0];
                point max = min;
                for (auto const& pt : subPath)
                {
                    min = min.min(pt);
                    max = max.max(pt);
                }

                if (std::holds_alternative<gradient>(aFill))
                    rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill), 
                        rect{ point{ min.x, min.y }, size{ max.x - min.y, max.y - min.y } });

                GLenum mode;
                auto vertices = path_vertices(aPath, subPath, 0.0, mode);

                {
                    use_vertex_arrays vertexArrays{ *this, mode, vertices.size() };
                    for (const auto& v : vertices)
                    {
                        vertexArrays.instance().push_back({v, std::holds_alternative<colour>(aFill) ?
                            vec4f{{
                                static_variant_cast<const colour&>(aFill).red<float>(),
                                static_variant_cast<const colour&>(aFill).green<float>(),
                                static_variant_cast<const colour&>(aFill).blue<float>(),
                                static_variant_cast<const colour&>(aFill).alpha<float>() * static_cast<float>(iOpacity)}} :
                            vec4f{}});
                    }
                }

                reset_clip();
            }
        }
    }

    void opengl_rendering_context::fill_shape(const graphics_operation::batch& aFillShapeOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };

        auto& firstOp = static_variant_cast<const graphics_operation::fill_shape&>(*aFillShapeOps.first);

        if (std::holds_alternative<gradient>(firstOp.fill))
        {
            auto const& vertices = firstOp.mesh.vertices;
            vec3 min = vertices[0].xyz;
            vec3 max = min;
            for (auto const& v : vertices)
            {
                min.x = std::min(min.x, v.x);
                max.x = std::max(max.x, v.x);
                min.y = std::min(min.y, v.y);
                max.y = std::max(max.y, v.y);
            }
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill),
                rect{
                    point{ min.x, min.y },
                    size{ max.x - min.y, max.y - min.y } });
        }

        {
            use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES };
            for (auto op = aFillShapeOps.first; op != aFillShapeOps.second; ++op)
            {
                auto& drawOp = static_variant_cast<const graphics_operation::fill_shape&>(*op);
                auto const& vertices = drawOp.mesh.vertices;
                auto const& uv = drawOp.mesh.uv;
                if (!vertexArrays.instance().room_for(drawOp.mesh.faces.size() * 3u))
                    vertexArrays.instance().execute();
                for (auto const& f : drawOp.mesh.faces)
                {
                    for (auto vi : f)
                    {
                        auto const& v = vertices[vi];
                        vertexArrays.instance().push_back({
                            v,
                            std::holds_alternative<colour>(drawOp.fill) ?
                                vec4f{{
                                    static_variant_cast<const colour&>(drawOp.fill).red<float>(),
                                    static_variant_cast<const colour&>(drawOp.fill).green<float>(),
                                    static_variant_cast<const colour&>(drawOp.fill).blue<float>(),
                                    static_variant_cast<const colour&>(drawOp.fill).alpha<float>() * static_cast<float>(iOpacity)}} :
                                vec4f{},
                            uv[vi]});
                    }
                }
            }
        }
    }

    namespace
    {
        template <typename Result>
        void texture_vertices(const size& aTextureStorageSize, const rect& aTextureRect, const neogfx::logical_coordinates& aLogicalCoordinates, Result& aResult)
        {
            rect normalizedRect = aTextureRect / aTextureStorageSize;
            aResult.emplace_back(normalizedRect.top_left().x, normalizedRect.top_left().y);
            aResult.emplace_back(normalizedRect.top_right().x, normalizedRect.top_right().y);
            aResult.emplace_back(normalizedRect.bottom_right().x, normalizedRect.bottom_right().y);
            aResult.emplace_back(normalizedRect.bottom_left().x, normalizedRect.bottom_left().y);
            if (aLogicalCoordinates.is_gui_orientation())
            {
                std::swap(aResult[0][1], aResult[2][1]);
                std::swap(aResult[1][1], aResult[3][1]);
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

    std::size_t opengl_rendering_context::max_operations(const graphics_operation::operation& aOperation)
    {
        auto need = 1u;
        if (std::holds_alternative<graphics_operation::draw_glyph>(aOperation))
        {
            need = 6u;
            auto& drawGlyphOp = static_variant_cast<const graphics_operation::draw_glyph&>(aOperation);
            if (drawGlyphOp.appearance.effect() && drawGlyphOp.appearance.effect()->type() == text_effect_type::Outline)
                need += 6u * static_cast<uint32_t>(std::ceil((drawGlyphOp.appearance.effect()->width() * 2 + 1) * (drawGlyphOp.appearance.effect()->width() * 2 + 1)));
        }
        return rendering_engine().vertex_arrays().capacity() / need;
    }

    void opengl_rendering_context::draw_glyph(const graphics_operation::batch& aDrawGlyphOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };
        disable_anti_alias daa{ *this };
        neolib::scoped_flag snap{ iSnapToPixel, false };

        thread_local std::vector<game::mesh_filter> meshFilters;
        thread_local std::vector<game::mesh_renderer> meshRenderers;
        meshFilters.clear();
        meshRenderers.clear();

        std::size_t normalGlyphCount = 0;

        for (int32_t pass = 1; pass <= 3; ++pass)
        {
            switch (pass)
            {
            case 1: // Paper (glyph background) and emoji
                for (auto op = aDrawGlyphOps.first; op != aDrawGlyphOps.second; ++op)
                {
                    auto& drawOp = static_variant_cast<const graphics_operation::draw_glyph&>(*op);

                    if (!drawOp.glyph.is_whitespace() && !drawOp.glyph.is_emoji())
                        ++normalGlyphCount;

                    if (drawOp.appearance.paper() != std::nullopt)
                    {
                        font const& glyphFont = drawOp.glyph.font();

                        auto const& mesh = to_ecs_component(
                            rect{
                                point{ drawOp.point },
                                size{ drawOp.glyph.advance().cx, glyphFont.height() } },
                            mesh_type::Triangles,
                            drawOp.point.z);

                        meshFilters.push_back(game::mesh_filter{ game::shared<game::mesh>{}, mesh });
                        meshRenderers.push_back(
                            game::mesh_renderer{
                                game::material{
                                    std::holds_alternative<colour>(*drawOp.appearance.paper()) ?
                                        to_ecs_component(std::get<colour>(*drawOp.appearance.paper())) : std::optional<game::colour>{},
                                    std::holds_alternative<gradient>(*drawOp.appearance.paper()) ?
                                        to_ecs_component(std::get<gradient>(*drawOp.appearance.paper())) : std::optional<game::gradient>{} } });
                    }

                    if (drawOp.glyph.is_emoji())
                    {
                        font const& glyphFont = drawOp.glyph.font();

                        auto const& mesh = to_ecs_component(
                            rect{
                                point{ drawOp.point },
                                size{ drawOp.glyph.advance().cx, glyphFont.height() } },
                                mesh_type::Triangles,
                                drawOp.point.z);

                        auto const& emojiAtlas = rendering_engine().font_manager().emoji_atlas();
                        auto const& emojiTexture = emojiAtlas.emoji_texture(drawOp.glyph.value()).as_sub_texture();
                        meshFilters.push_back(game::mesh_filter{ game::shared<game::mesh>{}, mesh });
                        meshRenderers.push_back(
                            game::mesh_renderer{
                                game::material{
                                    {},
                                    {},
                                    {},
                                    to_ecs_component(emojiTexture)
                                } });
                    }
                }
                break;
            case 2: // Special effects
            case 3: // Final glyph render
                {
                }
                break;
            }
        }

        thread_local std::vector<mesh_drawable> drawables;
        drawables.clear();
        for (std::size_t i = 0; i < meshFilters.size(); ++i)
            drawables.emplace_back(meshFilters[i], meshRenderers[i]);
        if (!drawables.empty())
            draw_meshes(&*drawables.begin(), &*drawables.begin() + drawables.size(), mat44::identity());
    }

    void opengl_rendering_context::draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const mat44& aTransformation)
    {
        draw_mesh(game::mesh_filter{ { &aMesh }, {}, {} }, game::mesh_renderer{ aMaterial, {} }, aTransformation);
    }
    
    void opengl_rendering_context::draw_mesh(const game::mesh_filter& aMeshFilter, const game::mesh_renderer& aMeshRenderer, const mat44& aTransformation)
    {
        mesh_drawable drawable
        {
            aMeshFilter,
            aMeshRenderer,
            aMeshFilter.transformation != std::nullopt ?
                *aMeshFilter.transformation : mat44::identity()
        };
        draw_meshes(&drawable, &drawable + 1, aTransformation);
    }

    void opengl_rendering_context::draw_meshes(mesh_drawable* aFirst, mesh_drawable* aLast, const mat44& aTransformation)
    {
        thread_local patch_drawable patch = {};
        patch.xyz.clear();
        patch.uv.clear();
        patch.items.clear();

        for (auto m = aFirst; m != aLast; ++m)
        {
            auto& mesh = *m;
            auto const& untransformed = (mesh.filter->mesh != std::nullopt ?
                *mesh.filter->mesh : *mesh.filter->sharedMesh.ptr).vertices;
            auto const transformation = aTransformation * (mesh.filter->transformation != std::nullopt ? *mesh.filter->transformation : mat44::identity()) * mesh.transformation;
            thread_local vec3_list xyz;
            xyz.clear();
            std::transform(untransformed.begin(), untransformed.end(), std::back_inserter(xyz), [&transformation](auto const& v)
            {
                return transformation * v;
            });
            auto const& uv = mesh.filter->mesh != std::nullopt ?
                mesh.filter->mesh->uv : mesh.filter->sharedMesh.ptr->uv;
            auto const& faces = mesh.filter->mesh != std::nullopt ?
                mesh.filter->mesh->faces : mesh.filter->sharedMesh.ptr->faces;
            auto const& material = mesh.renderer->material;

            auto const offsetVertices = patch.xyz.size();
            auto const offsetTextureVertices = patch.uv.size();
            patch.xyz.insert(patch.xyz.end(), xyz.begin(), xyz.end());
            patch.uv.insert(patch.uv.end(), uv.begin(), uv.end());
            patch.items.emplace_back(mesh, offsetVertices, offsetTextureVertices, material, faces);

            for (auto const& meshPatch : mesh.renderer->patches)
                patch.items.emplace_back(mesh, offsetVertices, offsetTextureVertices, meshPatch.material, meshPatch.faces);
        }

        draw_patch(patch);
    }

    void opengl_rendering_context::draw_patch(patch_drawable& aPatch)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program() };
        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto const logicalCoordinates = logical_coordinates();

        auto const& vertices = aPatch.xyz;
        auto const& textureVertices = aPatch.uv;

        for (auto item = aPatch.items.begin(); item != aPatch.items.end();)
        {
            GLint previousTexture = 0;

            auto const& batchMaterial = *item->material;
            vec2 textureStorageExtents;

            auto calc_bounding_rect = [&aPatch](const patch_drawable::item& aItem) -> rect
            {
                return game::bounding_rect(aPatch.xyz, *aItem.faces, aItem.offsetVertices);
            };

            auto calc_sampling = [&aPatch, &calc_bounding_rect](const patch_drawable::item& aItem) -> texture_sampling
            {
                const game::material& material = *aItem.material;
                if (material.texture == std::nullopt)
                    return texture_sampling::Normal;
                auto const& texture = *service<i_texture_manager>().find_texture(material.texture->id.cookie());
                auto sampling =
                    (material.texture->sampling != std::nullopt ?
                        *material.texture->sampling : texture.sampling());
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
            while (next != aPatch.items.end() && game::batchable(*item->material, *next->material) && sampling == calc_sampling(*next))
            {
                faceCount += next->faces->size();
                ++next;
            }

            {
                if (batchMaterial.texture != std::nullopt)
                {
                    auto const& texture = *service<i_texture_manager>().find_texture(batchMaterial.texture->id.cookie());

                    textureStorageExtents = texture.storage_extents().to_vec2();

                    glCheck(glActiveTexture(texture.sampling() != texture_sampling::Multisample ? GL_TEXTURE1 : GL_TEXTURE2));

                    glCheck(glGetIntegerv(texture.sampling() != texture_sampling::Multisample ? GL_TEXTURE_BINDING_2D : GL_TEXTURE_BINDING_2D_MULTISAMPLE, &previousTexture));
                    glCheck(glBindTexture(texture.sampling() != texture_sampling::Multisample ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(texture.native_handle())));
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
                }
                else
                    rendering_engine().default_shader_program().texture_shader().clear_texture();

                use_vertex_arrays vertexArrays{ *this, GL_TRIANGLES, with_textures, faceCount * 3 };

                for (; item != next; ++item)
                {
                    vec2 uvFixupCoefficient;
                    vec2 uvFixupOffset;

                    auto const& material = *item->material;

                    if (material.texture != std::nullopt)
                    {
                        auto const& texture = *service<i_texture_manager>().find_texture(material.texture->id.cookie());
                        uvFixupCoefficient = material.texture->extents;
                        if (material.texture->type == texture_type::Texture)
                            uvFixupOffset = vec2{ 1.0, 1.0 };
                        else if (material.texture->subTexture == std::nullopt)
                            uvFixupOffset = texture.as_sub_texture().atlas_location().top_left().to_vec2();
                        else
                            uvFixupOffset = material.texture->subTexture->min;
                    }

                    auto const offsetVertices = item->offsetVertices;
                    auto const offsetTextureVertices = item->offsetTextureVertices;
                    auto const& faces = *item->faces;

                    colour colourizationColour{ 0xFF, 0xFF, 0xFF, 0xFF };
                    if (material.colour != std::nullopt)
                        colourizationColour = material.colour->rgba;

                    for (auto const& face : faces)
                    {
                        for (auto faceVertexIndex : face)
                        {
                            auto const vertexIndexOffset  = faceVertexIndex + offsetVertices;
                            auto const textureVertexIndexOffset = faceVertexIndex + offsetTextureVertices;
                            auto const& v = vertices[vertexIndexOffset];
                            if (v.x >= std::min(logicalCoordinates.bottomLeft.x, logicalCoordinates.topRight.x) &&
                                v.x <= std::max(logicalCoordinates.bottomLeft.x, logicalCoordinates.topRight.x) &&
                                v.y >= std::min(logicalCoordinates.bottomLeft.y, logicalCoordinates.topRight.y) &&
                                v.y <= std::max(logicalCoordinates.bottomLeft.y, logicalCoordinates.topRight.y))
                                item->mesh->drawn = true;
                            vec2 uv = {};
                            if (material.texture != std::nullopt)
                                uv = (textureVertices[textureVertexIndexOffset].scale(uvFixupCoefficient) + uvFixupOffset).scale(1.0 / textureStorageExtents);
                            vertexArrays.instance().emplace_back(
                                v,
                                vec4f{ {
                                    colourizationColour.red<float>(),
                                    colourizationColour.green<float>(),
                                    colourizationColour.blue<float>(),
                                    colourizationColour.alpha<float>() * static_cast<float>(iOpacity)} },
                                uv);
                        }
                    }
                }
                vertexArrays.instance().draw();
            }

            if (batchMaterial.texture != std::nullopt)
            {
                auto const& texture = *service<i_texture_manager>().find_texture(batchMaterial.texture->id.cookie());
                glCheck(glBindTexture(texture.sampling() != texture_sampling::Multisample ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE, static_cast<GLuint>(previousTexture)));
            }

            disable_sample_shading();
        }
    }

    xyz opengl_rendering_context::to_shader_vertex(const point& aPoint, coordinate aZ) const
    {
        return xyz{{ aPoint.x, aPoint.y, aZ }};
    }
}