// sdl_window.hpp
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
#include <SDL.h>
#include <SDL_mouse.h>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/hid/video_mode.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gui/window/window_bits.hpp>
#include "opengl_window.hpp"


namespace neogfx
{
    class sdl_renderer;

    class sdl_window : public opengl_window
    {
        friend class sdl_renderer;
    public:
        struct failed_to_install_window_creation_hook : std::runtime_error { 
            failed_to_install_window_creation_hook(const std::string& aReason) : 
                std::runtime_error("neogfx::sdl_window::failed_to_install_window_creation_hook, reason: " + aReason) {} 
        };
        struct failed_to_create_window : std::runtime_error {
            failed_to_create_window(const std::string& aReason) :
                std::runtime_error("neogfx::sdl_window::failed_to_create_window: Failed to create window, reason: " + aReason) {}
        };
        struct failed_to_create_opengl_context : std::runtime_error {
            failed_to_create_opengl_context(const std::string& aReason) :
                std::runtime_error("neogfx::sdl_window::failed_to_create_opengl_context: Failed to create OpenGL context, reason: " + aReason) {}
        };
        struct failed_to_activate_opengl_context : std::runtime_error {
            failed_to_activate_opengl_context(const std::string& aReason) :
                std::runtime_error("neogfx::sdl_window::failed_to_activate_opengl_context: Failed to activate OpenGL context, reason: " + aReason) {}
        };
        struct failed_to_get_window_information : std::runtime_error {
            failed_to_get_window_information(const std::string& aReason) :
                std::runtime_error("neogfx::sdl_window::failed_to_get_window_information: Failed to get window information, reason: " + aReason) {}
        };
        struct failed_to_detach_from_sdl_window : std::runtime_error {
            failed_to_detach_from_sdl_window(const std::string& aReason) :
                std::runtime_error("neogfx::sdl_window::failed_to_detach_from_sdl_window: Failed to detach from SDL window, reason: " + aReason) {}
        };
    public:
        static uint32_t convert_style(window_style aStyle);
    public:
        sdl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
        sdl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
        sdl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
        sdl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, sdl_window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
        sdl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, sdl_window& aParent, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
        sdl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, sdl_window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
        ~sdl_window();
    public:
        void* target_handle() const override;
    public:
        bool has_parent() const override;
        const i_native_window& parent() const override;
        i_native_window& parent() override;
    public:
        bool initialising() const override;
        void* handle() const override;
        void* native_handle() const override;
        point surface_position() const override;
        void move_surface(const point& aPosition) override;
        size surface_size() const override;
        void resize_surface(const size& aSize) override;
    public:
        bool can_render() const override;
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
        bool enabled() const override;
        void enable(bool aEnable) override;
        bool is_capturing() const override;
        void set_capture() override;
        void release_capture() override;
        void non_client_set_capture() override;
        void non_client_release_capture() override;
        void set_title_text(const std::string& aTitleText) override;
    protected:
        void set_destroying() override;
        void set_destroyed() override;
    private:
        static void install_creation_hook(sdl_window& aNewWindow);
        static sdl_window* new_window();
        void init();
        void process_event(const SDL_Event& aEvent);
        margins border_thickness() const;
        void push_mouse_button_event_extra_info(key_modifiers_e aKeyModifiers);
        void raw_input_mouse_button_event_extra_info(key_modifiers_e aKeyModifiers);
        key_modifiers_e pop_mouse_button_event_extra_info();
#ifdef WIN32
        static LRESULT CALLBACK CustomWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#endif
    private:
        virtual void display();
    private:
        sdl_window* iParent;
        surface_style iStyle;
        SDL_Window* iHandle;
        mutable void* iNativeHandle;
        static sdl_window* sNewWindow;
#ifdef WIN32
        static WNDPROC sSDLWindowProc;
#endif
        size iExtents;
        bool iVisible;
        bool iCapturingMouse;
        bool iNonClientCapturing;
        bool iReady;
        mutable margins iBorderThickness;
        std::deque<key_modifiers_e> iMouseButtonEventExtraInfo;
        key_modifiers_e iRawInputMouseButtonEventExtraInfo;
        widget_part iClickedWidgetPart;
        bool iSystemMenuOpen;
    };
}