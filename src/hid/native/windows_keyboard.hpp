// windows_keyboard.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/hid/keyboard.hpp>

namespace neogfx
{
    namespace native::windows
    {
        class keyboard : public neogfx::keyboard
        {
        public:
            keyboard();
        public:
            bool is_key_pressed(scan_code_e aScanCode) const override;
            keyboard_locks locks() const override;
            key_modifiers_e modifiers() const override;
            static scan_code_e scan_code_from_message(LPARAM aLParam, WPARAM aWParam);
            static key_code_e scan_code_to_key_code(scan_code_e aScanCode);
        };
    }
}