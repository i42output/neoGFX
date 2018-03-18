// opengl_texture_manager.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
	std::unique_ptr<i_native_texture> opengl_texture_manager::create_texture(const neogfx::size& aExtents, dimension aDpiScaleFactor, texture_sampling aSampling, const optional_colour& aColour)
	{
		return add_texture(std::make_shared<opengl_texture>(aExtents, aDpiScaleFactor, aSampling, aColour));
	}

	std::unique_ptr<i_native_texture> opengl_texture_manager::create_texture(const i_image& aImage)
	{
		auto existing = find_texture(aImage);
		if (existing != textures().end())
			return join_texture(*existing->lock());
		return add_texture(std::make_shared<opengl_texture>(aImage));
	}
}