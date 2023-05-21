// vulkan_texture_manager.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>
#include "vulkan_texture_manager.hpp"
#include "vulkan_texture.hpp"

namespace neogfx
{
    void vulkan_texture_manager::create_texture(const neogfx::size& aExtents, dimension aDpiScaleFactor, texture_sampling aSampling, texture_data_format aDataFormat, texture_data_type aDataType, color_space aColorSpace, const optional_color& aColor, i_ref_ptr<i_texture>& aResult)
    {
        switch (aDataFormat)
        {
        case texture_data_format::RGBA:
        case texture_data_format::SubPixel:
        default:
            switch (aDataType)
            {
            case texture_data_type::UnsignedByte:
            default:
                aResult = add_texture(make_ref<vulkan_texture<std::array<uint8_t,4>>>(*this, allocate_texture_id(), aExtents, aDpiScaleFactor, aSampling, aDataFormat, aColorSpace, aColor));
                break;
            case texture_data_type::Float:
                aResult = add_texture(make_ref<vulkan_texture<std::array<float, 4>>>(*this, allocate_texture_id(), aExtents, aDpiScaleFactor, aSampling, aDataFormat, aColorSpace, aColor));
                break;
            }
            break;
        case texture_data_format::Red:
            switch (aDataType)
            {
            case texture_data_type::UnsignedByte:
            default:
                aResult = add_texture(make_ref<vulkan_texture<uint8_t>>(*this, allocate_texture_id(), aExtents, aDpiScaleFactor, aSampling, aDataFormat, aColorSpace, aColor));
                break;
            case texture_data_type::Float:
                aResult = add_texture(make_ref<vulkan_texture<float>>(*this, allocate_texture_id(), aExtents, aDpiScaleFactor, aSampling, aDataFormat, aColorSpace, aColor));
                break;
            }
            break;
        }
    }

    void vulkan_texture_manager::create_texture(const i_image& aImage, const rect& aImagePart, texture_data_format aDataFormat, texture_data_type aDataType, i_ref_ptr<i_texture>& aResult)
    {
        auto existing = find_texture(aImage, aImagePart);
        if (existing != textures().end())
        {
            aResult = existing->first();
            return;
        }
        switch (aDataFormat)
        {
        case texture_data_format::RGBA:
        case texture_data_format::SubPixel:
        default:
            switch (aDataType)
            {
            case texture_data_type::UnsignedByte:
            default:
                aResult = add_texture(make_ref<vulkan_texture<avec4u8>>(*this, allocate_texture_id(), aImage, aImagePart, aDataFormat));
                break;
            case texture_data_type::Float:
                aResult = add_texture(make_ref<vulkan_texture<std::array<float, 4>>>(*this, allocate_texture_id(), aImage, aImagePart, aDataFormat));
                break;
            }
            break;
        case texture_data_format::Red:
            switch (aDataType)
            {
            case texture_data_type::UnsignedByte:
            default:
                aResult = add_texture(make_ref<vulkan_texture<uint8_t>>(*this, allocate_texture_id(), aImage, aImagePart, aDataFormat));
                break;
            case texture_data_type::Float:
                aResult = add_texture(make_ref<vulkan_texture<float>>(*this, allocate_texture_id(), aImage, aImagePart, aDataFormat));
                break;
            }
            break;
        }
    }
}
