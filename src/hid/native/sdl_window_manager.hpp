// sdl_window_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neogfx/hid/window_manager.hpp>

namespace neogfx
{
	class sdl_window_manager : public window_manager
	{
	public:
		sdl_window_manager();
		~sdl_window_manager();
	public:
		point mouse_position() const override;
		point mouse_position(const i_window& aWindow) const override;
		bool is_mouse_button_pressed(mouse_button aButton) const override;
	public:
		void save_mouse_cursor() override;
		void set_mouse_cursor(mouse_system_cursor aSystemCursor) override;
		void restore_mouse_cursor() override;
		void update_mouse_cursor() override;
	};
}