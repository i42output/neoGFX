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

#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/component.hpp>
#include <neogfx/game/color.hpp>
#include <neogfx/game/gradient.hpp>
#include <neogfx/game/texture.hpp>

namespace neogfx::game
{
    // glTF metallic-roughness material parameters, beyond the base colour (material::color and material::texture), for physically
    // based shading (see i_pbr_shader); colour factors are linear. The textures use the base colour texture's coordinates.
    struct pbr_material
    {
        scalar metallic = 1.0;
        scalar roughness = 1.0;
        scalar normalScale = 1.0;
        scalar occlusionStrength = 1.0;
        vec3 emissive;
        std::optional<scalar> alphaCutoff;
        bool doubleSided = false;
        std::optional<texture> metallicRoughnessTexture;
        std::optional<texture> normalTexture;
        std::optional<texture> occlusionTexture;
        std::optional<texture> emissiveTexture;

        auto operator<=>(pbr_material const&) const = default;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x3c6f1a52, 0x8e4d, 0x4b7a, 0x9d21, { 0x5f, 0x0b, 0xc4, 0x7e, 0x13, 0xa9 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "PBR Material";
                return sName;
            }
            static std::uint32_t field_count()
            {
                return 11;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                case 1:
                case 2:
                case 3:
                    return component_data_field_type::Scalar;
                case 4:
                    return component_data_field_type::Vec3;
                case 5:
                    return component_data_field_type::Scalar | component_data_field_type::Optional;
                case 6:
                    return component_data_field_type::Bool;
                case 7:
                case 8:
                case 9:
                case 10:
                    return component_data_field_type::ComponentData | component_data_field_type::Optional;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                    return neolib::uuid{};
                case 7:
                case 8:
                case 9:
                case 10:
                    return texture::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Metallic",
                    "Roughness",
                    "Normal Scale",
                    "Occlusion Strength",
                    "Emissive",
                    "Alpha Cutoff",
                    "Double Sided",
                    "Metallic Roughness Texture",
                    "Normal Texture",
                    "Occlusion Texture",
                    "Emissive Texture"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    struct material
    {
        std::optional<color> color;
        std::optional<gradient> gradient;
        std::optional<shared<texture>> sharedTexture;
        std::optional<texture> texture;
        std::optional<shader_effect> shaderEffect;
        std::optional<vec4> shaderEffectGain;
        bool subpixel;
        std::optional<pbr_material> pbr;

        auto operator<=>(material const&) const = default;

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
            static std::uint32_t field_count()
            {
                return 8;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
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
                    return component_data_field_type::Vec4 | component_data_field_type::Optional;
                case 6:
                    return component_data_field_type::Bool;
                case 7:
                    return component_data_field_type::ComponentData | component_data_field_type::Optional;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(std::uint32_t aFieldIndex)
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
                case 4:
                case 5:
                case 6:
                    return neolib::uuid{};
                case 7:
                    return pbr_material::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Color",
                    "Gradient",
                    "Shared Texture",
                    "Texture",
                    "Shader Effect",
                    "Shader Effect Gain",
                    "Subpixel",
                    "PBR"
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
            lhs.shaderEffectGain == rhs.shaderEffectGain &&
            lhs.subpixel == rhs.subpixel &&
            lhs.pbr == rhs.pbr;
    }
}