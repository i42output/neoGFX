// windows_window.hpp
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
#include <neogfx/core/geometrical.hpp>
#include <neogfx/hid/video_mode.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gui/window/window_bits.hpp>
#include "opengl_window.hpp"


namespace neogfx
{
    namespace native::windows
    {
        class renderer;

        class window : public opengl_window
        {
            friend class renderer;
        public:
            struct failed_to_install_window_creation_hook : std::runtime_error {
                failed_to_install_window_creation_hook(std::string const& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_install_window_creation_hook, reason: " + aReason) {}
            };
            struct failed_to_create_window : std::runtime_error {
                failed_to_create_window(std::string const& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_create_window: Failed to create window, reason: " + aReason) {}
            };
            struct failed_to_create_opengl_context : std::runtime_error {
                failed_to_create_opengl_context(std::string const& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_create_opengl_context: Failed to create OpenGL context, reason: " + aReason) {}
            };
            struct failed_to_activate_opengl_context : std::runtime_error {
                failed_to_activate_opengl_context(std::string const& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_activate_opengl_context: Failed to activate OpenGL context, reason: " + aReason) {}
            };
            struct failed_to_get_window_information : std::runtime_error {
                failed_to_get_window_information(std::string const& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_get_window_information: Failed to get window information, reason: " + aReason) {}
            };
            struct failed_to_set_window_attributes : std::runtime_error {
                failed_to_set_window_attributes(std::string const& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_set_window_attributes: Failed to set window attributes, reason: " + aReason) {}
            };
            struct failed_to_detach_from_windows_window : std::runtime_error {
                failed_to_detach_from_windows_window(std::string const& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_detach_from_windows_window: Failed to detach from SDL window, reason: " + aReason) {}
            };
        public:
            static DWORD convert_style(window_style aStyle);
            static DWORD convert_ex_style(window_style aStyle);
        public:
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, std::string const& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const video_mode& aVideoMode, std::string const& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle = window_style::Default);
            ~window();
        public:
            void* target_handle() const final;
            void* target_device_handle() const final;
            pixel_format_t pixel_format() const final;
        public:
            void activate_target() const final;
        public:
            bool has_parent() const final;
            const i_native_window& parent() const final;
            i_native_window& parent() final;
            bool is_nested() const final;
        public:
            bool initialising() const final;
            void initialisation_complete() final;
            void* handle() const final;
            void* native_handle() const final;
            point surface_position() const final;
            void move_surface(const point& aPosition) final;
            size surface_extents() const final;
            void resize_surface(const size& aExtents) final;
        public:
            bool can_render() const final;
            void render(bool aOOBRequest = false) final;
        public:
            std::unique_ptr<i_rendering_context> create_graphics_context(blending_mode aBlendingMode = blending_mode::Default) const final;
            std::unique_ptr<i_rendering_context> create_graphics_context(const i_widget& aWidget, blending_mode aBlendingMode = blending_mode::Default) const final;
        public:
            void close(bool aForce = false) final;
            bool placement_changed_explicitly() const final;
            void set_placement_changed_explicitly() final;
            bool visible() const final;
            void show(bool aActivate = false) final;
            void hide() final;
            double opacity() const final;
            void set_opacity(double aOpacity) final;
            double transparency() const final;
            void set_transparency(double aTransparency) final;
            bool is_effectively_active() const final;
            bool is_active() const final;
            void activate() final;
            void deactivate() final;
            bool is_iconic() const final;
            void iconize() final;
            bool is_maximized() const final;
            void maximize() final;
            bool is_restored() const final;
            void restore() final;
            bool is_fullscreen() const final;
            void enter_fullscreen(const video_mode& aVideoMode) final;
            bool enabled() const final;
            void enable(bool aEnable) final;
            bool is_capturing() const final;
            void set_capture() final;
            void release_capture() final;
            void non_client_set_capture() final;
            void non_client_release_capture() final;
            void set_title_text(i_string const& aTitleText) final;
            border border_thickness() const final;
        protected:
            void set_destroying() final;
            void set_destroyed() final;
        private:
            static window* new_window();
            void init();
        private:
            static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
        private:
            virtual void display();
        private:
            window* iParent;
            surface_style iStyle;
            HWND iHandle;
            HDC iHdc;
            mutable bool iFirstActivation = true;
            mutable std::optional<pixel_format_t> iPixelFormat;
            static window* sNewWindow;
            mutable optional_point iPosition;
            mutable optional_size iExtents;
            bool iInMoveResizeCall = false;
            bool iPlacementChangedExplicitly = false;
            bool iVisible = false;
            bool iActive = false;
            std::optional<point> iActivationMousePos;
            bool iMouseEntered = false;
            bool iCapturingMouse = false;
            bool iNonClientCapturing = false;
            bool iReady = false;
            mutable border iBorderThickness;
            widget_part_e iClickedWidgetPart = widget_part::Nowhere;
            bool iSystemMenuOpen = false;
        };
    }
}