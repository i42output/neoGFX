// box_collider.hpp
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
    struct box_collider
    {
        uint64_t mask;
        std::optional<aabb> untransformedAabb;
        std::optional<aabb> previousAabb;
        std::optional<aabb> currentAabb;
        uint32_t collisionEventId;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x55468971, 0xb33e, 0x4e54, 0xa563, { 0xb8, 0x98, 0x3f, 0x22, 0xa, 0xfc } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Box Collider";
                return sName;
            }
            static uint32_t field_count()
            {
                return 5;
            }
            static component_data_field_type field_type(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Uint64;
                case 1:
                case 2:
                case 3:
                    return component_data_field_type::Aabb | component_data_field_type::Optional | component_data_field_type::Internal;
                case 4:
                    return component_data_field_type::Uint32 | component_data_field_type::Internal;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Mask",
                    "AABB (Untransformed)",
                    "AABB (Previous)",
                    "AABB (Current)",
                    "Collision Event Id"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    struct box_collider_2d
    {
        uint64_t mask;
        std::optional<aabb_2d> untransformedAabb;
        std::optional<aabb_2d> previousAabb;
        std::optional<aabb_2d> currentAabb;
        uint32_t collisionEventId;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xc3dcb106, 0xc862, 0x42a7, 0xa07b, { 0x90, 0x7f, 0x7e, 0x47, 0x17, 0x2a } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Box Collider (2D)";
                return sName;
            }
            static uint32_t field_count()
            {
                return 5;
            }
            static component_data_field_type field_type(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Uint64;
                case 1:
                case 2:
                case 3:
                    return component_data_field_type::Aabb2d | component_data_field_type::Optional | component_data_field_type::Internal;
                case 4:
                    return component_data_field_type::Uint32 | component_data_field_type::Internal;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Mask",
                    "AABB (Untransformed)",
                    "AABB (Previous)",
                    "AABB (Current)",
                    "Collision Event Id"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}