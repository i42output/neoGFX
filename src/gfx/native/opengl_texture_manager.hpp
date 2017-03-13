// opengl_texture_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
		virtual std::unique_ptr<i_native_texture> create_texture(const neogfx::size& aExtents, const optional_colour& aColour = optional_colour());
		virtual std::unique_ptr<i_native_texture> create_texture(const i_image& aImage);
	};
}