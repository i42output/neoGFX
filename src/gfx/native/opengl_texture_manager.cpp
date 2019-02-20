// opengl_texture_manager.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include "opengl_texture_manager.hpp"
#include "opengl_texture.hpp"

namespace neogfx
{
    std::shared_ptr<i_texture> opengl_texture_manager::create_texture(const neogfx::size& aExtents, dimension aDpiScaleFactor, texture_sampling aSampling, texture_data_format aDataFormat, texture_data_type aDataType, const optional_colour& aColour)
    {
        return add_texture(std::make_shared<opengl_texture>(*this, allocate_texture_id(), aExtents, aDpiScaleFactor, aSampling, aDataFormat, aDataType, aColour));
    }

    std::shared_ptr<i_texture> opengl_texture_manager::create_texture(const i_image& aImage, texture_data_format aDataFormat, texture_data_type aDataType)
    {
        auto existing = find_texture(aImage);
        if (existing != textures().end())
            return existing->first;
        return add_texture(std::make_shared<opengl_texture>(*this, allocate_texture_id(), aImage, aDataFormat, aDataType));
    }
}