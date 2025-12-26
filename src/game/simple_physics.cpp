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
#include <neogfx/game/mesh_render_cache.hpp>
#include <neogfx/game/simple_physics.hpp>

namespace neogfx::game
{
    physics& standard_physics(i_ecs& aEcs)
    {
        if (!aEcs.shared_component_registered<physics>())
            aEcs.register_shared_component<physics>();
        if (aEcs.shared_component<physics>().component_data().empty())
            aEcs.populate_shared<physics>("Standard Universe", physics{ 6.67408e-11f });
        return aEcs.shared_component<physics>()[0];
    }

    template <typename ColliderType>
    simple_physics<ColliderType>::simple_physics(i_ecs& aEcs) :
        system<rigid_body, ColliderType>{ aEcs },
        iTime{ aEcs.system<game::time>() },
        iWorldClock{ aEcs.shared_component<game::clock>()[0] },
        iGameWorld{ aEcs.system<game_world>() },
        iCollisionDetector{ aEcs.system<collision_detector_t<ColliderType>>() },
        iPhysicalConstants{ standard_physics(aEcs) },
        iInfos{ aEcs.component<entity_info>() },
        iRigidBodies{ aEcs.component<rigid_body>() },
        iColliders{ aEcs.component<collider_type>() }
    {
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
        if (this->paused())
            return false;

        this->start_update();

        std::optional<scoped_component_lock<decltype(iRigidBodies), decltype(iColliders)>> lock;
        lock.emplace(iRigidBodies, iColliders);

        auto const now = iTime.system_time();
        auto const uniformGravity = iPhysicalConstants.uniformGravity != std::nullopt ?
            *iPhysicalConstants.uniformGravity : vec3f{};
        bool didWork = false;
        auto currentTimestep = iWorldClock.timestep;
        auto nextTime = iWorldClock.time + currentTimestep;
        while (iWorldClock.time <= now)
        {
            didWork = true;
            this->start_update(1);
            iGameWorld.ApplyingPhysics(iWorldClock.time);
            this->start_update(2);
            bool useUniversalGravitation = (universal_gravitation_enabled() && iPhysicalConstants.gravitationalConstant != 0.0);
            if (useUniversalGravitation)
                iRigidBodies.sort([](const rigid_body& lhs, const rigid_body& rhs) { return lhs.mass > rhs.mass; });
            auto firstMassless = useUniversalGravitation ?
                std::find_if(iRigidBodies.component_data().begin(), iRigidBodies.component_data().end(), [](const rigid_body& body) { return body.mass == 0.0; }) :
                iRigidBodies.component_data().begin();
            for (auto& rigidBody1 : iRigidBodies.component_data())
            {
                auto entity1 = iRigidBodies.entity(rigidBody1);
                auto const& entity1Info = iInfos.entity_record_no_lock(entity1);
                if (entity1Info.destroyed)
                    continue; // todo: add support for skip iterators
                vec3f totalForce = rigidBody1.mass * uniformGravity;
                if (useUniversalGravitation)
                {
                    for (auto iterRigidBody2 = iRigidBodies.component_data().begin(); iterRigidBody2 != firstMassless; ++iterRigidBody2)
                    {
                        auto& rigidBody2 = *iterRigidBody2;
                        auto entity2 = iRigidBodies.entity(rigidBody2);
                        auto const& entity2Info = iInfos.entity_record(entity2);
                        if (entity2Info.destroyed)
                            continue; // todo: add support for skip iterators
                        vec3f distance = rigidBody1.position - rigidBody2.position;
                        if (distance.magnitude() > 0.0f) // avoid division by zero or rigidBody1 == rigidBody2
                            totalForce += -iPhysicalConstants.gravitationalConstant * rigidBody2.mass * rigidBody1.mass * distance / std::pow(distance.magnitude(), 3.0f);
                    }
                }
                // GCSE-level physics (Newtonian) going on here... :)
                // v = u + at
                // F = ma; a = F/m
                auto v0 = rigidBody1.velocity;
                auto p0 = rigidBody1.position;
                auto a0 = rigidBody1.angle;
                auto elapsedTime = static_cast<float>(from_step_time(nextTime - iWorldClock.time));
                rigidBody1.velocity = v0 + ((rigidBody1.mass == 0.0f ? vec3f{} : totalForce / rigidBody1.mass) + 
                    (rotation_matrix(rigidBody1.angle) * rigidBody1.acceleration)).scale(vec3f{ elapsedTime, elapsedTime, elapsedTime });
                rigidBody1.position = rigidBody1.position + vec3f{ 1.0f, 1.0f, 1.0f }.scale(elapsedTime * (v0 + rigidBody1.velocity) / 2.0f);
                rigidBody1.angle = (rigidBody1.angle + rigidBody1.spin * elapsedTime) % (2.0f * boost::math::constants::pi<float>());
                if (p0 != rigidBody1.position || a0 != rigidBody1.angle)
                    set_render_cache_dirty_no_lock(this->ecs(), entity1);
            }
            this->end_update(2);
            {
                iCollisionDetector.apply();
                iTime.apply();
            }
            iGameWorld.PhysicsApplied(iWorldClock.time);
            scoped_shared_component_data_lock<game::clock> lockClock{ this->ecs() };
            iWorldClock.time = nextTime;
            currentTimestep = std::min(static_cast<i64>(currentTimestep * iWorldClock.timestepGrowth), 
                std::max(iWorldClock.timestep, iWorldClock.maximumTimestep));
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