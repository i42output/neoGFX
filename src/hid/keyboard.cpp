// keyboard.cpp
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
#include <neogfx/hid/keyboard.hpp>

namespace neogfx
{
    keyboard_grabber::keyboard_grabber(keyboard& aKeyboard) : iKeyboard(aKeyboard)
    {
    }

    bool keyboard_grabber::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        for (auto& g : iKeyboard.iGrabs)
            if (g->key_pressed(aScanCode, aKeyCode, aKeyModifiers))
                return true;
        return false;
    }

    bool keyboard_grabber::key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        for (auto& g : iKeyboard.iGrabs)
            if (g->key_released(aScanCode, aKeyCode, aKeyModifiers))
                return true;
        return false;
    }

    bool keyboard_grabber::text_input(const std::string& aText)
    {
        for (auto& g : iKeyboard.iGrabs)
            if (g->text_input(aText))
                return true;
        return false;
    }

    bool keyboard_grabber::sys_text_input(const std::string& aText)
    {
        for (auto& g : iKeyboard.iGrabs)
            if (g->sys_text_input(aText))
                return true;
        return false;
    }

    keyboard::keyboard(const i_string& aName) :
        hid_device<i_keyboard>{ hid_device_type::Input, hid_device_class::Keyboard, hid_device_subclass::Keyboard, aName }, iGrabber { *this }
    {
    }

    bool keyboard::is_keyboard_grabbed() const
    {
        return !iGrabs.empty();
    }

    bool keyboard::is_keyboard_grabbed_by(i_keyboard_handler& aKeyboardHandler) const
    {
        return std::find(iGrabs.begin(), iGrabs.end(), &aKeyboardHandler) != iGrabs.end();
    }

    bool keyboard::is_front_grabber(i_keyboard_handler& aKeyboardHandler) const
    {
        return !iGrabs.empty() && &**iGrabs.begin() == &aKeyboardHandler;
    }

    void keyboard::grab_keyboard(i_keyboard_handler& aKeyboardHandler)
    {
        if (is_keyboard_grabbed_by(aKeyboardHandler))
            throw already_grabbed();
        iGrabs.push_front(&aKeyboardHandler);
    }

    void keyboard::ungrab_keyboard(i_keyboard_handler& aKeyboardHandler)
    {
        auto grab = std::find(iGrabs.begin(), iGrabs.end(), &aKeyboardHandler);
        if (grab != iGrabs.end())
            iGrabs.erase(grab);
    }

    i_keyboard_handler& keyboard::grabber() const
    {
        return iGrabber;
    }
}