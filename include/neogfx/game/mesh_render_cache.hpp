// mesh_render_cache.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
    struct mesh_render_cache
    {
        uint32_t vertexArrayIndexStart;
        uint32_t vertexArrayIndexEnd;
        bool dirty;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x4fdfc50f, 0x6562, 0x4cf1, 0x80cf, { 0x87, 0x2c, 0xff, 0xf, 0xa4, 0x2f } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "mesh_render_cache";
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
                case 1:
                    return component_data_field_type::Uint32 | component_data_field_type::Internal;
                case 2:
                    return component_data_field_type::Bool | component_data_field_type::Internal;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Vertex Array Index Start",
                    "Vertex Array Index End",
                    "Dirty"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}