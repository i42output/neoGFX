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
            iOptimisedQueue.clear();
            return iQueue;
        }
        graphics_operation::i_optimised_queue const& optimised_render_queue() const final
        {
            if (!iOptimisedQueue.empty())
                return iOptimisedQueue;

            bool const optimiseQueue = service<i_rendering_engine>().is_render_queue_optimization_on();

            std::int32_t ordinal = 1;

            for (auto& queueEntry : render_queue())
            {
                auto const opType = static_cast<graphics_operation::operation_type>(queueEntry.index());
                bool keepOp = true;
                switch (opType)
                {
                case graphics_operation::ScissorOn:
                    iScissorOn = true;
                    keepOp = !optimiseQueue;
                    break;
                case graphics_operation::ScissorOff:
                    iScissorOn = false;
                    keepOp = !optimiseQueue;
                    break;
                case graphics_operation::PushScissor:
                    iClipRegionStack.push_back(static_variant_cast<const graphics_operation::push_scissor&>(queueEntry).rect);
                    keepOp = !optimiseQueue;
                    break;
                case graphics_operation::PopScissor:
                    if (iClipRegionStack.empty())
                        throw std::logic_error("neogfx::opengl_rendering_context::flush: unmatched PopScissor");
                    iClipRegionStack.pop_back();
                    keepOp = !optimiseQueue;
                    break;
                case graphics_operation::SetOrigin:
                    iOrigin = static_variant_cast<const graphics_operation::set_origin&>(queueEntry).origin;
                    keepOp = !optimiseQueue;
                    break;
                }
                if (keepOp)
                    iOptimisedQueue.emplace_back(&queueEntry, ordinal++, iOrigin, optimiseQueue && !iClipRegionStack.empty() && iScissorOn ? iClipRegionStack.back() : std::optional<rect>{});
            }

            if (service<i_rendering_engine>().is_render_queue_optimization_on())
            {
                // todo...
                std::stable_sort(iOptimisedQueue.begin(), iOptimisedQueue.end(), [](auto const& lhs, auto const& rhs)
                    {
                        if (lhs->index() != rhs->index())
                            return lhs->index() < rhs->index();
                        else if (!graphics_operation::batchable(*lhs, *rhs))
                            return &*lhs < &*rhs;
                        else
                            return lhs.ordinal < rhs.ordinal;
                    });
            }

            return iOptimisedQueue;
        }
        void clear_render_queues() const final
        {
            iQueue.clear();
            iOptimisedQueue.clear();
        }
    private:
        mutable graphics_operation::queue iQueue;
        mutable graphics_operation::optimised_queue iOptimisedQueue;
        mutable point iOrigin;
        mutable std::vector<rect> iClipRegionStack;
        mutable bool iScissorOn = false;
    };

}