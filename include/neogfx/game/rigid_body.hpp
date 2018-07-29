// rigid_body.hpp
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
#include <neolib/uuid.hpp>
#include <neolib/string.hpp>
#include <neogfx/game/i_component_data.hpp>

namespace neogfx
{
	struct rigid_body : i_component_data
	{
		vec3 position;
		vec3 angle;
		vec3 velocity;
		vec3 acceleration;
		vec3 spin;
		vec3 centreOfMass;
		scalar drag;
		scalar mass;

		struct meta
		{
			static const neolib::uuid& id()
			{
				static const neolib::uuid sId = {};
				return sId;
			}
			static const neolib::i_string& name()
			{
				static const neolib::string sName = "Rigid Body";
				return sName;
			}
			static uint32_t field_count()
			{ 
				return 8; 
			}
			static component_data_field_type field_type(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					return component_data_field_type::Vec3 | component_data_field_type::Float64;
				case 6:
				case 7:
					return component_data_field_type::Float64;
				}
			}
			static const neolib::i_string& field_name(uint32_t aFieldIndex)
			{
				static const neolib::string sFieldNames[] = 
				{
					"Position",
					"Angle",
					"Velocity",
					"Acceleration",
					"Spin",
					"Centre Of Mass",
					"Drag",
					"Mass"
				};
				return sFieldNames[aFieldIndex];
			}
		};
	};
}