// opengl_texture_manager.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/texture_manager.hpp>

namespace neogfx
{
    class opengl_texture_manager : public texture_manager
    {
    public:
        std::shared_ptr<i_texture> create_texture(const neogfx::size& aExtents, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::NormalMipmap, texture_data_format aDataFormat = texture_data_format::RGBA, texture_data_type aDataType = texture_data_type::UnsignedByte, color_space aColorSpace = color_space::sRGB, const optional_color& aColor = optional_color()) override;
        std::shared_ptr<i_texture> create_texture(const i_image& aImage, texture_data_format aDataFormat = texture_data_format::RGBA, texture_data_type aDataType = texture_data_type::UnsignedByte) override;
    };
}