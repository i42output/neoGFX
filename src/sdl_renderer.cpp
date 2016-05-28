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

#include "neogfx.hpp"
#include <SDL.h>
#include <neolib/raii.hpp>
#include "app.hpp"
#include "surface_manager.hpp"
#include "sdl_window.hpp"
#include "sdl_renderer.hpp"

namespace neogfx
{
	sdl_renderer::sdl_renderer(i_basic_services& aBasicServices, i_keyboard& aKeyboard) : iBasicServices(aBasicServices), iKeyboard(aKeyboard), iCreatingWindow(0)
	{
		SDL_Init(SDL_INIT_VIDEO);
	}

	sdl_renderer::~sdl_renderer()
	{
		SDL_Quit();
	}

	void* sdl_renderer::create_context(i_native_surface& aSurface)
	{
		if (iContexts.find(&aSurface) != iContexts.end())
			throw context_exists();
		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
		return (iContexts[&aSurface] = SDL_GL_CreateContext(static_cast<SDL_Window*>(aSurface.handle())));
	}

	void sdl_renderer::destroy_context(i_native_surface& aSurface)
	{
		auto c = iContexts.find(&aSurface);
		if (c == iContexts.end())
			throw context_not_found();
		SDL_GL_DeleteContext(c->second);
		iContexts.erase(c);
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aEventHandler, aVideoMode, aWindowTitle, aStyle));
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, uint32_t aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aEventHandler, aPosition, aDimensions, aWindowTitle, aStyle));
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, i_native_surface& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		sdl_window* parent = dynamic_cast<sdl_window*>(&aParent);
		if (parent != 0)
			return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aEventHandler, *parent, aVideoMode, aWindowTitle, aStyle));
		else
			return create_window(aSurfaceManager, aEventHandler, aVideoMode, aWindowTitle, aStyle);
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, i_native_surface& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, uint32_t aStyle)
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
		bool handledEvents = false;
		SDL_Event event;
		auto lastRenderTime = neolib::thread::program_elapsed_ms();
		while (SDL_PollEvent(&event))
		{
			handledEvents = true;
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
					if (!iKeyboard.grabber().key_pressed(
						static_cast<scan_code_e>(event.key.keysym.scancode),
						static_cast<key_code_e>(event.key.keysym.sym),
						static_cast<key_modifiers_e>(event.key.keysym.mod)))
					{
						iKeyboard.key_pressed.trigger(
							static_cast<scan_code_e>(event.key.keysym.scancode),
							static_cast<key_code_e>(event.key.keysym.sym),
							static_cast<key_modifiers_e>(event.key.keysym.mod));
						SDL_Window* window = SDL_GetWindowFromID(event.key.windowID);
						if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
							static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
					}
				}
				break;
			case SDL_KEYUP:
				{
					if (!iKeyboard.grabber().key_released(
						static_cast<scan_code_e>(event.key.keysym.scancode),
						static_cast<key_code_e>(event.key.keysym.sym),
						static_cast<key_modifiers_e>(event.key.keysym.mod)))
					{
						iKeyboard.key_released.trigger(
							static_cast<scan_code_e>(event.key.keysym.scancode),
							static_cast<key_code_e>(event.key.keysym.sym),
							static_cast<key_modifiers_e>(event.key.keysym.mod));
						SDL_Window* window = SDL_GetWindowFromID(event.key.windowID);
						if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
							static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
					}
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
					if (!iKeyboard.grabber().text_input(event.text.text))
					{
						SDL_Window* window = SDL_GetWindowFromID(event.text.windowID);
						if (window != NULL && app::instance().surface_manager().is_surface_attached(window))
							static_cast<sdl_window&>(app::instance().surface_manager().attached_surface(window).native_surface()).process_event(event);
					}
				}
				break;
			default:
				break;
			}
			if (neolib::thread::program_elapsed_ms() - lastRenderTime > 10)
			{
				lastRenderTime = neolib::thread::program_elapsed_ms();
				render_now();
			}
		}
		return handledEvents;
	}
}