// simple_physics.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/game/entity_info.hpp>
#include <neogfx/game/game_world.hpp>
#include <neogfx/game/clock.hpp>
#include <neogfx/game/collision_detector.hpp>
#include <neogfx/game/animator.hpp>
#include <neogfx/game/mesh_render_cache.hpp>
#include <neogfx/game/simple_physics.hpp>
#include <neogfx/game/time.hpp>
#include <neogfx/game/physics.hpp>

namespace neogfx::game
{
    template <typename ColliderType>
    simple_physics<ColliderType>::simple_physics(i_ecs& aEcs) :
        system<entity_info, ColliderType, mesh_filter, rigid_body, mesh_render_cache>{ aEcs }
    {
        if (!this->ecs().shared_component_registered<physics>())
            this->ecs().register_shared_component<physics>();
        if (this->ecs().shared_component<physics>().component_data().empty())
            this->ecs().populate_shared<physics>("Standard Universe", physics{ 6.67408e-11f });
        this->start_thread_if();
    }

    template <typename ColliderType>
    simple_physics<ColliderType>::~simple_physics()
    {
    }

    template <typename ColliderType>
    const system_id& simple_physics<ColliderType>::id() const
    {
        return meta::id();
    }

    template <typename ColliderType>
    const i_string& simple_physics<ColliderType>::name() const
    {
        return meta::name();
    }

    template <typename ColliderType>
    bool simple_physics<ColliderType>::apply()
    {
        if (!this->can_apply())
            throw cannot_apply();
        if (!this->ecs().component_instantiated<rigid_body>())
            return false;

        this->start_update();

        std::optional<scoped_component_lock<entity_info, mesh_render_cache, rigid_body>> lock{ this->ecs() };

        auto const& time = this->ecs().system<game::time>();
        auto const now = time.system_time();
        auto& clock = this->ecs().shared_component<game::clock>();
        auto& worldClock = clock[0];
        auto const& physicalConstants = this->ecs().shared_component<physics>()[0];
        auto const uniformGravity = physicalConstants.uniformGravity != std::nullopt ?
            *physicalConstants.uniformGravity : vec3f{};
        auto& infos = this->ecs().component<entity_info>();
        auto& rigidBodies = this->ecs().component<rigid_body>();
        bool didWork = false;
        auto currentTimestep = worldClock.timestep;
        auto nextTime = worldClock.time + currentTimestep;
        auto startTime = std::chrono::high_resolution_clock::now();
        while (worldClock.time <= now)
        {
            if (std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::high_resolution_clock::now() - startTime) > iYieldTime)
            {
                lock.reset();
                this->yield();
                lock.emplace(this->ecs());
                startTime = std::chrono::high_resolution_clock::now();
            }
            this->start_update(1);
            didWork = true;
            this->ecs().system<game_world>().ApplyingPhysics(worldClock.time);
            this->start_update(2);
            bool useUniversalGravitation = (universal_gravitation_enabled() && physicalConstants.gravitationalConstant != 0.0);
            if (useUniversalGravitation)
                rigidBodies.sort([](const rigid_body& lhs, const rigid_body& rhs) { return lhs.mass > rhs.mass; });
            auto firstMassless = useUniversalGravitation ?
                std::find_if(rigidBodies.component_data().begin(), rigidBodies.component_data().end(), [](const rigid_body& body) { return body.mass == 0.0; }) :
                rigidBodies.component_data().begin();
            for (auto& rigidBody1 : rigidBodies.component_data())
            {
                auto entity1 = rigidBodies.entity(rigidBody1);
                auto const& entity1Info = infos.entity_record(entity1);
                if (entity1Info.destroyed)
                    continue; // todo: add support for skip iterators
                vec3f totalForce = rigidBody1.mass * uniformGravity;
                if (useUniversalGravitation)
                {
                    for (auto iterRigidBody2 = rigidBodies.component_data().begin(); iterRigidBody2 != firstMassless; ++iterRigidBody2)
                    {
                        auto& rigidBody2 = *iterRigidBody2;
                        auto entity2 = rigidBodies.entity(rigidBody2);
                        auto const& entity2Info = infos.entity_record(entity2);
                        if (entity2Info.destroyed)
                            continue; // todo: add support for skip iterators
                        vec3f distance = rigidBody1.position - rigidBody2.position;
                        if (distance.magnitude() > 0.0f) // avoid division by zero or rigidBody1 == rigidBody2
                            totalForce += -physicalConstants.gravitationalConstant * rigidBody2.mass * rigidBody1.mass * distance / std::pow(distance.magnitude(), 3.0f);
                    }
                }
                // GCSE-level physics (Newtonian) going on here... :)
                // v = u + at
                // F = ma; a = F/m
                auto v0 = rigidBody1.velocity;
                auto p0 = rigidBody1.position;
                auto a0 = rigidBody1.angle;
                auto elapsedTime = static_cast<float>(from_step_time(nextTime - worldClock.time));
                rigidBody1.velocity = v0 + ((rigidBody1.mass == 0.0f ? vec3f{} : totalForce / rigidBody1.mass) + (rotation_matrix(rigidBody1.angle) * rigidBody1.acceleration)).scale(vec3f{ elapsedTime, elapsedTime, elapsedTime });
                rigidBody1.position = rigidBody1.position + vec3f{ 1.0f, 1.0f, 1.0f }.scale(elapsedTime * (v0 + rigidBody1.velocity) / 2.0f);
                rigidBody1.angle = (rigidBody1.angle + rigidBody1.spin * elapsedTime) % (2.0f * boost::math::constants::pi<float>());
                if (p0 != rigidBody1.position || a0 != rigidBody1.angle)
                    set_render_cache_dirty(this->ecs(), entity1);
            }
            this->end_update(2);
            if (this->ecs().system_instantiated<collision_detector_2d>() && !this->ecs().system<collision_detector_2d>().paused())
                this->ecs().system<collision_detector_2d>().run_cycle(collision_detection_cycle::UpdateColliders);
            if (this->ecs().system_instantiated<animator>() && this->ecs().system<animator>().can_apply())
                this->ecs().system<animator>().apply();
            this->ecs().system<game::time>().apply();
            this->ecs().system<game_world>().PhysicsApplied(worldClock.time);
            shared_component_scoped_lock<game::clock> lockClock{ this->ecs() };
            worldClock.time = nextTime;
            currentTimestep = std::min(static_cast<i64>(currentTimestep * worldClock.timestepGrowth), std::max(worldClock.timestep, worldClock.maximumTimestep));
            nextTime += currentTimestep;
            this->end_update(1);
        }

        lock.reset();

        this->end_update();

        return didWork;
    }

    template <typename ColliderType>
    bool simple_physics<ColliderType>::universal_gravitation_enabled() const
    {
        return this->ecs().system<game_world>().universal_gravitation_enabled();
    }

    template <typename ColliderType>
    void simple_physics<ColliderType>::enable_universal_gravitation()
    {
        return this->ecs().system<game_world>().enable_universal_gravitation();
    }

    template <typename ColliderType>
    void simple_physics<ColliderType>::disable_universal_gravitation()
    {
        return this->ecs().system<game_world>().disable_universal_gravitation();
    }

    template <typename ColliderType>
    void simple_physics<ColliderType>::yield_after(std::chrono::duration<double, std::milli> aTime)
    {
        iYieldTime = aTime;
    }

    template class simple_physics<box_collider_2d>;
    template class simple_physics<box_collider_3d>;
}