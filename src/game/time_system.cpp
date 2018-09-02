// time_system.cpp
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
#include <neogfx/game/chrono.hpp>
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/time_system.hpp>
#include <neogfx/game/clock.hpp>

namespace neogfx::game
{
	time_system::time_system(game::i_ecs& aEcs) :
		system{ aEcs }
	{
		if (!ecs().component_registered<clock>())
		{
			ecs().register_component<clock>();
			ecs().populate_shared<clock>(clock{ 0ll, chrono::to_flicks(0.010).count() });
			apply();
		}
	}

	const system_id& time_system::id() const
	{
		return meta::id();
	}

	const neolib::i_string& time_system::name() const
	{
		return meta::name();
	}

	void time_system::apply()
	{
		auto& worldClock = ecs().shared_component<clock>().component_data()[0];
		worldClock.time = system_time();
	}

	step_time time_system::system_time() const
	{
		auto& worldClock = ecs().shared_component<clock>().component_data()[0];
		return to_step_time(
			chrono::to_seconds(std::chrono::duration_cast<chrono::flicks>(std::chrono::high_resolution_clock::now().time_since_epoch())),
			worldClock.timeStep);
	}

	step_time time_system::world_time() const
	{
		auto& worldClock = ecs().shared_component<clock>().component_data()[0];
		return worldClock.time;
	}
}