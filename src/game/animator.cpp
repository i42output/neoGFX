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
#include <neogfx/game/animation_filter.hpp>

namespace neogfx::game
{
    class animator::thread : public async_thread
    {
    public:
        thread(animator& aOwner) : async_thread{ "neogfx::game::animator::thread" }, iOwner{ aOwner }
        {
            start();
        }
    public:
        bool do_work(neolib::yield_type aYieldType = neolib::yield_type::NoYield) override
        {
            bool didWork = async_thread::do_work(aYieldType);
            didWork = iOwner.apply() || didWork;
            iOwner.yield();
            return didWork;
        }
    private:
        animator& iOwner;
    };


    animator::animator(game::i_ecs& aEcs) :
        system<entity_info, animation_filter>{ aEcs }
    {
        Animate.set_trigger_type(neolib::event_trigger_type::SynchronousDontQueue);
        if (!ecs().system_registered<game::time>())
            ecs().register_system<game::time>();
        ecs().system<game::time>();
        iThread = std::make_unique<thread>(*this);
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
        if (!ecs().component_instantiated<animation_filter>())
            return false;
        else if (paused())
            return false;
        else if (!iThread->in()) // ignore ECS apply request (we have our own thread that does this)
            return false;

        update_animations();

        return true;
    }

    void animator::terminate()
    {
        if (!iThread->aborted())
            iThread->abort();
    }

    void animator::update_animations()
    {
        auto now = ecs().system<game::time>().world_time();

        Animate.trigger(now);

        scoped_component_lock<entity_info, animation_filter> lockAnimationFilters{ ecs() };

        for (auto entity : ecs().component<animation_filter>().entities())
        {
            auto const& info = ecs().component<entity_info>().entity_record(entity);
            if (info.destroyed)
                continue;
            auto& filter = ecs().component<animation_filter>().entity_record(entity);
            if (!filter.currentFrameStartTime)
                filter.currentFrameStartTime = info.creationTime;
            auto const& frames = (filter.animation ? filter.animation->frames : filter.sharedAnimation.ptr->frames);
            auto const& worldClock = ecs().shared_component<game::clock>()[0];
            while (*filter.currentFrameStartTime + to_step_time(frames[filter.currentFrame].duration, worldClock.timeStep) < now)
            {
                *filter.currentFrameStartTime += to_step_time(frames[filter.currentFrame % frames.size()].duration, worldClock.timeStep);
                filter.currentFrame = (filter.currentFrame + 1u) % frames.size();
                if (filter.currentFrame == 0 && filter.autoDestroy)
                {
                    ecs().async_destroy_entity(entity, false);
                    break;
                }
            }
        }
    }
}