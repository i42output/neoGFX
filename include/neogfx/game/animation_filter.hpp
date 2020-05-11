// animation_filter.hpp
/*
neogfx C++ GUI Library
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
#include <neolib/uuid.hpp>
#include <neolib/string.hpp>
#include <neogfx/game/animation.hpp>

namespace neogfx::game
{
    struct animation_filter
    {
        shared<animation> sharedAnimation;
        animation animation;
        scalar currentFrame;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xe876ec93, 0x21b2, 0x410e, 0xbd81, { 0x8c, 0x1d, 0x9, 0xec, 0xbc, 0x45 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Animation Filter";
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
                    return component_data_field_type::ComponentData | component_data_field_type::Shared;
                case 1:
                    return component_data_field_type::ComponentData;
                case 2:
                    return component_data_field_type::Scalar;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                case 1:
                    return animation::meta::id();
                case 2:
                    return neolib::uuid{};
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Shared Animation",
                    "Animation",
                    "Current Frame"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}