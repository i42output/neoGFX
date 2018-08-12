// material.hpp
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
#include <neogfx/game/component.hpp>
#include <neogfx/game/colour.hpp>
#include <neogfx/game/gradient.hpp>
#include <neogfx/game/texture.hpp>

namespace neogfx::game
{
	struct material
	{
		std::optional<colour> colour;
		std::optional<gradient> gradient;
		std::optional<texture> texture;

		struct meta : i_component_data::meta
		{
			static const neolib::uuid& id()
			{
				static const neolib::uuid sId = { 0x5e04e3ad, 0xb4dd, 0x4bd2, 0x888d, { 0xaa, 0x58, 0xe9, 0x4f, 0x3a, 0x5e } };
				return sId;
			}
			static const neolib::i_string& name()
			{
				static const neolib::string sName = "Material";
				return sName;
			}
			static uint32_t field_count()
			{
				return 3;
			}
			static component_data_field_type field_type(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
				case 1:
				case 2:
					return component_data_field_type::ComponentData | component_data_field_type::Optional;
				}
			}
			static neolib::uuid field_type_id(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
					return colour::meta::id();
				case 1:
					return gradient::meta::id();
				case 2:
					return texture::meta::id();
				}
			}
			static const neolib::i_string& field_name(uint32_t aFieldIndex)
			{
				static const neolib::string sFieldNames[] =
				{
					"Colour",
					"Gradient",
					"Texture"
				};
				return sFieldNames[aFieldIndex];
			}
		};
	};
}