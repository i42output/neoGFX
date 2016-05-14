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
#include <thread>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_mouse.h>
#include "opengl_error.hpp"
#include "sdl_window.hpp"
#include "app.hpp"

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

	std::map<void*, sdl_window*> sHandleMap;

	sdl_window::sdl_window(i_basic_services&, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aEventHandler),
		iParent(0),
		iStyle(aStyle),
		iHandle(0),
		iNativeHandle(0),
		iContext(0),
		iProcessingEvent(false),
		iCapturingMouse(false),
		iDestroyed(false)
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		iHandle = SDL_CreateWindow(
			aWindowTitle.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			aVideoMode.width(),
			aVideoMode.height(),
			SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | convert_style(aStyle));
		if (iHandle == 0)
			throw failed_to_create_window(SDL_GetError());
		init();
		iContext = reinterpret_cast<SDL_GLContext>(aRenderingEngine.create_context(*this));
		if (iContext == 0)
		{
			SDL_DestroyWindow(iHandle);
			throw failed_to_create_opengl_context(SDL_GetError());
		}
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		iExtents = basic_size<int>{w, h};

		show((aStyle & window::NoActivate) != window::NoActivate);
	}

	sdl_window::sdl_window(i_basic_services&, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, uint32_t aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aEventHandler),
		iParent(0),
		iStyle(aStyle),
		iHandle(0),
		iNativeHandle(0),
		iContext(0),
		iProcessingEvent(false),
		iCapturingMouse(false),
		iDestroyed(false)
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		iHandle = SDL_CreateWindow(
			aWindowTitle.c_str(),
			aPosition.x,
			aPosition.y,
			aDimensions.cx,
			aDimensions.cy,
			SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | convert_style(aStyle));
		if (iHandle == 0)
			throw failed_to_create_window(SDL_GetError());
		init();
		iContext = reinterpret_cast<SDL_GLContext>(aRenderingEngine.create_context(*this));
		if (iContext == 0)
		{
			SDL_DestroyWindow(iHandle);
			throw failed_to_create_opengl_context(SDL_GetError());
		}
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		iExtents = basic_size<int>{ w, h };

		show((aStyle & window::NoActivate) != window::NoActivate);
	}

	sdl_window::sdl_window(i_basic_services&, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, sdl_window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aEventHandler),
		iParent(&aParent),
		iStyle(aStyle),
		iHandle(0),
		iNativeHandle(0),
		iContext(0),
		iProcessingEvent(false),
		iCapturingMouse(false),
		iDestroyed(false)
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		iHandle = SDL_CreateWindow(
			aWindowTitle.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			aVideoMode.width(),
			aVideoMode.height(),
			SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | convert_style(aStyle));
		if (iHandle == 0)
			throw failed_to_create_window(SDL_GetError());
		init();
		iContext = reinterpret_cast<SDL_GLContext>(aRenderingEngine.create_context(*this));
		if (iContext == 0)
		{
			SDL_DestroyWindow(iHandle);
			throw failed_to_create_opengl_context(SDL_GetError());
		}
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		iExtents = basic_size<int>{ w, h };

		show((aStyle & window::NoActivate) != window::NoActivate);
	}

	sdl_window::sdl_window(i_basic_services&, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_native_window_event_handler& aEventHandler, sdl_window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, uint32_t aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aEventHandler),
		iParent(&aParent),
		iStyle(aStyle),
		iHandle(0),
		iNativeHandle(0),
		iContext(0),
		iProcessingEvent(false),
		iCapturingMouse(false),
		iDestroyed(false)
	{
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		iHandle = SDL_CreateWindow(
			aWindowTitle.c_str(),
			aPosition.x,
			aPosition.y,
			aDimensions.cx,
			aDimensions.cy,
			SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | convert_style(aStyle));
		if (iHandle == 0)
			throw failed_to_create_window(SDL_GetError());
		init();
		iContext = reinterpret_cast<SDL_GLContext>(aRenderingEngine.create_context(*this));
		if (iContext == 0)
		{
			SDL_DestroyWindow(iHandle);
			throw failed_to_create_opengl_context(SDL_GetError());
		}
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		iExtents = basic_size<int>{ w, h };

		show((aStyle & window::NoActivate) != window::NoActivate);
	}

	sdl_window::~sdl_window()
	{
		close();
		rendering_engine().destroy_context(*this);
	}

	void* sdl_window::handle() const
	{
		return iHandle;
	}

	void* sdl_window::native_handle() const
	{
		if (iNativeHandle == 0 && !iDestroyed)
		{
			SDL_SysWMinfo info;
			SDL_VERSION(&info.version);
			if (SDL_GetWindowWMInfo(iHandle, &info))
			{
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
				iNativeHandle = info.info.win.window;
#endif
			}
			else
				throw failed_to_get_window_information(SDL_GetError());
		}
		return iNativeHandle;
	}

	void* sdl_window::native_context() const
	{
		return iContext;
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
		return iExtents;
	}

	void sdl_window::resize_surface(const size& aSize)
	{
		if (iExtents != aSize)
		{
			SDL_SetWindowSize(iHandle, static_cast<int>(aSize.cx), static_cast<int>(aSize.cy));
			iExtents = aSize;
		}
	}

	point sdl_window::mouse_position() const
	{
		int x, y;
		SDL_Window* mouseFocus = SDL_GetMouseFocus();
		SDL_GetMouseState(&x, &y);
		if (mouseFocus != 0)
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
		return (aButton & convert_mouse_button(SDL_GetMouseState(0, 0))) != mouse_button::None;
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
		if (iHandle != 0)
		{
			release_capture();
			event_handler().native_window_closing();
			if (!iDestroyed)
			{
#ifdef WIN32
				DestroyWindow(static_cast<HWND>(native_handle()));
#endif
				SDL_DestroyWindow(iHandle);
			}
			iHandle = 0;
			event_handler().native_window_closed();
		}
	}

	void sdl_window::show(bool aActivate)
	{
#ifdef WIN32
		ShowWindow(static_cast<HWND>(native_handle()), aActivate ? SW_SHOW : SW_SHOWNA);
#else
		SDL_ShowWindow(iHandle);
#endif
	}

	void sdl_window::hide()
	{
#ifdef WIN32
		ShowWindow(static_cast<HWND>(native_handle()), SW_HIDE);
#else
		SDL_HideWindow(iHandle);
#endif
	}

	bool sdl_window::is_active() const
	{
		return SDL_GetKeyboardFocus() == iHandle;
	}

	void sdl_window::activate()
	{
		SDL_RaiseWindow(iHandle);
#ifdef WIN32
		SetWindowPos(static_cast<HWND>(native_handle()), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif
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
			SDL_CaptureMouse(SDL_TRUE);
#ifdef WIN32
			SetCapture(static_cast<HWND>(native_handle()));
#endif
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

	bool sdl_window::is_destroyed() const
	{
		return iDestroyed;
	}

	void sdl_window::activate_context() const
	{
		if (context_activation_stack().empty() || context_activation_stack().back() != this)
			do_activate_context();
		context_activation_stack().push_back(this);
	}

	void sdl_window::deactivate_context() const
	{
		if (context_activation_stack().empty() || context_activation_stack().back() != this)
			throw context_mismatch();
		context_activation_stack().pop_back();
		if (context_activation_stack().empty())
		{
			if (!iDestroyed)
				do_activate_context();
			else
				do_activate_default_context();
		}
		else if (context_activation_stack().back() != this)
			context_activation_stack().back()->do_activate_context();
	}

#ifdef WIN32
	LRESULT CALLBACK sdl_window::CustomWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		auto mapEntry = sHandleMap.find(sHandleMap[hwnd]->iNativeHandle);
		auto wndproc = mapEntry->second->iSDLWindowProc;
		LRESULT result;
		switch(msg)
		{
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
			mapEntry->second->event_handler().native_window_dismiss_children(); // call this before default wndproc (which enters its own NC drag message loop)
			result = CallWindowProc(wndproc, hwnd, msg, wparam, lparam);
			break;
		case WM_DESTROY:
			{
				mapEntry->second->destroying();
				result = CallWindowProc(wndproc, hwnd, msg, wparam, lparam);
			}
			break;
		case WM_NCDESTROY:
			{
				mapEntry->second->destroyed();
				sHandleMap.erase(mapEntry);
				result = CallWindowProc(wndproc, hwnd, msg, wparam, lparam);
			}
			break;
		case WM_MOUSEACTIVATE:
			if (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE)
				result = MA_NOACTIVATE;
			else
				result = CallWindowProc(wndproc, hwnd, msg, wparam, lparam);
			break;
		default:
			result = CallWindowProc(wndproc, hwnd, msg, wparam, lparam);
			break;
		}
		return result;
	}
#endif

	void sdl_window::init()
	{
		sHandleMap[native_handle()] = this;
#ifdef WIN32
		iSDLWindowProc = (WNDPROC)SetWindowLongPtr(static_cast<HWND>(native_handle()), GWLP_WNDPROC, (LONG_PTR)&CustomWindowProc);
		if (iStyle & window::None)
			SetWindowLongPtr(static_cast<HWND>(native_handle()), GWL_STYLE, GetWindowLongPtr(static_cast<HWND>(native_handle()), GWL_STYLE) | WS_POPUP);
		if (iStyle & window::NoActivate)
			SetWindowLongPtr(static_cast<HWND>(native_handle()), GWL_EXSTYLE, GetWindowLongPtr(static_cast<HWND>(native_handle()), GWL_EXSTYLE) | WS_EX_NOACTIVATE | WS_EX_TOPMOST);
		if (iParent != 0)
			SetWindowLongPtr(static_cast<HWND>(native_handle()), GWL_HWNDPARENT, reinterpret_cast<LONG>(iParent->native_handle()));
		SetWindowPos(static_cast<HWND>(native_handle()), 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif
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
				close();
				break;
			case SDL_WINDOWEVENT_RESIZED:
				iExtents = basic_size<decltype(aEvent.window.data1)>{aEvent.window.data1, aEvent.window.data2};
				event_handler().native_window_resized();
				break;
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				iExtents = basic_size<decltype(aEvent.window.data1)>{aEvent.window.data1, aEvent.window.data2};
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

	void sdl_window::destroying()
	{
		opengl_window::destroying();
	}
		
	void sdl_window::destroyed()
	{
		if (!iDestroyed)
		{
			iDestroyed = true;
			opengl_window::destroyed();
			iNativeHandle = 0;
		}
		bool activateDefaultContext = true;
		while (!context_activation_stack().empty() && context_activation_stack().back() == this)
		{
			activateDefaultContext = false;
			deactivate_context();
		}
		if (activateDefaultContext)
			do_activate_default_context();
	}

	void sdl_window::do_activate_context() const
	{
		if (SDL_GL_MakeCurrent(iHandle, iContext) != 0)
			throw failed_to_activate_opengl_context(SDL_GetError());
		glCheck("");
	}

	void sdl_window::do_activate_default_context() const
	{
		for (std::size_t i = 0; i != surface_manager().surface_count(); ++i)
			if (!surface_manager().surface(i).destroyed())
			{
				surface_manager().surface(i).native_surface().activate_context();
				context_activation_stack().pop_back();
				break;
			}
	}

	std::deque<const sdl_window*>& sdl_window::context_activation_stack()
	{
		static std::deque<const sdl_window*> sStack;
		return sStack;
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