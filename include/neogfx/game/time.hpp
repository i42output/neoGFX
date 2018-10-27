// time.hpp
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
#include <neogfx/game/system.hpp>

namespace neogfx::game
{
	class time : public system
	{

	public:
		time(game::i_ecs& aEcs);
	public:
		const system_id& id() const override;
		const neolib::i_string& name() const override;
	public:
		void apply() override;
	public:
		step_time system_time() const;
		step_time world_time() const;
	public:
		struct meta
		{
			static const neolib::uuid& id()
			{
				static const neolib::uuid sId = { 0x714a0e4a, 0xd0be, 0x4737, 0xbd25, { 0xe8, 0x3e, 0x2a, 0x5c, 0xd7, 0x65 } };
				return sId;
			}
			static const neolib::i_string& name()
			{
				static const neolib::string sName = "Time";
				return sName;
			}
		};
	private:
		mutable optional_step_time iSystemTimeOffset;
	};
}