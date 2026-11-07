// wayland_display_server.hpp
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

#include <set>

#include <neogfx/gui/widget/timer.hpp>
#include "i_native_clipboard.hpp"
#include "linux_display_server.hpp"

struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_shm;
struct wl_seat;
struct wl_pointer;
struct wl_keyboard;
struct wl_output;
struct wl_surface;
struct wl_data_device_manager;
struct wl_data_device;
struct wl_data_offer;
struct wl_data_source;
struct wl_cursor_theme;
struct xdg_wm_base;
struct zxdg_decoration_manager_v1;
struct xkb_context;
struct xkb_keymap;
struct xkb_state;
struct xkb_compose_table;
struct xkb_compose_state;

namespace neogfx
{
    namespace native::wayland
    {
        class window;

        class display_server : public linux::display_server
        {
            friend class native_clipboard;
        public:
            struct output_state
            {
                wl_output* handle = nullptr;
                std::uint32_t name = 0u;
                point position;
                size physicalSize; // mm
                size extents; // the current mode (pixels)
                std::uint32_t scale = 1u;
                std::int32_t subpixel = 0;
            };
        private:
            class native_clipboard : public i_native_clipboard
            {
            public:
                native_clipboard(display_server& aDisplayServer);
                ~native_clipboard();
            public:
                bool has_text() const final;
                i_string const& text() const final;
                void set_text(i_string const& aText) final;
                bool has_image() const final;
                neogfx::image image() const final;
                void set_image(const neogfx::image& aImage) final;
            public:
                void offer(wl_data_offer* aOffer);
                void offer_mime_type(wl_data_offer* aOffer, char const* aMimeType);
                void selection(wl_data_offer* aOffer);
                void send(wl_data_source* aSource, char const* aMimeType, int aFd);
                void cancelled(wl_data_source* aSource);
            private:
                std::optional<std::string> text_mime_type() const;
            private:
                display_server& iDisplayServer;
                std::map<wl_data_offer*, std::set<std::string>> iOffers;
                wl_data_offer* iSelection = nullptr;
                wl_data_source* iSource = nullptr;
                std::string iSourceText;
                mutable string iText;
            };
        public:
            display_server(char const* aDisplayName = nullptr);
            ~display_server();
        public:
            neogfx::windowing_system windowing_system() const final;
            void* native_display() const final;
            bool dispatch_events() final;
            void flush() final;
        public:
            std::uint32_t output_count() const final;
            linux::output_info output(std::uint32_t aIndex) const final;
        public:
            bool is_key_pressed(std::uint32_t aKeyCode) const final;
            std::uint32_t key_symbol(std::uint32_t aKeyCode) const final;
            key_modifier modifiers() const final;
            keyboard_locks locks() const final;
        public:
            point pointer_position() const final;
            mouse_button pointer_buttons() const final;
            i_native_window* window_under_pointer() const final;
            void set_cursor(mouse_system_cursor aCursor) final;
            void capture_pointer(i_native_window& aWindow) final;
            void release_pointer() final;
        public:
            i_native_clipboard& clipboard() final;
            void beep() final;
            color read_pixel(point const& aPosition) const final;
        public:
            wl_display* display() const;
            wl_compositor* compositor() const;
            xdg_wm_base* wm_base() const;
            std::uint32_t wm_base_version() const;
            zxdg_decoration_manager_v1* decoration_manager() const;
            wl_seat* seat() const;
            // the serial of the latest input event (for requests that must be in response to one)
            std::uint32_t input_serial() const;
            std::uint32_t pointer_button_serial() const;
            std::optional<std::uint32_t> output_scale(wl_output* aOutput) const;
            std::uint32_t max_output_scale() const;
            bool has_preferred_buffer_scale() const;
        public:
            void add_window(window& aWindow);
            void remove_window(window& aWindow);
            window* keyboard_focus() const;
        public:
            // listeners (see wayland_display_server.cpp)
            void registry_global(wl_registry* aRegistry, std::uint32_t aName, char const* aInterface, std::uint32_t aVersion);
            void registry_global_remove(wl_registry* aRegistry, std::uint32_t aName);
            void seat_capabilities(std::uint32_t aCapabilities);
            void output_geometry(wl_output* aOutput, std::int32_t aX, std::int32_t aY, std::int32_t aPhysicalWidth, std::int32_t aPhysicalHeight, std::int32_t aSubpixel);
            void output_mode(wl_output* aOutput, std::uint32_t aFlags, std::int32_t aWidth, std::int32_t aHeight);
            void output_scale_changed(wl_output* aOutput, std::int32_t aScale);
            void output_done(wl_output* aOutput);
            void pointer_enter(std::uint32_t aSerial, wl_surface* aSurface, double aX, double aY);
            void pointer_leave(std::uint32_t aSerial, wl_surface* aSurface);
            void pointer_motion(double aX, double aY);
            void pointer_button(std::uint32_t aSerial, std::uint32_t aButton, std::uint32_t aState);
            void pointer_axis(std::uint32_t aAxis, double aValue);
            void pointer_axis_discrete(std::uint32_t aAxis, std::int32_t aDiscrete);
            void pointer_axis_value120(std::uint32_t aAxis, std::int32_t aValue120);
            void pointer_frame();
            void keyboard_keymap(std::uint32_t aFormat, std::int32_t aFd, std::uint32_t aSize);
            void keyboard_enter(std::uint32_t aSerial, wl_surface* aSurface);
            void keyboard_leave(std::uint32_t aSerial, wl_surface* aSurface);
            void keyboard_key(std::uint32_t aSerial, std::uint32_t aKey, std::uint32_t aState);
            void keyboard_modifiers(std::uint32_t aSerial, std::uint32_t aDepressed, std::uint32_t aLatched, std::uint32_t aLocked, std::uint32_t aGroup);
            void keyboard_repeat_info(std::int32_t aRate, std::int32_t aDelay);
            void data_device_offer(wl_data_offer* aOffer);
            void data_device_selection(wl_data_offer* aOffer);
            void data_offer_offer(wl_data_offer* aOffer, char const* aMimeType);
            void data_source_send(wl_data_source* aSource, char const* aMimeType, std::int32_t aFd);
            void data_source_cancelled(wl_data_source* aSource);
        private:
            void release();
            void key(std::uint32_t aKey, bool aPressed, bool aRepeat);
            void stop_key_repeat();
            window* keyboard_target() const;
            void update_cursor();
            void update_cursor_theme(std::uint32_t aScale);
        private:
            wl_display* iDisplay = nullptr;
            wl_registry* iRegistry = nullptr;
            wl_compositor* iCompositor = nullptr;
            std::uint32_t iCompositorVersion = 0u;
            wl_shm* iShm = nullptr;
            xdg_wm_base* iWmBase = nullptr;
            std::uint32_t iWmBaseVersion = 0u;
            zxdg_decoration_manager_v1* iDecorationManager = nullptr;
            wl_data_device_manager* iDataDeviceManager = nullptr;
            wl_seat* iSeat = nullptr;
            std::uint32_t iSeatVersion = 0u;
            wl_pointer* iPointer = nullptr;
            wl_keyboard* iKeyboard = nullptr;
            wl_data_device* iDataDevice = nullptr;
            std::vector<output_state> iOutputs;
            std::vector<window*> iWindows;
            // pointer
            window* iPointerFocus = nullptr;
            point iPointerPosition;
            mouse_button iPointerButtons = mouse_button::None;
            std::uint32_t iPointerEnterSerial = 0u;
            std::uint32_t iPointerButtonSerial = 0u;
            delta iAxis;
            delta iAxisDiscrete;
            bool iAxisValue120 = false;
            std::optional<window*> iCapture;
            mouse_system_cursor iCursor = mouse_system_cursor::Arrow;
            wl_cursor_theme* iCursorTheme = nullptr;
            std::uint32_t iCursorThemeScale = 0u;
            wl_surface* iCursorSurface = nullptr;
            // keyboard
            xkb_context* iXkbContext = nullptr;
            xkb_keymap* iXkbKeymap = nullptr;
            xkb_state* iXkbState = nullptr;
            xkb_compose_table* iXkbComposeTable = nullptr;
            xkb_compose_state* iXkbComposeState = nullptr;
            window* iKeyboardFocus = nullptr;
            std::set<std::uint32_t> iKeysPressed;
            bool iInsertLock = true; // (as Windows, where neoGFX turns it on at startup)
            std::uint32_t iInputSerial = 0u;
            std::int32_t iRepeatRate = 25;
            std::int32_t iRepeatDelay = 600;
            std::optional<std::uint32_t> iRepeatKey;
            std::optional<neolib::callback_timer> iRepeatTimer;
            // clipboard
            std::unique_ptr<native_clipboard> iClipboard;
        };
    }
}
