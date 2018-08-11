// animation.hpp
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
#include <neogfx/core/colour.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_component.hpp>
#include <neogfx/game/mesh_filter.hpp>

namespace neogfx::game
{
	struct animation
	{
		std::vector<scalar> frameDurations;
		std::vector<mesh_filter> frameFilters;

		struct meta : i_component_data::meta
		{
			static const neolib::uuid& id()
			{
				static const neolib::uuid sId = { 0x57fe17a3, 0x4f8, 0x4386, 0x8dad, { 0xf2, 0x1d, 0x5f, 0xd8, 0xe2, 0x6b } };
				return sId;
			}
			static const neolib::i_string& name()
			{
				static const neolib::string sName = "Animation";
				return sName;
			}
			static uint32_t field_count()
			{
				return 2;
			}
			static component_data_field_type field_type(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
					return component_data_field_type::Scalar | component_data_field_type::Array;
				case 1:
					return component_data_field_type::ComponentData | component_data_field_type::Array;
				}
			}
			static neolib::uuid field_type_id(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
					return neolib::uuid{};
				case 1:
					return mesh_filter::meta::id();
				}
			}
			static const neolib::i_string& field_name(uint32_t aFieldIndex)
			{
				static const neolib::string sFieldNames[] =
				{
					"Frame Durations",
					"Frame Filters"
				};
				return sFieldNames[aFieldIndex];
			}
		};
	};
}