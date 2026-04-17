// render_target.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.
  
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_render_target.hpp>

namespace neogfx
{
    struct render_queue_context
    {
        point origin;
        std::vector<rect> clipRegionStack;
        bool scissorOn = false;
    };

    void optimise_render_queue(render_queue_context& aContext, graphics_operation::queue const& aInput, graphics_operation::optimised_queue& aOutput);

    template <typename T>
    concept RenderTargetInterface = std::is_base_of_v<i_render_target, T>;

    template <typename RenderTargetInterface = i_render_target>
    class render_target : public RenderTargetInterface
    {
    public:
        define_declared_event(TargetActivating, target_activating)
        define_declared_event(TargetActivated, target_activated)
        define_declared_event(TargetDeactivating, target_deactivating)
        define_declared_event(TargetDeactivated, target_deactivated)
        define_declared_event(TargetDestroying, target_destroying)
    public:
        graphics_operation::i_queue& render_queue() const final
        {
            if (iOptimisedQueueExtant)
                throw std::logic_error("neogfx::render_target::render_queue: optimised render queue extant!");

            return iQueue;
        }
        graphics_operation::i_optimised_queue const& optimised_render_queue() const final
        {
            if (iOptimisedQueueExtant)
                return iOptimisedQueue;

            iOptimisedQueueExtant = true;

            optimise_render_queue(iRenderQueueContext, iQueue, iOptimisedQueue);

            return iOptimisedQueue;
        }
        void clear_render_queues() const final
        {
            iQueue.clear();
            iOptimisedQueue.clear();
            iOptimisedQueueExtant = false;
        }
    private:
        mutable render_queue_context iRenderQueueContext;
        mutable graphics_operation::queue iQueue;
        mutable graphics_operation::optimised_queue iOptimisedQueue;
        mutable bool iOptimisedQueueExtant = false;
    };

}