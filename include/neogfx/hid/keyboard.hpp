// keyboard.hpp
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
#include "i_keyboard.hpp"

namespace neogfx
{
	class keyboard;

	class keyboard_grabber : public i_keyboard_handler
	{
	public:
		keyboard_grabber(keyboard& aKeyboard);
	public:
		virtual bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool text_input(const std::string& aText);
		virtual bool sys_text_input(const std::string& aText);
	private:
		keyboard& iKeyboard;
	};

	class keyboard : public i_keyboard
	{
		friend class keyboard_grabber;
	public:
		keyboard();
	public:
		virtual bool is_keyboard_grabbed() const;
		virtual bool is_keyboard_grabbed_by(i_keyboard_handler& aKeyboardHandler) const;
		virtual void grab_keyboard(i_keyboard_handler& aKeyboardHandler);
		virtual void ungrab_keyboard(i_keyboard_handler& aKeyboardHandler);
		virtual i_keyboard_handler& grabber() const;
	private:
		mutable keyboard_grabber iGrabber;
		std::deque<i_keyboard_handler*> iGrabs;
	};
}