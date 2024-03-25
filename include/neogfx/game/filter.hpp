// filter.hpp
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

#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/primitives.hpp>

namespace neogfx::game
{
    struct filter
    {
        shader_filter type;
        scalar arg1;
        scalar arg2;
        scalar arg3;
        scalar arg4;
        std::optional<aabb_2d> boundingBox;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x91d07f77, 0x775c, 0x42f5, 0x881f, { 0x9, 0x6e, 0x1d, 0xa9, 0xba, 0x9 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Filter";
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
                    return component_data_field_type::Enum;
                case 1:
                case 2:
                case 3:
                case 4:
                    return component_data_field_type::Scalar;
                case 5:
                    return component_data_field_type::Aabb2d | component_data_field_type::Optional;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Type",
                    "Arg1",
                    "Arg2",
                    "Arg3",
                    "Arg4",
                    "Bounding Box"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline bool batchable(const filter& lhs, const filter& rhs)
    {
        // todo
        return lhs.type == rhs.type &&
            lhs.arg1 == rhs.arg1 &&
            lhs.arg2 == rhs.arg2 &&
            lhs.arg3 == rhs.arg3 &&
            lhs.arg4 == rhs.arg4;
    }
}