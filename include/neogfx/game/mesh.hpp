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
#include <neogfx/game/i_component_data.hpp>

namespace neogfx::game
{
	struct mesh
	{
		vertices_t vertices;
		vertices_2d_t uv;
		faces_t faces;

		struct meta : i_component_data::meta
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
					return component_data_field_type::Face | component_data_field_type::Array;
				default:
					throw invalid_field_index();
				}
			}
			static const neolib::i_string& field_name(uint32_t aFieldIndex)
			{
				static const neolib::string sFieldNames[] =
				{
					"Vertices",
					"UV",
					"Faces"
				};
				return sFieldNames[aFieldIndex];
			}
		};
	};

	inline mesh operator*(const mat44& aLhs, const mesh& aRhs)
	{
		return mesh{ aLhs * aRhs.vertices, aRhs.uv, aRhs.faces };
	}

	inline rect bounding_rect(const vertices_t& aVertices)
	{
		if (aVertices.empty())
			return rect{};
		point topLeft{ aVertices[0].x, aVertices[0].y };
		point bottomRight = topLeft;
		for (auto const& v : aVertices)
		{
			topLeft.x = std::min<coordinate>(topLeft.x, v.x);
			topLeft.y = std::min<coordinate>(topLeft.y, v.y);
			bottomRight.x = std::max<coordinate>(bottomRight.x, v.x);
			bottomRight.y = std::max<coordinate>(bottomRight.y, v.y);
		}
		return rect{ topLeft, bottomRight };
	}

	inline rect bounding_rect(const mesh& aMesh)
	{
		return bounding_rect(aMesh.vertices);
	}

	inline faces_t default_faces(const vertices_t& aVertices, std::size_t aOffset = 0)
	{
		faces_t faces;
		for (std::size_t i = aOffset; i < aVertices.size() + aOffset; i += 3)
			faces.push_back(face{ i, i + 1u, i + 2u });
		return faces;
	}
}