// simple_physics.hpp
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
#pragma once

#include <neogfx/neogfx.hpp>

#include <neogfx/core/event.hpp>
#include <neogfx/game/system.hpp>
#include <neogfx/game/collision_detector.hpp>
#include <neogfx/game/entity_info.hpp>
#include <neogfx/game/box_collider.hpp>
#include <neogfx/game/rigid_body.hpp>
#include <neogfx/game/physics.hpp>
#include <neogfx/game/time.hpp>
#include <neogfx/game/clock.hpp>
#include <neogfx/game/game_world.hpp>

namespace neogfx::game
{
    template <typename ColliderType> struct collision_detector_type {};
    template <> struct collision_detector_type<box_collider_2d> { using detector = collision_detector_2d; };
    template <> struct collision_detector_type<box_collider_3d> { using detector = collision_detector_3d; };
    template <typename ColliderType>
    using collision_detector_t = typename collision_detector_type<ColliderType>::detector;

    template <typename ColliderType>
    class simple_physics : public game::system<rigid_body, ColliderType>
    {
    private:
        using base_type = game::system<rigid_body, ColliderType>;
    public:
        using base_type::cannot_apply;
    public:
        simple_physics(i_ecs& aEcs);
        ~simple_physics();
    public:
        const system_id& id() const override;
        const i_string& name() const override;
    public:
        bool apply() override;
    public:
        bool universal_gravitation_enabled() const;
        void enable_universal_gravitation();
        void disable_universal_gravitation();
    public:
        void yield_after(std::chrono::duration<double, std::milli> aTime);
    public:
        struct meta
        {
            static const neolib::uuid& id()
            {
                if constexpr (std::is_same_v<ColliderType, box_collider_2d>)
                {
                    static const neolib::uuid sId = { 0x49443e26, 0x762e, 0x4517, 0xbbb8,{ 0xc3, 0xd6, 0x95, 0x7b, 0xe9, 0xd4 } };
                    return sId;
                }
                else if constexpr (std::is_same_v<ColliderType, box_collider_3d>)
                {
                    static const neolib::uuid sId = { 0x79b3adca, 0xbdab, 0x4d38, 0xa0eb, { 0xb5, 0x91, 0x4a, 0xb0, 0x24, 0x3e } };
                    return sId;
                }
            }
            static const i_string& name()
            {
                if constexpr (std::is_same_v<ColliderType, box_collider_2d>)
                {
                    static const string sName = "Simple Physics (2D)";
                    return sName;
                }
                else
                {
                    static const string sName = "Simple Physics (3D)";
                    return sName;
                }
            }
        };
    private:
        std::chrono::duration<double, std::milli> iYieldTime = std::chrono::duration<double, std::milli>{ 1.0 };
        neolib::ecs::time& iTime;
        neolib::ecs::clock& iWorldClock;
        game_world& iGameWorld;
        collision_detector_t<ColliderType>& iCollisionDetector;
        physics& iPhysicalConstants;
        neolib::ecs::component<neolib::ecs::entity_info>& iInfos;
        neolib::ecs::component<rigid_body>& iRigidBodies;
    };

    using simple_physics_2d = simple_physics<box_collider_2d>;
    using simple_physics_3d = simple_physics<box_collider_3d>;
}