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

#include "neogfx.hpp"
#include <SDL.h>
#include <SDL_mouse.h>
#include "neogfx.hpp"
#include "sdl_renderer.hpp"
#include "opengl_window.hpp"
#include "sdl_graphics_context.hpp"


namespace neogfx
{
	class sdl_window : public opengl_window
	{
		friend class sdl_renderer;
	public:
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
		struct no_cursors_saved : std::logic_error { no_cursors_saved() : std::logic_error("neogfx::sdl_window::no_cursors_saved") {} };
	private:
		typedef std::shared_ptr<SDL_Cursor> cursor_pointer;
	public:
		static uint32_t convert_style(uint32_t aStyle);
		static mouse_button convert_mouse_button(Uint32 aButton);
		static mouse_button convert_mouse_button(Uint8 aButtonIndex);
		static Uint32 convert_mouse_button(mouse_button aButton);
	public:
		sdl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle = window::Default);
		sdl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, sdl_window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle = window::Default);
		~sdl_window();
	public:
		virtual void* handle() const;
		virtual void* native_handle() const;
		virtual void* native_context() const;
		virtual point surface_position() const;
		virtual void move_surface(const point& aPosition);
		virtual size surface_size() const;
		virtual void resize_surface(const size& aSize);
		virtual point mouse_position() const;
		virtual bool is_mouse_button_pressed(mouse_button aButton) const;
	public:
		virtual void save_mouse_cursor();
		virtual void set_mouse_cursor(mouse_system_cursor aSystemCursor);
		virtual void restore_mouse_cursor();
	public:
		virtual std::unique_ptr<i_native_graphics_context> create_graphics_context() const;
		virtual std::unique_ptr<i_native_graphics_context> create_graphics_context(const i_widget& aWidget) const;
	public:
		virtual void close();
		virtual bool is_active() const;
		virtual void activate();
		virtual void enable(bool aEnable);
		virtual void set_capture();
		virtual void release_capture();
	private:
		void process_event(const SDL_Event& aEvent);
	private:
		virtual void activate_context();
		virtual void deactivate_context();
		virtual void display();
		virtual bool processing_event() const;
	private:
		SDL_Window* iHandle;
		SDL_GLContext iContext;
		size iExtents;
		bool iProcessingEvent;
		bool iCapturingMouse;
		cursor_pointer iCurrentCursor;
		std::vector<cursor_pointer> iSavedCursors;
	};
}