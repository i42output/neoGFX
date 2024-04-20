// shader_array.hpp
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

#include <neogfx/gfx/i_texture_manager.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/gfx/i_shader_array.hpp>

namespace neogfx
{
    template <typename T>
    struct crack_shader_array_data_type;

    template <>
    struct crack_shader_array_data_type<std::uint8_t>
    { 
        static constexpr texture_data_format DATA_FORMAT = texture_data_format::Red;
        static constexpr texture_data_type DATA_TYPE = texture_data_type::UnsignedByte;
    };
    template <>
    struct crack_shader_array_data_type<float> 
    { 
        static constexpr texture_data_format DATA_FORMAT = texture_data_format::Red;
        static constexpr texture_data_type DATA_TYPE = texture_data_type::Float;
    };
    template <>
    struct crack_shader_array_data_type<avec4u8>
    {
        static constexpr texture_data_format DATA_FORMAT = texture_data_format::RGBA;
        static constexpr texture_data_type DATA_TYPE = texture_data_type::UnsignedByte;
    };
    template <>
    struct crack_shader_array_data_type<avec4f> 
    { 
        static constexpr texture_data_format DATA_FORMAT = texture_data_format::RGBA;
        static constexpr texture_data_type DATA_TYPE = texture_data_type::Float;
    };

    template <typename T>
    class shader_array : public i_shader_array<T>
    {        
    public:
        shader_array(const size_u32& aInitialSize) : 
            iTexture{ aInitialSize, 1.0, texture_sampling::Data, crack_shader_array_data_type<T>::DATA_FORMAT, crack_shader_array_data_type<T>::DATA_TYPE }
        {
        }
    public:
        const i_texture& data() const override
        {
            return iTexture;
        }
        i_texture& data() override
        {
            return iTexture;
        }
    private:
        texture iTexture;
    };
}