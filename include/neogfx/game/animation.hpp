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
#include <ranges>

#include <neolib/core/uuid.hpp>
#include <neolib/core/string.hpp>

#include <neogfx/gfx/color.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_component.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/material.hpp>
#include <neogfx/game/patch.hpp>

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
        neolib::small_vector<easing, 1u> easings;
        neolib::small_vector<scalar, 1u> weights;

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
                    "Easings",
                    "Weights"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    struct animation_tween
    {
        scalar duration;
        game::patches patches;
        vec3f_range translation{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
        vec3f_range scaling{ { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f } };
        vec3f_range rotation{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
        std::optional<std::array<animation_easing, 3u>> translationEasings;
        std::optional<std::array<animation_easing, 3u>> scalingEasings;
        std::optional<std::array<animation_easing, 3u>> rotationEasings;
        mutable std::optional<std::function<mat44f(vec3f const&, vec3f const&, vec3f const&)>> transformationMatrixFunction;
        function_factory<animation_tween> transformationMatrixFunctionFactory{
            [](animation_tween const& self)
            {
                if (!self.transformationMatrixFunction)
                    self.transformationMatrixFunction = neolib::affine_transformation_lerp_generator(
                        self.translation,
                        self.scaling,
                        self.rotation);
            } };

        mat44f operator()(scalar timestep) const
        {
            static std::array<animation_easing, 3u> const sDefaultEasings{
                animation_easing{ { easing::Linear }, { 1.0 } }, 
                animation_easing{ { easing::Linear }, { 1.0 } }, 
                animation_easing{ { easing::Linear }, { 1.0 } } };

            thread_local std::vector<ease_segment<double>> tSegments;

            auto const t = duration > 0.0 ? std::fmod(timestep, duration) / duration : 0.0;
            vec3f et;
            vec3f es;
            vec3f er;

            for (auto axis : { 0u, 1u, 2u })
            {
                tSegments.clear();
                auto const& segment = (translationEasings ? *translationEasings : sDefaultEasings)[axis];
                for (auto [e, w] : std::views::zip(segment.easings, segment.weights))
                    tSegments.emplace_back(e, w);
                et[axis] = static_cast<float>(partitioned_ease({ tSegments.begin(), tSegments.end() }, t));
            }
               
            for (auto axis : { 0u, 1u, 2u })
            {
                tSegments.clear();
                auto const& segment = (scalingEasings ? *scalingEasings : sDefaultEasings)[axis];
                for (auto [e, w] : std::views::zip(segment.easings, segment.weights))
                    tSegments.emplace_back(e, w);
                es[axis] = static_cast<float>(partitioned_ease({ tSegments.begin(), tSegments.end() }, t));
            }

            for (auto axis : { 0u, 1u, 2u })
            {
                tSegments.clear();
                auto const& segment = (rotationEasings ? *rotationEasings : sDefaultEasings)[axis];
                for (auto [e, w] : std::views::zip(segment.easings, segment.weights))
                    tSegments.emplace_back(e, w);
                er[axis] = static_cast<float>(partitioned_ease({ tSegments.begin(), tSegments.end() }, t));
            }

            if (!transformationMatrixFunction)
                transformationMatrixFunctionFactory.make(*this);

            return transformationMatrixFunction.value()(
                translation.start + (translation.end - translation.start).hadamard_product(et),
                scaling.start + (scaling.end - scaling.start).hadamard_product(es),
                rotation.start + (rotation.end - rotation.start).hadamard_product(er));
        }

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
                return 10;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Scalar;
                case 1:
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::SharedPointer;
                case 2:
                    return component_data_field_type::Vec3f | component_data_field_type::Range;
                case 3:
                    return component_data_field_type::Vec3f | component_data_field_type::Range;
                case 4:
                    return component_data_field_type::Vec3f | component_data_field_type::Range;
                case 5:
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::Optional;
                case 6:
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::Optional;
                case 7:
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::Optional;
                case 8:
                    return component_data_field_type::Mat44f | component_data_field_type::Function3Vec3f | component_data_field_type::Optional | component_data_field_type::Cache;
                case 9:
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
                    return patch::meta::id();
                case 2:
                    return neolib::uuid{};
                case 3:
                    return neolib::uuid{};
                case 4:
                    return neolib::uuid{};
                case 5:
                    return animation_easing::meta::id();
                case 6:
                    return animation_easing::meta::id();
                case 7:
                    return animation_easing::meta::id();
                case 8:
                    return neolib::uuid{};
                case 9:
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
                    "Patches",
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

    using animation_tweens = std::vector<animation_tween>;

    struct animation
    {
        std::optional<animation_frames> frames;
        std::optional<animation_tweens> tweens;
        bool active = true;

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
                return 3;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::Optional;
                case 1:
                    return component_data_field_type::ComponentData | component_data_field_type::Array | component_data_field_type::Optional;
                case 2:
                    return component_data_field_type::Bool;
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
                case 2:
                    return neolib::uuid{};
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Animation Frames",
                    "Animation Tweens",
                    "Active"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline void translate(animation_tween& aTween, vec3_range const& aRange)
    {
        aTween.translation = vec3f_range{ aRange.start.as<float>(), aRange.end.as<float>() };
        aTween.transformationMatrixFunction = std::nullopt;
    }

    inline void scale(animation_tween& aTween, vec3_range const& aRange)
    {
        aTween.scaling = vec3f_range{ aRange.start.as<float>(), aRange.end.as<float>() };
        aTween.transformationMatrixFunction = std::nullopt;
    }

    inline void rotate(animation_tween& aTween, vec3_range const& aRange)
    {
        aTween.rotation = vec3f_range{ aRange.start.as<float>(), aRange.end.as<float>() };
        aTween.transformationMatrixFunction = std::nullopt;
    }

    inline void rotate_deg(animation_tween& aTween, vec3_range const& aRange)
    {
        rotate(aTween, { aRange.start * std::numbers::pi / 180.0, aRange.end * std::numbers::pi / 180.0 });
    }

    inline animation_tween& add_tween(animation& aAnimation, scalar aDuration, patches const& aPatches)
    {
        if (!aAnimation.tweens)
            aAnimation.tweens.emplace();
        aAnimation.tweens->emplace_back(aDuration, aPatches);
        return aAnimation.tweens->back();
    }

    inline void translate(animation& aAnimation, scalar aDuration, patches const& aPatches, vec3_range const& aRange)
    {
        translate(add_tween(aAnimation, aDuration, aPatches), aRange);
    }

    inline void scale(animation& aAnimation, scalar aDuration, patches const& aPatches, vec3_range const& aRange)
    {
        scale(add_tween(aAnimation, aDuration, aPatches), aRange);
    }

    inline void rotate(animation& aAnimation, scalar aDuration, patches const& aPatches, vec3_range const& aRange)
    {
        rotate(add_tween(aAnimation, aDuration, aPatches), aRange);
    }

    inline void rotate_deg(animation& aAnimation, scalar aDuration, patches const& aPatches, vec3_range const& aRange)
    {
        rotate(aAnimation, aDuration, aPatches, { aRange.start * std::numbers::pi / 180.0, aRange.end * std::numbers::pi / 180.0 });
    }
}
