// animation_filter.hpp
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

#include <neogfx/game/animation.hpp>

namespace neogfx::game
{
    struct animation_filter
    {
        shared<animation> sharedAnimation;
        std::optional<animation> animation;
        std::optional<mat44> transformation;
        u32 currentFrame;
        bool autoDestroy;
        std::optional<i64> currentFrameStartTime; // ECS internal

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
            static std::uint32_t field_count()
            {
                return 6;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::ComponentData | component_data_field_type::Shared;
                case 1:
                    return component_data_field_type::ComponentData | component_data_field_type::Optional;
                case 2:
                    return component_data_field_type::Mat44 | component_data_field_type::Optional;
                case 3:
                    return component_data_field_type::Uint32;
                case 4:
                    return component_data_field_type::Bool;
                case 5:
                    return component_data_field_type::Int64 | component_data_field_type::Internal;
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
                    return animation::meta::id();
                case 2:
                case 3:
                case 4:
                case 5:
                    return neolib::uuid{};
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Shared Animation",
                    "Animation",
                    "Transformation",
                    "Current Frame",
                    "Auto Destroy",
                    "Current Frame Start Time"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline mesh_filter const& current_animation_frame(animation_filter const& aAnimationFilter)
    {
        return aAnimationFilter.animation ?
            aAnimationFilter.animation->frames[aAnimationFilter.currentFrame].filter :
            aAnimationFilter.sharedAnimation.ptr->frames[aAnimationFilter.currentFrame].filter;
    }

    inline mat44 const& to_transformation_matrix(animation_filter const& aAnimationFilter)
    {
        return aAnimationFilter.transformation ? *aAnimationFilter.transformation : mat44::identity();
    }
}