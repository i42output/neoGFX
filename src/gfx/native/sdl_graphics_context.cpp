// sdl_graphics_context.cpp
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
#include "../../gui/window/native/sdl_window.hpp"
#include "sdl_graphics_context.hpp"

namespace neogfx
{
	sdl_graphics_context::sdl_graphics_context(const i_render_target& aTarget) :
		opengl_graphics_context(aTarget)
	{
	}

	sdl_graphics_context::sdl_graphics_context(const i_render_target& aTarget, const i_widget& aWidget) :
		opengl_graphics_context(aTarget, aWidget)
	{
	}

	sdl_graphics_context::sdl_graphics_context(const sdl_graphics_context& aOther) :
		opengl_graphics_context(aOther)
	{
	}

	std::unique_ptr<i_native_graphics_context> sdl_graphics_context::clone() const
	{
		return std::unique_ptr<i_native_graphics_context>(new sdl_graphics_context(*this));
	}

	sdl_graphics_context::~sdl_graphics_context()
	{
	}

	rect sdl_graphics_context::rendering_area(bool aConsiderScissor) const
	{
		if (scissor_rect() == std::nullopt || !aConsiderScissor)
			return rect{ point{}, render_target().target_extents() };
		else
			return *scissor_rect();
	}
}
