// collider.hpp
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
    struct box_collider
    {
        vec3 origin;
        vec3 size;
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
                static const string sName = "Collider";
                return sName;
            }
            static uint32_t field_count()
            {
                return 4;
            }
            static component_data_field_type field_type(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                case 1:
                    return component_data_field_type::Vec3;
                case 3:
                    return component_data_field_type::Uint32 | component_data_field_type::Internal;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Origin",
                    "Size",
                    "Collision Event Id",
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}