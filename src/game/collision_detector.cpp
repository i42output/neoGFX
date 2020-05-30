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
    class collision_detector::thread : public async_thread
    {
    public:
        thread(collision_detector& aOwner) : async_thread{ "neogfx::collision_detector::thread" }, iOwner{ aOwner }
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

    collision_detector::collision_detector(i_ecs& aEcs) :
        system<entity_info, box_collider, box_collider_2d>{ aEcs },
        iBroadphaseTree{ aEcs },
        iBroadphase2dTree{ aEcs }
    {
        Collision.set_trigger_type(neolib::event_trigger_type::SynchronousDontQueue);
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
        if (ecs().component_instantiated<box_collider>())
        {
            scoped_component_lock<entity_info, box_collider> lock{ ecs() };
            thread_local std::vector<entity_id> hits;
            hits.clear();
            iBroadphaseTree.pick(aPoint, hits);
            if (!hits.empty())
                return hits[0];
        }

        if (ecs().component_instantiated<box_collider_2d>())
        {
            scoped_component_lock<entity_info, box_collider_2d> lock{ ecs() };
            thread_local std::vector<entity_id> hits;
            hits.clear();
            iBroadphase2dTree.pick(aPoint.xy, hits);
            if (!hits.empty())
                return hits[0];
        }

        return {};
    }

    bool collision_detector::apply()
    {
        if (ecs().system_registered<simple_physics>())
            return false;
        else if (!ecs().component_instantiated<box_collider>() && !ecs().component_instantiated<box_collider_2d>())
            return false;
        else if (paused())
            return false;
        else if (!iThread->in()) // ignore ECS apply request (we have our own thread that does this)
            return false;
        
        start_update();
        run_cycle();
        end_update();

        return true;
    }

    void collision_detector::terminate()
    {
        if (!iThread->aborted())
            iThread->abort();
    }

    void collision_detector::run_cycle(bool aDetect)
    {
        update_colliders();
        update_trees();
        if (aDetect)
            detect_collisions();
    }

    void collision_detector::update_colliders()
    {
        if (ecs().component_instantiated<box_collider>())
        {
            scoped_component_lock<entity_info, box_collider, mesh_filter, rigid_body> lock{ ecs() };
            auto const& meshFilters = ecs().component<mesh_filter>();
            auto const& rigidBodies = ecs().component<rigid_body>();
            auto& boxColliders = ecs().component<box_collider>();
            for (auto entity : boxColliders.entities())
            {
                auto const& info = ecs().component<entity_info>().entity_record(entity);
                if (info.destroyed)
                    continue; // todo: add support for skip iterators
                // todo: only update collider AABBs if rigid_body changes require it
                auto const& meshFilter = meshFilters.entity_record(entity);
                auto const& transformation = rigidBodies.has_entity_record(entity) ?
                    to_transformation_matrix(rigidBodies.entity_record(entity)) : 
                    meshFilter.transformation ? *meshFilter.transformation : mat44::identity();
                auto& collider = boxColliders.entity_record(entity);
                collider.previousAabb = collider.currentAabb;
                auto const& untransformed = (meshFilter.mesh != std::nullopt ?
                    *meshFilter.mesh : *meshFilter.sharedMesh.ptr);
                collider.currentAabb = to_aabb(untransformed.vertices, transformation);
                if (!collider.previousAabb)
                    collider.previousAabb = collider.currentAabb;
            }
        }

        if (ecs().component_instantiated<box_collider_2d>())
        {
            scoped_component_lock<entity_info, box_collider_2d, mesh_filter, rigid_body> lock{ ecs() };
            auto const& meshFilters = ecs().component<mesh_filter>();
            auto const& rigidBodies = ecs().component<rigid_body>();
            auto& boxColliders2d = ecs().component<box_collider_2d>();
            for (auto entity : boxColliders2d.entities())
            {
                auto const& info = ecs().component<entity_info>().entity_record(entity);
                if (info.destroyed)
                    continue; // todo: add support for skip iterators
                // todo: only update collider AABBs if rigid_body changes require it
                auto const& meshFilter = meshFilters.entity_record(entity);
                auto const& transformation = rigidBodies.has_entity_record(entity) ?
                    to_transformation_matrix(rigidBodies.entity_record(entity)) : 
                    meshFilter.transformation ? *meshFilter.transformation : mat44::identity();
                auto& collider = boxColliders2d.entity_record(entity);
                collider.previousAabb = collider.currentAabb;
                auto const& untransformed = (meshFilter.mesh != std::nullopt ?
                    *meshFilter.mesh : *meshFilter.sharedMesh.ptr);
                collider.currentAabb = to_aabb_2d(untransformed.vertices, transformation);
                if (!collider.previousAabb)
                    collider.previousAabb = collider.currentAabb;
            }
        }
    }

    void collision_detector::update_trees()
    {
        if (ecs().component_instantiated<box_collider>())
        {
            scoped_component_lock<entity_info, box_collider> lock{ ecs() };
            iBroadphaseTree.full_update();
        }

        if (ecs().component_instantiated<box_collider_2d>())
        {
            scoped_component_lock<entity_info, box_collider_2d> lock{ ecs() };
            iBroadphase2dTree.full_update();
        }
    }

    void collision_detector::detect_collisions()
    {
        if (ecs().component_instantiated<box_collider>())
        {
            scoped_component_lock<entity_info, box_collider> lock{ ecs() };
            iBroadphaseTree.collisions([this](entity_id e1, entity_id e2)
            {
                Collision.trigger(e1, e2);
            });
        }

        if (ecs().component_instantiated<box_collider_2d>())
        {
            scoped_component_lock<entity_info, box_collider_2d> lock{ ecs() };
            iBroadphase2dTree.full_update();
            iBroadphase2dTree.collisions([this](entity_id e1, entity_id e2)
            {
                Collision.trigger(e1, e2);
            });
        }
    }

    const aabb_octree<box_collider>& collision_detector::broadphase_tree() const
    {
        return iBroadphaseTree;
    }

    const aabb_quadtree<box_collider_2d>& collision_detector::broadphase_2d_tree() const
    {
        return iBroadphase2dTree;
    }
}