// outline.hpp
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

#include <neogfx/gfx/color.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/component.hpp>
#include <neogfx/game/color.hpp>
#include <neogfx/game/gradient.hpp>
#include <neogfx/game/texture.hpp>

namespace neogfx::game
{
    struct outline
    {
        scalar width;
        bool antiAliased;
        std::optional<color> color;
        std::optional<gradient> gradient;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xcf2bbde0, 0xc7be, 0x48a1, 0xb9dd, { 0x1, 0xc7, 0x6, 0xfb, 0x7b, 0x8d } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Outline";
                return sName;
            }
            static std::uint32_t field_count()
            {
                return 4;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Scalar;
                case 1:
                    return component_data_field_type::Bool;
                case 2:
                case 3:
                    return component_data_field_type::ComponentData | component_data_field_type::Optional;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 2:
                    return color::meta::id();
                case 3:
                    return gradient::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Width",
                    "Antialiased"
                    "Color",
                    "Gradient"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}