// model_transformation.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <vector>

#include <neolib/core/uuid.hpp>
#include <neolib/core/string.hpp>

#include <neogfx/game/i_component_data.hpp>

namespace neogfx::game
{
    // Opt-in GPU model transformation. An entity with this component has its vertices cached between
    // frames (they are only rebuilt when its render cache is made dirty) and transformed on the GPU by
    // a per-frame table of model matrices, so moving it does not touch its vertices:
    //
    //     position = origin * rigid_body * mesh_filter::transformation * matrix * skinned(vertex)
    //     skinned(vertex) = sum(vertexWeights[v][i] * joints[vertexJoints[v][i]] * vertex) (or vertex if not skinned)
    //
    // vertexJoints/vertexWeights/vertexNormals are indexed as the mesh's vertices; changing them (or the mesh,
    // or the material) requires the entity's render cache to be made dirty as usual. vertexNormals (model space,
    // for lighting) are calculated from the mesh's faces if not supplied.
    struct model_transformation
    {
        mat44f matrix = mat44f::identity();
        std::vector<mat44f> joints;
        std::vector<vec4f> vertexJoints;
        std::vector<vec4f> vertexWeights;
        std::vector<vec3f> vertexNormals;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x2f5bf228, 0x171c, 0x49e9, 0x8cd6, { 0x34, 0x4b, 0x32, 0x03, 0xf5, 0xef } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Model Transformation";
                return sName;
            }
            static std::uint32_t field_count()
            {
                return 5;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Mat44f;
                case 1:
                    return component_data_field_type::Mat44f | component_data_field_type::Array;
                case 2:
                case 3:
                    return component_data_field_type::Vec4f | component_data_field_type::Array;
                case 4:
                    return component_data_field_type::Vec3f | component_data_field_type::Array;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Matrix",
                    "Joints",
                    "Vertex Joints",
                    "Vertex Weights",
                    "Vertex Normals"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}
