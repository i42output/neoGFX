// sprite.hpp
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
    struct sprite
    {
        vertices path;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xd61d920b, 0xde71, 0x400d, 0x9532, { 0x3b, 0x76, 0x1f, 0x26, 0x65, 0x56 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Sprite";
                return sName;
            }
            static uint32_t field_count()
            {
                return 1;
            }
            static component_data_field_type field_type(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Vec3 | component_data_field_type::Array;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Path"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}