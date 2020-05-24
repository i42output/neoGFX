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
    class collision_detector : public system
    {
    public:
        define_event(Collision, collision, entity_id, entity_id)
    private:
        class thread;
    public:
        collision_detector(game::i_ecs& aEcs);
        ~collision_detector();
    public:
        const system_id& id() const override;
        const i_string& name() const override;
    public:
        std::optional<entity_id> entity_at(const vec3& aPoint) const;
    public:
        bool apply() override;
        void terminate() override;
    public:
        void update_colliders();
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
                static const string sName = "Collision Detector";
                return sName;
            }
        };
    private:
        std::unique_ptr<thread> iThread;
        aabb_quadtree<box_collider_2d> iBroadphase2dTree;
        aabb_octree<box_collider> iBroadphaseTree;
    };
}