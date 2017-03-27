// sdl_renderer.hpp
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
#include <set>
#include <map>
#include "opengl_renderer.hpp"
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/keyboard.hpp>

namespace neogfx
{
	class i_native_surface;

	class sdl_renderer : public opengl_renderer
	{
	public:
		struct failed_to_activate_gl_context : std::runtime_error { failed_to_activate_gl_context(const std::string& aReason) : std::runtime_error("neogfx::sdl_renderer::failed_to_activate_gl_context: " + aReason) {} };
		struct failed_to_create_system_cache_window : std::runtime_error { failed_to_create_system_cache_window(const std::string& aReason) : std::runtime_error("neogfx::sdl_renderer::failed_to_create_system_cache_window: " + aReason) {} };
	public:
		sdl_renderer(i_basic_services& aBasicServices, i_keyboard& aKeyboard);
		~sdl_renderer();
	public:
		virtual const i_native_surface* active_context_surface() const;
		virtual void activate_context(const i_native_surface& aSurface);
		virtual void deactivate_context();
		virtual opengl_context create_context(const i_native_surface& aSurface);
		virtual void destroy_context(opengl_context aContext);
		virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const video_mode& aVideoMode, const std::string& aWindowTitle, window::style_e aStyle);
		virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const size& aDimensions, const std::string& aWindowTitle, window::style_e aStyle);
		virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window::style_e aStyle);
		virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, i_native_surface& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window::style_e aStyle);
		virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, i_native_surface& aParent, const size& aDimensions, const std::string& aWindowTitle, window::style_e aStyle);
		virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, i_native_surface& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window::style_e aStyle);
		virtual bool creating_window() const;
		virtual void render_now();
	public:
		virtual bool process_events();
	private:
		bool queue_events();
	private:
		i_basic_services& iBasicServices;
		i_keyboard& iKeyboard;
		void* iSystemCacheWindowHandle;
		opengl_context iContext;
		uint32_t iCreatingWindow;
		const i_native_surface* iActiveContextSurface;
	};
}