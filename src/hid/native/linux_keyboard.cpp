// linux_keyboard.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <xkbcommon/xkbcommon.h>

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include "../../app/native/linux_basic_services.hpp"
#include "linux_keyboard.hpp"

namespace neogfx
{
    namespace native::linux
    {
        namespace
        {
            linux::display_server& the_display_server()
            {
                return static_cast<linux::basic_services&>(service<i_basic_services>()).display_server();
            }
        }

        keyboard_layout::keyboard_layout(i_keyboard& aKeyboard) :
            iKeyboard{ aKeyboard }
        {
        }

        keyboard_layout::~keyboard_layout()
        {
        }

        bool keyboard_layout::has_ime() const
        {
            return false;
        }

        bool keyboard_layout::ime_open() const
        {
            return false;
        }

        bool keyboard_layout::ime_active() const
        {
            return false;
        }

        bool keyboard_layout::ime_active(i_widget const&) const
        {
            return false;
        }

        i_widget const& keyboard_layout::input_widget() const
        {
            throw ime_not_active();
        }

        point const& keyboard_layout::position() const
        {
            throw ime_not_active();
        }

        void keyboard_layout::open_ime()
        {
        }

        void keyboard_layout::close_ime()
        {
        }

        void keyboard_layout::activate_ime(i_widget const&, optional_point const&)
        {
        }

        void keyboard_layout::deactivate_ime(i_widget const&)
        {
        }

        void keyboard_layout::update_ime_position(point const&)
        {
        }

        keyboard::keyboard() :
            iLayout{ new keyboard_layout{ *this } }
        {
        }

        i_keyboard_layout& keyboard::layout() const
        {
            return *iLayout;
        }

        bool keyboard::is_key_pressed(scan_code_e aScanCode) const
        {
            return the_display_server().is_key_pressed(scan_code_to_evdev(aScanCode));
        }

        keyboard_locks keyboard::locks() const
        {
            return the_display_server().locks();
        }

        key_modifier keyboard::modifiers() const
        {
            return the_display_server().modifiers();
        }

        key_code_e keyboard::scan_code_to_key_code(scan_code_e aScanCode) const
        {
            // as Windows: the character the key produces unshifted (lower case for a letter), if it produces one,
            // otherwise the key code made from the scan code
            if (aScanCode == ScanCode_UNKNOWN)
                return KeyCode_UNKNOWN;
            auto const keyCode = scan_code_to_evdev(aScanCode);
            if (keyCode != 0u)
            {
                auto const character = ::xkb_keysym_to_utf32(the_display_server().key_symbol(keyCode));
                switch (character)
                {
                case 0x08:
                case 0x09:
                case 0x0D:
                case 0x1B:
                    return static_cast<key_code_e>(character);
                default:
                    if (character >= 0x20u && character != 0x7Fu)
                    {
                        if (character >= 'A' && character <= 'Z')
                            return static_cast<key_code_e>(KeyCode_a + (character - 'A'));
                        return static_cast<key_code_e>(character);
                    }
                    break;
                }
            }
            return static_cast<key_code_e>(NEOGFX_SCANCODE_TO_KEYCODE(aScanCode));
        }

        native_key_code_e keyboard::native_key_code_to_usb_hid_key_code(native_key_code_e aKeyCode) const
        {
            return aKeyCode;
        }
    }
}
