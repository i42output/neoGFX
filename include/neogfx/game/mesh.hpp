// mesh.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present Leigh Johnston
  
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
	class mesh : public virtual i_mesh
	{
	public:
		mesh();
		mesh(const i_mesh& aMesh);
		mesh(const i_mesh& aMesh, const mat44& aTransformationMatrix);
		mesh(const mesh& aMesh);
		mesh(const mesh& aMesh, const mat44& aTransformationMatrix);
	public:
		vertex_list_pointer vertices() const override;
		texture_list_pointer textures() const override;
		face_list faces() const override;
		face_list active_faces() const override;
		void activate_faces(face_list aActiveFaces) const override;
		mat44 transformation_matrix() const override;
		const vertex_list& transformed_vertices() const override;
	public:
		void set_vertices(vertex_list_pointer aVertices) override;
		void set_textures(texture_list_pointer aTextures) override;
		void set_faces(face_list aFaces) override;
	private:
		vertex_list_pointer iVertices;
		texture_list_pointer iTextures;
		face_list iFaces;
		mat44 iTransformationMatrix;
		mutable vertex_list iTransformedVertices;
		mutable face_list iActiveFaces;
	};
}