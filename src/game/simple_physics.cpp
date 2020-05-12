// simple_physics.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/async_thread.hpp>
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/game_world.hpp>
#include <neogfx/game/clock.hpp>
#include <neogfx/game/simple_physics.hpp>
#include <neogfx/game/time.hpp>
#include <neogfx/game/physics.hpp>
#include <neogfx/game/rigid_body.hpp>

namespace neogfx::game
{
    class simple_physics::thread : public async_thread
    {
    public:
        thread(simple_physics& aOwner) : async_thread{ "neogfx::game::simple_physics::thread" }, iOwner{ aOwner }
        {
            start();
        }
    public:
        void do_work(neolib::yield_type aYieldType = neolib::yield_type::NoYield) override
        {
            async_thread::do_work(aYieldType);
            iOwner.apply();
            iOwner.yield();
        }
    private:
        simple_physics& iOwner;
    };


    simple_physics::simple_physics(game::i_ecs& aEcs) :
        system{ aEcs }
    {
        if (!ecs().system_registered<time>())
            ecs().register_system<time>();
        ecs().system<time>();
        if (!ecs().shared_component_registered<physics>())
            ecs().register_shared_component<physics>();
        if (ecs().shared_component<physics>().component_data().empty())
            ecs().populate_shared<physics>("Standard Universe", physics{ 6.67408e-11 });
        iThread = std::make_unique<thread>(*this);
    }

    simple_physics::~simple_physics()
    {
    }

    const system_id& simple_physics::id() const
    {
        return meta::id();
    }

    const i_string& simple_physics::name() const
    {
        return meta::name();
    }

    void simple_physics::apply()
    {
        if (!ecs().component_instantiated<rigid_body>())
            return;
        if (paused())
            return;
        if (!iThread->in()) // ignore ECS apply request (we have our own thread that does this)
            return;
        auto& worldClock = ecs().shared_component<clock>()[0];
        auto const& physicalConstants = ecs().shared_component<physics>()[0];
        auto const uniformGravity = physicalConstants.uniformGravity != std::nullopt ?
            *physicalConstants.uniformGravity : vec3{};
        auto const now = ecs().system<time>().system_time();
        auto& rigidBodies = ecs().component<rigid_body>();
        while (worldClock.time <= now)
        {
            yield();
            ecs().system<game_world>().ApplyingPhysics.trigger(worldClock.time);
            std::optional<scoped_component_lock<rigid_body>> lockRigidBodies{ ecs() };
            bool useUniversalGravitation = (universal_gravitation_enabled() && physicalConstants.gravitationalConstant != 0.0);
            if (useUniversalGravitation)
                rigidBodies.sort([](const rigid_body& lhs, const rigid_body& rhs) { return lhs.mass > rhs.mass; });
            auto firstMassless = useUniversalGravitation ?
                std::find_if(rigidBodies.component_data().begin(), rigidBodies.component_data().end(), [](const rigid_body& body) { return body.mass == 0.0; }) :
                rigidBodies.component_data().begin();
            for (auto& rigidBody1 : rigidBodies.component_data())
            {
                auto entity1 = rigidBodies.entity(rigidBody1);
                if (entity1 == null_entity)
                    continue; // todo: add support for skip iterators
                vec3 totalForce = rigidBody1.mass * uniformGravity;
                if (useUniversalGravitation)
                {
                    for (auto iterRigidBody2 = rigidBodies.component_data().begin(); iterRigidBody2 != firstMassless; ++iterRigidBody2)
                    {
                        auto& rigidBody2 = *iterRigidBody2;
                        auto entity2 = rigidBodies.entity(rigidBody2);
                        if (entity2 == null_entity)
                            continue; // todo: add support for skip iterators
                        vec3 distance = rigidBody1.position - rigidBody2.position;
                        if (distance.magnitude() > 0.0) // avoid division by zero or rigidBody1 == rigidBody2
                            totalForce += -physicalConstants.gravitationalConstant * rigidBody2.mass * rigidBody1.mass * distance / std::pow(distance.magnitude(), 3.0);
                    }
                }
                // GCSE-level physics (Newtonian) going on here... :)
                // v = u + at
                // F = ma; a = F/m
                auto v0 = rigidBody1.velocity;
                auto p0 = rigidBody1.position;
                auto a0 = rigidBody1.angle;
                auto elapsedTime = from_step_time(worldClock.timeStep);
                rigidBody1.velocity = v0 + ((rigidBody1.mass == 0 ? vec3{} : totalForce / rigidBody1.mass) + (rotation_matrix(rigidBody1.angle) * rigidBody1.acceleration)).scale(vec3{ elapsedTime, elapsedTime, elapsedTime });
                rigidBody1.position = rigidBody1.position + vec3{ 1.0, 1.0, 1.0 }.scale(elapsedTime * (v0 + rigidBody1.velocity) / 2.0);
                rigidBody1.angle = (rigidBody1.angle + rigidBody1.spin * elapsedTime) % (2.0 * boost::math::constants::pi<scalar>());
            }
            lockRigidBodies.reset();
            ecs().system<game_world>().PhysicsApplied.trigger(worldClock.time);
            shared_component_scoped_lock<clock> lockClock{ ecs() };
            worldClock.time += worldClock.timeStep;
        }
    }

    void simple_physics::terminate()
    {
        if (!iThread->aborted())
            iThread->abort();
    }

    bool simple_physics::universal_gravitation_enabled() const
    {
        return ecs().system<game_world>().universal_gravitation_enabled();
    }

    void simple_physics::enable_universal_gravitation()
    {
        return ecs().system<game_world>().enable_universal_gravitation();
    }

    void simple_physics::disable_universal_gravitation()
    {
        return ecs().system<game_world>().disable_universal_gravitation();
    }
}