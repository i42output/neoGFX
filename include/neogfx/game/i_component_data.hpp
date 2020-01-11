// i_component_data.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/numerical.hpp>
#include <neogfx/game/ecs_ids.hpp>

namespace neogfx::game
{
    template<class _Ty>
    using ecs_data_type_t = std::remove_cv_t<std::remove_reference_t<_Ty>>;

    typedef vec3u32 face;
    typedef std::vector<face> faces;

    class i_ecs;

    enum class component_data_field_type : uint32_t
    {
        Invalid             = 0x00000000,
        Bool                = 0x00000001,
        Int8                = 0x00000002,
        Uint8               = 0x00000003,
        Int16               = 0x00000004,
        Uint16              = 0x00000005,
        Int32               = 0x00000006,
        Uint32              = 0x00000007,
        Int64               = 0x00000008,
        Uint64              = 0x00000009,
        Float32             = 0x0000000A,
        Float64             = 0x0000000B,
        Scalar              = Float64,
        BasicVec2           = 0x00000100,
        BasicVec3           = 0x00000200,
        BasicVec4           = 0x00000300,
        Vec2                = BasicVec2 | Float64,
        Vec3                = BasicVec3 | Float64,
        Vec4                = BasicVec4 | Float64,
        Triangle            = BasicVec3 | Uint32,
        Face                = Triangle,
        BasicMat22          = 0x00000400,
        BasicMat33          = 0x00000500,
        BasicMat44          = 0x00000600,
        Mat22               = BasicMat22 | Float64,
        Mat33               = BasicMat33 | Float64,
        Mat44               = BasicMat44 | Float64,
        Aabb                = 0x00001000,
        Aabb2d              = 0x00002000,
        String              = 0x00010000,
        Enum                = 0x000B0000,
        Uuid                = 0x000C0000,
        Id                  = 0x000D0000,
        ComponentData       = 0x000E0000,
        Optional            = 0x01000000,
        Array               = 0x02000000,
        Shared              = 0x04000000,
        Internal            = 0x80000000,
    };

    inline constexpr component_data_field_type operator|(component_data_field_type aLhs, component_data_field_type aRhs)
    {
        return static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr component_data_field_type operator&(component_data_field_type aLhs, component_data_field_type aRhs)
    {
        return static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr component_data_field_type& operator|=(component_data_field_type& aLhs, component_data_field_type aRhs)
    {
        return aLhs = static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr component_data_field_type& operator&=(component_data_field_type& aLhs, component_data_field_type aRhs)
    {
        return aLhs = static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    struct i_component_data
    {
        struct meta
        {
            struct invalid_field_index : std::logic_error { invalid_field_index() : std::logic_error("neogfx::game::i_component_data::meta::invalid_field_index") {} };

            static neolib::uuid field_type_id(uint32_t)
            {
                return neolib::uuid{};
            }

            static constexpr bool has_handles = false;
            static constexpr bool has_updater = false;
        };
    };
}