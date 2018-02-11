// i_window_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include <neogfx/core/geometry.hpp>
#include <neogfx/hid/mouse.hpp>

namespace neogfx
{
	class i_window;

	class i_window_manager
	{
	public:
		struct window_not_found : std::logic_error { window_not_found() : std::logic_error("neogfx::i_surface_manager::window_not_found") {} };
	public:
		virtual ~i_window_manager() {}
	public:
		virtual void add_window(i_window& aWindow) = 0;
		virtual void remove_window(i_window& aWindow) = 0;
		virtual bool has_window(i_window& aWindow) const = 0;
		virtual std::size_t window_count() const = 0;
		virtual i_window& window(std::size_t aIndex) = 0;
		virtual bool any_strong_windows() const = 0;
	public:
		virtual rect desktop_rect(const i_window& aWindow) const = 0;
		virtual rect window_rect(const i_window& aWindow) const = 0;
		virtual void move_window(i_window& aWindow, const point& aPosition) = 0;
		virtual void resize_window(i_window& aWindow, const size& aExtents) = 0;
	public:
		virtual point mouse_position() const = 0;
		virtual point mouse_position(const i_window& aWindow) const = 0;
		virtual bool is_mouse_button_pressed(mouse_button aButton) const = 0;
	public:
		virtual void save_mouse_cursor() = 0;
		virtual void set_mouse_cursor(mouse_system_cursor aSystemCursor) = 0;
		virtual void restore_mouse_cursor(const i_window& aWindow) = 0;
		virtual void update_mouse_cursor(const i_window& aWindow) = 0;
	};
}