// sdl_graphics_context.hpp
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
#include "opengl_graphics_context.hpp"

namespace neogfx
{
	class i_rendering_engine;
	class sdl_window;

	class sdl_graphics_context : public opengl_graphics_context
	{
	public:
		sdl_graphics_context(i_rendering_engine& aRenderingEngine, const sdl_window& aRenderTarget);
		sdl_graphics_context(i_rendering_engine& aRenderingEngine, const sdl_window& aRenderTarget, const i_widget& aWidget);
		sdl_graphics_context(const sdl_graphics_context& aOther);
		virtual std::unique_ptr<i_native_graphics_context> clone() const;
		~sdl_graphics_context();
	public:
		virtual rect rendering_area(bool aConsiderScissor = true) const;
	private:
		const sdl_window& iRenderTarget;
	};
}