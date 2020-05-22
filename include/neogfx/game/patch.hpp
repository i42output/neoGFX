// patch.hpp
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
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_component_data.hpp>
#include <neogfx/game/material.hpp>

namespace neogfx::game
{
    struct patch
    {
        material material;
        faces faces;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x2a4e96aa, 0x2e1e, 0x48d7, 0xa384, { 0xb, 0xe8, 0x79, 0x3a, 0x7e, 0x94 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Patch";
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
                    return component_data_field_type::ComponentData;
                case 1:
                    return component_data_field_type::Face | component_data_field_type::Array;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return material::meta::id();
                case 1:
                    return neolib::uuid{};
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Material",
                    "Faces"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    typedef std::vector<patch> patches;
}