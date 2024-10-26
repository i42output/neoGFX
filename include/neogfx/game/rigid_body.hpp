// rigid_body.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/game/i_component_data.hpp>

namespace neogfx::game
{
    struct rigid_body
    {
        vec3f position;
        float mass;
        vec3f velocity;
        vec3f acceleration;
        vec3f angle;
        vec3f spin;
        vec3f centerOfMass;
        float drag;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xf0481779, 0xc203, 0x4c7c, 0x9d8d, { 0x9d, 0x3d, 0xaf, 0x34, 0x71, 0x58 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Rigid Body";
                return sName;
            }
            static std::uint32_t field_count()
            { 
                return 8; 
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Vec3f;
                case 1:
                    return component_data_field_type::Float32;
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                    return component_data_field_type::Vec3f;
                case 7:
                    return component_data_field_type::Float32;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] = 
                {
                    "Position",
                    "Mass",
                    "Velocity",
                    "Acceleration",
                    "Angle",
                    "Spin",
                    "Center Of Mass",
                    "Drag"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline mat44f to_transformation_matrix(const game::rigid_body& aRigidBody, bool aIncludeTranslation = true)
    {
        float az = aRigidBody.angle.z;
        // todo: following rotation is 2D, make it 3D...
        return aIncludeTranslation ?
            mat44
            {
                { std::cos(az), std::sin(az), 0.0f, 0.0f },
                { -std::sin(az), std::cos(az), 0.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f, 0.0f },
                { aRigidBody.position.x, aRigidBody.position.y, aRigidBody.position.z, 1.0f }
            } :
            mat44
            {
                { std::cos(az), std::sin(az), 0.0f, 0.0f },
                { -std::sin(az), std::cos(az), 0.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f, 0.0f },
                { 0.0f, 0.0f, 0.0f, 1.0f }
            };
    }
}