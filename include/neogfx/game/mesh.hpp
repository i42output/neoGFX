// mesh.hpp
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
#include "i_mesh.hpp"

namespace neogfx
{
	class mesh : public i_mesh
	{
	public:
		mesh();
		mesh(const i_mesh& aMesh);
		mesh(const i_mesh& aMesh, const mat44& aTransformationMatrix);
		mesh(const mesh& aMesh);
		mesh(const mesh& aMesh, const mat44& aTransformationMatrix);
	public:
		const vertex_list& vertices() const override;
		const face_list& faces() const override;
		mat44 transformation_matrix() const override;
		vertex_list transformed_vertices() const override;
	private:
		vertex_list iVertices;
		face_list iFaces;
		mat44 iTransformationMatrix;
	};
}