// opengl_rendering_context.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015-2026 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/game/animator.hpp>
#include <neogfx/hid/i_native_surface.hpp>
#include "../i_native_texture.hpp"
#include "../../text/native/i_native_font_face.hpp"
#include "opengl_rendering_context.hpp"

namespace neogfx
{
    namespace 
    {
        inline bool uv_swap_v(logical_coordinate_system lhs, logical_coordinate_system rhs)
        {
            switch (lhs)
            {
            case neogfx::logical_coordinate_system::AutomaticGame:
                if (rhs == neogfx::logical_coordinate_system::AutomaticGui)
                    return true;
                break;
            case neogfx::logical_coordinate_system::AutomaticGui:
                if (rhs == neogfx::logical_coordinate_system::AutomaticGame)
                    return true;
                break;
            }
            return false;
        }

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
            if (aLineLoop.empty())
                return {};

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

        inline game::vertices line_strip_to_lines(const game::vertices& aLineStrip)
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

        template <typename ColorContainer>
        inline vec4f to_function(i_rendering_context& aContext, ColorContainer const& aColor, rect const& aBoundingRect)
        {
            if (std::holds_alternative<gradient>(aColor))
            {
                auto const& value = static_variant_cast<const gradient&>(aColor).bounding_box();
                if (value != std::nullopt)
                    return (*value + aContext.origin()).as<float>().to_vec4();
            }
            return aBoundingRect.as<float>().to_vec4();
        }
    }

    opengl_rendering_context::opengl_rendering_context(const i_render_target& aTarget, neogfx::blending_mode aBlendingMode) :
        iRenderingEngine{ service<i_rendering_engine>() },
        iTarget{ aTarget }, 
        iWidget{ nullptr },
        iInFlush{ false },
        iSubpixelRendering{ rendering_engine().is_subpixel_rendering_on() },
        iSnapToPixel{ false },
        iSnapToPixelUsesOffset{ true },
        iUseDefaultShaderProgram{ *this, rendering_engine().default_shader_program() }
    {
        set_front_face(front_face());
        set_face_culling(face_culling());
        set_multisample(true);
        set_blending_mode(aBlendingMode);
        set_smoothing_mode(neogfx::smoothing_mode::AntiAlias);

        iSink += render_target().target_deactivating([&]() 
        { 
            flush(); 
        });
        iSink += render_target().target_activating([&]()
        {
            apply_state();
        });

        iTarget.begin_rendering();
    }

    opengl_rendering_context::opengl_rendering_context(const i_render_target& aTarget, const i_widget& aWidget, neogfx::blending_mode aBlendingMode) :
        iRenderingEngine{ service<i_rendering_engine>() },
        iTarget{ aTarget },
        iWidget{ &aWidget },
        iInFlush{ false },
        iSubpixelRendering{ rendering_engine().is_subpixel_rendering_on() },
        iSnapToPixel{ false },
        iSnapToPixelUsesOffset{ true },
        iUseDefaultShaderProgram{ *this, rendering_engine().default_shader_program() }
    {
        set_logical_coordinate_system(aWidget.logical_coordinate_system());
        set_front_face(front_face());
        set_face_culling(face_culling());
        set_multisample(true);
        set_blending_mode(aBlendingMode);
        set_smoothing_mode(neogfx::smoothing_mode::AntiAlias);

        iSink += render_target().target_deactivating([&]()
        {
            flush();
        });
        iSink += render_target().target_activating([&]()
        {
            apply_state();
        });

        iTarget.begin_rendering();
    }

    opengl_rendering_context::opengl_rendering_context(const opengl_rendering_context& aOther) :
        iRenderingEngine{ aOther.iRenderingEngine },
        iTarget{ aOther.iTarget },
        iWidget{ aOther.iWidget },
        iInFlush{ false },
        iSubpixelRendering{ aOther.iSubpixelRendering },
        iFastState{ aOther.iFastState },
        iSlowState{ aOther.iSlowState },
        iSnapToPixel{ false },
        iSnapToPixelUsesOffset{ true },
        iUseDefaultShaderProgram{ *this, rendering_engine().default_shader_program() }
    {
        set_front_face(aOther.front_face());
        set_face_culling(aOther.face_culling());
        set_multisample(aOther.multisample());
        set_blending_mode(aOther.blending_mode());
        set_smoothing_mode(aOther.smoothing_mode());

        iSink += render_target().target_deactivating([&]()
        {
            flush();
        });
        iSink += render_target().target_activating([&]()
        {
            apply_state();
        });

        iTarget.begin_rendering();
    }

    opengl_rendering_context::~opengl_rendering_context()
    {
        iTarget.end_rendering();
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
        if (iSlowState.logicalCoordinateSystem != std::nullopt)
            return *iSlowState.logicalCoordinateSystem;
        return render_target().logical_coordinate_system();
    }

    void opengl_rendering_context::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
    {
        iSlowState.logicalCoordinateSystem = aSystem;
        render_target().set_logical_coordinate_system(aSystem);
    }

    logical_coordinates opengl_rendering_context::logical_coordinates() const
    {
        if (iSlowState.logicalCoordinates != std::nullopt)
            return *iSlowState.logicalCoordinates;
        auto result = render_target().logical_coordinates();
        if (uv_swap_v(logical_coordinate_system(), render_target().logical_coordinate_system()))
            std::swap(result.bottomLeft.y, result.topRight.y);
        return result;
    }

    void opengl_rendering_context::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates)
    {
        iSlowState.logicalCoordinates = aCoordinates;
        render_target().set_logical_coordinates(aCoordinates);
    }

    point opengl_rendering_context::origin() const
    {
        return iFastState.origin;
    }

    void opengl_rendering_context::set_origin(const point& aOrigin)
    {
        iFastState.origin = aOrigin;
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

    void opengl_rendering_context::blit(const rect& aDestinationRect, const i_texture& aTexture, const rect& aSourceRect, neogfx::blending_mode aBlendingMode)
    {
        scoped_blending_mode sbm{ *this, aBlendingMode };
        auto shaderEffect = shader_effect::None;
        switch (aBlendingMode)
        {
        case neogfx::blending_mode::None:
            shaderEffect = shader_effect::Blit;
            break;
        case neogfx::blending_mode::FilterFinish:
            shaderEffect = shader_effect::MultiplyAlpha;
            break;
        }
        draw_texture(aDestinationRect, aTexture, aSourceRect, {}, shaderEffect);
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

    i_rendering_queue& opengl_rendering_context::queue() const
    {
        return iTarget.rendering_queue();
    }

    i_optimised_rendering_queue const& opengl_rendering_context::optimised_queue() const
    {
        return iTarget.optimised_rendering_queue();
    }

    void opengl_rendering_context::enqueue(const graphics_operation::operation& aOperation)
    {
        queue().push_back(aOperation);
    }

    inline bool batchable(queue_batch_item const& aLeft, queue_batch_item const& aRight, bool aStencilBasedInvalidation)
    {
        if (aLeft.fastState->clipRegion != aRight.fastState->clipRegion)
        {
            if (aLeft.fastState->clipRegion && aRight.fastState->clipRegion)
            {
                if (aLeft.fastState->clipRegion->intersects(*aRight.fastState->clipRegion))
                    return false;
                else if (!aStencilBasedInvalidation)
                    return false;
            }
            else
                return false;
        }
        return batchable(*aLeft, *aRight);
    }

    void opengl_rendering_context::flush()
    {
        if (iInFlush)
            return;

        neolib::scoped_flag sf{ iInFlush };

        auto const& optimisedQueue = optimised_queue();
        std::size_t const queueSize = optimisedQueue.size();

        if (queueSize == 0u)
            return;

        bool const stencilBasedInvalidation = service<i_rendering_engine>().is_stencil_based_invalidation_on();

        scoped_render_target srt{ render_target() };

        apply_state();

        thread_local std::vector<std::pair<graphics_operation::operation_type, std::size_t>> batches;
        batches.clear();

        for (auto batchStart = optimisedQueue.begin(); batchStart != optimisedQueue.end();)
        {
            auto batchEnd = std::next(batchStart);
            while (batchEnd != optimisedQueue.end() && batchable(*batchStart, *batchEnd, stencilBasedInvalidation))
                ++batchEnd;
            render_batch const opBatch{ &*batchStart, &*batchStart + (batchEnd - batchStart) };
            auto const opType = static_cast<graphics_operation::operation_type>((**opBatch.cbegin()).index());
            batches.emplace_back(opType, std::distance(batchStart, batchEnd));
            batchStart = batchEnd;
            switch (opType)
            {
            case graphics_operation::SetViewport:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    auto const& setViewport = static_variant_cast<const graphics_operation::set_viewport&>(**op);
                    if (setViewport.viewport)
                        render_target().set_viewport(setViewport.viewport.value().as<std::int32_t>());
                    else
                        render_target().set_viewport(rect{ render_target().target_origin(), render_target().extents() }.as<std::int32_t>());
                }
                break;
            case graphics_operation::SnapToPixelOn:
                set_snap_to_pixel(true);
                break;
            case graphics_operation::SnapToPixelOff:
                set_snap_to_pixel(false);
                break;
            case graphics_operation::PushLogicalOperation:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    push_logical_operation(static_variant_cast<const graphics_operation::push_logical_operation&>(**op).logicalOperation);
                break;
            case graphics_operation::PopLogicalOperation:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    (void)op;
                    pop_logical_operation();
                }
                break;
            case graphics_operation::LineStippleOn:
                {
                    auto const& lso = static_variant_cast<const graphics_operation::line_stipple_on&>(**(std::prev(opBatch.cend())));
                    line_stipple_on(lso.stipple);
                }
                break;
            case graphics_operation::LineStippleOff:
                line_stipple_off();
                break;
            case graphics_operation::SubpixelRenderingOn:
                subpixel_rendering_on();
                break;
            case graphics_operation::SubpixelRenderingOff:
                subpixel_rendering_off();
                break;
            case graphics_operation::Clear:
                clear(static_variant_cast<const graphics_operation::clear&>(**(std::prev(opBatch.cend()))).color);
                break;
            case graphics_operation::ClearDepthBuffer:
                clear_depth_buffer();
                break;
            case graphics_operation::ClearStencilBuffer:
                clear_stencil_buffer();
                break;
            case graphics_operation::EnableStencilTest:
                enable_stencil_test();
                break;
            case graphics_operation::DisableStencilTest:
                disable_stencil_test();
                break;
            case graphics_operation::EnableStencilUpdate:
                enable_stencil_update(static_variant_cast<const graphics_operation::enable_stencil_update&>(**(std::prev(opBatch.cend()))).ref);
                break;
            case graphics_operation::DisableStencilUpdate:
                disable_stencil_update();
                break;
            case graphics_operation::SetGradient:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    set_gradient(static_variant_cast<const graphics_operation::set_gradient&>(**op).gradient);
                break;
            case graphics_operation::ClearGradient:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                    clear_gradient();
                break;
            case graphics_operation::SetPixel:
                set_pixel(opBatch);
                break;
            case graphics_operation::Blit:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    update_state(*op);
                    auto const& args = static_variant_cast<const graphics_operation::blit&>(**op);
                    blit(args.destinationRect, *args.texture, args.sourceRect, args.blendingMode);
                }
                break;
            case graphics_operation::DrawPixel:
                draw_pixels(opBatch);
                break;
            case graphics_operation::DrawLine:
                draw_lines(opBatch);
                break;
            case graphics_operation::DrawTriangle:
                draw_triangles(opBatch);
                break;
            case graphics_operation::DrawRect:
                draw_rects(opBatch);
                break;
            case graphics_operation::DrawRoundedRect:
                draw_rounded_rects(opBatch);
                break;
            case graphics_operation::DrawEllipseRect:
                draw_ellipse_rects(opBatch);
                break;
            case graphics_operation::DrawCheckerboard:
                draw_checkerboards(opBatch);
                break;
            case graphics_operation::DrawCircle:
                draw_circles(opBatch);
                break;
            case graphics_operation::DrawEllipse:
                draw_ellipses(opBatch);
                break;
            case graphics_operation::DrawPie:
                draw_pies(opBatch);
                break;
            case graphics_operation::DrawArc:
                draw_arcs(opBatch);
                break;
            case graphics_operation::DrawCubicBezier:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    update_state(*op);
                    auto const& args = static_variant_cast<const graphics_operation::draw_cubic_bezier&>(**op);
                    draw_cubic_bezier(args.p0, args.p1, args.p2, args.p3, args.pen);
                }
                break;
            case graphics_operation::DrawPath:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    update_state(*op);
                    auto const& args = static_variant_cast<const graphics_operation::draw_path&>(**op);
                    draw_path(args.path, args.shape, args.boundingRect, args.pen, args.fill);
                }
                break;
            case graphics_operation::DrawShape:
                draw_shapes(opBatch);
                break;
            case graphics_operation::DrawEntities:
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    update_state(*op);
                    auto const& args = static_variant_cast<const graphics_operation::draw_entities&>(**op);
                    draw_entities(*args.ecs, args.layer, args.transformation);
                }
                break;
            case graphics_operation::DrawGlyph:
                draw_glyphs(opBatch);
                break;
            case graphics_operation::DrawMesh:
                // todo: use draw_meshes
                for (auto op = opBatch.cbegin(); op != opBatch.cend(); ++op)
                {
                    update_state(*op);
                    auto const& args = static_variant_cast<const graphics_operation::draw_mesh&>(**op);
                    draw_mesh(args.mesh, args.material, args.transformation, args.filter);
                }
                break;
            }
        }

        if (iRenderingEngine.is_rendering_queue_optimization_on())
        {
#if 0
            for (auto const& batch : batches)
                std::cerr << "[" << to_string(batch.first) << ":" << batch.second << "]";
            std::cerr << std::endl;
#endif
        }
    }

    bool opengl_rendering_context::redirecting() const
    {
        throw std::logic_error("not yet implemented");
    }

    point opengl_rendering_context::redirect_origin() const
    {
        throw std::logic_error("not yet implemented");
    }

    void opengl_rendering_context::begin_redirect(i_rendering_context& aRcBase, point const& aOrigin)
    {
        throw std::logic_error("not yet implemented");
    }

    void opengl_rendering_context::end_redirect()
    {
        throw std::logic_error("not yet implemented");
    }

    void opengl_rendering_context::update_state(queue_batch_item const& aQbi)
    {
        if (iFastState.generation != aQbi.fastState->generation)
        {
            iFastState = *aQbi.fastState;
            invalidate_fast_state();
        }

        if (iSlowState.generation != aQbi.slowState->generation)
        {
            iSlowState.generation = aQbi.slowState->generation;
            if (aQbi.slowState->logicalCoordinateSystem) iSlowState.logicalCoordinateSystem = aQbi.slowState->logicalCoordinateSystem;
            if (aQbi.slowState->logicalCoordinates) iSlowState.logicalCoordinates = aQbi.slowState->logicalCoordinates;
            if (aQbi.slowState->multisample) iSlowState.multisample = aQbi.slowState->multisample;
            if (aQbi.slowState->sampleShadingRate) iSlowState.sampleShadingRate = aQbi.slowState->sampleShadingRate;
            if (aQbi.slowState->frontFace) iSlowState.frontFace = aQbi.slowState->frontFace;
            if (aQbi.slowState->faceCulling) iSlowState.faceCulling = aQbi.slowState->faceCulling;
            if (aQbi.slowState->blendingMode) iSlowState.blendingMode = aQbi.slowState->blendingMode;
            if (aQbi.slowState->smoothingMode) iSlowState.smoothingMode = aQbi.slowState->smoothingMode;
            invalidate_slow_state();
        }

        apply_state();
    }

    void opengl_rendering_context::apply_state()
    {
        neolib::scoped_flag sf{ iApplyingState };

        if (fast_state_invalid())
        {
            apply_scissor();
            apply_stencil();

            validate_fast_state();
        }

        if (slow_state_invalid())
        {
            set_multisample(multisample());
            set_front_face(front_face());
            set_face_culling(face_culling());
            set_blending_mode(blending_mode());
            set_smoothing_mode(smoothing_mode());

            validate_slow_state();
        }
    }

    bool opengl_rendering_context::fast_state_invalid() const
    {
        return shared_fast_state_generation().load() != iSharedFastStateGeneration;
    }

    bool opengl_rendering_context::slow_state_invalid() const
    {
        return shared_slow_state_generation().load() != iSharedSlowStateGeneration;
    }

    void opengl_rendering_context::invalidate_fast_state()
    {
        ++shared_fast_state_generation();
    }

    void opengl_rendering_context::invalidate_slow_state()
    {
        ++shared_slow_state_generation();
    }

    void opengl_rendering_context::validate_fast_state()
    {
        iSharedFastStateGeneration = shared_fast_state_generation().load();
    }

    void opengl_rendering_context::validate_slow_state()
    {
        iSharedSlowStateGeneration = shared_slow_state_generation().load();
    }

    void opengl_rendering_context::scissor_on()
    {
        glCheck((applying_scissor() || iFastState.scissorCounter >= 0 ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST)));
    }
    
    void opengl_rendering_context::scissor_off()
    {
        glCheck((applying_scissor() || iFastState.scissorCounter < 0 ? glDisable(GL_SCISSOR_TEST) : glEnable(GL_SCISSOR_TEST)));
    }

    std::optional<rect> const& opengl_rendering_context::scissor_rect() const
    {
        return iFastState.clipRegion;
    }

    bool opengl_rendering_context::applying_scissor() const
    {
        return iApplyingScissor;
    }

    void opengl_rendering_context::apply_scissor()
    {
        apply_scissor(scissor_rect());
    }

    void opengl_rendering_context::apply_scissor(std::optional<rect> const& aScissorRect)
    {
        neolib::scoped_flag sf{ iApplyingScissor };
        if (aScissorRect)
        {
            auto const& sr = *aScissorRect;
            GLint x = static_cast<GLint>(std::ceil(sr.x));
            GLint y = static_cast<GLint>(std::ceil(sr.y));
            GLsizei cx = static_cast<GLsizei>(std::ceil(sr.cx));
            GLsizei cy = static_cast<GLsizei>(std::ceil(sr.cy));
            glCheck(glScissor(x, y, cx, cy));
            scissor_on();
        }
        else
            scissor_off();
    }

    bool opengl_rendering_context::multisample() const
    {
        return iSlowState.multisample.value_or(true);
    }
    
    void opengl_rendering_context::set_multisample(bool aMultisample)
    {
        if (iSlowState.multisample != aMultisample || slow_state_invalid())
        {
            iSlowState.multisample = aMultisample;
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
        if (iSlowState.sampleShadingRate != aSampleShadingRate)
        {
            iSlowState.sampleShadingRate = aSampleShadingRate;
            glCheck(glEnable(GL_SAMPLE_SHADING));
            glCheck(glMinSampleShading(static_cast<float>(iSlowState.sampleShadingRate.value())));
        }
    }

    void opengl_rendering_context::disable_sample_shading()
    {
        if (iSlowState.sampleShadingRate != std::nullopt)
        {
            iSlowState.sampleShadingRate = std::nullopt;
            glCheck(glDisable(GL_SAMPLE_SHADING));
        }
    }

    front_face opengl_rendering_context::front_face() const
    {
        return iSlowState.frontFace.value_or(neogfx::front_face::CCW);
    }

    void opengl_rendering_context::set_front_face(neogfx::front_face aFrontFace)
    {
        if (iSlowState.frontFace != aFrontFace || slow_state_invalid())
        {
            iSlowState.frontFace = aFrontFace;
            switch (iSlowState.frontFace.value())
            {
            case neogfx::front_face::CCW:
                glCheck(glFrontFace(GL_CCW));
                break;
            case neogfx::front_face::CW:
                glCheck(glFrontFace(GL_CW));
                break;
            }
        }
    }

    face_culling opengl_rendering_context::face_culling() const
    {
        return iSlowState.faceCulling.value_or(face_culling::None);
    }

    void opengl_rendering_context::set_face_culling(neogfx::face_culling aCulling)
    {
        if (iSlowState.faceCulling != aCulling || slow_state_invalid())
        {
            iSlowState.faceCulling = aCulling;
            switch (iSlowState.faceCulling.value())
            {
            case neogfx::face_culling::None:
                glCheck(glDisable(GL_CULL_FACE));
                break;
            case neogfx::face_culling::Front:
                glCheck(glCullFace(GL_FRONT));
                glCheck(glEnable(GL_CULL_FACE));
                break;
            case neogfx::face_culling::Back:
                glCheck(glCullFace(GL_BACK));
                glCheck(glEnable(GL_CULL_FACE));
                break;
            case neogfx::face_culling::FrontAndBack:
                glCheck(glCullFace(GL_FRONT_AND_BACK));
                glCheck(glEnable(GL_CULL_FACE));
                break;
            }
        }
    }

    void opengl_rendering_context::set_opacity(double aOpacity)
    {
        iFastState.opacity = aOpacity;
    }

    neogfx::blending_mode opengl_rendering_context::blending_mode() const
    {
        return *iSlowState.blendingMode;
    }

    void opengl_rendering_context::set_blending_mode(neogfx::blending_mode aBlendingMode)
    {
        if (iSlowState.blendingMode != aBlendingMode || slow_state_invalid())
        {
            iSlowState.blendingMode = aBlendingMode;
            switch (*iSlowState.blendingMode)
            {
            case neogfx::blending_mode::None:
                glCheck(glDisable(GL_BLEND));
                break;
            case neogfx::blending_mode::Default:
                glCheck(glEnable(GL_BLEND));
                glCheck(glBlendEquation(GL_FUNC_ADD));
                glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                break;
            case neogfx::blending_mode::Sprite:
                glCheck(glEnable(GL_BLEND));
                glCheck(glBlendEquation(GL_FUNC_ADD));
                glCheck(glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                break;
            case neogfx::blending_mode::Blit:
                glCheck(glEnable(GL_BLEND));
                glCheck(glBlendEquation(GL_FUNC_ADD));
                glCheck(glBlendFunc(GL_ONE, GL_ZERO));
                break;
            case neogfx::blending_mode::Filter:
                glCheck(glEnable(GL_BLEND));
                glCheck(glBlendEquation(GL_FUNC_ADD));
                glCheck(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
                break;
            case neogfx::blending_mode::FilterFinish:
                glCheck(glEnable(GL_BLEND));
                glCheck(glBlendEquation(GL_FUNC_ADD));
                glCheck(glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                break;
            }
        }
    }

    smoothing_mode opengl_rendering_context::smoothing_mode() const
    {
        return *iSlowState.smoothingMode;
    }

    void opengl_rendering_context::set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode)
    {
        if (iSlowState.smoothingMode != aSmoothingMode || slow_state_invalid())
        {
            iSlowState.smoothingMode = aSmoothingMode;
            if (*iSlowState.smoothingMode == neogfx::smoothing_mode::AntiAlias)
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
        auto const currentBlendingMode = iSlowState.blendingMode;

        if (!logical_operation_active())
        {
            iSlowState.blendingMode = std::nullopt;
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
                iSlowState.blendingMode = std::nullopt;
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
        glCheck(glStencilMask(0xFF));
        glCheck(glClearStencil(0));
        glCheck(glClear(GL_STENCIL_BUFFER_BIT));
    }

    void opengl_rendering_context::fill_stencil_buffer()
    {
        if (!iStencilRef.has_value())
            throw std::logic_error("neogfx::opengl_rendering_context: fill_stencil_buffer called without active stencil ref");
        glCheck(glStencilMask(0xFF));
        glCheck(glClearStencil(static_cast<GLint>(*iStencilRef)));
        glCheck(glClear(GL_STENCIL_BUFFER_BIT));
    }

    void opengl_rendering_context::enable_stencil_test()
    {
        iStencilEnabled = true;
        apply_stencil();
    }

    void opengl_rendering_context::disable_stencil_test()
    {
        iStencilEnabled = false;
        apply_stencil();
    }

    void opengl_rendering_context::enable_stencil_update(std::int32_t aRef)
    {
        if (iUpdatingStencil)
            throw std::logic_error("neogfx::opengl_rendering_context: enable_stencil_update called without matching disable_stencil_update");
        iUpdatingStencil = true;
        iStencilRef = aRef;
        apply_stencil();
    }

    void opengl_rendering_context::disable_stencil_update()
    {
        if (!iUpdatingStencil)
            throw std::logic_error("neogfx::opengl_rendering_context: disable_stencil_update called without matching enable_stencil_update");
        iUpdatingStencil = false;
        apply_stencil();
    }

    void opengl_rendering_context::apply_stencil()
    {
        if (iStencilEnabled)
        {
            glCheck(glEnable(GL_STENCIL_TEST));
            if (iUpdatingStencil)
            {
                glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
                glCheck(glDepthMask(GL_FALSE));
                glCheck(glStencilFunc(GL_ALWAYS, iStencilRef.value_or(1), 0xFF));
                glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
                glCheck(glStencilMask(0xFF));
            }
            else
            {
                glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
                glCheck(glDepthMask(GL_TRUE));
                glCheck(glStencilFunc(GL_EQUAL, iStencilRef.value_or(1), 0xFF));
                glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
                glCheck(glStencilMask(0x00));
            }
        }
        else
        {
            glCheck(glDisable(GL_STENCIL_TEST));
        }
    }

    void opengl_rendering_context::set_pixel(const render_batch& aSetPixelOps)
    {
        /* todo: faster alternative to this... */
        disable_anti_alias daa{ *this };
        for (auto const& op : aSetPixelOps)
        {
            auto& drawOp = static_variant_cast<const graphics_operation::set_pixel&>(*op);
            draw_pixel(drawOp.point, drawOp.color.with_alpha(1.0));
        }
    }

    void opengl_rendering_context::draw_pixel(const point& aPoint, const color& aColor)
    {
        graphics_operation::operation const op{ graphics_operation::draw_pixel{ aPoint, aColor } };
        queue_batch_item const qbi{ &op, 1, &iFastState, &iSlowState };
        draw_pixels(render_batch{ &qbi, &qbi + 1 });
    }

    void opengl_rendering_context::draw_pixels(const render_batch& aDrawPixelOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };
        scoped_anti_alias saa{ *this, smoothing_mode::None };
        disable_multisample disableMultisample{ *this };

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u * (aDrawPixelOps.cend() - aDrawPixelOps.cbegin()))};

            for (auto op = aDrawPixelOps.cbegin(); op != aDrawPixelOps.cend(); ++op)
            {
                update_state(*op);

                auto& drawOp = static_variant_cast<const graphics_operation::draw_pixel&>(**op);
                auto rectVertices = rect_vertices<vec3f>(rect{ drawOp.point + origin(), size{1.0, 1.0}}, mesh_type::Triangles);
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
        graphics_operation::operation const op{ graphics_operation::draw_line{ aFrom, aTo, aPen } };
        queue_batch_item const qbi{ &op, 1, &iFastState, &iSlowState };
        draw_lines(render_batch{ &qbi, &qbi + 1 });
    }

    void opengl_rendering_context::draw_lines(const render_batch& aDrawLineOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Line);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u * (aDrawLineOps.cend() - aDrawLineOps.cbegin())) };

            for (auto op = aDrawLineOps.cbegin(); op != aDrawLineOps.cend(); ++op)
            {
                update_state(*op);

                auto& drawOp = static_variant_cast<const graphics_operation::draw_line&>(**op);
                auto const& adjust = (drawOp.pen.anti_aliased() || static_cast<std::int32_t>(drawOp.pen.width()) % 2 == 0 ? point{} : point{0.5, 0.5});
                auto const& from = drawOp.from + origin() + adjust;
                auto const& to = drawOp.to + origin() + adjust;
                auto boundingRect = rect{ from.min(to), from.max(to) }.inflated(drawOp.pen.width());
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(*this, drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ from.x, from.y, to.x, to.y }.as<float>(),
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

    void opengl_rendering_context::draw_triangles(const render_batch& aDrawTriangleOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_triangle&>(**aDrawTriangleOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Triangle);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u * (aDrawTriangleOps.cend() - aDrawTriangleOps.cbegin()))};

            for (auto op = aDrawTriangleOps.cbegin(); op != aDrawTriangleOps.cend(); ++op)
            {
                update_state(*op);

                auto& drawOp = static_variant_cast<const graphics_operation::draw_triangle&>(**op);
                auto const& p0 = drawOp.p0 + origin();
                auto const& p1 = drawOp.p1 + origin();
                auto const& p2 = drawOp.p2 + origin();
                auto boundingRect = rect{ p0.min(p1.min(p2)), p0.max(p1.max(p2)) }.inflated(drawOp.pen.width());
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(*this, drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ p0.x, p0.y, p1.x, p1.y }.as<float>(),
                        vec4{ p2.x, p2.y }.as<float>(),
                        vec4{
                            drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                            !logical_operation_active() ?
                                drawOp.pen.anti_aliased() ? 
                                    0.5 : 0.0 :
                                0.0,
                            0.0,
                            drawOp.pen.width() }.as<float>(),
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).with_combined_alpha(iFastState.opacity).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).with_combined_alpha(iFastState.opacity).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_rects(const render_batch& aDrawRectOps)
    {
        std::optional<use_shader_program> usp;
        std::optional<neolib::scoped_flag> snap;

        {
            std::optional<use_vertex_arrays> maybeVertexArrays;

            for (auto op = aDrawRectOps.cbegin(); op != aDrawRectOps.cend(); ++op)
            {
                update_state(*op);

                auto& drawOp = static_variant_cast<const graphics_operation::draw_rect&>(**op);
                auto const& rc = drawOp.rect + origin();

                if (rc.top_left().z == 0.0 && rc.bottom_right().z == 0.0 &&
                    ((std::holds_alternative<color>(drawOp.fill) && 
                        static_variant_cast<color>(drawOp.fill).alpha() == 0xFF) ||
                        std::holds_alternative<std::monostate>(drawOp.fill)))
                {
                    bool optimise = false;
                    if (!logical_operation_active() && iFastState.opacity == 1.0 && (!iStencilEnabled || iUpdatingStencil))
                    {
                        auto const penWidth = drawOp.pen.width();
                        if (penWidth == 0.0)
                            optimise = true;
                        else if (scissor_rect() != std::nullopt)
                        {
                            auto const& tl = scissor_rect().value().top_left() - rc.top_left();
                            auto const& br = rc.bottom_right() - scissor_rect().value().bottom_right();
                            if (tl.x > penWidth && tl.y > penWidth && br.x > penWidth && br.y > penWidth)
                                optimise = true;
                        }
                    }
                    if (optimise)
                    {
                        if (std::holds_alternative<color>(drawOp.fill))
                        {
                            point const textureMargin{ 1.0, 1.0 };
                            auto const& fixedRc = rc.with_y(
                                logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGame ?
                                    rc.y : 
                                    rendering_area(false).cy - rc.cy - rc.y) +
                                        (render_target().target_type() == render_target_type::Texture ?
                                            textureMargin : point{});
                            apply_scissor(fixedRc.intersection(scissor_rect() ? *scissor_rect() : fixedRc));
                            clear(static_variant_cast<color>(drawOp.fill));
                            if (iUpdatingStencil)
                                fill_stencil_buffer();
                            apply_scissor();
                        }
                        continue;
                    }
                }

                if (usp == std::nullopt)
                {
                    usp.emplace(*this, rendering_engine().default_shader_program(), iFastState.opacity);
                    snap.emplace(iSnapToPixelUsesOffset, false);

                    auto& firstOp = static_variant_cast<const graphics_operation::draw_rect&>(**aDrawRectOps.cbegin());

                    if (std::holds_alternative<gradient>(firstOp.fill))
                        rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
                    else if (std::holds_alternative<gradient>(firstOp.pen.color()))
                        rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

                    rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Rect);

                    maybeVertexArrays.emplace(as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u * (aDrawRectOps.cend() - aDrawRectOps.cbegin())));
                }

                auto& vertexArrays = maybeVertexArrays.value();

                auto const sdfRect = snap_to_pixel() ? rc.deflated(drawOp.pen.width() / 2.0) : rc;
                auto const boundingRect = rc.inflated(drawOp.pen.width() / 2.0);
                auto const vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(*this, drawOp.fill, boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ sdfRect.center().x, sdfRect.center().y, sdfRect.width(), sdfRect.height() }.as<float>(),
                        vec4f{},
                        vec4{
                            drawOp.pen.width() ? drawOp.pen.secondary_color().has_value() ? 2.0 : 1.0 : 0.0,
                            !logical_operation_active() && !snap_to_pixel() ?
                                drawOp.pen.anti_aliased() ? 
                                    0.5 : 0.0 :
                                0.0,
                            0.0,
                            drawOp.pen.width() }.as<float>(),
                        std::holds_alternative<color>(drawOp.pen.color()) ?
                            static_variant_cast<color>(drawOp.pen.color()).with_combined_alpha(iFastState.opacity).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).with_combined_alpha(iFastState.opacity).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_rounded_rects(const render_batch& aDrawRoundedRectOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixelUsesOffset, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_rounded_rect&>(**aDrawRoundedRectOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::RoundedRect);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u * (aDrawRoundedRectOps.cend() - aDrawRoundedRectOps.cbegin()))};

            for (auto op = aDrawRoundedRectOps.cbegin(); op != aDrawRoundedRectOps.cend(); ++op)
            {
                update_state(*op);

                auto& drawOp = static_variant_cast<const graphics_operation::draw_rounded_rect&>(**op);
                auto const& rc = drawOp.rect + origin();
                auto const sdfRect = snap_to_pixel() ? rc.deflated(drawOp.pen.width() / 2.0) : rc;
                auto const boundingRect = rc.inflated(drawOp.pen.width() / 2.0);
                auto const vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(*this, drawOp.fill, boundingRect);

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
                            static_variant_cast<color>(drawOp.pen.color()).with_combined_alpha(iFastState.opacity).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).with_combined_alpha(iFastState.opacity).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_ellipse_rects(const render_batch& aDrawEllpseRectOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixelUsesOffset, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_ellipse_rect&>(**aDrawEllpseRectOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::EllipseRect);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u * (aDrawEllpseRectOps.cend() - aDrawEllpseRectOps.cbegin())) };

            for (auto op = aDrawEllpseRectOps.cbegin(); op != aDrawEllpseRectOps.cend(); ++op)
            {
                update_state(*op);

                auto& drawOp = static_variant_cast<const graphics_operation::draw_ellipse_rect&>(**op);
                auto const& rc = drawOp.rect + origin();
                auto const sdfRect = snap_to_pixel() ? rc.deflated(drawOp.pen.width() / 2.0) : rc;
                auto const boundingRect = rc.inflated(drawOp.pen.width() / 2.0);
                auto const vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(*this, drawOp.fill, boundingRect);

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
                            static_variant_cast<color>(drawOp.pen.color()).with_combined_alpha(iFastState.opacity).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).with_combined_alpha(iFastState.opacity).as<float>(),
                        drawOp.radiusY.as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_checkerboards(const render_batch& aDrawCheckerboardOps)
    {
        std::optional<use_shader_program> usp;
        std::optional<neolib::scoped_flag> snap;

        {
            std::optional<use_vertex_arrays> maybeVertexArrays;

            for (auto op = aDrawCheckerboardOps.cbegin(); op != aDrawCheckerboardOps.cend(); ++op)
            {
                update_state(*op);

                for (std::uint32_t pass = 0u; pass <= 1u; ++pass)
                {
                    usp.emplace(*this, rendering_engine().default_shader_program(), iFastState.opacity);
                    snap.emplace(iSnapToPixelUsesOffset, false);

                    auto& drawOp = static_variant_cast<const graphics_operation::draw_checkerboard&>(**op);
                    auto const& rc = drawOp.rect + origin();
                    auto& fill = (pass == 0u ? drawOp.fill1 : drawOp.fill2);

                    if (std::holds_alternative<gradient>(fill))
                        rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(fill));
                    else if (std::holds_alternative<gradient>(drawOp.pen.color()))
                        rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(drawOp.pen.color()));

                    rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Checkerboard);

                    maybeVertexArrays.emplace(as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u));

                    auto& vertexArrays = maybeVertexArrays.value();

                    auto const sdfRect = snap_to_pixel() ? rc.deflated(drawOp.pen.width() / 2.0) : rc;
                    auto const boundingRect = rc.inflated(drawOp.pen.width() / 2.0);
                    auto const vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                    auto const function = to_function(*this, fill, boundingRect);

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
                                static_variant_cast<color>(drawOp.pen.color()).with_combined_alpha(iFastState.opacity).as<float>() :
                                vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                            drawOp.pen.secondary_color().value_or(vec4{}).with_combined_alpha(iFastState.opacity).as<float>() });

                    maybeVertexArrays = std::nullopt;
                    usp = std::nullopt;
                }
            }
        }
    }

    void opengl_rendering_context::draw_ellipses(const render_batch& aDrawEllipseOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_ellipse&>(**aDrawEllipseOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Ellipse);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u * (aDrawEllipseOps.cend() - aDrawEllipseOps.cbegin()))};

            for (auto op = aDrawEllipseOps.cbegin(); op != aDrawEllipseOps.cend(); ++op)
            {
                update_state(*op);

                auto& drawOp = static_variant_cast<const graphics_operation::draw_ellipse&>(**op);
                auto const& center = drawOp.center + origin();
                auto boundingRect = rect{ center - point{ std::max(drawOp.radiusA, drawOp.radiusB), std::max(drawOp.radiusA, drawOp.radiusB) }, size{ std::max(drawOp.radiusA, drawOp.radiusB) * 2.0 } }.inflated(drawOp.pen.width() / 2.0);
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(*this, drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ center.x, center.y, drawOp.radiusA, drawOp.radiusB }.as<float>(),
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
                            static_variant_cast<color>(drawOp.pen.color()).with_combined_alpha(iFastState.opacity).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).with_combined_alpha(iFastState.opacity).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_circles(const render_batch& aDrawCircleOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_circle&>(**aDrawCircleOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Circle);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u * (aDrawCircleOps.cend() - aDrawCircleOps.cbegin()))};

            for (auto op = aDrawCircleOps.cbegin(); op != aDrawCircleOps.cend(); ++op)
            {
                update_state(*op);

                auto& drawOp = static_variant_cast<const graphics_operation::draw_circle&>(**op);
                auto const& center = drawOp.center + origin();
                auto boundingRect = rect{ center - point{ drawOp.radius, drawOp.radius }, size{ drawOp.radius * 2.0 } }.inflated(drawOp.pen.width() / 2.0);
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(*this, drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ center.x, center.y, drawOp.radius }.as<float>(),
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
                            static_variant_cast<color>(drawOp.pen.color()).with_combined_alpha(iFastState.opacity).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).with_combined_alpha(iFastState.opacity).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_pies(const render_batch& aDrawPieOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_pie&>(**aDrawPieOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Pie);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u * (aDrawPieOps.cend() - aDrawPieOps.cbegin()))};

            for (auto op = aDrawPieOps.cbegin(); op != aDrawPieOps.cend(); ++op)
            {
                update_state(*op);

                auto& drawOp = static_variant_cast<const graphics_operation::draw_pie&>(**op);
                auto const& center = drawOp.center + origin();
                auto boundingRect = rect{ center - point{ drawOp.radius, drawOp.radius }, size{ drawOp.radius * 2.0 } }.inflated(drawOp.pen.width() / 2.0);
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(*this, drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ center.x, center.y, drawOp.radius, drawOp.startAngle }.as<float>(),
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
                            static_variant_cast<color>(drawOp.pen.color()).with_combined_alpha(iFastState.opacity).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).with_combined_alpha(iFastState.opacity).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_arcs(const render_batch& aDrawArcOps)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        auto& firstOp = static_variant_cast<const graphics_operation::draw_arc&>(**aDrawArcOps.cbegin());

        if (std::holds_alternative<gradient>(firstOp.fill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.fill));
        else if (std::holds_alternative<gradient>(firstOp.pen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(firstOp.pen.color()));

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Arc);

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u * (aDrawArcOps.cend() - aDrawArcOps.cbegin()))};

            for (auto op = aDrawArcOps.cbegin(); op != aDrawArcOps.cend(); ++op)
            {
                update_state(*op);

                auto& drawOp = static_variant_cast<const graphics_operation::draw_arc&>(**op);
                auto const& center = drawOp.center + origin();
                auto boundingRect = rect{ center - point{ drawOp.radius, drawOp.radius }, size{ drawOp.radius * 2.0 } }.inflated(drawOp.pen.width() / 2.0);
                auto vertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                auto const function = to_function(*this, drawOp.pen.color(), boundingRect);

                for (auto const& v : vertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(drawOp.fill) ?
                            static_variant_cast<color>(drawOp.fill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.fill) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ center.x, center.y, drawOp.radius, drawOp.startAngle }.as<float>(),
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
                            static_variant_cast<color>(drawOp.pen.color()).with_combined_alpha(iFastState.opacity).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(drawOp.pen.color()) ? 0.0f : 1.0f },
                        drawOp.pen.secondary_color().value_or(vec4{}).with_combined_alpha(iFastState.opacity).as<float>() });
            }
        }
    }

    void opengl_rendering_context::draw_cubic_bezier(const point& aP0, const point& aP1, const point& aP2, const point& aP3, const pen& aPen)
    {
        if (!aPen.width())
            return;

        auto const p0 = aP0 + origin();
        auto const p1 = aP1 + origin();
        auto const p2 = aP2 + origin();
        auto const p3 = aP3 + origin();

        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::CubicBezier);

        if (std::holds_alternative<gradient>(aPen.color()))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aPen.color()));

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u) };

            auto boundingRect = rect{ p0.min(p1.min(p2.min(p3))), p0.max(p1.max(p2.max(p3))) }.inflated(aPen.width());
            auto const function = to_function(*this, aPen.color(), boundingRect);
            auto rectVertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
            if (aPen.width())
                for (auto const& v : rectVertices)
                    vertexArrays.push_back({ v,
                        std::holds_alternative<color>(aPen.color()) ?
                            static_variant_cast<color>(aPen.color()).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(aPen.color()) ? 0.0f : 1.0f },
                        {},
                        function,
                        vec4{ p0.x, p0.y, p1.x, p1.y }.as<float>(),
                        vec4{ p2.x, p2.y, p3.x, p3.y }.as<float>(),
                        vec4{ 0.0, 0.0, 0.0, aPen.width() }.as<float>() });
        }
    }

    void opengl_rendering_context::draw_path(const ssbo_range& aPath, path_shape aPathShape, const rect aBoundingRect, const pen& aPen, const brush& aFill)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixelUsesOffset, false };

        switch (aPathShape)
        {
        case path_shape::Polygon:
            {
                if (std::holds_alternative<gradient>(aFill))
                    rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill));
                else if (std::holds_alternative<gradient>(aPen.color()))
                    rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aPen.color()));

                rendering_engine().default_shader_program().shape_shader().set_shape(shader_shape::Polygon);

                {
                    use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, static_cast<std::size_t>(2u * 3u) };

                    auto boundingRect = aBoundingRect.inflated(aPen.width() * 2.0) + origin();
                    auto boundingRectVertices = rect_vertices<vec3f>(boundingRect, mesh_type::Triangles);
                    auto const function = to_function(*this, aPen.color(), boundingRect);

                    for (auto const& v : boundingRectVertices)
                        vertexArrays.push_back({ v,
                            std::holds_alternative<color>(aFill) ?
                                static_variant_cast<color>(aFill).as<float>() :
                                vec4f{ 0.0f, 0.0f, 0.0f, std::holds_alternative<std::monostate>(aFill) ? 0.0f : 1.0f },
                            {},
                            function,
                            vec4u32{ aPath.first, aPath.last }.as<float>(),
                            vec4{ origin().to_vec3() }.as<float>(),
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
                            aPen.secondary_color().value_or(vec4{}).with_combined_alpha(iFastState.opacity).as<float>() });
                }
            }
            break;
        default:
            throw std::logic_error("opengl_rendering_context::draw_path: path shape not yet implemented");
        }
    }

    void opengl_rendering_context::draw_shapes(const render_batch& aDrawShapeOps)
    {
        for (auto const& op : aDrawShapeOps)
        {
            update_state(op);
            auto const& shapeOp = static_variant_cast<const graphics_operation::draw_shape&>(*op);
            fill_shape(shapeOp.mesh, shapeOp.position, shapeOp.fill);
            draw_shape(shapeOp.mesh, shapeOp.position, shapeOp.pen);
        }
    }
        
    void opengl_rendering_context::draw_shape(const game::mesh& aMesh, const vec3& aPosition, const pen& aPen)
    {
        if (!aPen.width())
            return;

        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

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

        use_vertex_arrays vertexArrays{ as_vertex_provider(), *this, triangles.size() };

        auto const function = to_function(*this, aPen.color(), bounding_rect(aMesh) + origin());

        auto const& pos = aPosition.as<float>() + origin().to_vec3().as<float>();

        for (auto const& v : triangles)
            vertexArrays.push_back({ v + pos,
                std::holds_alternative<color>(aPen.color()) ? 
                    static_variant_cast<color>(aPen.color()).as<float>() : 
                    vec4f{ 0.0f, 0.0f, 0.0f, 1.0f },
                {},
                function });
    }

    void opengl_rendering_context::fill_shape(const game::mesh& aMesh, const vec3& aPosition, const brush& aFill)
    {
        if (std::holds_alternative<std::monostate>(aFill))
            return;

        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        if (std::holds_alternative<gradient>(aFill))
            rendering_engine().default_shader_program().gradient_shader().set_gradient(*this, static_variant_cast<const gradient&>(aFill));

        {
            use_vertex_arrays vertexArrays{ as_vertex_provider(), *this };

            auto const& pos = (aPosition + origin().to_vec3()).as<float>();
            auto const& vertices = aMesh.vertices;
            auto const& uv = aMesh.uv;
            vec3f min, max;
            if (std::holds_alternative<gradient>(aFill))
            {
                min = vertices[0].xyz;
                max = min;
                for (auto const& v : vertices)
                {
                    min.x = std::min(min.x, v.x + pos.x);
                    max.x = std::max(max.x, v.x + pos.x);
                    min.y = std::min(min.y, v.y + pos.y);
                    max.y = std::max(max.y, v.y + pos.y);
                }
            }
            auto const function = to_function(*this, aFill, rect{ point{ min.x, min.y }, size{ max.x - min.x, max.y - min.y } });
            if (!vertexArrays.room_for(aMesh.faces.size() * 3u))
                vertexArrays.draw_and_execute();
            for (auto const& f : aMesh.faces)
            {
                for (auto vi : f)
                {
                    auto const& v = vertices[vi];
                    vertexArrays.push_back({
                        v + pos,
                        std::holds_alternative<color>(aFill) ?
                            static_variant_cast<color>(aFill).as<float>() :
                            vec4f{ 0.0f, 0.0f, 0.0f, 1.0f },
                        uv[vi],
                        function });
                }
            }
        }
    }

    void opengl_rendering_context::draw_entities(game::i_ecs& aEcs, game::scene_layer aLayer, const mat44& aTransformation)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };

        neolib::scoped_flag snap{ iSnapToPixel, false };

        thread_local std::vector<std::vector<mesh_drawable>> tMeshDrawables;
        thread_local game::scene_layer tMaxLayer = 0;
        thread_local optional_ecs_render_lock tLock;

        if (tMeshDrawables.size() <= aLayer)
            tMeshDrawables.resize(aLayer + 1);

        if (aLayer == 0)
        {
            for (auto& d : tMeshDrawables)
                d.clear();
            tLock.emplace(aEcs);

            if (aEcs.system_instantiated<game::animator>() && aEcs.system<game::animator>().can_apply())
                aEcs.system<game::animator>().apply();

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
                    service<debug::logger>() << neolib::logger::severity::Debug << "Rendering service<i_debug>().layout_item() entity..." << std::endl;
#endif // NEOGFX_DEBUG
                auto const& info = infos.entity_record_no_lock(entity);
                if (info.destroyed)
                    continue;
                auto const& meshRenderer = meshRenderers.entity_record_no_lock(entity);
                tMaxLayer = std::max(tMaxLayer, meshRenderer.layer);
                for (auto const& patch : meshRenderer.patches)
                    if (patch.layer.has_value())
                        tMaxLayer = std::max(tMaxLayer, patch.layer.value());
                if (tMeshDrawables.size() <= tMaxLayer)
                    tMeshDrawables.resize(tMaxLayer + 1);
                auto const& meshFilter = meshFilters.has_entity_record_no_lock(entity) ?
                    meshFilters.entity_record_no_lock(entity) :
                    game::current_animation_frame(animatedMeshFilters.entity_record_no_lock(entity));
                tMeshDrawables[meshRenderer.layer].emplace_back(
                    origin(),
                    meshFilter,
                    meshRenderer,
                    optional_mat44f{},
                    entity);
                for (auto const& patch : meshRenderer.patches)
                    if (patch.layer.has_value() && patch.layer.value() != meshRenderer.layer && 
                        (tMeshDrawables[patch.layer.value()].empty() || tMeshDrawables[patch.layer.value()].back().entity != entity))
                        tMeshDrawables[patch.layer.value()].emplace_back(
                            origin(),
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
                    tMeshDrawables[meshRenderer.layer].back().transformation = transformation;
                    for (auto const& patch : meshRenderer.patches)
                        if (patch.layer.has_value() && patch.layer.value() != meshRenderer.layer && 
                            !tMeshDrawables[patch.layer.value()].back().transformation.has_value())
                            tMeshDrawables[patch.layer.value()].back().transformation = transformation;
                }
                if (info.debug)
                    tMeshDrawables[meshRenderer.layer].back().debug = true;
            }
        }
        if (!tMeshDrawables[aLayer].empty())
            draw_meshes(tLock, dynamic_cast<i_vertex_provider&>(aEcs), aLayer, &*tMeshDrawables[aLayer].begin(), &*tMeshDrawables[aLayer].begin() + tMeshDrawables[aLayer].size(), aTransformation);
        if (aLayer >= tMaxLayer)
        {
            tMaxLayer = 0;
            for (auto& d : tMeshDrawables)
                d.clear();
            tLock.reset();
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

    void opengl_rendering_context::draw_glyphs(const render_batch& aDrawGlyphOps)
    {
        thread_local neolib::variable_stack<std::vector<draw_glyph>> glyphCacheStack;
        neolib::variable_stack_context<std::vector<draw_glyph>> context{ glyphCacheStack };

        auto& drawGlyphCache = glyphCacheStack.current();
        drawGlyphCache.clear();

        for (auto op = aDrawGlyphOps.cbegin(); op != aDrawGlyphOps.cend(); ++op)
        {
            update_state(*op);

            auto& drawOp = static_variant_cast<const graphics_operation::draw_glyphs&>(**op);
            auto a = drawOp.attributes.begin();
            for (auto g = drawOp.begin; g != drawOp.end; ++g)
            {
                while (a != drawOp.attributes.end() && (g - drawOp.begin) >= a->end)
                    ++a;
                auto& glyphChar = *g;
                drawGlyphCache.emplace_back(origin(), drawOp.point, &drawOp.glyphText.content(), 
                    &glyphChar, a != drawOp.attributes.end() && (g - drawOp.begin) >= a->start ? &a->attributes : nullptr, drawOp.showMnemonics);
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

        thread_local std::size_t tNesting;
        neolib::scoped_counter sc{ tNesting };

        thread_local std::vector<std::unique_ptr<draw_glyph_arrays>> tDrawGlyphArraysStack;
        if (tDrawGlyphArraysStack.size() < tNesting)
            tDrawGlyphArraysStack.resize(tNesting);
        if (tDrawGlyphArraysStack[tNesting - 1u] == nullptr)
            tDrawGlyphArraysStack[tNesting - 1u] = std::make_unique<draw_glyph_arrays>();

        auto& tDrawGlyphArrays = *tDrawGlyphArraysStack[tNesting - 1u];
        auto& tMeshOrigins = tDrawGlyphArrays.meshOrigins;
        auto& tMeshFilters = tDrawGlyphArrays.meshFilters;
        auto& tMeshRenderers = tDrawGlyphArrays.meshRenderers;
        auto& tMeshDrawables = tDrawGlyphArrays.drawables;

        auto draw = [&]()
        {
            for (std::size_t i = 0; i < tMeshFilters.size(); ++i)
                tMeshDrawables.emplace_back(tMeshOrigins[i], tMeshFilters[i], tMeshRenderers[i]);
            optional_ecs_render_lock ignore;
            if (!tMeshDrawables.empty())
                draw_meshes(ignore, as_vertex_provider(), 0, &*tMeshDrawables.begin(), &*tMeshDrawables.begin() + tMeshDrawables.size(), mat44::identity());
            tMeshOrigins.clear();
            tMeshFilters.clear();
            tMeshRenderers.clear();
            tMeshDrawables.clear();
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

        std::chrono::duration<double> since = std::chrono::steady_clock::now().time_since_epoch();

        auto paper_maybe_animated = [&](draw_glyph const& aGlyph) -> std::optional<game::color>
            {
                if (!std::holds_alternative<color>(*aGlyph.appearance->paper()))
                    return std::optional<game::color>{};
                else if (!aGlyph.appearance->animation() || !std::holds_alternative<color>(aGlyph.appearance->ink()) ||
                    aGlyph.appearance->animation()->type() != text_animation_type::Flash)
                    return to_ecs_component(std::get<color>(*aGlyph.appearance->paper()));
                else
                    return to_ecs_component(
                        mix(
                            std::get<color>(*aGlyph.appearance->paper()), 
                            std::get<color>(aGlyph.appearance->ink()), 
                            partitioned_ease(aGlyph.appearance->animation()->easing(), 
                                std::fmod(since.count() * aGlyph.appearance->animation()->frequency(), 1.0))));
            };

        auto ink_maybe_animated = [&](draw_glyph const& aGlyph, text_color const& aInk) -> std::optional<game::color>
            {
                if (!std::holds_alternative<color>(aInk))
                    return std::optional<game::color>{};
                else if (!aGlyph.appearance->animation() ||
                    aGlyph.appearance->animation()->type() == text_animation_type::None)
                    return to_ecs_component(std::get<color>(aInk));
                else if (aGlyph.appearance->animation()->type() == text_animation_type::Flash &&
                    aGlyph.appearance->paper() && std::holds_alternative<color>(*aGlyph.appearance->paper()))
                    return to_ecs_component(
                        mix(
                            std::get<color>(aInk),
                            std::get<color>(*aGlyph.appearance->paper()),
                            partitioned_ease(aGlyph.appearance->animation()->easing(),
                                std::fmod(since.count() * aGlyph.appearance->animation()->frequency(), 1.0))));
                else if (aGlyph.appearance->animation()->type() == text_animation_type::Blink)
                    return to_ecs_component(
                        mix(
                            std::get<color>(aGlyph.appearance->ink().with_alpha(1.0)),
                            std::get<color>(aGlyph.appearance->ink().with_alpha(0.0)),
                            partitioned_ease(aGlyph.appearance->animation()->easing(),
                                std::fmod(since.count() * aGlyph.appearance->animation()->frequency(), 1.0))));
                else
                    return to_ecs_component(std::get<color>(aInk));
            };

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
                            filterRegion = bounding_rect().value();
                    }
                        
                    if (drawOp.appearance->paper() != std::nullopt && !drawOp.appearance->being_filtered())
                    {
                        auto const& mesh = to_ecs_component(drawOp.point + quadf{glyphChar.cell[0], glyphChar.cell[1], glyphChar.cell[2], glyphChar.cell[3]}, mesh_type::Triangles);

                        tMeshOrigins.push_back(drawOp.origin);
                        tMeshFilters.push_back(game::mesh_filter{ {}, mesh });
                        tMeshRenderers.push_back(
                            game::mesh_renderer{
                                game::material{
                                    paper_maybe_animated(drawOp),
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
                            filter.emplace(*this, blur_filter{ *filterRegion, drawOp.appearance->effect()->width(), blurring_algorithm::Gaussian,
                                drawOp.appearance->effect()->aux1(), drawOp.appearance->effect()->aux2() });

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
                    auto const& ink = !drawOp.appearance->effect() || !drawOp.appearance->being_filtered() ?
                        (drawOp.appearance->ignore_emoji() ? neolib::none : drawOp.appearance->ink()) :
                        (drawOp.appearance->effect()->ignore_emoji() ? neolib::none : drawOp.appearance->effect()->color());
                    tMeshOrigins.push_back(drawOp.origin);
                    tMeshFilters.push_back(game::mesh_filter{ game::shared<game::mesh>{}, mesh });
                    tMeshRenderers.push_back(game::mesh_renderer
                        {
                            game::material
                            {
                                ink_maybe_animated(drawOp, ink),
                                std::holds_alternative<gradient>(ink) ? 
                                    to_ecs_component(static_variant_cast<const gradient&>(ink).with_bounding_box_if_none(rect{ to_aabb_2d(glyphQuad.begin(), glyphQuad.end()) })) : 
                                    std::optional<game::gradient>{},
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
                                auto const& ink = drawOp.appearance->effect()->color();
                                tMeshOrigins.push_back(drawOp.origin);
                                tMeshFilters.push_back(game::mesh_filter{ {}, mesh });
                                tMeshRenderers.push_back(
                                    game::mesh_renderer{
                                        game::material{
                                            ink_maybe_animated(drawOp, ink),
                                            std::holds_alternative<gradient>(ink) ? to_ecs_component(static_variant_cast<const gradient&>(ink).with_bounding_box_if_none(to_aabb_2d(glyphQuad.begin(), glyphQuad.end()))) : std::optional<game::gradient>{},
                                            {},
                                            to_ecs_component(theGlyph.outline_texture()),
                                            shader_effect::Ignore
                                        },
                                        {},
                                        true,
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
                        auto const& ink = !drawOp.appearance->effect() || !drawOp.appearance->being_filtered() ?
                            drawOp.appearance->ink() : drawOp.appearance->effect()->color();
                        tMeshOrigins.push_back(drawOp.origin);
                        tMeshFilters.push_back(game::mesh_filter{ {}, mesh });
                        tMeshRenderers.push_back(
                            game::mesh_renderer{
                                game::material{
                                    ink_maybe_animated(drawOp, ink),
                                    std::holds_alternative<gradient>(ink) ? 
                                        to_ecs_component(static_variant_cast<const gradient&>(ink).with_bounding_box_if_none(to_aabb_2d(glyphQuad.begin(), glyphQuad.end()))) : 
                                        std::optional<game::gradient>{},
                                    {},
                                    to_ecs_component(theGlyph.texture()),
                                    shader_effect::Ignore
                                },
                                {},
                                true,
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
                        auto const& pos = drawOp.point;

                        if (underline(glyphChar) || (drawOp.showMnemonics && neogfx::mnemonic(glyphChar)))
                        {
                            auto const& ink = !drawOp.appearance->effect() || !drawOp.appearance->being_filtered() ?
                                drawOp.appearance->ink() : drawOp.appearance->effect()->color();

                            auto const& glyphFont = glyphText.glyph_font(glyphChar);
                            auto const& shapeQuad = shape_quad(glyphFont, glyphChar);

                            auto const& majorFont = glyphText.major_font();
                            auto const yUnderline = std::floor(static_cast<float>(majorFont.native_font_face().underline_position()));
                            auto const cyUnderline = std::round(static_cast<float>(majorFont.native_font_face().underline_thickness()));

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
                                        pos + vec3f{ glyphChar.cell[0].x, glyphChar.cell[0].y + baseline - yUnderline },
                                        pos + vec3f{ glyphChar.cell[1].x, glyphChar.cell[1].y + baseline - yUnderline },
                                        pen{ ink, cyUnderline, false });
                                else
                                {
                                    auto lineSegment = lineSegments.begin();
                                    for (float x = glyphChar.cell[0].x; lineSegment != lineSegments.end() && x < glyphChar.cell[1].x;)
                                    {
                                        draw_line(
                                            pos + vec3f{ x, glyphChar.cell[0].y + baseline - yUnderline },
                                            pos + vec3f{ static_cast<float>(lineSegment->v1.x), glyphChar.cell[1].y + baseline - yUnderline },
                                            pen{ ink, cyUnderline, false });
                                        x = static_cast<float>(lineSegment->v2.x);
                                        if (++lineSegment == lineSegments.end())
                                        {
                                            draw_line(
                                                pos + vec3f{ x, glyphChar.cell[0].y + baseline - yUnderline },
                                                pos + vec3f{ glyphChar.cell[1].x, glyphChar.cell[1].y + baseline - yUnderline },
                                                pen{ ink, cyUnderline, false });
                                        }
                                    }
                                }
                            }
                            else
                            {
                                draw_line(
                                    pos + vec3f{ glyphChar.cell[0].x, glyphChar.cell[0].y + baseline - yUnderline },
                                    pos + vec3f{ glyphChar.cell[1].x, glyphChar.cell[1].y + baseline - yUnderline },
                                    pen{ ink, cyUnderline, false });
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
        draw_mesh(game::mesh_filter{ { &aMesh }, {}, {} }, game::mesh_renderer{ aMaterial, {}, true, 0, aFilter }, aTransformation);
    }
    
    void opengl_rendering_context::draw_mesh(const game::mesh_filter& aMeshFilter, const game::mesh_renderer& aMeshRenderer, const mat44& aTransformation)
    {
        if (!aMeshRenderer.render)
            return;

        mesh_drawable drawable
        {
            origin(),
            aMeshFilter,
            aMeshRenderer
        };
        optional_ecs_render_lock ignore;
        draw_meshes(ignore, as_vertex_provider(), aMeshRenderer.layer, &drawable, &drawable + 1, aTransformation);
    }

    void opengl_rendering_context::draw_meshes(optional_ecs_render_lock& aLock, i_vertex_provider& aVertexProvider, game::scene_layer aLayer, mesh_drawable* aFirst, mesh_drawable* aLast, const mat44& aTransformation)
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
            if (!meshRenderer.render)
                continue;
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
            auto& meshRenderer = *meshDrawable.renderer;
            if (!meshRenderer.render)
                continue;

            auto& meshFilter = *meshDrawable.filter;
            thread_local game::mesh_render_cache ignore;
            ignore = {};
            auto const& meshRenderCache = (meshDrawable.entity != null_entity ? cache->entity_record_no_lock(meshDrawable.entity, true) : ignore);
            auto& mesh = (meshFilter.mesh != std::nullopt ? *meshFilter.mesh : *meshFilter.sharedMesh.ptr);
            auto const& origin = meshDrawable.origin.to_vec3().as<float>();
            auto const& transformation = meshDrawable.transformation;
            auto const& faces = mesh.faces;
            auto const& material = meshRenderer.material;
            vec4f const defaultColor{ 1.0f, 1.0f, 1.0f, 1.0f };

            auto add_item = [&](vec2u32& cacheIndices, game::scene_layer aItemLayer, auto const& mesh, auto const& material, auto const& faces)
            {
                if (aItemLayer != aLayer)
                    return false;

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
                            vec2f const textureMargin{ 1.0f, 1.0f };
                            textureId = nextTextureId;
                            auto const& texture = *service<i_texture_manager>().find_texture(nextTextureId);
                            textureStorageExtents = texture.storage_extents().to_vec2().as<float>();
                            uvFixupCoefficient = materialTexture.extents;
                            if (materialTexture.type == texture_type::Texture)
                                uvFixupOffset = textureMargin;
                            else if (materialTexture.subTexture == std::nullopt)
                                uvFixupOffset = texture.as_sub_texture().atlas_location().top_left().to_vec2().as<float>() + textureMargin;
                            else
                                uvFixupOffset = materialTexture.subTexture->min + textureMargin;
                            if (texture.is_render_target() && texture.as_render_target().logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui)
                                uvGui = static_cast<float>((texture.extents().to_vec2().as<float>().y + textureMargin.y * 2.0f) / textureStorageExtents.y);
                        }
                    }
                    auto const vertexCount = faces.size() * 3;
                    auto const currentCachedVertexCount = cacheIndices[1] - cacheIndices[0];
                    auto vertexStartIndex = cacheIndices[0];
                    if (vertexCount != currentCachedVertexCount || meshRenderCache.state == game::cache_state::Invalid)
                    {
                        vertices.reclaim(cacheIndices[0], cacheIndices[1]);
                        vertexStartIndex = static_cast<std::uint32_t>(vertices.find_space_for(vertexCount));
                    }
                    auto nextIndex = vertexStartIndex;
                    for (auto const& face : faces)
                    {
                        for (auto faceVertexIndex : face)
                        {
                            auto const& xyz = (transformation? *transformation * mesh.vertices[faceVertexIndex] : mesh.vertices[faceVertexIndex]) + origin;
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
                            {
                                auto& st = vertices[nextIndex].st;
                                st.y = *uvGui - st.y;
                            }
                            ++nextIndex;
                        }
                    }
                    cacheIndices[0] = static_cast<std::uint32_t>(vertexStartIndex);
                    cacheIndices[1] = static_cast<std::uint32_t>(nextIndex);
                }

                patchDrawable.items.emplace_back(meshDrawable, cacheIndices[0], cacheIndices[1], material, faces);

                return true;
            };

#if defined(NEOGFX_DEBUG) && !defined(NDEBUG)
            if (meshDrawable.entity != game::null_entity &&
                dynamic_cast<game::i_ecs&>(aVertexProvider).component<game::entity_info>().entity_record(meshDrawable.entity).debug)
                service<debug::logger>() << neolib::logger::severity::Debug << "Adding service<i_debug>().layout_item() entity drawable..." << std::endl;
#endif // NEOGFX_DEBUG

            bool addedPrimaryMesh = false;
            if (!faces.empty())
                addedPrimaryMesh = add_item(meshRenderCache.meshVertexArrayIndices, meshRenderer.layer, mesh, material, faces);
            auto const patchCount = meshRenderer.patches.size();
            meshRenderCache.patchVertexArrayIndices.resize(patchCount);
            bool outstandingPatches = false;
            for (std::size_t patchIndex = 0; patchIndex < patchCount; ++patchIndex)
            {
                auto& patch = meshRenderer.patches[patchIndex];
                add_item(meshRenderCache.patchVertexArrayIndices[patchIndex], patch.layer.value_or(meshRenderer.layer), mesh, patch.material, patch.faces);
                if (patch.layer.has_value() && patch.layer.value() > aLayer)
                    outstandingPatches = true;
            }
            if (cache && !outstandingPatches)
                meshRenderCache.state = game::cache_state::Clean;
        }

        draw_patch(patchDrawable, aTransformation);
    }

    void opengl_rendering_context::draw_patch(patch_drawable& aPatch, const mat44& aTransformation)
    {
        use_shader_program usp{ *this, rendering_engine().default_shader_program(), iFastState.opacity };
        neolib::scoped_flag snap{ iSnapToPixel, false };

        std::optional<std::pair<point, mat44f>> originTranslatedTransformation;

        std::optional<use_vertex_arrays> vertexArrayUsage;

        auto const logicalCoordinates = logical_coordinates();

        i_texture const* previousTexture = nullptr;

        for (auto item = aPatch.items.begin(); item != aPatch.items.end();)
        {
            auto& vertexBuffer = static_cast<opengl_vertex_buffer<>&>(service<i_rendering_engine>().vertex_buffer(*aPatch.provider));
            auto& vertices = vertexBuffer.vertices();

            auto const& batchRenderer = *item->meshDrawable->renderer;
            auto const& batchMaterial = *item->material;
            auto const& transformation = aTransformation.as<float>();

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

                if (previousTexture != &texture)
                {
                    if (previousTexture != nullptr)
                        previousTexture->unbind();
                    texture.bind(sampling != texture_sampling::Multisample ? static_cast<std::uint32_t>(reserved_texture_unit::Tex) : static_cast<std::uint32_t>(reserved_texture_unit::TexMS));
                    previousTexture = &texture;
                }

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
                    vertexArrayUsage.emplace(*aPatch.provider, *this, transformation, with_textures, 0, batchRenderer.barrier);

#if defined(NEOGFX_DEBUG) && !defined(NDEBUG)
                if (item->meshDrawable->entity != game::null_entity &&
                    dynamic_cast<game::i_ecs&>(*aPatch.provider).component<game::entity_info>().entity_record(item->meshDrawable->entity).debug)
                    service<debug::logger>() << neolib::logger::severity::Debug << "Drawing service<i_debug>().layout_item() entity (texture)..." << std::endl;

#endif // NEOGFX_DEBUG
                vertexArrayUsage->draw(item->vertexArrayIndexStart, faceCount * 3);
            }
            else
            {
                rendering_engine().default_shader_program().texture_shader().clear_texture();

                if (vertexArrayUsage == std::nullopt || vertexArrayUsage->with_textures())
                    vertexArrayUsage.emplace(*aPatch.provider, *this, transformation, 0, batchRenderer.barrier);

#if defined(NEOGFX_DEBUG) && !defined(NDEBUG)
                if (item->meshDrawable->entity != game::null_entity &&
                    dynamic_cast<game::i_ecs&>(*aPatch.provider).component<game::entity_info>().entity_record(item->meshDrawable->entity).debug)
                    service<debug::logger>() << neolib::logger::severity::Debug << "Drawing service<i_debug>().layout_item() entity (non-texture)..." << std::endl;

#endif // NEOGFX_DEBUG
                vertexArrayUsage->draw(item->vertexArrayIndexStart, faceCount * 3);
            }

            item = next;

            disable_sample_shading();
        }

        if (previousTexture != nullptr)
            previousTexture->unbind();
    }

    void opengl_rendering_context::draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect, const optional_color& aColor, shader_effect aShaderEffect)
    {
        bool const guiRect = logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui &&
            (aTexture.as_render_target().logical_coordinate_system() != neogfx::logical_coordinate_system::AutomaticGui);
        auto mesh = guiRect ? to_ecs_component(aRect) : to_ecs_component(game_rect{ aRect });
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