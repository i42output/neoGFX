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

#include <numbers>

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

    using animation_frames = std::vector<animation_frame>;

    struct animation_easing
    {
        neolib::small_vector<easing, 1u> easing;
        neolib::small_vector<scalar, 1u> weight;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x236f5e5f, 0xc5b3, 0x4dab, 0x9db5, { 0xc1, 0xe9, 0x7f, 0x9f, 0xf5, 0x17 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Animation Easing";
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
                    return component_data_field_type::Enum | component_data_field_type::Array;
                case 1:
                    return component_data_field_type::Scalar | component_data_field_type::Array;
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
                    return neolib::uuid{};
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Easing",
                    "Weight"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    struct animation_tween
    {
        scalar duration;
        vec3f_range translation{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
        vec3f_range scaling{ { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } };
        vec3f_range rotation{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
        std::optional<std::array<animation_easing, 3u>> translationEasings;
        std::optional<std::array<animation_easing, 3u>> scalingEasings;
        std::optional<std::array<animation_easing, 3u>> rotationEasings;
        std::optional<std::function<mat44f(vec3f const&, vec3f const&, vec3f const&)>> transformationMatrixFunction;
        function_factory<animation_tween> transformationMatrixFunctionFactory{
            [](animation_tween& self)
            {
                if (!self.transformationMatrixFunction)
                    self.transformationMatrixFunction = neolib::affine_transformation_lerp_generator(
                        self.translation,
                        self.scaling,
                        self.rotation);
            } };

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
                return 9;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Scalar;
                case 1:
                    return component_data_field_type::Vec3f | component_data_field_type::Range;
                case 2:
                    return component_data_field_type::Vec3f | component_data_field_type::Range;
                case 3:
                    return component_data_field_type::Vec3f | component_data_field_type::Range;
                case 4:
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::Optional;
                case 5:
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::Optional;
                case 6:
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::Optional;
                case 7:
                    return component_data_field_type::Mat44f | component_data_field_type::Function3Vec3f | component_data_field_type::Optional | component_data_field_type::Cache;
                case 8:
                    return component_data_field_type::FunctionFactory;
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
                    return neolib::uuid{};
                case 2:
                    return neolib::uuid{};
                case 3:
                    return neolib::uuid{};
                case 4:
                    return animation_easing::meta::id();
                case 5:
                    return animation_easing::meta::id();
                case 6:
                    return animation_easing::meta::id();
                case 7:
                    return neolib::uuid{};
                case 8:
                    return neolib::uuid{};
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Duration",
                    "Translation",
                    "Scaling",
                    "Rotation",
                    "Translation Easing",
                    "Scaling Easing",
                    "Rotation Easing",
                    "Transformation Matrix",
                    "Transformation Matrix Factory"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    struct animation
    {
        std::optional<animation_frames> frames;
        std::optional<animation_tween> tween;

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
                return 2;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::Optional;
                case 1:
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
                    return animation_frame::meta::id();
                case 1:
                    return animation_tween::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Animation Frames",
                    "Animation Tween"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline void translate(animation& aAnimation, vec3_range const& aRange)
    {
        if (!aAnimation.tween)
            aAnimation.tween.emplace();
        aAnimation.tween->translation = vec3f_range{ aRange.start.as<float>(), aRange.end.as<float>() };
        aAnimation.tween->transformationMatrixFunction = std::nullopt;
    }

    inline void scale(animation& aAnimation, vec3_range const& aRange)
    {
        if (!aAnimation.tween)
            aAnimation.tween.emplace();
        aAnimation.tween->scaling = vec3f_range{ aRange.start.as<float>(), aRange.end.as<float>() };
        aAnimation.tween->transformationMatrixFunction = std::nullopt;
    }

    inline void rotate(animation& aAnimation, vec3_range const& aRange)
    {
        if (!aAnimation.tween)
            aAnimation.tween.emplace();
        aAnimation.tween->rotation = vec3f_range{ aRange.start.as<float>(), aRange.end.as<float>() };
        aAnimation.tween->transformationMatrixFunction = std::nullopt;
    }

    inline void rotate_deg(animation& aAnimation, vec3_range const& aRange)
    {
        rotate(aAnimation, { aRange.start * std::numbers::pi / 180.0, aRange.end * std::numbers::pi / 180.0 });
    }
}
