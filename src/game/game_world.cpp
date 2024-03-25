// game_world.cpp
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

#include <neolib/task/thread.hpp>
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/time.hpp>
#include <neogfx/game/clock.hpp>
#include <neogfx/game/game_world.hpp>

namespace neogfx::game
{
    game_world::game_world(game::i_ecs& aEcs) :
        game::system<>{ aEcs }, iUniversalGravitationEnabled{ false }
    {
        ApplyingPhysics.set_trigger_type(neolib::trigger_type::SynchronousDontQueue);
        PhysicsApplied.set_trigger_type(neolib::trigger_type::SynchronousDontQueue);
    }

    game_world::~game_world()
    {
    }

    const system_id& game_world::id() const
    {
        return meta::id();
    }

    const i_string& game_world::name() const
    {
        return meta::name();
    }

    bool game_world::apply()
    {
        // do nothing
        return true;
    }

    void game_world::set_time_step(double aTimestep_s)
    {
        ecs().system<game::time>();
        ecs().shared_component<game::clock>().component_data().begin()->second.timestep = chrono::to_flicks(aTimestep_s).count();
    }

    bool game_world::universal_gravitation_enabled() const
    {
        return iUniversalGravitationEnabled;
    }

    void game_world::enable_universal_gravitation()
    {
        iUniversalGravitationEnabled = true;
    }

    void game_world::disable_universal_gravitation()
    {
        iUniversalGravitationEnabled = false;
    }

}