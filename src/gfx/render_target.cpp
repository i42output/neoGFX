// render_target.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
    void optimise_render_queue(render_queue_context& aContext, graphics_operation::queue const& aInput, graphics_operation::optimised_queue& aOutput)
    {
        bool const optimiseQueue = service<i_rendering_engine>().is_render_queue_optimization_on();

        std::int32_t ordinal = 1;

        for (auto const& queueEntry : aInput)
        {
            auto const opType = static_cast<graphics_operation::operation_type>(queueEntry.index());
            bool keepOp = true;
            switch (opType)
            {
            case graphics_operation::ScissorOn:
                aContext.scissorOn = true;
                keepOp = !optimiseQueue;
                break;
            case graphics_operation::ScissorOff:
                aContext.scissorOn = false;
                keepOp = !optimiseQueue;
                break;
            case graphics_operation::PushScissor:
                aContext.clipRegionStack.push_back(static_variant_cast<const graphics_operation::push_scissor&>(queueEntry).rect);
                keepOp = !optimiseQueue;
                break;
            case graphics_operation::PopScissor:
                if (aContext.clipRegionStack.empty())
                    throw std::logic_error("neogfx::opengl_rendering_context::flush: unmatched PopScissor");
                aContext.clipRegionStack.pop_back();
                keepOp = !optimiseQueue;
                break;
            case graphics_operation::SetOrigin:
                aContext.origin = static_variant_cast<const graphics_operation::set_origin&>(queueEntry).origin;
                keepOp = !optimiseQueue;
                break;
            }
            if (keepOp)
                aOutput.emplace_back(&queueEntry, ordinal++, aContext.origin, 
                    optimiseQueue && !aContext.clipRegionStack.empty() && aContext.scissorOn ? aContext.clipRegionStack.back() : std::optional<rect>{});
        }

        if (service<i_rendering_engine>().is_render_queue_optimization_on())
        {
            // todo...
            std::stable_sort(aOutput.begin(), aOutput.end(), [](auto const& lhs, auto const& rhs)
                {
                    if (lhs->index() != rhs->index())
                        return lhs->index() < rhs->index();
                    else if (!graphics_operation::batchable(*lhs, *rhs))
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