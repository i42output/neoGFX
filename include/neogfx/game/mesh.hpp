// i_mesh.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.

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
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neolib/uuid.hpp>
#include <neolib/string.hpp>
#include <neogfx/game/i_shared_component_data.hpp>

namespace neogfx
{
	struct mesh
	{
		std::vector<vec3> vertices;
		std::vector<vec2> uv;
		std::vector<triangle> triangles;

		struct meta : i_shared_component_data::meta
		{
			static const neolib::uuid& id()
			{
				static const neolib::uuid sId = { 0x5a6608ed, 0x2809, 0x4c1f, 0x9620, { 0xa6, 0xf1, 0x77, 0xab, 0x9c, 0x2a } };
				return sId;
			}
			static const neolib::i_string& name()
			{
				static const neolib::string sName = "Mesh";
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
					return component_data_field_type::Vec3 | component_data_field_type::Array;
				case 1:
					return component_data_field_type::Vec2 | component_data_field_type::Array;
				case 2:
					return component_data_field_type::Triangle | component_data_field_type::Array;
				}
			}
			static const neolib::i_string& field_name(uint32_t aFieldIndex)
			{
				static const neolib::string sFieldNames[] =
				{
					"Vertices",
					"UV",
					"Triangles"
				};
				return sFieldNames[aFieldIndex];
			}
		};
	};

	inline rect bounding_rect(const mesh& aMesh)
	{
		if (aMesh.vertices.empty())
			return rect{};
		point topLeft{ aMesh.vertices[0].x, aMesh.vertices[0].y };
		point bottomRight = topLeft;
		for (auto const& v : aMesh.vertices)
		{
			topLeft.x = std::min<coordinate>(topLeft.x, v.x);
			topLeft.y = std::min<coordinate>(topLeft.y, v.y);
			bottomRight.x = std::max<coordinate>(bottomRight.x, v.x);
			bottomRight.y = std::max<coordinate>(bottomRight.y, v.y);
		}
		return rect{ topLeft, bottomRight };
	}
}