// sdl_renderer.hpp
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

#include "neogfx.hpp"
#include <set>
#include <map>
#include "opengl_renderer.hpp"
#include "keyboard.hpp"

namespace neogfx
{
	class sdl_renderer : public opengl_renderer
	{
	public:
		sdl_renderer(i_keyboard& aKeyboard);
		~sdl_renderer();
	public:
		virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle);
		virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, i_native_window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle);
	public:
		virtual bool process_events();
	private:
		i_keyboard& iKeyboard;
	};
}