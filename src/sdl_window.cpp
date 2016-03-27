// sdl_window.cpp
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
#include <SDL_syswm.h>
#include <SDL_mouse.h>
#include "sdl_window.hpp"


namespace neogfx
{
	Uint32 sdl_window::convert_style(uint32_t aStyle)
	{   
		uint32_t result = 0u;
		if (aStyle & window::None)
			result |= SDL_WINDOW_BORDERLESS;
		if (aStyle & window::Titlebar)
			result &= ~SDL_WINDOW_BORDERLESS;
		if (aStyle & window::Resize)
			result |= SDL_WINDOW_RESIZABLE;
		if (aStyle & window::Close)
			result &= ~SDL_WINDOW_BORDERLESS;
		if (aStyle & window::Fullscreen)
			result |= SDL_WINDOW_FULLSCREEN;
		return result;
	}

	mouse_button sdl_window::convert_mouse_button(Uint32 aButton)
	{
		mouse_button result = mouse_button::None;
		if (aButton & SDL_BUTTON(SDL_BUTTON_LEFT))
			result = static_cast<mouse_button>(result | mouse_button::Left);
		if (aButton & SDL_BUTTON(SDL_BUTTON_RIGHT))
			result = static_cast<mouse_button>(result | mouse_button::Right);
		if (aButton & SDL_BUTTON(SDL_BUTTON_MIDDLE))
			result = static_cast<mouse_button>(result | mouse_button::Middle);
		if (aButton & SDL_BUTTON(SDL_BUTTON_X1))
			result = static_cast<mouse_button>(result | mouse_button::X1);
		if (aButton & SDL_BUTTON(SDL_BUTTON_X2))
			result = static_cast<mouse_button>(result | mouse_button::X2);
		return result;
	}

	mouse_button sdl_window::convert_mouse_button(Uint8 aButtonIndex)
	{
		switch (aButtonIndex)
		{
		case SDL_BUTTON_LEFT:
			return mouse_button::Left;
		case SDL_BUTTON_RIGHT:
			return mouse_button::Right;
		case SDL_BUTTON_MIDDLE:
			return mouse_button::Middle;
		case SDL_BUTTON_X1:
			return mouse_button::X1;
		case SDL_BUTTON_X2:
			return mouse_button::X2;
		default:
			return mouse_button::Left;
		}
	}

	Uint32 sdl_window::convert_mouse_button(mouse_button aButton)
	{
		Uint32 result = 0u;
		if ((aButton & mouse_button::Left) != mouse_button::None)
			result = result | SDL_BUTTON(SDL_BUTTON_LEFT);
		if ((aButton & mouse_button::Right) != mouse_button::None)
			result = result | SDL_BUTTON(SDL_BUTTON_RIGHT);
		if ((aButton & mouse_button::Middle) != mouse_button::None)
			result = result | SDL_BUTTON(SDL_BUTTON_MIDDLE);
		if ((aButton & mouse_button::X1) != mouse_button::None)
			result = result | SDL_BUTTON(SDL_BUTTON_X1);
		if ((aButton & mouse_button::X2) != mouse_button::None)
			result = result | SDL_BUTTON(SDL_BUTTON_X2);
		return result;
	}

	sdl_window::sdl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aEventHandler),
		iHandle(NULL),
		iContext(NULL),
		iProcessingEvent(false),
		iCapturingMouse(false)
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		iHandle = SDL_CreateWindow(
			aWindowTitle.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			aVideoMode.width(),
			aVideoMode.height(),
			SDL_WINDOW_OPENGL | convert_style(aStyle));
		if (iHandle == NULL)
			throw failed_to_create_window(SDL_GetError());
		iContext = SDL_GL_CreateContext(iHandle);
		if (iContext == NULL)
		{
			SDL_DestroyWindow(iHandle);
			throw failed_to_create_opengl_context(SDL_GetError());
		}
#ifdef WIN32
		SetClassLongPtr(static_cast<HWND>(native_handle()), GCL_STYLE, CS_DBLCLKS);
#endif
	}

	sdl_window::sdl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, sdl_window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aEventHandler),
		iHandle(NULL),
		iContext(NULL),
		iProcessingEvent(false),
		iCapturingMouse(false)
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		iHandle = SDL_CreateWindow(
			aWindowTitle.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			aVideoMode.width(),
			aVideoMode.height(),
			SDL_WINDOW_OPENGL | convert_style(aStyle));
		if (iHandle == NULL)
			throw failed_to_create_window(SDL_GetError());
		iContext = SDL_GL_CreateContext(iHandle);
		if (iContext == NULL)
		{
			SDL_DestroyWindow(iHandle);
			throw failed_to_create_opengl_context(SDL_GetError());
		}
#ifdef WIN32
		SetClassLongPtr(static_cast<HWND>(native_handle()), GCL_STYLE, CS_DBLCLKS);
		SetParent(static_cast<HWND>(native_handle()), static_cast<HWND>(aParent.native_handle()));
#endif
	}

	sdl_window::~sdl_window()
	{
		release_capture();
		SDL_GL_DeleteContext(iContext);
		SDL_DestroyWindow(iHandle);
	}

	void* sdl_window::handle() const
	{
		return iHandle;
	}

	void* sdl_window::native_handle() const
	{
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if (SDL_GetWindowWMInfo(iHandle, &info))
		{
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
			return info.info.win.window;
#else
			return 0;
#endif
		}
		else
			throw failed_to_get_window_information(SDL_GetError());
	}

	point sdl_window::surface_position() const
	{
		int x, y;
		SDL_GetWindowPosition(iHandle, &x, &y);
		return point(static_cast<coordinate>(x), static_cast<coordinate>(y));
	}

	void sdl_window::move_surface(const point& aPosition)
	{
		SDL_SetWindowPosition(iHandle, static_cast<int>(aPosition.x), static_cast<int>(aPosition.y));
	}

	size sdl_window::surface_size() const
	{
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		return size(static_cast<dimension>(w), static_cast<dimension>(h));
	}

	void sdl_window::resize_surface(const size& aSize)
	{
		SDL_SetWindowSize(iHandle, static_cast<int>(aSize.cx), static_cast<int>(aSize.cy));
	}

	point sdl_window::mouse_position() const
	{
		int x, y;
		SDL_Window* mouseFocus = SDL_GetMouseFocus();
		SDL_GetMouseState(&x, &y);
		if (mouseFocus != NULL)
		{
			int mfx, mfy;
			SDL_GetWindowPosition(mouseFocus, &mfx, &mfy);
			x += mfx;
			y += mfy;
			SDL_GetWindowPosition(iHandle, &mfx, &mfy);
			x -= mfx;
			y -= mfy;
		}
		return point(static_cast<coordinate>(x), static_cast<coordinate>(y));
	}

	bool sdl_window::is_mouse_button_pressed(mouse_button aButton) const
	{
		return (aButton & convert_mouse_button(SDL_GetMouseState(NULL, NULL))) != mouse_button::None;
	}

	void sdl_window::save_mouse_cursor()
	{
		iSavedCursors.push_back(cursor_pointer(cursor_pointer(), SDL_GetCursor()));
	}

	void sdl_window::set_mouse_cursor(mouse_system_cursor aSystemCursor)
	{
		SDL_SystemCursor sdlCursor = SDL_SYSTEM_CURSOR_ARROW;
		switch (aSystemCursor)
		{
		case mouse_system_cursor::Arrow:
			sdlCursor = SDL_SYSTEM_CURSOR_ARROW;
			break;
		case mouse_system_cursor::Ibeam:
			sdlCursor = SDL_SYSTEM_CURSOR_IBEAM;
			break;
		case mouse_system_cursor::Wait:
			sdlCursor = SDL_SYSTEM_CURSOR_WAIT;
			break;
		case mouse_system_cursor::Crosshair:
			sdlCursor = SDL_SYSTEM_CURSOR_CROSSHAIR;
			break;
		case mouse_system_cursor::WaitArrow:
			sdlCursor = SDL_SYSTEM_CURSOR_WAITARROW;
			break;
		case mouse_system_cursor::SizeNWSE:
			sdlCursor = SDL_SYSTEM_CURSOR_SIZENWSE;
			break;
		case mouse_system_cursor::SizeNESW:
			sdlCursor = SDL_SYSTEM_CURSOR_SIZENESW;
			break;
		case mouse_system_cursor::SizeWE:
			sdlCursor = SDL_SYSTEM_CURSOR_SIZEWE;
			break;
		case mouse_system_cursor::SizeNS:
			sdlCursor = SDL_SYSTEM_CURSOR_SIZENS;
			break;
		case mouse_system_cursor::SizeAll:
			sdlCursor = SDL_SYSTEM_CURSOR_SIZEALL;
			break;
		case mouse_system_cursor::No:
			sdlCursor = SDL_SYSTEM_CURSOR_NO;
			break;
		case mouse_system_cursor::Hand:
			sdlCursor = SDL_SYSTEM_CURSOR_HAND;
			break;
		}
		iCurrentCursor = cursor_pointer(cursor_pointer(), SDL_CreateSystemCursor(sdlCursor));
		SDL_SetCursor(&*iCurrentCursor);
	}

	void sdl_window::restore_mouse_cursor()
	{
		if (iSavedCursors.empty())
			throw no_cursors_saved();
		iCurrentCursor = iSavedCursors.back();
		iSavedCursors.pop_back();
		SDL_SetCursor(&*iCurrentCursor);
		if (iSavedCursors.empty())
			event_handler().native_window_set_default_mouse_cursor();
	}

	std::unique_ptr<i_native_graphics_context> sdl_window::create_graphics_context() const
	{
		return std::unique_ptr<i_native_graphics_context>(new sdl_graphics_context(rendering_engine(), *this));
	}

	std::unique_ptr<i_native_graphics_context> sdl_window::create_graphics_context(const i_widget& aWidget) const
	{
		return std::unique_ptr<i_native_graphics_context>(new sdl_graphics_context(rendering_engine(), *this, aWidget));
	}

	void sdl_window::close()
	{
		SDL_DestroyWindow(iHandle);
		iHandle = NULL;
	}

	bool sdl_window::is_active() const
	{
		return SDL_GetKeyboardFocus() == iHandle;
	}

	void sdl_window::activate()
	{
		SDL_RaiseWindow(iHandle);
	}

	void sdl_window::enable(bool aEnable)
	{
#ifdef WIN32
		EnableWindow(static_cast<HWND>(native_handle()), aEnable);
#endif
	}

	void sdl_window::set_capture()
	{
		if (!iCapturingMouse)
		{
			iCapturingMouse = true;
#ifdef WIN32
			SetCapture(static_cast<HWND>(native_handle()));
#endif
			SDL_CaptureMouse(SDL_TRUE);
		}
	}

	void sdl_window::release_capture()
	{
		if (iCapturingMouse)
		{
			iCapturingMouse = false;
			SDL_CaptureMouse(SDL_FALSE);
#ifdef WIN32
			ReleaseCapture();
#endif
		}
	}

	void sdl_window::process_event(const SDL_Event& aEvent)
	{
		iProcessingEvent = true;
		switch (aEvent.type)
		{
		case SDL_WINDOWEVENT:
			switch (aEvent.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				event_handler().native_window_closing();
				close();
				event_handler().native_window_closed();
				break;
			case SDL_WINDOWEVENT_RESIZED:
				event_handler().native_window_resized();
				break;
			case SDL_WINDOWEVENT_ENTER:
				event_handler().native_window_mouse_entered();
				break;
			case SDL_WINDOWEVENT_LEAVE:
				event_handler().native_window_mouse_left();
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				event_handler().native_window_focus_gained();
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				event_handler().native_window_focus_lost();
				break;
			}
			break;
		case SDL_MOUSEWHEEL:
			event_handler().native_window_mouse_wheel_scrolled(
				static_cast<mouse_wheel>(
					(aEvent.wheel.y != 0 ? mouse_wheel::Vertical : mouse_wheel::None) | (aEvent.wheel.x != 0 ? mouse_wheel::Horizontal : mouse_wheel::None)),
				delta(static_cast<coordinate>(aEvent.wheel.x), static_cast<coordinate>(aEvent.wheel.y)));
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (aEvent.button.clicks == 1)
				event_handler().native_window_mouse_button_pressed(convert_mouse_button(aEvent.button.button), point(static_cast<coordinate>(aEvent.button.x), static_cast<coordinate>(aEvent.button.y)));
			else
				event_handler().native_window_mouse_button_double_clicked(convert_mouse_button(aEvent.button.button), point(static_cast<coordinate>(aEvent.button.x), static_cast<coordinate>(aEvent.button.y)));
			break;
		case SDL_MOUSEBUTTONUP:
			event_handler().native_window_mouse_button_released(convert_mouse_button(aEvent.button.button), point(static_cast<coordinate>(aEvent.button.x), static_cast<coordinate>(aEvent.button.y)));
			break;
		case SDL_MOUSEMOTION:
			event_handler().native_window_mouse_moved(point(static_cast<coordinate>(aEvent.motion.x), static_cast<coordinate>(aEvent.motion.y)));
			break;
		case SDL_KEYDOWN:
			event_handler().native_window_key_pressed(static_cast<scan_code_e>(aEvent.key.keysym.scancode), static_cast<key_code_e>(aEvent.key.keysym.sym), static_cast<key_modifiers_e>(aEvent.key.keysym.mod));
			break;
		case SDL_KEYUP:
			event_handler().native_window_key_released(static_cast<scan_code_e>(aEvent.key.keysym.scancode), static_cast<key_code_e>(aEvent.key.keysym.sym), static_cast<key_modifiers_e>(aEvent.key.keysym.mod));
			break;
		case SDL_TEXTEDITING:
			/* todo */
			break;
		case SDL_TEXTINPUT:
			event_handler().native_window_text_input(aEvent.text.text);
			break;
		default:
			break;
		}
		iProcessingEvent = false;
	}

	void sdl_window::activate_context()
	{
		if (SDL_GL_MakeCurrent(iHandle, iContext) != 0)
			throw failed_to_activate_opengl_context(SDL_GetError());
	}

	void sdl_window::deactivate_context()	
	{
		/* nothing to do */
	}
	
	void sdl_window::display()
	{
		SDL_GL_SwapWindow(iHandle);
	}

	bool sdl_window::processing_event() const
	{
		return iProcessingEvent;
	}
}