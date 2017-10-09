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

#include <neogfx/neogfx.hpp>
#include <thread>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_mouse.h>
#include <neogfx/app/app.hpp>
#include "../../../gfx/native/opengl.hpp"
#include "../../../gfx/native/sdl_graphics_context.hpp"
#include "sdl_window.hpp"

#ifdef WIN32
#include <Windowsx.h>
#include <Dwmapi.h>
#include <Uxtheme.h>
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Uxtheme.lib")

std::string GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

extern "C"
{
	BOOL WIN_ConvertUTF32toUTF8(UINT32 codepoint, char * text);
	void SDL_ResetMouse(void);
	extern LPTSTR SDL_Appname;
}
#endif

namespace neogfx
{
	sdl_window* sdl_window::sNewWindow;
#ifdef WIN32
	WNDPROC sdl_window::sSDLWindowProc;
#endif

	Uint32 sdl_window::convert_style(window_style aStyle)
	{   
		uint32_t result = 0u;
		if ((aStyle & window_style::NativeTitleBar) != window_style::NativeTitleBar)
			result |= SDL_WINDOW_BORDERLESS;
		if ((aStyle & window_style::Resize) == window_style::Resize)
			result |= SDL_WINDOW_RESIZABLE;
		if ((aStyle & window_style::Fullscreen) == window_style::Fullscreen)
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

#ifdef WIN32
	LRESULT convert_widget_part(widget_part aWidgetPart)
	{
		switch (aWidgetPart)
		{
		case widget_part::NonClientTitleBar:
		case widget_part::NonClientGrab:
			return HTCAPTION;
		case widget_part::NonClientBorder:
			return HTBORDER;
		case widget_part::NonClientBorderLeft:
			return HTLEFT;
		case widget_part::NonClientBorderTopLeft:
			return HTTOPLEFT;
		case widget_part::NonClientBorderTop:
			return HTTOP;
		case widget_part::NonClientBorderTopRight:
			return HTTOPRIGHT;
		case widget_part::NonClientBorderRight:
			return HTRIGHT;
		case widget_part::NonClientBorderBottomRight:
			return HTBOTTOMRIGHT;
		case widget_part::NonClientBorderBottom:
			return HTBOTTOM;
		case widget_part::NonClientBorderBottomLeft:
			return HTBOTTOMLEFT;
		case widget_part::NonClientGrowBox:
			return HTGROWBOX;
		case widget_part::NonClientCloseButton:
			return HTCLOSE;
		case widget_part::NonClientMaximizeButton:
			return HTMAXBUTTON;
		case widget_part::NonClientMinimizeButton:
			return HTMINBUTTON;
		case widget_part::NonClientRestoreButton:
			return HTMAXBUTTON;
		case widget_part::NonClientMenu:
			return HTMENU;
		case widget_part::NonClientSystemMenu:
			return HTSYSMENU;
		case widget_part::Nowhere:
			return HTNOWHERE;
		case widget_part::NowhereError:
			return HTERROR;
		case widget_part::NonClientVerticalScrollbar:
		case widget_part::NonClientHorizontalScrollbar:
		case widget_part::Client:
		case widget_part::NonClient:
		case widget_part::NonClientOther:
		default:
			return HTCLIENT;
		}
	}
#endif

	std::map<void*, sdl_window*> sHandleMap;

	sdl_window::sdl_window(i_basic_services&, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
		iParent(0),
		iStyle(aStyle),
		iHandle(0),
		iNativeHandle(0),
		iVisible(false),
		iCapturingMouse(false),
		iReady(false),
		iDestroyed(false),
		iClickedWidgetPart(widget_part::Nowhere),
		iSystemMenuOpen(false)
	{
		install_creation_hook(*this);
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
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		iExtents = basic_size<int>{w, h};

		if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
			show((aStyle & window_style::NoActivate) != window_style::NoActivate);

		aRenderingEngine.activate_context(*this);

		iReady = true;
	}

	sdl_window::sdl_window(i_basic_services&, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
		iParent(0),
		iStyle(aStyle),
		iHandle(0),
		iNativeHandle(0),
		iVisible(false),
		iCapturingMouse(false),
		iReady(false),
		iDestroyed(false),
		iClickedWidgetPart(widget_part::Nowhere),
		iSystemMenuOpen(false)
	{
		install_creation_hook(*this);
		iHandle = SDL_CreateWindow(
			aWindowTitle.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			aDimensions.cx,
			aDimensions.cy,
			SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | convert_style(aStyle));
		if (iHandle == 0)
			throw failed_to_create_window(SDL_GetError());
		init();
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		iExtents = basic_size<int>{ w, h };

		if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
			show((aStyle & window_style::NoActivate) != window_style::NoActivate);

		aRenderingEngine.activate_context(*this);

		iReady = true;
	}

	sdl_window::sdl_window(i_basic_services&, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
		iParent(0),
		iStyle(aStyle),
		iHandle(0),
		iNativeHandle(0),
		iVisible(false),
		iCapturingMouse(false),
		iReady(false),
		iDestroyed(false),
		iClickedWidgetPart(widget_part::Nowhere),
		iSystemMenuOpen(false)
	{
		install_creation_hook(*this);
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
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		iExtents = basic_size<int>{ w, h };

		if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
			show((aStyle & window_style::NoActivate) != window_style::NoActivate);

		aRenderingEngine.activate_context(*this);

		iReady = true;
	}

	sdl_window::sdl_window(i_basic_services&, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, sdl_window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
		iParent(&aParent),
		iStyle(aStyle),
		iHandle(0),
		iNativeHandle(0),
		iVisible(false),
		iCapturingMouse(false),
		iReady(false),
		iDestroyed(false),
		iClickedWidgetPart(widget_part::Nowhere),
		iSystemMenuOpen(false)
	{
		install_creation_hook(*this);
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
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		iExtents = basic_size<int>{ w, h };

		if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
			show((aStyle & window_style::NoActivate) != window_style::NoActivate);

		aRenderingEngine.activate_context(*this);

		iReady = true;
	}

	sdl_window::sdl_window(i_basic_services&, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, sdl_window& aParent, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
		iParent(&aParent),
		iStyle(aStyle),
		iHandle(0),
		iNativeHandle(0),
		iVisible(false),
		iCapturingMouse(false),
		iReady(false),
		iDestroyed(false),
		iClickedWidgetPart(widget_part::Nowhere),
		iSystemMenuOpen(false)
	{
		install_creation_hook(*this);
		iHandle = SDL_CreateWindow(
			aWindowTitle.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			aDimensions.cx,
			aDimensions.cy,
			SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | convert_style(aStyle));
		if (iHandle == 0)
			throw failed_to_create_window(SDL_GetError());
		init();
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		iExtents = basic_size<int>{ w, h };

		if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
			show((aStyle & window_style::NoActivate) != window_style::NoActivate);

		aRenderingEngine.activate_context(*this);

		iReady = true;
	}

	sdl_window::sdl_window(i_basic_services&, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_window& aWindow, sdl_window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
		opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
		iParent(&aParent),
		iStyle(aStyle),
		iHandle(0),
		iNativeHandle(0),
		iVisible(false),
		iCapturingMouse(false),
		iReady(false),
		iDestroyed(false),
		iClickedWidgetPart(widget_part::Nowhere),
		iSystemMenuOpen(false)
	{
		install_creation_hook(*this);
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
		int w, h;
		SDL_GetWindowSize(iHandle, &w, &h);
		iExtents = basic_size<int>{ w, h };

		if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
			show((aStyle & window_style::NoActivate) != window_style::NoActivate);

		aRenderingEngine.activate_context(*this);

		iReady = true;
	}

	sdl_window::~sdl_window()
	{
		close();
	}

	bool sdl_window::initialising() const
	{
		return !iReady;
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
			iExtents = aSize;
			SDL_SetWindowSize(iHandle, static_cast<int>(aSize.cx), static_cast<int>(aSize.cy));
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
		update_mouse_cursor();
	}

	void sdl_window::update_mouse_cursor()
	{
		if (iSavedCursors.empty())
			set_mouse_cursor(window().native_window_mouse_cursor().system_cursor());
	}

	bool sdl_window::can_render() const
	{
		return is_visible() && opengl_window::can_render();
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
			if (window().native_window_can_close())
			{
				release_capture();
				window().native_window_closing();
				if (!iDestroyed)
				{
					destroying();
#ifdef WIN32
					DestroyWindow(static_cast<HWND>(native_handle()));
#endif
					SDL_DestroyWindow(iHandle);
				}
				iHandle = 0;
				window().native_window_closed();
			}
		}
	}

	bool sdl_window::is_visible() const
	{
		return iVisible;
	}

	void sdl_window::show(bool aActivate)
	{
		iVisible = true;
#ifdef WIN32
		ShowWindow(static_cast<HWND>(native_handle()), aActivate ? SW_SHOW : SW_SHOWNA);
#else
		SDL_ShowWindow(iHandle);
#endif
	}

	void sdl_window::hide()
	{
		iVisible = false;
#ifdef WIN32
		ShowWindow(static_cast<HWND>(native_handle()), SW_HIDE);
#else
		SDL_HideWindow(iHandle);
#endif
	}

	void sdl_window::set_transparency(double aAlpha)
	{
		SDL_SetWindowOpacity(iHandle, static_cast<float>(aAlpha));
	}

	bool sdl_window::is_active() const
	{
		return SDL_GetKeyboardFocus() == iHandle;
	}

	void sdl_window::activate()
	{
		if (!is_enabled())
			return;
		SDL_RaiseWindow(iHandle);
#ifdef WIN32
		SetWindowPos(static_cast<HWND>(native_handle()), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif
	}

	bool sdl_window::is_iconic() const
	{
		return (SDL_GetWindowFlags(iHandle) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
	}

	void sdl_window::iconize()
	{
		SDL_MinimizeWindow(iHandle);
	}

	bool sdl_window::is_maximized() const
	{
		return (SDL_GetWindowFlags(iHandle) & SDL_WINDOW_MAXIMIZED) == SDL_WINDOW_MAXIMIZED;
	}

	void sdl_window::maximize()
	{
		SDL_MaximizeWindow(iHandle);
	}

	bool sdl_window::is_restored() const
	{
		return !is_iconic() && !is_maximized();
	}

	void sdl_window::restore()
	{
		SDL_RestoreWindow(iHandle);
	}

	bool sdl_window::is_enabled() const
	{
#ifdef WIN32
		return ::IsWindowEnabled(static_cast<HWND>(native_handle())) == TRUE;
#else
		return true;
#endif
	}

	void sdl_window::enable(bool aEnable)
	{
#ifdef WIN32
		bool wasEnabled = (IsWindowEnabled(static_cast<HWND>(native_handle())) == TRUE);
		EnableWindow(static_cast<HWND>(native_handle()), aEnable);
		if (wasEnabled != aEnable)
		{
			if (aEnable)
				push_event(window_event(window_event::Enabled));
			else
				push_event(window_event(window_event::Disabled));
		}
#endif
	}

	bool sdl_window::is_capturing() const
	{
		return iCapturingMouse;
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

	void sdl_window::set_title_text(const std::string& aTitleText)
	{
		SDL_SetWindowTitle(iHandle, aTitleText.c_str());
	}

#ifdef WIN32
	class suppress_style
	{
	public:
		suppress_style(HWND aWindow, DWORD aStyle) : 
			iWindow{ aWindow },
			iStyle{ aStyle }
		{
			DWORD style = GetWindowLongPtr(iWindow, GWL_STYLE);
			if ((style & iStyle) == iStyle)
				SetWindowLongPtr(iWindow, GWL_STYLE, style & ~iStyle);
			else
				iStyle = 0;
		}
		~suppress_style()
		{
			DWORD style = GetWindowLongPtr(iWindow, GWL_STYLE);
			SetWindowLongPtr(iWindow, GWL_STYLE, style | iStyle);
		}
	private:
		HWND iWindow;
		DWORD iStyle;
	};

	LRESULT CALLBACK sdl_window::CustomWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (msg == WM_NCCREATE)
		{
			auto newWindow = new_window();
			if (newWindow != nullptr)
			{
				sHandleMap[hwnd] = newWindow;
				sHandleMap[hwnd]->iNativeHandle = hwnd;
			}
		}
		auto wndproc = (reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwnd, GWLP_WNDPROC)) == CustomWindowProc ? sSDLWindowProc : DefWindowProc);
		auto mapEntry = sHandleMap.find(hwnd);
		if (mapEntry == sHandleMap.end())
			return DefWindowProc(hwnd, msg, wparam, lparam);
		auto& self = *mapEntry->second;
		LRESULT result;
		bool const CUSTOM_DECORATION = (self.window().style() & window_style::TitleBar) == window_style::TitleBar;
		switch (msg)
		{
		case WM_NCCREATE:
			result = wndproc(hwnd, msg, wparam, lparam);
			break;
		case WM_CREATE:
			result = wndproc(hwnd, msg, wparam, lparam);
			if (CUSTOM_DECORATION)
			{
				MARGINS margins = { 0 };
				DwmExtendFrameIntoClientArea(hwnd, &margins);
			}
			break;
		case WM_SYSCOMMAND:
			result = wndproc(hwnd, msg, wparam, lparam);
			if (wparam == SC_CLOSE)
				self.handle_event(window_event{ window_event::Close });
			break;
		case WM_NCACTIVATE:
			{
				suppress_style ss{ hwnd, WS_VISIBLE };
				result = wndproc(hwnd, msg, wparam, lparam);
			}
			break;
		case WM_SETTEXT:
			result = wndproc(hwnd, msg, wparam, lparam);
			self.native_window::set_title_text(SDL_GetWindowTitle(self.iHandle));
			self.handle_event(window_event{ window_event::TitleTextChanged });
			break;
		case WM_NCPAINT:
			if (CUSTOM_DECORATION)
				result = 0;
			else
				result = wndproc(hwnd, msg, wparam, lparam);
			break;
		case WM_PAINT:
			ValidateRect(hwnd, NULL);
			self.handle_event(window_event(window_event::Paint));
			result = 0;
			break;
		case WM_SYSCHAR:
			result = wndproc(hwnd, msg, wparam, lparam);
			{
				char16_t characterCode = static_cast<char16_t>(wparam);
				std::string text = neolib::utf16_to_utf8(std::u16string(&characterCode, 1));
				self.push_event(keyboard_event(keyboard_event::SysTextInput, text));
			}
			break;
		case WM_CHAR:
			result = wndproc(hwnd, msg, wparam, lparam);
			{
				// SDL doesn't send characters with ASCII value < 32 (e.g. tab) so we do it properly here...
				char16_t characterCode = static_cast<char16_t>(wparam);
				std::string text = neolib::utf16_to_utf8(std::u16string(&characterCode, 1));
				uint8_t ch = static_cast<uint8_t>(text[0]);
				if ((ch >= 32 && ch != 127) || ch == '\t' || ch == '\n')
					self.push_event(keyboard_event(keyboard_event::TextInput, text));
			}
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
			{
				key_modifiers_e modifiers = KeyModifier_NONE;
				if (wparam & MK_SHIFT)
					modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_SHIFT);
				if (wparam & MK_CONTROL)
					modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_CTRL);
				self.push_mouse_button_event_extra_info(modifiers);
				result = wndproc(hwnd, msg, wparam, lparam);
			}
			break;
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
			self.window().native_window_dismiss_children(); // call this before default wndproc (which enters its own NC drag message loop)
			if (CUSTOM_DECORATION)
			{
				switch (wparam)
				{
				case HTCAPTION:
					{
						POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
						ScreenToClient(hwnd, &pt);
						self.iClickedWidgetPart = self.window().native_window_hit_test(basic_point<LONG>{ pt.x, pt.y });
					}
					result = wndproc(hwnd, msg, wparam, lparam);
					break;
				case HTSYSMENU:
					self.iClickedWidgetPart = widget_part::NonClientSystemMenu;
					result = 0;
					break;
				case HTMINBUTTON:
					self.iClickedWidgetPart = widget_part::NonClientMinimizeButton;
					result = 0;
					break;
				case HTMAXBUTTON:
					self.iClickedWidgetPart = widget_part::NonClientMaximizeButton;
					result = 0;
					break;
				case HTCLOSE:
					self.iClickedWidgetPart = widget_part::NonClientCloseButton;
					result = 0;
					break;
				default:
					result = wndproc(hwnd, msg, wparam, lparam);
					break;
				}
			}
			else
				result = wndproc(hwnd, msg, wparam, lparam);
			break;
		case WM_NCLBUTTONUP:
		case WM_NCRBUTTONUP:
		case WM_NCMBUTTONUP:
			if (CUSTOM_DECORATION)
			{
				switch (wparam)
				{
				case HTCAPTION:
					if (msg == WM_NCRBUTTONUP && self.iClickedWidgetPart == widget_part::NonClientTitleBar)
					{
						self.iSystemMenuOpen = true;
						auto cmd = TrackPopupMenu(GetSystemMenu(hwnd, FALSE), TPM_LEFTALIGN | TPM_RETURNCMD, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), 0, hwnd, NULL);
						if (cmd != 0)
							PostMessage(hwnd, WM_SYSCOMMAND, cmd, lparam);
						self.iSystemMenuOpen = false;
					}
					result = 0;
					break;
				case HTSYSMENU:
					if (self.iClickedWidgetPart == widget_part::NonClientSystemMenu)
					{
						self.iSystemMenuOpen = true;
						basic_rect<int> rectTitleBar = self.window().native_window_widget_part_rect(widget_part::NonClientTitleBar) + self.surface_position();
						auto cmd = TrackPopupMenu(GetSystemMenu(hwnd, FALSE), TPM_LEFTALIGN | TPM_RETURNCMD, rectTitleBar.x, rectTitleBar.bottom(), 0, hwnd, NULL);
						if (cmd != 0)
							PostMessage(hwnd, WM_SYSCOMMAND, cmd, lparam);
						self.iSystemMenuOpen = false;
					}
					else if (self.iClickedWidgetPart == widget_part::Nowhere)
						PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, lparam);
					result = 0;
					break;
				case HTMINBUTTON:
					if (self.iClickedWidgetPart == widget_part::NonClientMinimizeButton)
						PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, lparam);
					result = 0;
					break;
				case HTMAXBUTTON:
					if (self.iClickedWidgetPart == widget_part::NonClientMaximizeButton)
						PostMessage(hwnd, WM_SYSCOMMAND, IsMaximized(hwnd) ? SC_RESTORE : SC_MAXIMIZE, lparam);
					result = 0;
					break;
				case HTCLOSE:
					if (self.iClickedWidgetPart == widget_part::NonClientCloseButton)
						PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, lparam);
					result = 0;
					break;
				default:
					result = wndproc(hwnd, msg, wparam, lparam);
					break;
				}
			}
			else
				result = wndproc(hwnd, msg, wparam, lparam);
			self.iClickedWidgetPart = widget_part::Nowhere;
			break;
		case WM_NCLBUTTONDBLCLK:
			if (CUSTOM_DECORATION)
			{
				switch (wparam)
				{
				case HTSYSMENU:
					PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, lparam);
					result = 0;
					break;
				default:
					result = wndproc(hwnd, msg, wparam, lparam);
					break;
				}
			}
			else
				result = wndproc(hwnd, msg, wparam, lparam);
			break;
		case WM_DESTROY:
			self.destroying();
			result = wndproc(hwnd, msg, wparam, lparam);
			break;
		case WM_ERASEBKGND:
			result = true;
			break;
		case WM_NCCALCSIZE:
			if (lparam && CUSTOM_DECORATION)
				result = 0;
			else
				result = wndproc(hwnd, msg, wparam, lparam);
			break;
		case WM_NCHITTEST:
			if (CUSTOM_DECORATION)
			{
				result = HTCLIENT;
				POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
				ScreenToClient(hwnd, &pt);
				RECT rc;
				GetClientRect(hwnd, &rc);
				enum { left = 1, top = 2, right = 4, bottom = 8 };
				int hit = 0;
				if (pt.x < self.border_thickness().left) hit |= left;
				if (pt.x > rc.right - self.border_thickness().right) hit |= right;
				if (pt.y < self.border_thickness().top) hit |= top;
				if (pt.y > rc.bottom - self.border_thickness().bottom) hit |= bottom;
				if (hit & top && hit & left) result = HTTOPLEFT;
				else if (hit & top && hit & right) result = HTTOPRIGHT;
				else if (hit & bottom && hit & left) result = HTBOTTOMLEFT;
				else if (hit & bottom && hit & right) result = HTBOTTOMRIGHT;
				else if (hit & left) result = HTLEFT;
				else if (hit & top) result = HTTOP;
				else if (hit & right) result = HTRIGHT;
				else if (hit & bottom) result = HTBOTTOM;
				if (result == HTCLIENT)
					result = convert_widget_part(self.window().native_window_hit_test(basic_point<LONG>{ pt.x, pt.y }));
			}		
			else
				result = wndproc(hwnd, msg, wparam, lparam);
			break;
		case WM_SIZING:
			{
				self.iExtents = size{
					static_cast<dimension>(reinterpret_cast<const RECT*>(lparam)->right - reinterpret_cast<const RECT*>(lparam)->left),
					static_cast<dimension>(reinterpret_cast<const RECT*>(lparam)->bottom - reinterpret_cast<const RECT*>(lparam)->top) };
				if (!CUSTOM_DECORATION)
				{
					const RECT referenceClientRect = { 0, 0, 256, 256 };
					RECT referenceWindowRect = referenceClientRect;
					AdjustWindowRectEx(&referenceWindowRect, GetWindowLongPtr(hwnd, GWL_STYLE), false, GetWindowLongPtr(hwnd, GWL_EXSTYLE));
					self.iExtents += size{
						basic_size<LONG>{ referenceClientRect.right - referenceClientRect.left, referenceClientRect.bottom - referenceClientRect.top } -
						basic_size<LONG>{ referenceWindowRect.right - referenceWindowRect.left, referenceWindowRect.bottom - referenceWindowRect.top } };
				}
				result = wndproc(hwnd, msg, wparam, lparam);
				self.handle_event(window_event(window_event::Resizing, self.iExtents));
			}
			break;
		case WM_NCDESTROY:
			{
				self.destroyed();
				sHandleMap.erase(mapEntry);
				result = wndproc(hwnd, msg, wparam, lparam);
			}
			break;
		case WM_MOUSEACTIVATE:
			if (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE)
				result = MA_NOACTIVATE;
			else
				result = wndproc(hwnd, msg, wparam, lparam);
			break;
		case WM_CAPTURECHANGED:
			if (self.window().has_capturing_widget())
				self.window().release_capture(self.window().capturing_widget());
			else
				self.release_capture();
			result = wndproc(hwnd, msg, wparam, lparam);
			break;
		default:
			result = wndproc(hwnd, msg, wparam, lparam);
			break;
		}
		return result;
	}
#endif

	void sdl_window::install_creation_hook(sdl_window& aNewWindow)
	{
		sNewWindow = &aNewWindow;
#ifdef WIN32
		if (!sSDLWindowProc)
		{
			HWND sdlWindow = FindWindow(SDL_Appname, NULL);
			if (sdlWindow == NULL)
				throw failed_to_install_window_creation_hook("Failed to find SDL window");
			sSDLWindowProc = reinterpret_cast<WNDPROC>(GetClassLongPtr(sdlWindow, GCLP_WNDPROC));
			if (!SetClassLongPtr(sdlWindow, GCLP_WNDPROC, reinterpret_cast<LONG_PTR>(&CustomWindowProc)))
				throw failed_to_install_window_creation_hook(GetLastErrorAsString());
		}
#endif
	}

	sdl_window* sdl_window::new_window()
	{
		auto newWindow = sNewWindow;
		sNewWindow = nullptr;
		return newWindow;
	}

	void sdl_window::init()
	{
		sHandleMap[native_handle()] = this;
		native_window::set_title_text(SDL_GetWindowTitle(iHandle));
#ifdef WIN32
		HWND hwnd = static_cast<HWND>(native_handle());
		SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, NULL);
		DWORD existingStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
		DWORD newStyle = existingStyle;
		if ((iStyle & window_style::None) == window_style::None || (iStyle & window_style::TitleBar) == window_style::TitleBar)
		{
			newStyle |= WS_POPUP;
			newStyle |= WS_SYSMENU;
			if ((iStyle & window_style::Resize) == window_style::Resize)
			{
				newStyle |= WS_MINIMIZEBOX;
				newStyle |= WS_MAXIMIZEBOX;
				newStyle |= WS_THICKFRAME;
			}
			else
			{
				newStyle &= ~WS_MINIMIZEBOX;
				newStyle &= ~WS_MAXIMIZEBOX;
				newStyle &= ~WS_THICKFRAME;
			}
		}
		if (newStyle != existingStyle)
			SetWindowLongPtr(hwnd, GWL_STYLE, newStyle);
		DWORD existingExtendedStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
		DWORD newExtendedStyle = existingExtendedStyle;
		if ((iStyle & window_style::NoActivate) == window_style::NoActivate)
			newExtendedStyle |= (WS_EX_NOACTIVATE | WS_EX_TOPMOST);
		if ((iStyle & window_style::TitleBar) == window_style::TitleBar)
			newExtendedStyle |= WS_EX_WINDOWEDGE;
		if (newExtendedStyle != existingExtendedStyle)
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, newExtendedStyle);
		if (iParent != 0)
			SetWindowLongPtr(hwnd, GWL_HWNDPARENT, reinterpret_cast<LONG>(iParent->native_handle()));
		SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif
	}

	void sdl_window::process_event(const SDL_Event& aEvent)
	{
		switch (aEvent.type)
		{
		case SDL_WINDOWEVENT:
			switch (aEvent.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				push_event(window_event(window_event::Close));
				break;
			case SDL_WINDOWEVENT_RESIZED:
				iExtents = basic_size<decltype(aEvent.window.data1)>{ aEvent.window.data1, aEvent.window.data2 };
				push_event(window_event(window_event::Resized, iExtents));
				break;
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				iExtents = basic_size<decltype(aEvent.window.data1)>{ aEvent.window.data1, aEvent.window.data2 };
				push_event(window_event(window_event::SizeChanged, iExtents));
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				push_event(window_event(window_event::Iconized));
				invalidate(surface_size());
				render(true);
				break;
			case SDL_WINDOWEVENT_MAXIMIZED:
				push_event(window_event(window_event::Maximized));
				invalidate(surface_size());
				render(true);
				break;
			case SDL_WINDOWEVENT_RESTORED:
				push_event(window_event(window_event::Restored));
				invalidate(surface_size());
				render(true);
				break;
			case SDL_WINDOWEVENT_ENTER:
				push_event(window_event(window_event::Enter));
				break;
			case SDL_WINDOWEVENT_LEAVE:
				push_event(window_event(window_event::Leave));
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				push_event(window_event(window_event::FocusGained));
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				SDL_ResetMouse();
				iMouseButtonEventExtraInfo.clear();
				push_event(window_event(window_event::FocusLost));
				break;
			case SDL_WINDOWEVENT_EXPOSED:
				invalidate(surface_size());
				render(true);
				break;
			}
			break;
		case SDL_MOUSEWHEEL:
			push_event(
				mouse_event(
					mouse_event::WheelScrolled,
					static_cast<mouse_wheel>(
						(aEvent.wheel.y != 0 ? mouse_wheel::Vertical : mouse_wheel::None) | (aEvent.wheel.x != 0 ? mouse_wheel::Horizontal : mouse_wheel::None)),
					delta(static_cast<coordinate>(aEvent.wheel.x), static_cast<coordinate>(aEvent.wheel.y))));
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!iMouseButtonEventExtraInfo.empty())
			{
				if (aEvent.button.clicks == 1)
					push_event(
						mouse_event(
							mouse_event::ButtonPressed,
							convert_mouse_button(aEvent.button.button),
							point{ static_cast<coordinate>(aEvent.button.x), static_cast<coordinate>(aEvent.button.y) },
							iMouseButtonEventExtraInfo.front()));

				else
					push_event(
						mouse_event(
							mouse_event::ButtonDoubleClicked,
							convert_mouse_button(aEvent.button.button),
							point{ static_cast<coordinate>(aEvent.button.x), static_cast<coordinate>(aEvent.button.y) },
							iMouseButtonEventExtraInfo.front()));
				iMouseButtonEventExtraInfo.pop_front();
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (!iMouseButtonEventExtraInfo.empty())
			{
				push_event(
					mouse_event(
						mouse_event::ButtonReleased,
						convert_mouse_button(aEvent.button.button),
						point{ static_cast<coordinate>(aEvent.button.x), static_cast<coordinate>(aEvent.button.y) },
						iMouseButtonEventExtraInfo.front()));
			}
			break;
		case SDL_MOUSEMOTION:
			update_mouse_cursor();
			push_event(mouse_event(mouse_event::Moved, point{ static_cast<coordinate>(aEvent.motion.x), static_cast<coordinate>(aEvent.motion.y) }));
			break;
		case SDL_KEYDOWN:
			push_event(keyboard_event(keyboard_event::KeyPressed, static_cast<scan_code_e>(aEvent.key.keysym.scancode), static_cast<key_code_e>(aEvent.key.keysym.sym), static_cast<key_modifiers_e>(aEvent.key.keysym.mod)));
			break;
		case SDL_KEYUP:
			push_event(keyboard_event(keyboard_event::KeyReleased, static_cast<scan_code_e>(aEvent.key.keysym.scancode), static_cast<key_code_e>(aEvent.key.keysym.sym), static_cast<key_modifiers_e>(aEvent.key.keysym.mod)));
			break;
		case SDL_TEXTEDITING:
			/* todo */
			break;
		case SDL_TEXTINPUT:
			/* do nothing; we handle text input separately. */
			break;
		default:
			break;
		}
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
	}

	margins sdl_window::border_thickness() const
	{
		if ((window().style() & window_style::Resize) == window_style::Resize)
		{
#ifdef WIN32
			RECT borderThickness;
			SetRectEmpty(&borderThickness);
			AdjustWindowRectEx(&borderThickness, GetWindowLongPtr(static_cast<HWND>(native_handle()), GWL_STYLE) | WS_THICKFRAME & ~WS_CAPTION, FALSE, NULL);
			borderThickness.left *= -1;
			borderThickness.top *= -1;
			iBorderThickness = basic_margins<LONG>{ borderThickness.left, borderThickness.top, borderThickness.right, borderThickness.bottom };
#else
			iBorderThickness = margins{ 6.0, 6.0, 6.0, 6.0 };
#endif
		}
		else
			iBorderThickness = margins{ 1.0, 1.0, 1.0, 1.0 };
		return iBorderThickness;
	}

	void sdl_window::push_mouse_button_event_extra_info(key_modifiers_e aKeyModifiers)
	{
		iMouseButtonEventExtraInfo.push_back(aKeyModifiers);
	}

	void sdl_window::display()
	{
		if (rendering_engine().double_buffering())
			SDL_GL_SwapWindow(iHandle);
		else
			glCheck(glFlush());
	}
}