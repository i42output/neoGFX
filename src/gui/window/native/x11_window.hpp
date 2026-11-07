// x11_window.hpp
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

#include "linux_window.hpp"

typedef struct _XIC* XIC;
typedef union _XEvent XEvent;
typedef unsigned long XID;
typedef unsigned long Atom;

namespace neogfx
{
    namespace native::x11
    {
        class display_server;

        // An X11 window. A popup is an override-redirect window (as a menu is on X11, and as a popup's window has no
        // frame or activation on Windows); a window that draws its own decoration asks the window manager for none.
        class window : public linux::window
        {
        public:
            struct failed_to_create_window : std::runtime_error {
                failed_to_create_window(std::string const& aReason) :
                    std::runtime_error("neogfx::native::x11::window::failed_to_create_window: Failed to create window, reason: " + aReason) {}
            };
        public:
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, string const& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_size<int>& aDimensions, string const& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, string const& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const video_mode& aVideoMode, string const& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const basic_size<int>& aDimensions, string const& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, string const& aWindowTitle, window_style aStyle = window_style::Default);
            ~window();
        public:
            void* target_handle() const final;
            void* target_device_handle() const final;
        public:
            void* handle() const final;
            void* native_handle() const final;
            void move_surface(const point& aPosition) final;
            void resize_surface(const size& aExtents) final;
            bool resizing_or_moving() const final;
        public:
            void close(bool aForce = false) final;
            void show(bool aActivate = false) final;
            void hide() final;
            double opacity() const final;
            void set_opacity(double aOpacity) final;
            bool is_effectively_active() const final;
            void activate() final;
            void iconize() final;
            void maximize() final;
            void restore() final;
            void enter_fullscreen(const video_mode& aVideoMode) final;
            void set_title_text(i_string const& aTitleText) final;
            void alert(window_alert aAlert, std::optional<std::chrono::milliseconds> const& aInterval = {}, std::optional<std::uint32_t> const& aCount = {}) final;
        public:
            bool can_render() const final;
            void display() final;
        public:
            XID xwindow() const;
            XIC input_context() const;
            bool override_redirect() const;
            // (from the display server)
            bool dragging() const;
            void drag(point const& aRootPosition);
            void end_drag();
            void configure_notify(XEvent const& aEvent);
            void map_notify();
            void unmap_notify();
            void property_notify(Atom aProperty);
            void expose(rect const& aArea);
            void delete_window();
        private:
            void begin_move(point const& aPosition, std::uint32_t aSerial) final;
            void begin_resize(widget_part_e aBorder, point const& aPosition, std::uint32_t aSerial) final;
            void show_window_menu(point const& aPosition, std::uint32_t aSerial) final;
        private:
            void init(string const& aWindowTitle);
            void set_net_wm_state(bool aAdd, Atom aState1, Atom aState2 = 0ul);
            void update_size_hints();
            void begin_move_resize(int aDirection, point const& aPosition);
        private:
            x11::display_server& iDisplayServer;
            XID iWindow = 0ul;
            XIC iInputContext = nullptr;
            bool iOverrideRedirect = false;
            bool iMapped = false;
            bool iResizingOrMoving = false;
            struct manual_drag
            {
                widget_part_e part;
                point start;
                rect startRect;
            };
            std::optional<manual_drag> iDrag;
        };
    }
}
