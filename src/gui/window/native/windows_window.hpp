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
                failed_to_install_window_creation_hook(const std::string& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_install_window_creation_hook, reason: " + aReason) {}
            };
            struct failed_to_create_window : std::runtime_error {
                failed_to_create_window(const std::string& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_create_window: Failed to create window, reason: " + aReason) {}
            };
            struct failed_to_create_opengl_context : std::runtime_error {
                failed_to_create_opengl_context(const std::string& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_create_opengl_context: Failed to create OpenGL context, reason: " + aReason) {}
            };
            struct failed_to_activate_opengl_context : std::runtime_error {
                failed_to_activate_opengl_context(const std::string& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_activate_opengl_context: Failed to activate OpenGL context, reason: " + aReason) {}
            };
            struct failed_to_get_window_information : std::runtime_error {
                failed_to_get_window_information(const std::string& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_get_window_information: Failed to get window information, reason: " + aReason) {}
            };
            struct failed_to_set_window_attributes : std::runtime_error {
                failed_to_set_window_attributes(const std::string& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_set_window_attributes: Failed to set window attributes, reason: " + aReason) {}
            };
            struct failed_to_detach_from_windows_window : std::runtime_error {
                failed_to_detach_from_windows_window(const std::string& aReason) :
                    std::runtime_error("neogfx::native::windows::window::failed_to_detach_from_windows_window: Failed to detach from SDL window, reason: " + aReason) {}
            };
        public:
            static DWORD convert_style(window_style aStyle);
            static DWORD convert_ex_style(window_style aStyle);
        public:
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
            ~window();
        public:
            void* target_handle() const override;
            void* target_device_handle() const override;
            i_rendering_engine::pixel_format_t pixel_format() const override;
        public:
            void activate_target() const override;
        public:
            bool has_parent() const override;
            const i_native_window& parent() const override;
            i_native_window& parent() override;
        public:
            bool initialising() const override;
            void initialisation_complete() override;
            void* handle() const override;
            void* native_handle() const override;
            point surface_position() const override;
            void move_surface(const point& aPosition) override;
            size surface_size() const override;
            void resize_surface(const size& aSize) override;
        public:
            bool can_render() const override;
            void render(bool aOOBRequest = false) override;
        public:
            std::unique_ptr<i_rendering_context> create_graphics_context(blending_mode aBlendingMode = blending_mode::Default) const override;
            std::unique_ptr<i_rendering_context> create_graphics_context(const i_widget& aWidget, blending_mode aBlendingMode = blending_mode::Default) const override;
        public:
            void close(bool aForce = false) override;
            bool visible() const override;
            void show(bool aActivate = false) override;
            void hide() override;
            double opacity() const override;
            void set_opacity(double aOpacity) override;
            double transparency() const override;
            void set_transparency(double aTransparency) override;
            bool is_active() const override;
            void activate() override;
            bool is_iconic() const override;
            void iconize() override;
            bool is_maximized() const override;
            void maximize() override;
            bool is_restored() const override;
            void restore() override;
            bool is_fullscreen() const override;
            void enter_fullscreen(const video_mode& aVideoMode) override;
            bool enabled() const override;
            void enable(bool aEnable) override;
            bool is_capturing() const override;
            void set_capture() override;
            void release_capture() override;
            void non_client_set_capture() override;
            void non_client_release_capture() override;
            void set_title_text(const std::string& aTitleText) override;
            padding border_thickness() const override;
        protected:
            void set_destroying() override;
            void set_destroyed() override;
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
            mutable std::optional<i_rendering_engine::pixel_format_t> iPixelFormat;
            static window* sNewWindow;
            mutable optional_point iPosition;
            mutable optional_size iExtents;
            bool iVisible;
            bool iMouseEntered;
            bool iCapturingMouse;
            bool iNonClientCapturing;
            bool iReady;
            mutable padding iBorderThickness;
            widget_part_e iClickedWidgetPart;
            bool iSystemMenuOpen;
        };
    }
}