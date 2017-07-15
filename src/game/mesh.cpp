// mesh.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/game/mesh.hpp>

namespace neogfx
{
	mesh::mesh()
	{
	}

	mesh::mesh(const i_mesh& aMesh) : 
		iVertices{ aMesh.vertices() }, iFaces{ aMesh.faces() }
	{
	}

	mesh::mesh(const i_mesh& aMesh, const mat44& aTransformationMatrix) :
		iVertices{ aMesh.vertices() }, iFaces{ aMesh.faces() }, iTransformationMatrix{ aTransformationMatrix }
	{
	}

	mesh::mesh(const mesh& aMesh) :
		iVertices{ aMesh.vertices() }, iFaces{ aMesh.faces() }
	{
	}

	mesh::mesh(const mesh& aMesh, const mat44& aTransformationMatrix) :
		iVertices{ aMesh.vertices() }, iFaces{ aMesh.faces() }, iTransformationMatrix{ aTransformationMatrix }
	{
	}

	const vec3_list& mesh::vertices() const
	{
		return iVertices;
	}

	const mesh::face_list& mesh::faces() const
	{
		return iFaces;
	}

	bool mesh::has_transformation_matrix() const
	{
		return iTransformationMatrix != boost::none;
	}

	mat44 mesh::transformation_matrix() const
	{
		if (has_transformation_matrix())
			return *iTransformationMatrix;
		else
			return mat44{ { 1.0, 0.0, 0.0, 0.0}, { 0.0, 1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0, 0.0 } ,{ 0.0, 0.0, 0.0, 1.0 } };
	}

	vec3_list mesh::transformed_vertices() const
	{
		if (!has_transformation_matrix())
			return iVertices;
		else
		{
			vec3_list result;
			result.reserve(iVertices.size());
			for (auto const& v : iVertices)
				result.push_back((*iTransformationMatrix * vec4{ v.x, v.y, v.z, 0.0 }).xyz);
			return result;
		}
	}
}