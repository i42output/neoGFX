// windows_mouse.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston
  
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

#include <neogfx/core/geometrical.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include <neogfx/hid/mouse.hpp>

namespace neogfx
{
    namespace native::windows
    {
        class mouse : public neogfx::mouse
        {
        public:
            typedef std::uint32_t virtual_key_code_t;
        public:
            mouse();
        public:
            point position() const override;
            mouse_button button_state() const override;
        public:
            bool capturing() const override;
            i_surface& capture_target() const override;
            mouse_capture_type capture_type() const override;
            void capture(i_surface& aTarget, bool aLockCursor = true) override;
            void capture_raw(i_surface& aTarget, bool aLockCursor = true) override;
            void release_capture() override;
        public:
            static mouse_button convert_button(virtual_key_code_t aVirtualKeyCode);
            static virtual_key_code_t convert_button(mouse_button aButton);
            static mouse_button button_from_message(WPARAM aWParam);
            static key_modifiers_e modifiers_from_message(WPARAM aWParam);
        private:
            i_surface* iCaptureTarget;
            mouse_capture_type iCaptureType;
        };
    }
}