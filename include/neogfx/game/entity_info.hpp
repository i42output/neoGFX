// entity_info.hpp
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
#include <neogfx/game/ecs_ids.hpp>

namespace neogfx::game
{
    struct entity_info
    {
        neolib::uuid archetypeId;
        int64_t creationTime;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x867e30c2, 0xaf8e, 0x452e, 0xa542, { 0xd, 0xd0, 0xd1, 0x1, 0xe4, 0x2d } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Entity Info";
                return sName;
            }
            static uint32_t field_count()
            { 
                return 2; 
            }
            static component_data_field_type field_type(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Uuid;
                case 1:
                    return component_data_field_type::Int64;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] = 
                {
                    "Archetype Id",
                    "Creation Time",
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}