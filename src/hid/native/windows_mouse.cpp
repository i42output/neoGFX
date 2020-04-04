// windows_mouse.cpp
/*
  neogfx C++ GUI Library
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

#include <neogfx/neogfx.hpp>
#include "windows_mouse.hpp"

namespace neogfx
{
    namespace native::windows
    {
        point mouse::position() const
        {
            POINT pt = {};
            ::GetCursorPos(&pt);
            return basic_point<LONG>{ pt.x, pt.y };
        }

        mouse_button mouse::button_state() const
        {
            mouse_button result = mouse_button::None;
            if (GetKeyState(convert_button(mouse_button::Left)) & 0x0001)
                result = result | mouse_button::Left;
            if (GetKeyState(convert_button(mouse_button::Right)) & 0x0001)
                result = result | mouse_button::Right;
            if (GetKeyState(convert_button(mouse_button::Middle)) & 0x0001)
                result = result | mouse_button::Middle;
            if (GetKeyState(convert_button(mouse_button::X1)) & 0x0001)
                result = result | mouse_button::X1;
            if (GetKeyState(convert_button(mouse_button::X2)) & 0x0001)
                result = result | mouse_button::X2;
            return result;
        }
 
        mouse_button mouse::convert_button(virtual_key_code_t aVirtualKeyCode)
        {
            switch (aVirtualKeyCode)
            {
            case VK_LBUTTON:
                return mouse_button::Left;
            case VK_RBUTTON:
                return mouse_button::Right;
            case VK_MBUTTON:
                return mouse_button::Middle;
            case VK_XBUTTON1:
                return mouse_button::X1;
            case VK_XBUTTON2:
                return mouse_button::X2;
            default:
                return mouse_button::None;
            }
        }

        mouse::virtual_key_code_t mouse::convert_button(mouse_button aButton)
        {
            switch (aButton)
            {
            case mouse_button::Left:
                return VK_LBUTTON;
            case mouse_button::Right:
                return VK_RBUTTON;
            case mouse_button::Middle:
                return VK_MBUTTON;
            case mouse_button::X1:
                return VK_XBUTTON1;
            case mouse_button::X2:
                return VK_XBUTTON2;
            default:
                return 0u;
            }
        }
    }
}