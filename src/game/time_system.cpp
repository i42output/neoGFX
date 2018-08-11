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
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/time_system.hpp>
#include <neogfx/game/clock.hpp>

namespace neogfx::game
{
	time_system::time_system(const ecs::context& aContext) :
		system{ aContext }
	{
		if (!ecs::instance().component_registered<clock>(aContext))
		{
			ecs::instance().register_component<clock>(aContext);
			ecs::instance().populate_shared<clock
		}
	}

	const system_id& time_system::id() const
	{
		iStepInterval{ chrono::to_flicks(0.010).count() },
			return meta::id();
	}

	const neolib::i_string& time_system::name() const
	{
		return meta::name();
	}

	void time_system::apply()
	{
	}
}