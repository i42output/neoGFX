// wayland_window.hpp
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

#include <neogfx/core/geometrical.hpp>
#include <neogfx/hid/video_mode.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gui/window/window_bits.hpp>
#include "linux_window.hpp"

struct wl_surface;
struct wl_output;
struct wl_egl_window;
struct xdg_surface;
struct xdg_toplevel;
struct xdg_popup;
struct zxdg_toplevel_decoration_v1;

namespace neogfx
{
    namespace native::wayland
    {
        class display_server;

        // A Wayland surface with the xdg-shell role of a toplevel or, when it is a popup with a parent, of a popup.
        // Wayland tells a client nothing about where its windows are: a toplevel's position is wherever neoGFX
        // put it (it cannot be moved by the client), and a popup's position, which is relative to its parent, is
        // in that coordinate space too.
        class window : public linux::window
        {
        public:
            struct failed_to_create_window : std::runtime_error {
                failed_to_create_window(std::string const& aReason) :
                    std::runtime_error("neogfx::native::wayland::window::failed_to_create_window: Failed to create window, reason: " + aReason) {}
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
            wayland::display_server& wayland_display_server() const;
            window* wayland_parent() const;
            wl_surface* surface() const;
            std::uint32_t scale() const;
            // (listeners)
            void surface_enter(wl_output* aOutput);
            void surface_leave(wl_output* aOutput);
            void surface_preferred_buffer_scale(std::int32_t aScale);
            void xdg_surface_configure(std::uint32_t aSerial);
            void toplevel_configure(std::int32_t aWidth, std::int32_t aHeight, std::vector<std::uint32_t> const& aStates);
            void toplevel_close();
            void popup_configure(std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight);
            void popup_done();
            void outputs_changed();
        private:
            void begin_move(point const& aPosition, std::uint32_t aSerial) final;
            void begin_resize(widget_part_e aBorder, point const& aPosition, std::uint32_t aSerial) final;
            void show_window_menu(point const& aPosition, std::uint32_t aSerial) final;
            void extents_changed() final;
            void position_changed() final;
        private:
            void init(string const& aWindowTitle);
            void create_role();
            void destroy_role();
            void reposition_popup();
            void set_scale(std::uint32_t aScale);
            void update_scale();
            void update_min_size();
        private:
            wayland::display_server& iDisplayServer;
            wl_surface* iSurface = nullptr;
            wl_egl_window* iEglWindow = nullptr;
            xdg_surface* iXdgSurface = nullptr;
            xdg_toplevel* iToplevel = nullptr;
            xdg_popup* iPopup = nullptr;
            zxdg_toplevel_decoration_v1* iDecoration = nullptr;
            std::vector<wl_output*> iOutputs;
            std::optional<std::uint32_t> iPreferredScale;
            std::uint32_t iScale = 1u;
            std::optional<basic_size<std::int32_t>> iConfigureSize;
            std::vector<std::uint32_t> iConfigureStates;
            bool iConfigured = false;
            bool iActivated = false;
            bool iFullscreen = false;
            bool iResizing = false;
            bool iRepositioning = false;
        };
    }
}
