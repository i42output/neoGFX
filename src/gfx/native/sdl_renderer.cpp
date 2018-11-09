// sdl_renderer.cpp
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

#include <neogfx/neogfx.hpp>
#include <SDL.h>
#include <neolib/raii.hpp>
#include <neogfx/hid/surface_manager.hpp>
#include "../../gui/window/native/sdl_window.hpp"
#include "sdl_renderer.hpp"

namespace neogfx
{
	class sdl_instance
	{
	public:
		sdl_instance()
		{
			SDL_Init(SDL_INIT_VIDEO);
			SDL_SetHint(SDL_HINT_WINDOWS_ENABLE_MESSAGELOOP, "0");
		}
		~sdl_instance()
		{
			SDL_Quit();
		}
	public:
		static void instantiate()
		{
			static sdl_instance sSdlInstance;
		}
	};

	sdl_renderer::sdl_renderer(neogfx::renderer aRenderer, bool aDoubleBufferedWindows, i_basic_services& aBasicServices, i_keyboard& aKeyboard) :
		opengl_renderer{ aRenderer },
		iDoubleBuffering{ aDoubleBufferedWindows },
		iBasicServices{ aBasicServices }, 
		iKeyboard{ aKeyboard }, 
		iCreatingWindow{ 0 },
		iContext{ nullptr }
	{
		SDL_AddEventWatch(&filter_event, this);

		sdl_instance::instantiate();
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, aDoubleBufferedWindows ? 1 : 0);
		switch (aRenderer)
		{
		case renderer::Vulkan:
		case renderer::Software:
			throw unsupported_renderer();
		case renderer::DirectX: // ANGLE
			#ifdef _WIN32
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			#else
			throw unsupported_renderer();
			#endif
			break;
		case renderer::OpenGL:
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);		
			break;
		default:
			break;
		}
		iSystemCacheWindowHandle = SDL_CreateWindow(
			"neogfx::system_cache_window",
			0,
			0,
			0,
			0,
			SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
		if (iSystemCacheWindowHandle == 0)
			throw failed_to_create_system_cache_window(SDL_GetError());
		iContext = create_context(iSystemCacheWindowHandle);
		if (iContext != nullptr)
		{
			SDL_GL_MakeCurrent(static_cast<SDL_Window*>(iSystemCacheWindowHandle), iContext);
			glCheck(glewInit());
		}
		else
			throw failed_to_create_opengl_context(SDL_GetError());
	}

	sdl_renderer::~sdl_renderer()
	{
		if (iContext != nullptr)
			SDL_GL_MakeCurrent(static_cast<SDL_Window*>(iSystemCacheWindowHandle), iContext);
	}

	bool sdl_renderer::double_buffering() const
	{
		return iDoubleBuffering;
	}

	const i_render_target* sdl_renderer::active_target() const
	{
		if (iTargetStack.empty())
			return nullptr;
		return iTargetStack.back();
	}

	void sdl_renderer::activate_context(const i_render_target& aTarget)
	{
		if (iContext == nullptr)
			iContext = create_context(aTarget);

		iTargetStack.push_back(&aTarget);
		
		static bool initialized = false;
		if (!initialized)
		{
			initialize();
			initialized = true;
		}

		activate_current_target();
	}

	void sdl_renderer::deactivate_context()
	{
		if (iTargetStack.empty())
			throw no_target_active();
		iTargetStack.pop_back();

		auto activeTarget = active_target();
		if (activeTarget != nullptr)
		{
			iTargetStack.pop_back();
			activeTarget->activate_target();
		}
	}

	i_rendering_engine::opengl_context sdl_renderer::create_context(const i_render_target& aTarget)
	{
		return create_context(aTarget.target_type() == render_target_type::Surface ? aTarget.target_handle() : iSystemCacheWindowHandle);
	}

	void sdl_renderer::destroy_context(opengl_context aContext)
	{
		SDL_GL_DeleteContext(static_cast<SDL_GLContext>(aContext));
		if (iContext == aContext)
			iContext = nullptr;
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aWindow, aVideoMode, aWindowTitle, aStyle));
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aWindow, aDimensions, aWindowTitle, aStyle));
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aWindow, aPosition, aDimensions, aWindowTitle, aStyle));
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_surface& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		sdl_window* parent = dynamic_cast<sdl_window*>(&aParent);
		if (parent != nullptr)
			return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aWindow, *parent, aVideoMode, aWindowTitle, aStyle));
		else
			return create_window(aSurfaceManager, aWindow, aVideoMode, aWindowTitle, aStyle);
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_surface& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		sdl_window* parent = dynamic_cast<sdl_window*>(&aParent);
		if (parent != nullptr)
			return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aWindow, *parent, aDimensions, aWindowTitle, aStyle));
		else
			return create_window(aSurfaceManager, aWindow, aDimensions, aWindowTitle, aStyle);
	}

	std::unique_ptr<i_native_window> sdl_renderer::create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_surface& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle)
	{
		neolib::scoped_counter sc(iCreatingWindow);
		sdl_window* parent = dynamic_cast<sdl_window*>(&aParent);
		if (parent != nullptr)
			return std::unique_ptr<i_native_window>(new sdl_window(iBasicServices, *this, aSurfaceManager, aWindow, *parent, aPosition, aDimensions, aWindowTitle, aStyle));
		else
			return create_window(aSurfaceManager, aWindow, aPosition, aDimensions, aWindowTitle, aStyle);
	}

	bool sdl_renderer::creating_window() const
	{
		return iCreatingWindow != 0;
	}

	void sdl_renderer::render_now()
	{
		service<i_surface_manager>::instance().render_surfaces();
	}

	bool sdl_renderer::use_rendering_priority() const
	{
		// todo
		return false;
	}

	bool sdl_renderer::process_events()
	{
		bool eventsAlreadyQueued = false;
		for (std::size_t s = 0; !eventsAlreadyQueued && s < service<i_surface_manager>::instance().surface_count(); ++s)
		{
			auto& surface = service<i_surface_manager>::instance().surface(s);
			if (!surface.has_native_surface())
				continue;
			if (surface.surface_type() == surface_type::Window && static_cast<i_native_window&>(surface.native_surface()).events_queued())
				eventsAlreadyQueued = true;
		}
		if (queue_events() || eventsAlreadyQueued)
			return opengl_renderer::process_events();
		else
			return false;
	}

	sdl_renderer::opengl_context sdl_renderer::create_context(void* aNativeSurfaceHandle)
	{
		return SDL_GL_CreateContext(static_cast<SDL_Window*>(aNativeSurfaceHandle));
	}

	void sdl_renderer::activate_current_target()
	{
		if (SDL_GL_MakeCurrent(static_cast<SDL_Window*>(active_target() != nullptr && active_target()->target_type() == render_target_type::Surface ? active_target()->target_handle() : iSystemCacheWindowHandle), static_cast<SDL_GLContext>(iContext)) == -1)
			throw failed_to_activate_opengl_context(SDL_GetError());
	}

	int sdl_renderer::filter_event(void* aSelf, SDL_Event* aEvent)
	{
		switch (aEvent->type)
		{
		case SDL_WINDOWEVENT:
			{
				SDL_Window* sdlWindow = SDL_GetWindowFromID(aEvent->window.windowID);
				if (sdlWindow != NULL && service<i_surface_manager>::instance().is_surface_attached(sdlWindow))
				{
					auto& window = static_cast<sdl_window&>(service<i_surface_manager>::instance().attached_surface(sdlWindow).native_surface());
					switch (aEvent->window.event)
					{
					case SDL_WINDOWEVENT_ENTER:
						aEvent->window.data1 = static_cast<Sint32>(window.surface_window().as_window().mouse_position().x);
						aEvent->window.data2 = static_cast<Sint32>(window.surface_window().as_window().mouse_position().y);
						break;
					}
				}
			}
			break;
		default:
			break;
		}
		return 0;
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
					if (window != NULL && service<i_surface_manager>::instance().is_surface_attached(window))
						static_cast<sdl_window&>(service<i_surface_manager>::instance().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEMOTION:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.motion.windowID);
					if (window != NULL && service<i_surface_manager>::instance().is_surface_attached(window))
						static_cast<sdl_window&>(service<i_surface_manager>::instance().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.button.windowID);
					if (window != NULL && service<i_surface_manager>::instance().is_surface_attached(window))
						static_cast<sdl_window&>(service<i_surface_manager>::instance().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.button.windowID);
					if (window != NULL && service<i_surface_manager>::instance().is_surface_attached(window))
						static_cast<sdl_window&>(service<i_surface_manager>::instance().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_MOUSEWHEEL:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.wheel.windowID);
					if (window != NULL && service<i_surface_manager>::instance().is_surface_attached(window))
						static_cast<sdl_window&>(service<i_surface_manager>::instance().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_KEYDOWN:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.key.windowID);
					if (window != NULL && service<i_surface_manager>::instance().is_surface_attached(window))
						static_cast<sdl_window&>(service<i_surface_manager>::instance().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_KEYUP:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.key.windowID);
					if (window != NULL && service<i_surface_manager>::instance().is_surface_attached(window))
						static_cast<sdl_window&>(service<i_surface_manager>::instance().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_TEXTEDITING:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.edit.windowID);
					if (window != NULL && service<i_surface_manager>::instance().is_surface_attached(window))
						static_cast<sdl_window&>(service<i_surface_manager>::instance().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			case SDL_TEXTINPUT:
				{
					SDL_Window* window = SDL_GetWindowFromID(event.text.windowID);
					if (window != NULL && service<i_surface_manager>::instance().is_surface_attached(window))
						static_cast<sdl_window&>(service<i_surface_manager>::instance().attached_surface(window).native_surface()).process_event(event);
				}
				break;
			default:
				break;
			}
		}
		return queuedEvents;
	}
}