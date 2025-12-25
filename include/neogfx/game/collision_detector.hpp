// collision_detector.hpp
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
#pragma once

#include <neogfx/neogfx.hpp>

#include <neogfx/core/event.hpp>
#include <neogfx/game/system.hpp>
#include <neogfx/game/aabb_quadtree.hpp>
#include <neogfx/game/aabb_octree.hpp>
#include <neogfx/game/box_collider.hpp>

namespace neogfx::game
{
    enum class collision_detection_cycle : std::uint32_t
    {
        None                = 0x00000000,

        UpdateColliders     = 0x00000001,
        UpdateTrees         = 0x00000002,
        DetectCollisions    = 0x00000004,

        Default             = UpdateColliders | UpdateTrees | DetectCollisions,
        Detect              = UpdateTrees | DetectCollisions,
        Test                = UpdateColliders | UpdateTrees
    };

    inline constexpr collision_detection_cycle operator|(collision_detection_cycle aLhs, collision_detection_cycle aRhs)
    {
        return static_cast<collision_detection_cycle>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr collision_detection_cycle operator&(collision_detection_cycle aLhs, collision_detection_cycle aRhs)
    {
        return static_cast<collision_detection_cycle>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    template<typename ColliderType, typename BroadphaseTreeType>
    class collision_detector : public game::system<ColliderType>
    {
    private:
        using base_type = game::system<ColliderType>;
    public:
        define_event(Collision, collision, entity_id, entity_id)
    public:
        using base_type::cannot_apply;
    public:
        using broadphase_tree_type = BroadphaseTreeType;
    public:
        collision_detector(i_ecs& aEcs);
        ~collision_detector();
    public:
        std::optional<entity_id> entity_at(const vec3& aPoint) const;
    public:
        bool apply() override;
    public:
        void run_cycle(collision_detection_cycle aCycle = collision_detection_cycle::Default);
        template <typename Visitor>
        void visit_aabbs(const Visitor& aVisitor) const
        {
            iBroadphaseTree.visit_aabbs(aVisitor);
        }
    public:
        const broadphase_tree_type& broadphase_tree() const;
    private:
        void update();
        void update_broadphase();
        void detect_collisions();
    private:
        broadphase_tree_type iBroadphaseTree;
        std::atomic<bool> iUpdated;
    };

    class collision_detector_3d : public collision_detector<box_collider_3d, aabb_octree<box_collider_3d>>
    {
    private:
        using base_type = collision_detector<box_collider_3d, aabb_octree<box_collider_3d>>;
    public:
        collision_detector_3d(i_ecs& aEcs) :
            base_type{ aEcs }
        {
            start_thread_if();
        }
    public:
        const system_id& id() const final
        {
            return meta::id();
        }
        const i_string& name() const final
        {
            return meta::name();
        }
    public:
        struct meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xdb9c2033, 0xae26, 0x463e, 0x9100,{ 0xd5, 0x41, 0xfe, 0x4a, 0xd, 0xae } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Collision Detector (3D)";
                return sName;
            }
        };
    };

    class collision_detector_2d : public collision_detector<box_collider_2d, aabb_quadtree<box_collider_2d>>
    {
    private:
        using base_type = collision_detector<box_collider_2d, aabb_quadtree<box_collider_2d>>;
    public:
        collision_detector_2d(i_ecs& aEcs) :
            base_type{ aEcs }
        {
            start_thread_if();
        }
    public:
        const system_id& id() const final
        {
            return meta::id();
        }
        const i_string& name() const final
        {
            return meta::name();
        }
    public:
        struct meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xdb9c2033, 0xae26, 0x463e, 0x9100,{ 0xd5, 0x41, 0xfe, 0x4a, 0xd, 0xae } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Collision Detector (3D)";
                return sName;
            }
        };
    };
}