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

        for (auto timer = iTimers.begin(); timer != iTimers.end();)
        {
            if (timer->expired())
                timer = iTimers.erase(timer);
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
            if (filter.frameState.active && has_animation_frames(filter))
            {
                if (!filter.frameState.currentFrameStartTime)
                    filter.frameState.currentFrameStartTime = info.creationTime;

                auto const& frames = to_animation_frames(filter);
                auto const previousFrame = static_cast<u32>(filter.frameState.currentFrame % frames.size());
                auto currentFrame = previousFrame;
                while (now > *filter.frameState.currentFrameStartTime + to_step_time(frames[currentFrame].duration, worldClock.timestep))
                {
                    auto const frameDuration = to_step_time(frames[currentFrame].duration, worldClock.timestep);
                    if (frameDuration == 0)
                        throw std::runtime_error("neogfx::game::animator: frame duration of zero!");
                    *filter.frameState.currentFrameStartTime += frameDuration;
                    currentFrame = static_cast<u32>((currentFrame + 1u) % frames.size());
                    filter.frameState.currentFrame = currentFrame;
                    if (currentFrame == 0 && filter.frameState.autoDestroy)
                    {
                        ecs().async_destroy_entity(entity);
                        break;
                    }
                }
                if (currentFrame != previousFrame)
                    set_render_cache_dirty_no_lock(cache, entity);
            }
            for (auto& tweenState : filter.tweenStates)
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

    animation_timer_ptr animator::create_timer()
    {
        return create_timer(ecs().system<game::time>().world_time());
    }

    animation_timer_ptr animator::create_timer(i64 epoch)
    {
        auto newTimer = std::make_shared<animation_timer>(epoch);
        iTimers.push_back(newTimer);
        return newTimer;
    }
}