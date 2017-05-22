// i_window.hpp
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
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/hid/i_keyboard.hpp>

namespace neogfx
{
	class i_native_window;

	typedef surface_style window_style;

	class i_window : public i_surface
	{
	public:
		virtual const i_native_window& native_window() const = 0;
		virtual i_native_window& native_window() = 0;
	public:
		virtual bool is_active() const = 0;
		virtual void activate() = 0;
		virtual void counted_enable(bool aEnable) = 0;
	public:
		virtual bool native_window_can_close() const = 0;
		virtual void native_window_closing() = 0;
		virtual void native_window_closed() = 0;
		virtual void native_window_focus_gained() = 0;
		virtual void native_window_focus_lost() = 0;
		virtual void native_window_resized() = 0;
		virtual bool native_window_has_rendering_priority() const = 0;
		virtual bool native_window_ready_to_render() const = 0;
		virtual void native_window_render(const rect& aInvalidatedRect) const = 0;
		virtual void native_window_dismiss_children() = 0;
		virtual void native_window_mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta) = 0;
		virtual void native_window_mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
		virtual void native_window_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
		virtual void native_window_mouse_button_released(mouse_button aButton, const point& aPosition) = 0;
		virtual void native_window_mouse_moved(const point& aPosition) = 0;
		virtual void native_window_mouse_entered() = 0;
		virtual void native_window_mouse_left() = 0;
		virtual void native_window_key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) = 0;
		virtual void native_window_key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) = 0;
		virtual void native_window_text_input(const std::string& aText) = 0;
		virtual void native_window_sys_text_input(const std::string& aText) = 0;
		virtual neogfx::mouse_cursor native_window_mouse_cursor() const = 0;
	};
}