 // animator.cpp
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

#include <neogfx/core/async_thread.hpp>
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/game_world.hpp>
#include <neogfx/game/clock.hpp>
#include <neogfx/game/time.hpp>
#include <neogfx/game/entity_info.hpp>
#include <neogfx/game/animator.hpp>
#include <neogfx/game/game_world.hpp>
#include <neogfx/game/simple_physics.hpp>
#include <neogfx/game/animation_filter.hpp>
#include <neogfx/game/mesh_render_cache.hpp>

namespace neogfx::game
{
    animator::animator(game::i_ecs& aEcs) :
        system<animation_filter>{ aEcs },
        iDefaultTimer{ std::make_shared<animation_timer>(ecs().system<game::time>().world_time()) }
    {
        Animate.set_trigger_type(neolib::trigger_type::SynchronousDontQueue);
    }

    animator::~animator()
    {
    }

    const system_id& animator::id() const
    {
        return meta::id();
    }

    const i_string& animator::name() const
    {
        return meta::name();
    }

    bool animator::apply()
    {
        if (!can_apply())
            throw cannot_apply();
        if (!ecs().component_instantiated<animation_filter>())
            return false;
        if (paused())
            return false;

        update_animations();

        for (auto& [id, timers] : iTimers)
            for (auto timer = timers.begin(); timer != timers.end();)
            {
                if (timer->expired())
                    timer = timers.erase(timer);
                else
                    ++timer;
            }

        return true;
    }

    bool animator::external_animation() const
    {
        return iExternalAnimation.load();
    }

    void animator::set_external_animation(bool aExternalAnimation)
    {
        iExternalAnimation.store(aExternalAnimation);
    }

    void animator::update_animations()
    {
        auto const& time = ecs().system<game::time>();
        auto now = time.world_time();

        scoped_component_data_lock<animation_filter> lock{ ecs() };

        auto& infos = ecs().component<entity_info>();
        auto& filters = ecs().component<animation_filter>();
        auto& cache = ecs().component<mesh_render_cache>();
        auto const& worldClock = ecs().shared_component<game::clock>()[0];

        if (!filters.entities().empty() || external_animation())
            Animate(now);

        for (auto entity : filters.entities())
        {
            auto const& info = infos.entity_record_no_lock(entity);
            if (info.destroyed)
                continue;
            auto& filter = filters.entity_record(entity);
            if (!has_animation(filter))
                continue;
            if (filter.frameAnimationState.active && has_animation_frames(filter))
            {
                if (!filter.frameAnimationState.currentFrameStartTime)
                    filter.frameAnimationState.currentFrameStartTime = info.creationTime;

                auto const& frames = to_animation_frames(filter);
                auto const previousFrame = static_cast<u32>(filter.frameAnimationState.currentFrame % frames.size());
                auto currentFrame = previousFrame;
                while (now > *filter.frameAnimationState.currentFrameStartTime + to_step_time(frames[currentFrame].duration, worldClock.timestep))
                {
                    auto const frameDuration = to_step_time(frames[currentFrame].duration, worldClock.timestep);
                    if (frameDuration == 0)
                        throw std::runtime_error("neogfx::game::animator: frame duration of zero!");
                    *filter.frameAnimationState.currentFrameStartTime += frameDuration;
                    currentFrame = static_cast<u32>((currentFrame + 1u) % frames.size());
                    filter.frameAnimationState.currentFrame = currentFrame;
                    if (currentFrame == 0 && filter.frameAnimationState.autoDestroy)
                    {
                        ecs().async_destroy_entity(entity);
                        break;
                    }
                }
                if (currentFrame != previousFrame)
                    set_render_cache_dirty_no_lock(cache, entity);
            }
            for (auto& tweenState : filter.tweenAnimationStates)
                if (tweenState.second.timer == nullptr)
                    tweenState.second.timer = default_timer();
            if (has_active_tweens(filter))
                set_render_cache_dirty_no_lock(cache, entity);
        }
    }

    animation_timer_ptr animator::default_timer()
    {
        return iDefaultTimer;
    }

    std::pair<animation_timer_ptr, bool> animator::create_timer()
    {
        return create_timer(neolib::uuid{}, false);
    }

    std::pair<animation_timer_ptr, bool> animator::create_timer(i64 aEpoch)
    {
        return create_timer(neolib::uuid{}, aEpoch, false);
    }

    std::pair<animation_timer_ptr, bool> animator::create_timer(neolib::uuid aId, bool aSingleton)
    {
        return create_timer(aId, ecs().system<game::time>().world_time(), aSingleton);
    }

    std::pair<animation_timer_ptr, bool> animator::create_timer(neolib::uuid aId, i64 aEpoch, bool aSingleton)
    {
        if (aSingleton)
            for (auto& timer : iTimers[aId])
                if (!timer.expired())
                    return std::make_pair(timer.lock(), false);
        auto newTimer = std::make_shared<animation_timer>(aEpoch);
        iTimers[aId].push_back(newTimer);
        return std::make_pair(newTimer, true);
    }
}