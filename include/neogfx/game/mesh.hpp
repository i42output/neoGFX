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
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neolib/uuid.hpp>
#include <neolib/string.hpp>
#include <neogfx/game/i_component_data.hpp>

namespace neogfx::game
{
    struct mesh
    {
        vertices vertices;
        vertices_2d uv;
        faces faces;

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
            static const i_string& field_name(uint32_t aFieldIndex)
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

    inline mesh operator*(const mat44& aLhs, const mesh& aRhs)
    {
        return mesh{ aLhs * aRhs.vertices, aRhs.uv, aRhs.faces };
    }

    inline rect bounding_rect(const vertices& aVertices)
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

    inline rect bounding_rect(const vertices& aVertices, const faces& aFaces, vertices::size_type aOffset = 0)
    {
        if (aVertices.empty())
            return rect{};
        point topLeft{ 
            aVertices[static_cast<vertices::size_type>(aFaces[0][0]) + aOffset].x, 
            aVertices[static_cast<vertices::size_type>(aFaces[0][0]) + aOffset].y };
        point bottomRight = topLeft;
        for (auto const& f : aFaces)
        {
            for (faces::size_type fv = 0; fv < 3; ++fv)
            {
                auto const& v = aVertices[static_cast<vertices::size_type>(f[static_cast<uint32_t>(fv)]) + aOffset];
                topLeft.x = std::min<coordinate>(topLeft.x, v.x);
                topLeft.y = std::min<coordinate>(topLeft.y, v.y);
                bottomRight.x = std::max<coordinate>(bottomRight.x, v.x);
                bottomRight.y = std::max<coordinate>(bottomRight.y, v.y);
            }
        }
        return rect{ topLeft, bottomRight };
    }

    inline rect bounding_rect(const mesh& aMesh)
    {
        return bounding_rect(aMesh.vertices);
    }

    inline faces default_faces(uint32_t aVertexCount, uint32_t aOffset = 0u)
    {
        faces faces;
        for (uint32_t i = aOffset; i < aVertexCount + aOffset; i += 3u)
            faces.push_back(face{ i, i + 1u, i + 2u });
        return faces;
    }

    inline faces default_faces(const vertices& aVertices, uint32_t aOffset = 0u)
    {
        return default_faces(static_cast<uint32_t>(aVertices.size()), aOffset);
    }
}