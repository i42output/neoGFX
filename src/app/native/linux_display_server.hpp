// linux_display_server.hpp
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

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/color.hpp>
#include <neogfx/hid/i_display.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include <neogfx/hid/i_mouse.hpp>

namespace neogfx
{
    class i_native_clipboard;
    class i_native_window;

    namespace native::linux
    {
        // A monitor, as the display server describes it (in pixels).
        struct output_info
        {
            rect monitor;
            rect workArea;
            size dpi;
            std::uint32_t scale = 1u;
            neogfx::subpixel_format subpixelFormat = neogfx::subpixel_format::None;
        };

        // The connection to the display server (the Wayland compositor or the X server): it is what the Linux
        // implementations of the platform services (keyboard, mouse, window manager, clipboard, displays) have
        // in common, so that only the windows and this connection differ between Wayland and X11.
        class display_server
        {
        public:
            struct failed_to_connect : std::runtime_error { failed_to_connect(std::string const& aReason) : std::runtime_error{ "neogfx::native::linux::display_server::failed_to_connect: " + aReason } {} };
        public:
            virtual ~display_server() = default;
        public:
            virtual neogfx::windowing_system windowing_system() const = 0;
            // wl_display* or Display*
            virtual void* native_display() const = 0;
            // Dispatches the events received from the display server, without waiting for any; returns true if
            // there were any.
            virtual bool dispatch_events() = 0;
            virtual void flush() = 0;
        public:
            virtual std::uint32_t output_count() const = 0;
            virtual output_info output(std::uint32_t aIndex) const = 0;
        public:
            // the key's (evdev) code, as the scan codes are
            virtual bool is_key_pressed(std::uint32_t aKeyCode) const = 0;
            // the symbol (xkb keysym) the key produces at the first level of the current layout
            virtual std::uint32_t key_symbol(std::uint32_t aKeyCode) const = 0;
            virtual key_modifier modifiers() const = 0;
            virtual keyboard_locks locks() const = 0;
        public:
            // In the coordinate space of the windows' surface positions (on Wayland, which tells clients nothing about
            // where their windows are, that is the window under the pointer's position plus where neoGFX put it).
            virtual point pointer_position() const = 0;
            virtual mouse_button pointer_buttons() const = 0;
            virtual i_native_window* window_under_pointer() const = 0;
            virtual void set_cursor(mouse_system_cursor aCursor) = 0;
            virtual void capture_pointer(i_native_window& aWindow) = 0;
            virtual void release_pointer() = 0;
        public:
            virtual i_native_clipboard& clipboard() = 0;
            virtual void beep() = 0;
            // the colour of a pixel on the screen (if the display server allows clients to see it)
            virtual color read_pixel(point const& aPosition) const = 0;
        };

        // Maps an evdev key code (as both Wayland and X11 (less 8) report keys) to a neoGFX (USB HID) scan code.
        scan_code_e evdev_to_scan_code(std::uint32_t aKeyCode);
        // and back
        std::uint32_t scan_code_to_evdev(scan_code_e aScanCode);
        // The UTF-8 text an xkb keysym stands for, if any.
        std::string key_symbol_to_utf8(std::uint32_t aKeySymbol);
        // The system cursor's names, most preferred first (both the CSS names, as cursor themes now use, and the
        // X11 names older themes have).
        std::vector<char const*> const& cursor_names(mouse_system_cursor aCursor);
        // When a press of the same button counts as a double click.
        std::chrono::milliseconds double_click_time();
        constexpr std::int32_t DOUBLE_CLICK_DISTANCE = 4;
        // The mouse wheel delta (as Windows: 120 per notch) of a wheel click.
        constexpr std::int16_t WHEEL_DELTA = 120;
    }
}
