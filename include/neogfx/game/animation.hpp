// animation.hpp
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
#include <neogfx/core/color.hpp>
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
            static uint32_t field_count()
            {
                return 2;
            }
            static component_data_field_type field_type(uint32_t aFieldIndex)
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
            static neolib::uuid field_type_id(uint32_t aFieldIndex)
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
            static const i_string& field_name(uint32_t aFieldIndex)
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

    struct animation
    {
        animation_frames frames;

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
            static uint32_t field_count()
            {
                return 1;
            }
            static component_data_field_type field_type(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::ComponentData | component_data_field_type::Array;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return animation_frame::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Animation"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };
 
    inline animation regular_sprite_sheet_to_animation(const vec2u32& aSpriteSheetExtents, const vec2u32& aCells, const vec2u32& aCellIndexTopLeft, const vec2u32& aCellIndexBottomRight, scalar aDefaultDuration = 0.0)
    {
        vec2 const uvCellExtents{ 1.0 / aCells.as<scalar>().x, 1.0 / aCells.as<scalar>().y };
        animation results;
        for (u32 y = aCellIndexTopLeft.y; y <= aCellIndexBottomRight.y; ++y)
        {
            for (u32 x = aCellIndexTopLeft.x; x <= aCellIndexBottomRight.x; ++x)
            {
                vec2 const uvOffset{ x * uvCellExtents.x, 1.0 - (y + 1) * uvCellExtents.y };
                results.frames.push_back(
                    animation_frame
                    {
                        aDefaultDuration,
                        mesh_filter
                        {
                            {},
                            mesh
                            {
                                { vec3{ -1.0, -1.0, 0.0 }, vec3{ 1.0, -1.0, 0.0 }, vec3{ 1.0, 1.0, 0.0 }, vec3{ -1.0, 1.0, 0.0 } },
                                { uvOffset, uvOffset + vec2{ uvCellExtents.x, 0.0 }, uvOffset + vec2{ uvCellExtents.x, uvCellExtents.y }, uvOffset + vec2{ 0.0, uvCellExtents.y } },
                                { face{ 3u, 2u, 0u }, face{ 2u, 1u, 0u } }
                            }
                        }
                    });
            }
        }
        return results;
    }

    inline animation regular_sprite_sheet_to_animation(const vec2u32& aSpriteSheetExtents, const vec2u32& aCells, scalar aDefaultDuration = 0.0)
    {
        return regular_sprite_sheet_to_animation(aSpriteSheetExtents, aCells, vec2u32{}, vec2u32{ aCells.x - 1u, aCells.y - 1u }, aDefaultDuration);
    }
        
    inline animation regular_sprite_sheet_to_animation(const material& aSpriteSheet, const vec2u32& aCells, const vec2u32& aCellIndexTopLeft, const vec2u32& aCellIndexBottomRight, scalar aDefaultDuration = 0.0)
    {
        return regular_sprite_sheet_to_animation(
            aSpriteSheet.sharedTexture ? aSpriteSheet.sharedTexture->ptr->extents.as<u32>() : aSpriteSheet.texture->extents.as<u32>(),
            aCells, aCellIndexTopLeft, aCellIndexBottomRight, aDefaultDuration);
    }

    inline animation regular_sprite_sheet_to_animation(const material& aSpriteSheet, const vec2u32& aCells, scalar aDefaultDuration = 0.0)
    {
        return regular_sprite_sheet_to_animation(
            aSpriteSheet.sharedTexture ? aSpriteSheet.sharedTexture->ptr->extents.as<u32>() : aSpriteSheet.texture->extents.as<u32>(),
            aCells, aDefaultDuration);
    }
}