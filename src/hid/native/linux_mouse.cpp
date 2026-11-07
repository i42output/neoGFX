// linux_mouse.cpp
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

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/hid/i_native_surface.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/gui/window/i_native_window.hpp>
#include "../../app/native/linux_basic_services.hpp"
#include "linux_mouse.hpp"

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

        mouse::mouse() :
            iCaptureTarget{ nullptr },
            iCaptureType{ mouse_capture_type::None }
        {
        }

        point mouse::position() const
        {
            return the_display_server().pointer_position();
        }

        mouse_button mouse::button_state() const
        {
            return the_display_server().pointer_buttons();
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

        void mouse::capture(i_surface& aTarget, bool aLockCursor)
        {
            if (iCaptureTarget != nullptr)
                throw already_capturing();
            the_display_server().capture_pointer(aTarget.as_surface_window().native_window());
            iCaptureTarget = &aTarget;
            iCaptureType = mouse_capture_type::Normal;
            if (aLockCursor)
                service<i_window_manager>().lock_mouse_cursor(aTarget.as_surface_window().as_window());
        }

        void mouse::capture_raw(i_surface& aTarget, bool aLockCursor)
        {
            if (iCaptureTarget != nullptr)
                throw already_capturing();
            the_display_server().capture_pointer(aTarget.as_surface_window().native_window());
            iCaptureTarget = &aTarget;
            iCaptureType = mouse_capture_type::Raw;
            if (aLockCursor)
                service<i_window_manager>().lock_mouse_cursor(aTarget.as_surface_window().as_window());
        }

        void mouse::release_capture()
        {
            if (iCaptureTarget == nullptr)
                throw not_capturing();
            the_display_server().release_pointer();
            iCaptureTarget = nullptr;
            iCaptureType = mouse_capture_type::None;
            if (service<i_window_manager>().cursor_locked())
                service<i_window_manager>().unlock_mouse_cursor();
        }
    }
}
