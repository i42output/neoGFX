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
	mesh::mesh() : 
		iTransformationMatrix{ { 1.0, 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0, 0.0 }, { 0.0, 0.0, 0.0, 1.0 } }
	{
	}

	mesh::mesh(const i_mesh& aMesh) : 
		iVertices{ aMesh.vertices() }, iFaces{ aMesh.faces() }, iTransformationMatrix{ aMesh.transformation_matrix() }
	{
	}

	mesh::mesh(const i_mesh& aMesh, const mat44& aTransformationMatrix) :
		iVertices{ aMesh.vertices() }, iFaces{ aMesh.faces() }, iTransformationMatrix{ aMesh.transformation_matrix() ^ aTransformationMatrix }
	{
	}

	mesh::mesh(const mesh& aMesh) :
		iVertices{ aMesh.vertices() }, iFaces{ aMesh.faces() }, iTransformationMatrix{ aMesh.transformation_matrix() }
	{
	}

	mesh::mesh(const mesh& aMesh, const mat44& aTransformationMatrix) :
		iVertices{ aMesh.vertices() }, iFaces{ aMesh.faces() }, iTransformationMatrix{ aMesh.transformation_matrix() ^ aTransformationMatrix }
	{
	}

	const mesh::vertex_list& mesh::vertices() const
	{
		return iVertices;
	}

	const mesh::face_list& mesh::faces() const
	{
		return iFaces;
	}

	mat44 mesh::transformation_matrix() const
	{
		return iTransformationMatrix;
	}

	mesh::vertex_list mesh::transformed_vertices() const
	{
		mesh::vertex_list result;
		result.reserve(iVertices.size());
		for (auto const& v : iVertices)
			result.push_back(vertex{ (transformation_matrix() * vec4{ v.coordinates.x, v.coordinates.y, v.coordinates.z, 1.0 }).xyz, v.textureCoordinates });
		return result;
	}
}