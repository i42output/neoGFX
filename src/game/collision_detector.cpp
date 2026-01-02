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
#include <neogfx/game/ecs_helpers.hpp>
#include <neogfx/game/simple_physics.hpp>
#include <neogfx/game/collision_detector.hpp>

namespace neogfx::game
{
    template<typename ColliderType, typename BroadphaseTreeType>
    collision_detector< ColliderType, BroadphaseTreeType>::collision_detector(i_ecs& aEcs) :
        system<ColliderType>{ aEcs },
        iInfos{ aEcs.component<entity_info>() },
        iRigidBodies{ aEcs.component<rigid_body>() },
        iBoxColliders{ aEcs.component<box_collider_type>() },
        iBroadphaseTree{ aEcs },
        iUpdated{ false }
    {
        Collision.set_trigger_type(neolib::trigger_type::SynchronousDontQueue);
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    collision_detector<ColliderType, BroadphaseTreeType>::~collision_detector()
    {
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    std::optional<entity_id> collision_detector<ColliderType, BroadphaseTreeType>::entity_at(const vec3& aPoint) const
    {
        if (!this->components_available())
            return {};

        if constexpr (std::is_same_v<ColliderType, box_collider_3d>)
        {
            scoped_component_data_lock<box_collider_3d> lock{ this->ecs() };
            thread_local std::vector<entity_id> hits;
            hits.clear();
            iBroadphaseTree.pick(aPoint, hits);
            if (!hits.empty())
                return hits[0];
        }
        else if constexpr (std::is_same_v<ColliderType, box_collider_2d>)
        {
            scoped_component_data_lock<box_collider_2d> lock{ this->ecs() };
            thread_local std::vector<entity_id> hits;
            hits.clear();
            iBroadphaseTree.pick(aPoint.xy, hits);
            if (!hits.empty())
                return hits[0];
        }

        return {};
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    bool collision_detector<ColliderType, BroadphaseTreeType>::apply()
    {
        if (!this->can_apply())
            throw cannot_apply();
        if (!this->components_available())
            return false;
        if (this->paused())
            return false;

        this->start_update();
        run_cycle(collision_detection_cycle::UpdateColliders);
        run_cycle(collision_detection_cycle::Detect);
        this->end_update();

        return true;
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    void collision_detector<ColliderType, BroadphaseTreeType>::run_cycle(collision_detection_cycle aCycle)
    {
        if (this->paused())
            return;
        if constexpr (std::is_same_v<ColliderType, box_collider_2d>)
        {
            if ((aCycle & collision_detection_cycle::UpdateColliders) == collision_detection_cycle::UpdateColliders ||
                ((aCycle & collision_detection_cycle::DetectCollisions) == collision_detection_cycle::DetectCollisions &&
                    !this->ecs().system_instantiated<simple_physics_2d>()))
                update();
        }
        else if constexpr (std::is_same_v<ColliderType, box_collider_3d>)
        {
            if ((aCycle & collision_detection_cycle::UpdateColliders) == collision_detection_cycle::UpdateColliders ||
                ((aCycle & collision_detection_cycle::DetectCollisions) == collision_detection_cycle::DetectCollisions &&
                    !this->ecs().system_instantiated<simple_physics_3d>()))
                update();
        }
        if (!iUpdated)
            return;
        scoped_component_lock lock{ iBoxColliders };
        if ((aCycle & collision_detection_cycle::UpdateTrees) == collision_detection_cycle::UpdateTrees)
            update_broadphase();
        if ((aCycle & collision_detection_cycle::DetectCollisions) == collision_detection_cycle::DetectCollisions)
            detect_collisions();
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    void collision_detector<ColliderType, BroadphaseTreeType>::update()
    {
        if (!this->components_available())
            return;

        if constexpr (std::is_same_v<ColliderType, box_collider_3d>)
        {
            scoped_component_lock lock{ iBoxColliders, iRigidBodies };
            for (auto entity : iBoxColliders.entities())
            {
                auto const& info = iInfos.entity_record_no_lock(entity);
                if (info.destroyed)
                    continue;
                auto& collider = iBoxColliders.entity_record_no_lock(entity);
                collider.previousAabb = collider.currentAabb;
                if (!collider.untransformedAabb)
                    collider.untransformedAabb = to_aabb(collider.hull);
                if (iRigidBodies.has_entity_record_no_lock(entity))
                {
                    auto const& rigidBody = iRigidBodies.entity_record_no_lock(entity);
                    if (collider.transformation.has_value())
                        collider.currentAabb = aabb_transform(*collider.untransformedAabb, *collider.transformation * to_transformation_matrix(rigidBody));
                    else
                        collider.currentAabb = aabb_transform(*collider.untransformedAabb, to_transformation_matrix(rigidBody));
                }
                else
                {
                    if (collider.transformation.has_value())
                        collider.currentAabb = aabb_transform(*collider.untransformedAabb, *collider.transformation);
                    else
                        collider.currentAabb = collider.untransformedAabb;
                }
                if (!collider.previousAabb)
                    collider.previousAabb = collider.currentAabb;
            }
        }
        else if constexpr (std::is_same_v<ColliderType, box_collider_2d>)
        {
            scoped_component_lock lock{ iBoxColliders, iRigidBodies };
            for (auto entity : iBoxColliders.entities())
            {
                auto const& info = iInfos.entity_record_no_lock(entity);
                if (info.destroyed)
                    continue;
                auto& collider = iBoxColliders.entity_record_no_lock(entity);
                collider.previousAabb = collider.currentAabb;
                if (!collider.untransformedAabb)
                    collider.untransformedAabb = to_aabb_2d(collider.hull);
                if (iRigidBodies.has_entity_record_no_lock(entity))
                {
                    auto const& rigidBody = iRigidBodies.entity_record_no_lock(entity);
                    if (collider.transformation.has_value())
                        collider.currentAabb = aabb_transform(*collider.untransformedAabb, *collider.transformation * to_transformation_matrix(rigidBody));
                    else
                        collider.currentAabb = aabb_transform(*collider.untransformedAabb, to_transformation_matrix(rigidBody));
                }
                else
                {
                    if (collider.transformation.has_value())
                        collider.currentAabb = aabb_transform(*collider.untransformedAabb, *collider.transformation);
                    else
                        collider.currentAabb = collider.untransformedAabb;
                }
                if (!collider.previousAabb)
                    collider.previousAabb = collider.currentAabb;
            }
        }

        iUpdated = true;
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    void collision_detector<ColliderType, BroadphaseTreeType>::update_broadphase()
    {
        scoped_component_lock lock{ iBoxColliders };
        iBroadphaseTree.full_update();
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    void collision_detector<ColliderType, BroadphaseTreeType>::detect_collisions()
    {
        if (!this->components_available())
            return;

        scoped_component_lock lock{ iBoxColliders };
        iBroadphaseTree.collisions([this](entity_id e1, entity_id e2)
        {
            Collision(e1, e2);
        });

        iUpdated = false;
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    const BroadphaseTreeType& collision_detector<ColliderType, BroadphaseTreeType>::broadphase_tree() const
    {
        return iBroadphaseTree;
    }

    template class collision_detector<box_collider_3d, aabb_octree<box_collider_3d>>;
    template class collision_detector<box_collider_2d, aabb_quadtree<box_collider_2d>>;
}