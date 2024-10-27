// color.hpp
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
    struct color
    {
        vec4f rgba;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x8e422ae1, 0x7036, 0x4803, 0x8d61, { 0x71, 0xf8, 0xa2, 0xb9, 0x89, 0x6d } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Color";
                return sName;
            }
            static std::uint32_t field_count()
            {
                return 1;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Vec4f;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "RGBA",
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline bool operator==(const color& lhs, const color& rhs)
    {
        return lhs.rgba == rhs.rgba;
    }
}