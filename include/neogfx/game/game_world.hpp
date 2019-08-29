// game_world.hpp
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
#include <neogfx/core/event.hpp>
#include <neogfx/game/system.hpp>

namespace neogfx::game
{
    class game_world : public system
    {
    public:
        define_event(ApplyingPhysics, step_time)
        define_event(PhysicsApplied, step_time)
    public:
        game_world(game::i_ecs& aEcs);
        ~game_world();
    public:
        const system_id& id() const override;
        const neolib::i_string& name() const override;
    public:
        void apply() override;
    public:
        bool universal_gravitation_enabled() const;
        void enable_universal_gravitation();
        void disable_universal_gravitation();
    public:
        struct meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x60495660, 0x7da9, 0x4016, 0x841, { 0x7f, 0x3a, 0xae, 0x7d, 0x1e, 0x53 } };
                return sId;
            }
            static const neolib::i_string& name()
            {
                static const neolib::string sName = "Game";
                return sName;
            }
        };
    };
}