// material.hpp
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
#include <neogfx/core/color.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/component.hpp>
#include <neogfx/game/color.hpp>
#include <neogfx/game/gradient.hpp>
#include <neogfx/game/texture.hpp>

namespace neogfx::game
{
    struct material
    {
        std::optional<color> color;
        std::optional<gradient> gradient;
        std::optional<shared<texture>> sharedTexture;
        std::optional<texture> texture;
        std::optional<shader_effect> shaderEffect;
        bool subpixel;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x5e04e3ad, 0xb4dd, 0x4bd2, 0x888d, { 0xaa, 0x58, 0xe9, 0x4f, 0x3a, 0x5e } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Material";
                return sName;
            }
            static uint32_t field_count()
            {
                return 6;
            }
            static component_data_field_type field_type(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                case 1:
                    return component_data_field_type::ComponentData | component_data_field_type::Optional;
                case 2:
                    return component_data_field_type::ComponentData | component_data_field_type::Optional | component_data_field_type::Shared;
                case 3:
                    return component_data_field_type::ComponentData | component_data_field_type::Optional;
                case 4:
                    return component_data_field_type::Enum | component_data_field_type::Uint32 | component_data_field_type::Optional;
                case 5:
                    return component_data_field_type::Bool;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return color::meta::id();
                case 1:
                    return gradient::meta::id();
                case 2:
                case 3:
                    return texture::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Color",
                    "Gradient",
                    "Shared Texture",
                    "Texture",
                    "Shader Effect",
                    "Subpixel"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline bool batchable(const material& lhs, const material& rhs)
    {
        return batchable(lhs.gradient, rhs.gradient) &&
            batchable(lhs.sharedTexture, rhs.sharedTexture) &&
            batchable(lhs.texture, rhs.texture) &&
            lhs.shaderEffect == rhs.shaderEffect &&
            lhs.subpixel == rhs.subpixel;
    }
}