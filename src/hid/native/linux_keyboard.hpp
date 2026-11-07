// linux_keyboard.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <neogfx/hid/keyboard.hpp>

namespace neogfx
{
    namespace native::linux
    {
        // n.b. no input method (IME) yet: text input is what the keyboard layout (xkb, with its compose
        // sequences, e.g. dead keys) produces
        class keyboard_layout : public i_keyboard_layout
        {
        public:
            keyboard_layout(i_keyboard& aKeyboard);
            ~keyboard_layout();
        public:
            bool has_ime() const final;
            bool ime_open() const final;
            bool ime_active() const final;
            bool ime_active(i_widget const& aInputWidget) const final;
            i_widget const& input_widget() const final;
            point const& position() const final;
            void open_ime() final;
            void close_ime() final;
            void activate_ime(i_widget const& aInputWidget, optional_point const& aPosition = {}) final;
            void deactivate_ime(i_widget const& aInputWidget) final;
            void update_ime_position(point const& aPosition) final;
        private:
            i_keyboard& iKeyboard;
        };

        class keyboard : public neogfx::keyboard
        {
        public:
            keyboard();
        public:
            i_keyboard_layout& layout() const final;
        public:
            bool is_key_pressed(scan_code_e aScanCode) const final;
            keyboard_locks locks() const final;
            key_modifier modifiers() const final;
        public:
            key_code_e scan_code_to_key_code(scan_code_e aScanCode) const final;
            native_key_code_e native_key_code_to_usb_hid_key_code(native_key_code_e aKeyCode) const final;
        private:
            std::unique_ptr<i_keyboard_layout> iLayout;
        };
    }
}
