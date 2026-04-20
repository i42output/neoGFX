// render_target.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020-2026 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/render_target.hpp>

namespace neogfx
{
    void optimise_rendering_queue(rendering_queue_context& aContext, rendering_queue const& aInput, optimised_rendering_queue& aOutput)
    {
        bool const optimiseQueue = service<i_rendering_engine>().is_rendering_queue_optimization_on();

        auto fastState = aContext.intern_state(aContext.lastFastState.value_or(rendering_context_fast_state{}));
        auto slowState = aContext.intern_state(aContext.lastSlowState.value_or(rendering_context_slow_state{}));

        std::int32_t ordinal = 1;

        std::optional<point> origin;
        std::optional<std::int32_t> scissorCounterAdjust;
        std::optional<double> opacity;

        std::optional<logical_coordinate_system> logicalCoordinateSystem;
        std::optional<logical_coordinates> logicalCoordinates;
        std::optional<front_face> frontFace;
        std::optional<face_culling> faceCulling;
        std::optional<blending_mode> blendingMode;
        std::optional<smoothing_mode> smoothingMode;

        for (auto const& queueEntry : aInput)
        {
            auto const opType = static_cast<graphics_operation::operation_type>(queueEntry.index());
            bool keepOp = true;
            switch (opType)
            {
            case graphics_operation::SetLogicalCoordinateSystem:
                logicalCoordinateSystem = static_variant_cast<const graphics_operation::set_logical_coordinate_system&>(queueEntry).system;
                keepOp = false;
                break;
            case graphics_operation::SetLogicalCoordinates:
                logicalCoordinates = static_variant_cast<const graphics_operation::set_logical_coordinates&>(queueEntry).coordinates;
                keepOp = false;
                break;
            case graphics_operation::SetFrontFace:
                frontFace = static_variant_cast<const graphics_operation::set_front_face&>(queueEntry).frontFace;
                keepOp = false;
                break;
            case graphics_operation::SetCullFaces:
                faceCulling = static_variant_cast<const graphics_operation::set_face_culling&>(queueEntry).culling;
                keepOp = false;
                break;
            case graphics_operation::SetBlendingMode:
                blendingMode = static_variant_cast<const graphics_operation::set_blending_mode&>(queueEntry).blendingMode;
                keepOp = false;
                break;
            case graphics_operation::SetSmoothingMode:
                smoothingMode = static_variant_cast<const graphics_operation::set_smoothing_mode&>(queueEntry).smoothingMode;
                keepOp = false;
                break;
            case graphics_operation::ScissorOn:
                scissorCounterAdjust = scissorCounterAdjust.value_or(0) + 1;
                keepOp = false;
                break;
            case graphics_operation::ScissorOff:
                scissorCounterAdjust = scissorCounterAdjust.value_or(0) - 1;
                keepOp = false;
                break;
            case graphics_operation::PushScissor:
                if (aContext.clipRegionStack.empty())
                    aContext.clipRegionStack.push_back(static_variant_cast<const graphics_operation::push_scissor&>(queueEntry).rect + fastState->origin);
                else
                    aContext.clipRegionStack.push_back(aContext.clipRegionStack.back().intersection(static_variant_cast<const graphics_operation::push_scissor&>(queueEntry).rect + fastState->origin));
                keepOp = false;
                break;
            case graphics_operation::PopScissor:
                if (aContext.clipRegionStack.empty())
                    throw std::logic_error("neogfx::opengl_rendering_context::flush: unmatched PopScissor");
                aContext.clipRegionStack.pop_back();
                keepOp = false;
                break;
            case graphics_operation::SetOrigin:
                origin = static_variant_cast<const graphics_operation::set_origin&>(queueEntry).origin;
                keepOp = false;
                break;
            case graphics_operation::SetOpacity:
                opacity = static_variant_cast<const graphics_operation::set_opacity&>(queueEntry).opacity;
                keepOp = false;
                break;
            }
            
            auto const clipRegion = !aContext.clipRegionStack.empty() ? aContext.clipRegionStack.back() : std::optional<rect>{};
            
            if (fastState->clipRegion != clipRegion || 
                scissorCounterAdjust ||
                (origin && fastState->origin != *origin) ||
                (opacity && fastState->opacity != *opacity))
            {
                auto state = *fastState;
                if (origin)
                    state.origin = *origin;
                state.clipRegion = clipRegion;
                if (scissorCounterAdjust)
                    state.scissorCounter += *scissorCounterAdjust;
                if (opacity)
                    state.opacity = *opacity;
                origin.reset();
                scissorCounterAdjust.reset();
                opacity.reset();
                fastState = aContext.intern_state(state);
            }
            
            if ((logicalCoordinateSystem && slowState->logicalCoordinateSystem != logicalCoordinateSystem) ||
                (logicalCoordinates && slowState->logicalCoordinates != logicalCoordinates) ||
                (frontFace && slowState->frontFace != frontFace) ||
                (faceCulling && slowState->faceCulling != faceCulling) ||
                (blendingMode && slowState->blendingMode != blendingMode) ||
                (smoothingMode && slowState->smoothingMode != smoothingMode))
            {
                auto state = *slowState;
                if (logicalCoordinateSystem)
                    state.logicalCoordinateSystem = *logicalCoordinateSystem;
                if (logicalCoordinates)
                    state.logicalCoordinates = *logicalCoordinates;
                if (frontFace)
                    state.frontFace = *frontFace;
                if (faceCulling)
                    state.faceCulling = *faceCulling;
                if (blendingMode)
                    state.blendingMode = *blendingMode;
                if (smoothingMode)
                    state.smoothingMode = *smoothingMode;
                logicalCoordinateSystem.reset();
                logicalCoordinates.reset();
                frontFace.reset();
                faceCulling.reset();
                blendingMode.reset();
                smoothingMode.reset();
                slowState = aContext.intern_state(state);
            }
            
            if (keepOp)
                aOutput.emplace_back(&queueEntry, ordinal++, fastState, slowState);
        }

        if (optimiseQueue)
        {
            // todo...
            std::stable_sort(aOutput.begin(), aOutput.end(), [](auto const& lhs, auto const& rhs)
                {
                    if (lhs->index() != rhs->index())
                        return lhs->index() < rhs->index();
                    else if (!batchable(*lhs, *rhs))
                        return &*lhs < &*rhs;
                    else if (std::holds_alternative<graphics_operation::draw_mesh>(*lhs))
                    {
                        auto const& leftMesh = std::get<graphics_operation::draw_mesh>(*lhs);
                        auto const& rightMesh = std::get<graphics_operation::draw_mesh>(*rhs);
                        if (leftMesh.material < rightMesh.material)
                            return true;
                        else if (leftMesh.material > rightMesh.material)
                            return false;
                    }
                    return lhs.ordinal < rhs.ordinal;
                });
        }
    }

    scoped_render_target::scoped_render_target() : iRenderTarget{ nullptr }, iPreviouslyActivatedTarget{ nullptr }
    {
        iPreviouslyActivatedTarget = service<i_rendering_engine>().active_target();
    }

    scoped_render_target::scoped_render_target(const i_render_target& aRenderTarget) : iRenderTarget{ &aRenderTarget }, iPreviouslyActivatedTarget{ nullptr }
    {
        iPreviouslyActivatedTarget = service<i_rendering_engine>().active_target();
        if (iPreviouslyActivatedTarget != iRenderTarget)
        {
            if (iPreviouslyActivatedTarget)
                iPreviouslyActivatedTarget->deactivate_target();
            iRenderTarget->activate_target();
            iRenderTarget->target_add_ref();
        }
    }

    scoped_render_target::scoped_render_target(const i_rendering_context& aRenderingContext) :
        scoped_render_target{ aRenderingContext.render_target() }
    {
        iContext = &aRenderingContext;
    }

    scoped_render_target::scoped_render_target(i_rendering_context& aRenderingContext) :
        scoped_render_target{ aRenderingContext.render_target() }
    {
        iContext = &aRenderingContext;
    }

    scoped_render_target::~scoped_render_target()
    {
        if (std::holds_alternative<i_rendering_context*>(iContext))
            std::get<i_rendering_context*>(iContext)->flush();
        if (iRenderTarget && iPreviouslyActivatedTarget != iRenderTarget)
        {
            if (iRenderTarget->target_active())
            {
                iRenderTarget->target_release();
                iRenderTarget->deactivate_target();
            }
            if (iPreviouslyActivatedTarget)
                iPreviouslyActivatedTarget->activate_target();
        }
        else if (!iRenderTarget && iPreviouslyActivatedTarget)
            iPreviouslyActivatedTarget->activate_target();
    }
}