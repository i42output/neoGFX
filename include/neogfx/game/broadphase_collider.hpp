// broadphase_collider.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/uuid.hpp>
#include <neolib/string.hpp>
#include <neogfx/game/i_component_data.hpp>

namespace neogfx::game
{
    struct broadphase_collider
    {
        uint64_t mask;
        std::optional<aabb> previousAabb;
        std::optional<aabb> currentAabb;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xd614481e, 0x8950, 0x4959, 0x8375, { 0xd8, 0xe0, 0x98, 0xd9, 0x5e, 0xa0 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Broadphase Collider";
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
                    return component_data_field_type::Uint64;
                case 1:
                case 2:
                    return component_data_field_type::Aabb | component_data_field_type::Optional;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Mask",
                    "Previous AABB",
                    "Current AABB",
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    struct broadphase_collider_2d
    {
        uint64_t mask;
        std::optional<aabb_2d> previousAabb;
        std::optional<aabb_2d> currentAabb;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xf13790ab, 0x9334, 0x43e4, 0xb5a, { 0x91, 0xc6, 0xfa, 0x8, 0x73, 0xf7 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Broadphase Collider 2D";
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
                    return component_data_field_type::Uint64;
                case 1:
                case 2:
                    return component_data_field_type::Aabb2d | component_data_field_type::Optional;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Mask",
                    "Previous AABB",
                    "Current AABB",
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}