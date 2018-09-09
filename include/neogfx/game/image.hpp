// image.hpp
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
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/component.hpp>
#include <neogfx/game/i_component_data.hpp>

namespace neogfx::game
{
	struct image
	{
		std::string uri;
		scalar dpiScaleFactor;
		texture_sampling textureSampling;
		id_t id;

		struct meta : i_component_data::meta
		{
			static const neolib::uuid& id()
			{
				static const neolib::uuid sId = { 0x10d54723, 0xbf6c, 0x4346, 0xbdb, { 0xf, 0x6d, 0x38, 0x91, 0x9b, 0xb7 } };
				return sId;
			}
			static const neolib::i_string& name()
			{
				static const neolib::string sName = "Image";
				return sName;
			}
			static uint32_t field_count()
			{
				return 4;
			}
			static component_data_field_type field_type(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
					return component_data_field_type::ComponentData | component_data_field_type::Shared | component_data_field_type::Optional;
				case 1:
					return component_data_field_type::Scalar;
				case 2:
					return component_data_field_type::Uint32 | component_data_field_type::Enum;
				case 3:
					return component_data_field_type::Id;
				default:
					throw invalid_field_index();
				}
			}
			static neolib::uuid field_type_id(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
					return image::meta::id();
				case 1:
				case 2:
				case 3:
					return neolib::uuid{};
				default:
					throw invalid_field_index();
				}
			}
			static const neolib::i_string& field_name(uint32_t aFieldIndex)
			{
				static const neolib::string sFieldNames[] =
				{
					"Image",
					"DPI Scale Factor",
					"Id"
				};
				return sFieldNames[aFieldIndex];
			}
		};
	};
}