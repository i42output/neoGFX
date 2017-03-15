// sdl_renderer.cpp
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

#include <neogfx/neogfx.hpp>
#include <SDL.h>
#include <neolib/raii.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/hid/surface_manager.hpp>
#include "../../gui/window/native/sdl_window.hpp"
#include "sdl_renderer.hpp"

namespace neogfx
{
	sdl_renderer::sdl_renderer(i_basic_services& aBasicServices, i_keyboard& aKeyboard) : iBasicServices(aBasicServices), iKeyboard(aKeyboard), iCreatingWindow(0), iContext(nullptr)
	{
		SDL_Init(SDL_INIT_VIDEO);
	}

	sdl_renderer::~sdl_renderer()
	{
		SDL_Quit();
	}

	void sdl_renderer::activate_context(const i_native_surface& aSurface)
	{
		if (iContext == nullptr)
			iContext = create_context(aSurface);
		else
			SDL_GL_MakeCurrent(static_cast<SDL_Window*>(aSurface.handle()), static_cast<SDL_GLContext>(iContext));

		static bool initialized = false;
		if (!initialized)
		{
			initialize();
			initialized = true;
		}
	}

	i_rendering_engine::opengl_context sdl_renderer::create_context(const i_native_surface& aSurface)
	{
		return SDL_GL_CreateContext(static_cast<SDL_Window*>(aSurface.handle()));
	}

	void sdl_renderer::destroy_context(opengl_context aContext)
	{
		SDL_GL_DeleteContext(static_cast<SDL_GLContext>(aContext));
		if (iContext == aContext)
			iContext = nullptr;
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const video_mode& aVideoMode, const std::string& aWindowTitle, window::style_e aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aEventHandler, aVideoMode, aWindowTitle, aStyle));
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const size& aDimensions, const std::string& aWindowTitle, window::style_e aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aEventHandler, aDimensions, aWindowTitle, aStyle));
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window::style_e aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aEventHandler, aPosition, aDimensions, aWindowTitle, aStyle));
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, i_native_surface& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window::style_e aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		sdl_window* parent = dynamic_cast<sdl_window*>(&aParent);
		if (parent != 0)
			return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aEventHandler, *parent, aVideoMode, aWindowTitle, aStyle));
		else
			return create_window(aSurfaceManager, aEventHandler, aVideoMode, aWindowTitle, aStyle);
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, i_native_surface& aParent, const size& aDimensions, const std::string& aWindowTitle, window::style_e aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		sdl_window* parent = dynamic_cast<sdl_window*>(&aParent);
		if (parent != 0)
			return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aEventHandler, *parent, aDimensions, aWindowTitle, aStyle));
		else
			return create_window(aSurfaceManager, aEventHandler, aDimensions, aWindowTitle, aStyle);
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, i_native_surface& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window::style_e aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		sdl_window* parent = dynamic_cast<sdl_window*>(&aParent);
		if (parent != 0)
			return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aEventHandler, *parent, aPosition, aDimensions, aWindowTitle, aStyle));
		else
			return create_window(aSurfaceManager, aEventHandler, aPosition, aDimensions, aWindowTitle, aStyle);
	}

	bool sdl_renderer::creating_window() const
	{
		return iCreatingWindow != 0;
	}

	void sdl_renderer::render_now()
	{
		app::instance().surface_manager().render_surfaces();
	}

	bool sdl_renderer::process_events()
	{
		if (queue_events())
			return opengl_renderer::process_events();
		else
			return false;
	}

	bool sdl_renderer::queue_events()
	{
		bool queuedEvents = false;
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			queuedEvents = true;
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
					if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
						static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEMOTION:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.motion.windowID);
					if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
						static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.button.windowID);
					if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
						static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.button.windowID);
					if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
						static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEWHEEL:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.wheel.windowID);
					if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
						static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_KEYDOWN:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.key.windowID);
					if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
						static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_KEYUP:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.key.windowID);
					if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
						static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_TEXTEDITING:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.edit.windowID);
					if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
						static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_TEXTINPUT:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.text.windowID);
					if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
						static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			default:
				break;
			}
		}
		return queuedEvents;
	}
}