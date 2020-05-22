// collision_detector.cpp
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
#include <neogfx/game/collision_detector.hpp>

namespace neogfx::game
{
    class collision_detector::thread : public async_thread
    {
    public:
        thread(collision_detector& aOwner) : async_thread{ "neogfx::game::collision_detector::thread" }, iOwner{ aOwner }
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
        collision_detector& iOwner;
    };


    collision_detector::collision_detector(game::i_ecs& aEcs) :
        system{ aEcs }
    {
        Animate.set_trigger_type(neolib::event_trigger_type::SynchronousDontQueue);
        if (!ecs().system_registered<time>())
            ecs().register_system<time>();
        ecs().system<time>();
        iThread = std::make_unique<thread>(*this);
    }

    collision_detector::~collision_detector()
    {
    }

    const system_id& collision_detector::id() const
    {
        return meta::id();
    }

    const i_string& collision_detector::name() const
    {
        return meta::name();
    }

    std::optional<entity_id> collision_detector::entity_at(const vec3& aPoint) const
    {
        return {};
    }

    bool collision_detector::apply()
    {
        if (!ecs().component_instantiated<animation_filter>())
            return false;
        if (paused())
            return false;
        if (!iThread->in()) // ignore ECS apply request (we have our own thread that does this)
            return false;

        auto now = ecs().system<time>().world_time();

        Animate.trigger(now);

        scoped_component_lock<animation_filter> lockAnimationFilters{ ecs() };

        for (auto entity : ecs().component<animation_filter>().entities())
        {
            auto const& info = ecs().component<entity_info>().entity_record(entity);
            auto& filter = ecs().component<animation_filter>().entity_record(entity);
            if (!filter.currentFrameStartTime)
                filter.currentFrameStartTime = info.creationTime;
            auto const& frames = (filter.animation ? filter.animation->frames : filter.sharedAnimation.ptr->frames);
            auto const& worldClock = ecs().shared_component<clock>()[0];
            while (*filter.currentFrameStartTime + to_step_time(frames[filter.currentFrame].duration, worldClock.timeStep) < now)
            {
                *filter.currentFrameStartTime += to_step_time(frames[filter.currentFrame % frames.size()].duration, worldClock.timeStep);
                filter.currentFrame = (filter.currentFrame + 1u) % frames.size();
            }
        }

        return true;
    }

    void collision_detector::terminate()
    {
        if (!iThread->aborted())
            iThread->abort();
    }
}