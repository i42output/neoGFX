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
    struct rendering_queue_context
    {
        i_render_target& renderTarget;

        std::vector<rect> clipRegionStack;

        std::uint64_t fastStateGeneration = 0u;
        std::uint64_t slowStateGeneration = 0u;

        std::vector<rendering_context_fast_state> fastState;
        std::vector<rendering_context_fast_state const*> fastStateIndex;
        std::vector<rendering_context_slow_state> slowState;
        std::vector<rendering_context_slow_state const*> slowStateIndex;

        std::optional<rendering_context_fast_state> lastFastState;
        std::optional<rendering_context_slow_state> lastSlowState;

        rendering_context_fast_state const* intern_state(rendering_context_fast_state const& aFastState)
        {
            auto it = std::lower_bound(fastStateIndex.begin(), fastStateIndex.end(), aFastState,
                [](rendering_context_fast_state const* p, rendering_context_fast_state const& s) { return *p < s; });
            if (it == fastStateIndex.end() || **it != aFastState)
            {
                auto& stored = fastState.emplace_back(aFastState);
                fastStateIndex.insert(it, &stored);
                stored.generation = ++fastStateGeneration;
                return &stored;
            }
            return *it;
        }
        rendering_context_slow_state const* intern_state(rendering_context_slow_state const& aSlowState)
        {
            auto it = std::lower_bound(slowStateIndex.begin(), slowStateIndex.end(), aSlowState,
                [](rendering_context_slow_state const* p, rendering_context_slow_state const& s) { return *p < s; });
            if (it == slowStateIndex.end() || **it != aSlowState)
            {
                auto& stored = slowState.emplace_back(aSlowState);
                slowStateIndex.insert(it, &stored);
                stored.generation = ++slowStateGeneration;
                return &stored;
            }
            return *it;
        }
    };

    void optimise_rendering_queue(rendering_queue_context& aContext, rendering_queue const& aInput, optimised_rendering_queue& aOutput);

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
        render_target() : 
            iRenderQueueContext{ *this }
        {
        }
    public:
        void begin_rendering() const final
        {
            iRenderQueueContext.lastFastState.reset();
            iRenderQueueContext.lastSlowState.reset();
        }
        void end_rendering() const final
        {
            clear_rendering_queues(true);
        }
        neogfx::rendering_queue& rendering_queue() const final
        {
            if (iOptimisedQueueExtant)
                clear_rendering_queues();

            return iQueue;
        }
        neogfx::optimised_rendering_queue const& optimised_rendering_queue() const final
        {
            if (iOptimisedQueueExtant)
                return iOptimisedQueue;

            iOptimisedQueueExtant = true;

            iRenderQueueContext.fastState.reserve(iQueue.size() + 1u);
            iRenderQueueContext.fastStateIndex.reserve(iQueue.size() + 1u);
            iRenderQueueContext.slowState.reserve(iQueue.size() + 1u);
            iRenderQueueContext.slowStateIndex.reserve(iQueue.size() + 1u);

            optimise_rendering_queue(iRenderQueueContext, iQueue, iOptimisedQueue);

            return iOptimisedQueue;
        }
    private:
        void clear_rendering_queues(bool aEndRendering = false) const
        { 
            if (aEndRendering)
            {
                iRenderQueueContext.clipRegionStack.clear();
                iRenderQueueContext.fastStateGeneration = 0u;
                iRenderQueueContext.slowStateGeneration = 0u;
                iRenderQueueContext.lastFastState.reset();
                iRenderQueueContext.lastSlowState.reset();
            }
            else
            {
                if (!iRenderQueueContext.fastState.empty())
                    iRenderQueueContext.lastFastState = iRenderQueueContext.fastState.back();
                else
                    iRenderQueueContext.lastFastState.reset();
                if (!iRenderQueueContext.slowState.empty())
                    iRenderQueueContext.lastSlowState = iRenderQueueContext.slowState.back();
                else
                    iRenderQueueContext.lastSlowState.reset();
            }

            iRenderQueueContext.fastState.clear();
            iRenderQueueContext.fastStateIndex.clear();
            iRenderQueueContext.slowState.clear();
            iRenderQueueContext.slowStateIndex.clear();

            iQueue.clear();
            iOptimisedQueue.clear();
            iOptimisedQueueExtant = false;
        }
    private:
        mutable rendering_queue_context iRenderQueueContext;
        mutable neogfx::rendering_queue iQueue;
        mutable neogfx::optimised_rendering_queue iOptimisedQueue;
        mutable bool iOptimisedQueueExtant = false;
    };

}