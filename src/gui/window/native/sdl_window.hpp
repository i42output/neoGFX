// sdl_window.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
#include <neogfx/core/geometry.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gui/window/window.hpp>
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
		struct no_cursors_saved : std::logic_error { no_cursors_saved() : std::logic_error("neogfx::sdl_window::no_cursors_saved") {} };
	private:
		typedef std::shared_ptr<SDL_Cursor> cursor_pointer;
	public:
		static uint32_t convert_style(window_style aStyle);
		static mouse_button convert_mouse_button(Uint32 aButton);
		static mouse_button convert_mouse_button(Uint8 aButtonIndex);
		static Uint32 convert_mouse_button(mouse_button aButton);
	public:
		sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
		sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
		sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
		sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, sdl_window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
		sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, sdl_window& aParent, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
		sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, sdl_window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default);
		~sdl_window();
	public:
		bool initialising() const override;
		void* handle() const override;
		void* native_handle() const override;
		point surface_position() const override;
		void move_surface(const point& aPosition) override;
		size surface_size() const override;
		void resize_surface(const size& aSize) override;
		point mouse_position() const override;
		bool is_mouse_button_pressed(mouse_button aButton) const override;
	public:
		void save_mouse_cursor() override;
		void set_mouse_cursor(mouse_system_cursor aSystemCursor) override;
		void restore_mouse_cursor() override;
		void update_mouse_cursor() override;
	public:
		bool can_render() const override;
	public:
		std::unique_ptr<i_native_graphics_context> create_graphics_context() const override;
		std::unique_ptr<i_native_graphics_context> create_graphics_context(const i_widget& aWidget) const override;
	public:
		void close() override;
		bool is_visible() const override;
		void show(bool aActivate = false) override;
		void hide() override;
		void set_transparency(double aAlpha) override;
		bool is_active() const override;
		void activate() override;
		bool is_enabled() const override;
		void enable(bool aEnable) override;
		bool is_capturing() const override;
		void set_capture() override;
		void release_capture() override;
		bool is_destroyed() const override;
	private:
		static void install_creation_hook(sdl_window& aNewWindow);
		static sdl_window* new_window();
		void init();
		void process_event(const SDL_Event& aEvent);
		virtual void destroying();
		virtual void destroyed();
		margins border_thickness() const;
		void push_mouse_button_event_extra_info(key_modifiers_e aKeyModifiers);
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
		cursor_pointer iCurrentCursor;
		std::vector<cursor_pointer> iSavedCursors;
		bool iReady;
		bool iDestroyed;
		mutable margins iBorderThickness;
		std::deque<key_modifiers_e> iMouseButtonEventExtraInfo;
		widget_part iClickedWidgetPart;
	};
}