// mesh.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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
		iVertices{ aMesh.vertices() }, iTextures{ aMesh.textures() }, iFaces{ aMesh.faces() }, iTransformationMatrix{ aMesh.transformation_matrix() }
	{
	}

	mesh::mesh(const i_mesh& aMesh, const mat44& aTransformationMatrix) :
		iVertices{ aMesh.vertices() }, iTextures{ aMesh.textures() }, iFaces{ aMesh.faces() }, iTransformationMatrix{ aTransformationMatrix * aMesh.transformation_matrix() }
	{
	}

	mesh::mesh(const mesh& aMesh) :
		iVertices{ aMesh.vertices() }, iTextures{ aMesh.textures() }, iFaces{ aMesh.faces() }, iTransformationMatrix{ aMesh.transformation_matrix() }
	{
	}

	mesh::mesh(const mesh& aMesh, const mat44& aTransformationMatrix) :
		iVertices{ aMesh.vertices() }, iTextures{ aMesh.textures() }, iFaces{ aMesh.faces() }, iTransformationMatrix{ aTransformationMatrix * aMesh.transformation_matrix() }
	{
	}

	vertex_list_pointer mesh::vertices() const
	{
		return iVertices;
	}

	texture_list_pointer mesh::textures() const
	{
		return iTextures;
	}

	face_list mesh::faces() const
	{
		return iActiveFaces.empty() ? iFaces : iActiveFaces;
	}

	face_list mesh::active_faces() const
	{
		return iActiveFaces;
	}

	void mesh::activate_faces(face_list aActiveFaces) const
	{
		iActiveFaces = aActiveFaces;
	}

	mat44 mesh::transformation_matrix() const
	{
		return iTransformationMatrix;
	}

	const vertex_list& mesh::transformed_vertices() const
	{
		iTransformedVertices.reserve(iVertices->size());
		iTransformedVertices.clear();
		for (auto const& v : *iVertices)
			iTransformedVertices.push_back(vertex{ (transformation_matrix() * vec4{ v.coordinates.x, v.coordinates.y, v.coordinates.z, 1.0 }).xyz, v.textureCoordinates });
		return iTransformedVertices;
	}

	void mesh::set_vertices(vertex_list_pointer aVertices)
	{
		iVertices = aVertices;
	}

	void mesh::set_textures(texture_list_pointer aTextures)
	{
		iTextures = aTextures;
	}

	void mesh::set_faces(face_list aFaces)
	{
		iFaces = aFaces;
	}
}