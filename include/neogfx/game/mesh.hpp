// i_mesh.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.

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

#include <neolib/core/uuid.hpp>
#include <neolib/core/string.hpp>

#include <neogfx/core/numerical.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/game/i_ecs.hpp>
#include <neogfx/game/i_component_data.hpp>

namespace neogfx::game
{
    using vertices = std::vector<vec3f>;
    using vertices_2d = std::vector<vec2f>;

    struct mesh
    {
        vertices vertices; // todo: neolib::vector not std::vector (modify neolib::vector copy ctor to make plugin compatible; use ref_ptr<i_vector> here?)
        vertices_2d uv; // todo: neolib::vector not std::vector (modify neolib::vector copy ctor to make plugin compatible; use ref_ptr<i_vector> here?)
        faces faces; // todo: neolib::vector not std::vector (modify neolib::vector copy ctor to make plugin compatible; use ref_ptr<i_vector> here?)

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x5a6608ed, 0x2809, 0x4c1f, 0x9620, { 0xa6, 0xf1, 0x77, 0xab, 0x9c, 0x2a } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Mesh";
                return sName;
            }
            static std::uint32_t field_count()
            {
                return 3;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
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
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Vertices",
                    "UV",
                    "Faces"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline mesh operator*(const mat44f& aLhs, const mesh& aRhs)
    {
        return mesh{ aLhs * aRhs.vertices, aRhs.uv, aRhs.faces };
    }

    inline rect bounding_rect(const vertices& aVertices, const mat44f& aTransformation = mat44f::identity())
    {
        if (aVertices.empty())
            return rect{};
        point topLeft{ aTransformation * aVertices[0] };
        point bottomRight;
        for (auto const& v : aVertices)
        {
            auto const tv = aTransformation * v;
            topLeft.x = std::min<coordinate>(topLeft.x, tv.x);
            topLeft.y = std::min<coordinate>(topLeft.y, tv.y);
            topLeft.z = std::min<coordinate>(topLeft.z, tv.z);
            bottomRight.x = std::max<coordinate>(bottomRight.x, tv.x);
            bottomRight.y = std::max<coordinate>(bottomRight.y, tv.y);
            bottomRight.z = std::max<coordinate>(bottomRight.z, tv.z);
        }
        return rect{ topLeft, bottomRight };
    }

    template <typename Container, typename T>
    inline rect bounding_rect(const Container& aVertices, const basic_matrix<T, 4, 4>& aTransformation = basic_matrix<T, 4, 4>::identity())
    {
        if (aVertices.empty())
            return rect{};
        point topLeft{ aTransformation * aVertices[0].xyz };
        point bottomRight;
        for (auto const& v : aVertices)
        {
            auto const tv = aTransformation * v.xyz;
            topLeft.x = std::min<coordinate>(topLeft.x, tv.x);
            topLeft.y = std::min<coordinate>(topLeft.y, tv.y);
            bottomRight.x = std::max<coordinate>(bottomRight.x, tv.x);
            bottomRight.y = std::max<coordinate>(bottomRight.y, tv.y);
        }
        return rect{ topLeft, bottomRight };
    }

    inline rect bounding_rect(const vertices& aVertices, const faces& aFaces, const mat44f& aTransformation = mat44f::identity(), vertices::size_type aOffset = 0)
    {
        if (aVertices.empty())
            return rect{};
        point topLeft{ aTransformation * aVertices[static_cast<vertices::size_type>(aFaces[0][static_cast<std::uint32_t>(0)]) + aOffset] };
        point bottomRight = topLeft;
        for (auto const& f : aFaces)
        {
            for (faces::size_type fv = 0; fv < 3; ++fv)
            {
                auto const& v = aTransformation * aVertices[static_cast<vertices::size_type>(f[static_cast<std::uint32_t>(fv)]) + aOffset];
                topLeft.x = std::min<coordinate>(topLeft.x, v.x);
                topLeft.y = std::min<coordinate>(topLeft.y, v.y);
                bottomRight.x = std::max<coordinate>(bottomRight.x, v.x);
                bottomRight.y = std::max<coordinate>(bottomRight.y, v.y);
            }
        }
        return rect{ topLeft, bottomRight };
    }

    template <typename Container, typename T>
    inline rect bounding_rect(const Container& aVertices, const faces& aFaces, const basic_matrix<T, 4, 4>& aTransformation = basic_matrix<T, 4, 4>::identity(), vertices::size_type aOffset = 0)
    {
        if (aVertices.empty())
            return rect{};
        point topLeft{ aTransformation * aVertices[static_cast<typename Container::size_type>(aFaces[0][static_cast<std::uint32_t>(0)]) + aOffset].xyz };
        point bottomRight = topLeft;
        for (auto const& f : aFaces)
        {
            for (faces::size_type fv = 0; fv < 3; ++fv)
            {
                auto const& v = aTransformation * aVertices[static_cast<typename Container::size_type>(f[static_cast<std::uint32_t>(fv)]) + aOffset].xyz;
                topLeft.x = std::min<coordinate>(topLeft.x, v.x);
                topLeft.y = std::min<coordinate>(topLeft.y, v.y);
                bottomRight.x = std::max<coordinate>(bottomRight.x, v.x);
                bottomRight.y = std::max<coordinate>(bottomRight.y, v.y);
            }
        }
        return rect{ topLeft, bottomRight };
    }

    inline rect bounding_rect(const mesh& aMesh, const mat44& aTransformation = mat44::identity())
    {
        return bounding_rect(aMesh.vertices);
    }

    inline faces default_faces(std::uint32_t aVertexCount, std::uint32_t aOffset = 0u)
    {
        faces faces;
        for (std::uint32_t i = aOffset; i < aVertexCount + aOffset; i += 3u)
            faces.push_back(face{ i, i + 1u, i + 2u });
        return faces;
    }

    template <typename T>
    inline faces default_faces(const std::vector<T>& aVertices, std::uint32_t aOffset = 0u)
    {
        return default_faces(static_cast<std::uint32_t>(aVertices.size()), aOffset);
    }
}