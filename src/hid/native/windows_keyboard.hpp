// windows_keyboard.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
    namespace native::windows
    {
        typedef int32_t vkey_t;
        struct kaymapping
        {
            key_code_e keyCode;
            vkey_t vk;
        };
        typedef std::array<kaymapping, ScanCodeCount> keymap_t;

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
            void input_language_changed();
            static bool set_ime_input_area(HIMC aContext, rect const& aArea);
            static bool set_ime_position(HIMC aContext, rect const& aArea, point const& aPosition);
        private:
            i_keyboard& iKeyboard;
            sink iSink;
            bool iOpen = false;
            HKL iLayout = nullptr;
            i_widget const* iInputWidget = nullptr;
            optional_point iPosition;
            HWND iSurface = nullptr;
            HIMC iContext = nullptr;
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
            key_modifiers_e modifiers() const final;
        public:
            key_code_e scan_code_to_key_code(scan_code_e aScanCode) const final;
        private:
            void update_keymap();
            void set_keymap(const keymap_t& aKeymap);
        public:
            static scan_code_e scan_code_from_message(LPARAM aLParam, WPARAM aWParam);
        private:
            keymap_t iKeymap;
            std::unique_ptr<i_keyboard_layout> iLayout;
        };
    }
}