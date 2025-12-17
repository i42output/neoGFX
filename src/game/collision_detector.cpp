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
#include <neogfx/game/entity_info.hpp>
#include <neogfx/game/simple_physics.hpp>
#include <neogfx/game/collision_detector.hpp>

namespace neogfx::game
{
    template<typename ColliderType, typename BroadphaseTreeType>
    collision_detector< ColliderType, BroadphaseTreeType>::collision_detector(i_ecs& aEcs) :
        system<entity_info, ColliderType>{ aEcs },
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
            scoped_component_lock<entity_info, box_collider_3d> lock{ this->ecs() };
            thread_local std::vector<entity_id> hits;
            hits.clear();
            iBroadphaseTree.pick(aPoint, hits);
            if (!hits.empty())
                return hits[0];
        }
        else if constexpr (std::is_same_v<ColliderType, box_collider_2d>)
        {
            scoped_component_lock<entity_info, box_collider_2d> lock{ this->ecs() };
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

        this->start_update();
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
        scoped_component_lock<entity_info, ColliderType> lock{ this->ecs() };
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
            scoped_component_lock<entity_info, box_collider_3d, mesh_filter, animation_filter, rigid_body> lock{ this->ecs() };
            thread_local auto const& infos = this->ecs().component<entity_info>();
            thread_local auto const& meshFilters = this->ecs().component<mesh_filter>();
            thread_local auto const& animatedMeshFilters = this->ecs().component<animation_filter>();
            thread_local auto const& rigidBodies = this->ecs().component<rigid_body>();
            thread_local auto& boxColliders = this->ecs().component<box_collider_3d>();
            for (auto entity : boxColliders.entities())
            {
                auto const& info = infos.entity_record(entity);
                if (info.destroyed)
                    continue; // todo: add support for skip iterators
                // todo: only update collider AABBs if rigid_body changes require it
                auto const& meshFilter = meshFilters.has_entity_record(entity) ?
                    meshFilters.entity_record(entity) : current_animation_frame(animatedMeshFilters.entity_record(entity));
                auto& collider = boxColliders.entity_record(entity);
                collider.previousAabb = collider.currentAabb;
                auto const& untransformed = (meshFilter.mesh != std::nullopt ?
                    *meshFilter.mesh : *meshFilter.sharedMesh.ptr);
                if (!collider.untransformedAabb)
                    collider.untransformedAabb = to_aabb(untransformed.vertices);
                collider.currentAabb = aabb_transform(*collider.untransformedAabb,
                    (animatedMeshFilters.has_entity_record(entity) ?
                        to_transformation_matrix(animatedMeshFilters.entity_record(entity)) : mat44f::identity()),
                    (meshFilter.transformation ?
                        *meshFilter.transformation : mat44f::identity()),
                    (rigidBodies.has_entity_record(entity) ?
                        to_transformation_matrix(rigidBodies.entity_record(entity)) : mat44f::identity()));
                if (!collider.previousAabb)
                    collider.previousAabb = collider.currentAabb;
            }
        }
        else if constexpr (std::is_same_v<ColliderType, box_collider_2d>)
        {
            scoped_component_lock<entity_info, box_collider_2d, mesh_filter, animation_filter, rigid_body> lock{ this->ecs() };
            thread_local auto const& infos = this->ecs().component<entity_info>();
            thread_local auto const& meshFilters = this->ecs().component<mesh_filter>();
            thread_local auto const& animatedMeshFilters = this->ecs().component<animation_filter>();
            thread_local auto const& rigidBodies = this->ecs().component<rigid_body>();
            thread_local auto& boxColliders2d = this->ecs().component<box_collider_2d>();
            for (auto entity : boxColliders2d.entities())
            {
                auto const& info = infos.entity_record(entity);
                if (info.destroyed)
                    continue; // todo: add support for skip iterators
                // todo: only update collider AABBs if rigid_body changes require it
                auto const& meshFilter = meshFilters.has_entity_record(entity) ?
                    meshFilters.entity_record(entity) : current_animation_frame(animatedMeshFilters.entity_record(entity));
                auto& collider = boxColliders2d.entity_record(entity);
                collider.previousAabb = collider.currentAabb;
                auto const& untransformed = (meshFilter.mesh != std::nullopt ?
                    *meshFilter.mesh : *meshFilter.sharedMesh.ptr);
                if (!collider.untransformedAabb)
                    collider.untransformedAabb = to_aabb_2d(untransformed.vertices);
                collider.currentAabb = aabb_transform(*collider.untransformedAabb, 
                    (animatedMeshFilters.has_entity_record(entity) ?
                        to_transformation_matrix(animatedMeshFilters.entity_record(entity)) : mat44f::identity()),
                    (meshFilter.transformation ?
                        *meshFilter.transformation : mat44f::identity()),
                    (rigidBodies.has_entity_record(entity) ?
                        to_transformation_matrix(rigidBodies.entity_record(entity)) : mat44f::identity()));
                if (!collider.previousAabb)
                    collider.previousAabb = collider.currentAabb;
            }
        }

        iUpdated = true;
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    void collision_detector<ColliderType, BroadphaseTreeType>::update_broadphase()
    {
        scoped_component_lock<entity_info, ColliderType> lock{ this->ecs() };
        iBroadphaseTree.full_update();
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    void collision_detector<ColliderType, BroadphaseTreeType>::detect_collisions()
    {
        if (!this->components_available())
            return;

        scoped_component_lock<entity_info, ColliderType> lock{ this->ecs() };
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