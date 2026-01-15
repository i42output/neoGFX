// animation.hpp
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
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_component.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/material.hpp>

namespace neogfx::game
{
    struct animation_frame
    {
        scalar duration;
        mesh_filter filter;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x31bf1ad4, 0x9d06, 0x409d, 0xbc4f, { 0x4f, 0x80, 0x86, 0x38, 0xfd, 0x4f } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Animation Frame";
                return sName;
            }
            static std::uint32_t field_count()
            {
                return 2;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Scalar;
                case 1:
                    return component_data_field_type::ComponentData;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return neolib::uuid{};
                case 1:
                    return mesh_filter::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Duration",
                    "Filter"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    typedef std::vector<animation_frame> animation_frames;

    struct tween
    {
        i32 i; // todo

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x65918d0e, 0x87a8, 0x42cd, 0x8203, { 0x19, 0xe5, 0xab, 0xef, 0x80, 0xe } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Tween";
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
                    return component_data_field_type::Int32;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return neolib::uuid{};
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Todo"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    struct tween_matrix
    {
        i32 i; // todo

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x11161f9a, 0x4708, 0x4ed3, 0xadea, { 0x0, 0x5c, 0xef, 0x18, 0xf6, 0xda } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Tween Matrix";
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
                    return component_data_field_type::Int32;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return tween::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Todo"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    struct animation_tween
    {
        scalar duration;
        tween_matrix tweenMatrix;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xc4981e75, 0x2199, 0x4a77, 0xaf54, { 0x7f, 0x67, 0xdd, 0xd8, 0x6c, 0x71 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Animation Tween";
                return sName;
            }
            static std::uint32_t field_count()
            {
                return 2;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Scalar;
                case 1:
                    return component_data_field_type::ComponentData;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return neolib::uuid{};
                case 1:
                    return tween_matrix::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Duration",
                    "Tween Matrix"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    struct animation
    {
        std::optional<animation_frames> frames;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x164a76c6, 0x7081, 0x4b38, 0xb3ec, { 0x5b, 0xf9, 0xe0, 0x2, 0xe5, 0xbf } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Animation";
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
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::Optional;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return animation_frame::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Animation"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
}