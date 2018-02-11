// i_mesh.hpp
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
#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/geometry.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/game/shapes.hpp>

namespace neogfx
{
	struct vertex
	{
		vec3 coordinates;
		vec2 textureCoordinates;
	};
	typedef std::vector<vertex> vertex_list;
	typedef boost::optional<vertex_list> optional_vertex_list;
	typedef vertex_list::size_type vertex_index;
	typedef std::shared_ptr<vertex_list> vertex_list_pointer;
	typedef std::array<vertex_index, 3> triangle;

	struct face
	{
		triangle vertices;
		texture_index texture;
	};

	class face_list
	{
	public:
		struct no_container : std::logic_error { no_container() : std::logic_error("neogfx::face_list::no_container") {} };
	public:
		typedef std::vector<face> container;
		typedef std::shared_ptr<container> container_pointer;
		typedef container::const_iterator const_iterator;
		typedef container::iterator iterator;
	public:
		face_list()
		{
		}
		face_list(container_pointer aFaces) :
			iFaces{ aFaces }
		{
		}
		face_list(const face_list& aOther, const_iterator aBegin, const_iterator aEnd) :
			iFaces{ aOther.iFaces }, 
			iBegin{ iFaces->begin() + (aBegin - iFaces->cbegin()) }, 
			iEnd{ iFaces->begin() + (aEnd - iFaces->cbegin()) }
		{
		}
	public:
		bool empty() const
		{
			return iFaces == nullptr || iFaces->empty();
		}
		const_iterator cbegin() const
		{
			if (iFaces == nullptr)
				throw no_container();
			return iBegin != boost::none ? *iBegin : iFaces->cbegin();
		}
		const_iterator begin() const
		{
			return cbegin();
		}
		iterator begin()
		{
			if (iFaces == nullptr)
				throw no_container();
			return iBegin != boost::none ? *iBegin : iFaces->begin();
		}
		const_iterator cend() const
		{
			if (iFaces == nullptr)
				throw no_container();
			return iEnd != boost::none ? *iEnd : iFaces->cend();
		}
		const_iterator end() const
		{
			return cend();
		}
		iterator end()
		{
			if (iFaces == nullptr)
				throw no_container();
			return iEnd != boost::none ? *iEnd : iFaces->end();
		}
		const container& faces() const
		{
			if (iFaces == nullptr)
				iFaces = std::make_shared<container>();
			return *iFaces;
		}
		container& faces()
		{
			if (iFaces == nullptr)
				iFaces = std::make_shared<container>();
			iBegin = boost::none;
			iEnd = boost::none;
			return *iFaces;
		}
	private:
		mutable container_pointer iFaces;
		boost::optional<iterator> iBegin;
		boost::optional<iterator> iEnd;
	};

	inline void add_faces(vertex_list_pointer aVertices, face_list& aFaces, const std::vector<vec3>& aShapeVertices)
	{
		const std::size_t existingVertexCount = aVertices->size();
		for (const auto& v : aShapeVertices)
			aVertices->push_back(neogfx::vertex{ v });
		for (neogfx::vertex_index vi = 1; vi + 1 < aShapeVertices.size(); ++vi)
			aFaces.faces().push_back(neogfx::face{ { existingVertexCount + 0, existingVertexCount + vi, existingVertexCount + vi + 1 } });
	}

	inline void add_faces(vertex_list_pointer aVertices, texture_list_pointer aTextures, face_list& aFaces, const neogfx::rect& aRect, const neogfx::i_texture& aTexture, bool aUpsideDown = false)
	{
		const std::size_t existingVertexCount = aVertices->size();
		aTextures->push_back(neogfx::texture_source{ neogfx::to_texture_pointer(aTexture), neogfx::optional_rect{} });
		auto rv = neogfx::rect_vertices(aRect, 0.0, neogfx::rect_type::FilledTriangles);
		const double zero = (!aUpsideDown ? 0.0 : 1.0);
		const double one = (!aUpsideDown ? 1.0 : 0.0);
		aVertices->push_back(neogfx::vertex{ rv[0], neogfx::vec2{ zero, zero } });
		aVertices->push_back(neogfx::vertex{ rv[1], neogfx::vec2{ one, zero } });
		aVertices->push_back(neogfx::vertex{ rv[2], neogfx::vec2{ zero, one } });
		aFaces.faces().push_back(neogfx::face{ neogfx::triangle{ existingVertexCount + 0, existingVertexCount + 1, existingVertexCount + 2 }, aTextures->size() - 1 });
		aVertices->push_back(neogfx::vertex{ rv[3], neogfx::vec2{ one, zero } });
		aVertices->push_back(neogfx::vertex{ rv[4], neogfx::vec2{ one, one } });
		aVertices->push_back(neogfx::vertex{ rv[5], neogfx::vec2{ zero, one } });
		aFaces.faces().push_back(neogfx::face{ neogfx::triangle{ existingVertexCount + 3, existingVertexCount + 4, existingVertexCount + 5 }, aTextures->size() - 1 });
	};

	class i_mesh
	{
	public:
		struct no_textures : std::logic_error { no_textures() : std::logic_error("neogfx::i_mesh::no_textures") {} };
	public:
		virtual vertex_list_pointer vertices() const = 0;
		virtual texture_list_pointer textures() const = 0;
		virtual face_list faces() const = 0;
		virtual face_list active_faces() const = 0;
		virtual void activate_faces(face_list aActiveFaces) const = 0;
		virtual mat44 transformation_matrix() const = 0;
		virtual const vertex_list& transformed_vertices() const = 0;
	public:
		virtual void set_vertices(vertex_list_pointer aVertices) = 0;
		virtual void set_textures(texture_list_pointer aTextures) = 0;
		virtual void set_faces(face_list aFaces) = 0;
	};

	class scoped_faces
	{
	public:
		scoped_faces(const i_mesh& aMesh, face_list::const_iterator aBegin, face_list::const_iterator aEnd) :
			iMesh{ aMesh }, iPreviouslyActiveFaces{ aMesh.active_faces() }
		{
			iMesh.activate_faces(face_list{ aMesh.faces(), aBegin, aEnd });
		}
		~scoped_faces()
		{
			iMesh.activate_faces(iPreviouslyActiveFaces);
		}
	private:
		const i_mesh& iMesh;
		face_list iPreviouslyActiveFaces;
	};

	inline rect bounding_rect(const vertex_list& aVertices)
	{
		if (aVertices.empty())
			return rect{};
		point topLeft{ aVertices[0].coordinates.x, aVertices[0].coordinates.y };
		point bottomRight = topLeft;
		for (auto const& v : aVertices)
		{
			topLeft.x = std::min<coordinate>(topLeft.x, v.coordinates.x);
			topLeft.y = std::min<coordinate>(topLeft.y, v.coordinates.y);
			bottomRight.x = std::max<coordinate>(bottomRight.x, v.coordinates.x);
			bottomRight.y = std::max<coordinate>(bottomRight.y, v.coordinates.y);
		}
		return rect{ topLeft, bottomRight };
	}
}