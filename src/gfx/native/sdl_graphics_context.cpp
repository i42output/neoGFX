// sdl_graphics_context.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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
	sdl_graphics_context::sdl_graphics_context(i_rendering_engine& aRenderingEngine, const sdl_window& aRenderTarget) :
		opengl_graphics_context(aRenderingEngine, aRenderTarget), iRenderTarget(aRenderTarget)
	{
	}

	sdl_graphics_context::sdl_graphics_context(i_rendering_engine& aRenderingEngine, const sdl_window& aRenderTarget, const i_widget& aWidget) :
		opengl_graphics_context(aRenderingEngine, aRenderTarget, aWidget), iRenderTarget(aRenderTarget)
	{
	}

	sdl_graphics_context::sdl_graphics_context(const sdl_graphics_context& aOther) :
		opengl_graphics_context(aOther), iRenderTarget(aOther.iRenderTarget)
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
		if (scissor_rect() == boost::none || !aConsiderScissor)
			return rect(point(), size(static_cast<dimension>(iRenderTarget.extents().cx), static_cast<dimension>(iRenderTarget.extents().cy)));
		else
			return *scissor_rect();
	}
}
