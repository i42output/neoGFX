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
#include "app.hpp"
#include "surface_manager.hpp"
#include "sdl_window.hpp"
#include "sdl_renderer.hpp"

namespace neogfx
{
	sdl_renderer::sdl_renderer(i_keyboard& aKeyboard) : iKeyboard(aKeyboard)
	{
		SDL_Init(SDL_INIT_VIDEO);
	}

	sdl_renderer::~sdl_renderer()
	{
		SDL_Quit();
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle)
	{
		return std::unique_ptr<i_native_window>(new sdl_window(*this, aSurfaceManager, aEventHandler, aVideoMode, aWindowTitle, aStyle));
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, i_native_window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle)
	{
		return std::unique_ptr<i_native_window>(new sdl_window(*this, aSurfaceManager, aEventHandler, static_cast<sdl_window&>(aParent), aVideoMode, aWindowTitle, aStyle));
	}

	bool sdl_renderer::process_events()
	{
		bool handledEvents = false;
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			handledEvents = true;
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
					if (window != NULL)
						static_cast<sdl_window&>(app::instance().surface_manager().surface_from_handle(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEMOTION:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.motion.windowID);
					if (window != NULL)
						static_cast<sdl_window&>(app::instance().surface_manager().surface_from_handle(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.button.windowID);
					if (window != NULL)
						static_cast<sdl_window&>(app::instance().surface_manager().surface_from_handle(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.button.windowID);
					if (window != NULL)
						static_cast<sdl_window&>(app::instance().surface_manager().surface_from_handle(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEWHEEL:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.wheel.windowID);
					if (window != NULL)
						static_cast<sdl_window&>(app::instance().surface_manager().surface_from_handle(window).native_surface()).process_event(event);
				}
				break;
			case SDL_KEYDOWN:
				{
					iKeyboard.key_pressed.trigger(
						static_cast<scan_code_e>(event.key.keysym.scancode), 
						static_cast<key_code_e>(event.key.keysym.sym), 
						static_cast<key_modifiers_e>(event.key.keysym.mod));
					SDL_Window* window = SDL_GetWindowFromID(event.key.windowID);
					if (window != NULL)
						static_cast<sdl_window&>(app::instance().surface_manager().surface_from_handle(window).native_surface()).process_event(event);
				}
				break;
			case SDL_KEYUP:
				{
					iKeyboard.key_released.trigger(
						static_cast<scan_code_e>(event.key.keysym.scancode),
						static_cast<key_code_e>(event.key.keysym.sym),
						static_cast<key_modifiers_e>(event.key.keysym.mod));
					SDL_Window* window = SDL_GetWindowFromID(event.key.windowID);
					if (window != NULL)
						static_cast<sdl_window&>(app::instance().surface_manager().surface_from_handle(window).native_surface()).process_event(event);
				}
				break;
			case SDL_TEXTEDITING:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.edit.windowID);
					if (window != NULL)
						static_cast<sdl_window&>(app::instance().surface_manager().surface_from_handle(window).native_surface()).process_event(event);
				}
				break;
			case SDL_TEXTINPUT:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.text.windowID);
					if (window != NULL)
						static_cast<sdl_window&>(app::instance().surface_manager().surface_from_handle(window).native_surface()).process_event(event);
				}
				break;
			default:
				break;
			}
		}
		return handledEvents;
	}
}