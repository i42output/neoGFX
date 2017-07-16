// i_mesh.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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

namespace neogfx
{
	class i_mesh
	{
	public:
		typedef std::array<vec3, 3> triangle;
		struct vertex
		{
			vec3 coordinates;
			vec2 textureCoordinates;
		};
		typedef std::vector<vertex> vertex_list;
		typedef vertex_list::size_type vertex_index;
		typedef std::array<vertex_index, 3> face;
		typedef std::vector<face> face_list;
	public:
		virtual const vertex_list& vertices() const = 0;
		virtual const face_list& faces() const = 0;
		virtual mat44 transformation_matrix() const = 0;
		virtual vertex_list transformed_vertices() const = 0;
	};
}