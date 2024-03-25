// gradient.hpp
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

namespace neogfx::game
{
    struct gradient
    {
        neolib::cookie_ref_ptr id;
        std::optional<aabb_2d> boundingBox;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x7056b018, 0x15a1, 0x4bb1, 0x8b32, { 0xa2, 0x2e, 0x2a, 0x9f, 0xb7, 0xe7 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Gradient";
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
                    return component_data_field_type::Id;
                case 1:
                    return component_data_field_type::Aabb2d | component_data_field_type::Optional;
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
                    return neolib::uuid{};
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Id",
                    "Bounding Box"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline bool batchable(const gradient& lhs, const gradient& rhs)
    {
        return lhs.id.cookie() == rhs.id.cookie() &&
            lhs.boundingBox == rhs.boundingBox; // todo: allow batching with different bounding boxes
    }
}