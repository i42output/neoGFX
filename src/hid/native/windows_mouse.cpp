// windows_mouse.cpp
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

#include <neogfx/neogfx.hpp>

#include <neogfx/hid/i_surface.hpp>
#include <neogfx/hid/i_native_surface.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/gui/window/i_native_window.hpp>
#include "windows_mouse.hpp"

namespace neogfx
{
    namespace native::windows
    {
        mouse::mouse() :
            iCaptureTarget{ nullptr },
            iCaptureType{ mouse_capture_type::None }
        {
        }

        point mouse::position() const
        {
            POINT pt = {};
            ::GetCursorPos(&pt);
            return basic_point<LONG>{ pt.x, pt.y };
        }

        mouse_button mouse::button_state() const
        {
            mouse_button result = mouse_button::None;
            if (GetKeyState(convert_button(mouse_button::Left)) >> 1)
                result = result | mouse_button::Left;
            if (GetKeyState(convert_button(mouse_button::Right)) >> 1)
                result = result | mouse_button::Right;
            if (GetKeyState(convert_button(mouse_button::Middle)) >> 1)
                result = result | mouse_button::Middle;
            if (GetKeyState(convert_button(mouse_button::X1)) >> 1)
                result = result | mouse_button::X1;
            if (GetKeyState(convert_button(mouse_button::X2)) >> 1)
                result = result | mouse_button::X2;
            return result;
        }

        bool mouse::capturing() const
        {
            return iCaptureTarget != nullptr;
        }

        i_surface& mouse::capture_target() const
        {
            if (iCaptureTarget == nullptr)
                throw not_capturing();
            return *iCaptureTarget;
        }

        mouse_capture_type mouse::capture_type() const
        {
            return iCaptureType;
        }

        void mouse::capture(i_surface& aTarget)
        {
            if (iCaptureTarget != nullptr)
                throw already_capturing();
            ::SetCapture(static_cast<HWND>(aTarget.as_surface_window().native_window().native_handle()));
            iCaptureTarget = &aTarget;
            iCaptureType = mouse_capture_type::Normal;
            service<i_window_manager>().lock_mouse_cursor(aTarget.as_surface_window().as_window());
        }

        void mouse::capture_raw(i_surface& aTarget)
        {
            if (iCaptureTarget != nullptr)
                throw already_capturing();
            RAWINPUTDEVICE rawMouse = { 0x01, 0x02, 0, NULL };
            ::RegisterRawInputDevices(&rawMouse, 1, sizeof(RAWINPUTDEVICE));
            iCaptureTarget = &aTarget;
            iCaptureType = mouse_capture_type::Raw;
            service<i_window_manager>().lock_mouse_cursor(aTarget.as_surface_window().as_window());
        }

        void mouse::release_capture()
        {
            if (iCaptureTarget == nullptr)
                throw not_capturing();
            if (capture_type() == mouse_capture_type::Normal)
                ::ReleaseCapture();
            else if (capture_type() == mouse_capture_type::Raw)
            {
                RAWINPUTDEVICE rawMouse = { 0x01, 0x02, RIDEV_REMOVE, NULL }; /* Mouse: UsagePage = 1, Usage = 2 */
                ::RegisterRawInputDevices(&rawMouse, 1, sizeof(RAWINPUTDEVICE));
            }
            iCaptureTarget = nullptr;
            iCaptureType = mouse_capture_type::None;
            service<i_window_manager>().unlock_mouse_cursor();
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

        mouse_button mouse::button_from_message(WPARAM aWParam)
        {
            mouse_button result = mouse_button::None;
            if (aWParam & MK_LBUTTON)
                result = result | mouse_button::Left;
            if (aWParam & MK_RBUTTON)
                result = result | mouse_button::Right;
            if (aWParam & MK_MBUTTON)
                result = result | mouse_button::Middle;
            if (aWParam & MK_XBUTTON1)
                result = result | mouse_button::X1;
            if (aWParam & MK_XBUTTON2)
                result = result | mouse_button::X2;
            return result;
        }

        key_modifiers_e mouse::modifiers_from_message(WPARAM aWParam)
        {
            key_modifiers_e result = KeyModifier_NONE;
            if (aWParam & MK_CONTROL)
                result = static_cast<key_modifiers_e>(result | KeyModifier_CTRL);
            if (aWParam & MK_SHIFT)
                result = static_cast<key_modifiers_e>(result | KeyModifier_SHIFT);
            return result;
        }
    }
}