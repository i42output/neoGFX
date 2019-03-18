// sdl_window.cpp
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
#include <thread>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_mouse.h>
#include <SDL_keyboard.h>
#include <neolib/string_utf.hpp>
#include <neogfx/app/i_app.hpp>
#include "../../../gfx/native/opengl.hpp"
#include "../../../gfx/native/opengl_graphics_context.hpp"
#include "../../../hid/native/sdl_keyboard.hpp"
#include "../../../hid/native/sdl_mouse.hpp"
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

    sdl_window::sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle) :
        opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
        iBasicServices(aBasicServices),
        iParent(0),
        iStyle(aStyle),
        iHandle(0),
        iNativeHandle(0),
        iVisible(false),
        iCapturingMouse(false),
        iNonClientCapturing(false),
        iReady(false),
        iRawInputMouseButtonEventExtraInfo{},
        iClickedWidgetPart(widget_part::Nowhere),
        iSystemMenuOpen(false)
    {
        install_creation_hook(*this);
        iHandle = SDL_CreateWindow(
            aWindowTitle.c_str(),
            0,
            0,
            aVideoMode.width(),
            aVideoMode.height(),
            SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | convert_style(aStyle));
        if (iHandle == nullptr)
            throw failed_to_create_window(SDL_GetError());
        init();
        int w, h;
        SDL_GetWindowSize(iHandle, &w, &h);
        iExtents = basic_size<int>{w, h};

        if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
            show((aStyle & window_style::NoActivate) != window_style::NoActivate);

        SDL_SetWindowFullscreen(iHandle, SDL_WINDOW_FULLSCREEN);

        iReady = true;
    }

    sdl_window::sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
        opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
        iBasicServices(aBasicServices),
        iParent(0),
        iStyle(aStyle),
        iHandle(0),
        iNativeHandle(0),
        iVisible(false),
        iCapturingMouse(false),
        iNonClientCapturing(false),
        iReady(false),
        iRawInputMouseButtonEventExtraInfo{},
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
        if (iHandle == nullptr)
            throw failed_to_create_window(SDL_GetError());
        init();
        int w, h;
        SDL_GetWindowSize(iHandle, &w, &h);
        iExtents = basic_size<int>{ w, h };

        if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
            show((aStyle & window_style::NoActivate) != window_style::NoActivate);

        iReady = true;
    }

    sdl_window::sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
        opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
        iBasicServices(aBasicServices),
        iParent(0),
        iStyle(aStyle),
        iHandle(0),
        iNativeHandle(0),
        iVisible(false),
        iCapturingMouse(false),
        iNonClientCapturing(false),
        iReady(false),
        iRawInputMouseButtonEventExtraInfo{},
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
        if (iHandle == nullptr)
            throw failed_to_create_window(SDL_GetError());
        init();
        int w, h;
        SDL_GetWindowSize(iHandle, &w, &h);
        iExtents = basic_size<int>{ w, h };

        if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
            show((aStyle & window_style::NoActivate) != window_style::NoActivate);

        iReady = true;
    }

    sdl_window::sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, sdl_window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle) :
        opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
        iBasicServices(aBasicServices),
        iParent(&aParent),
        iStyle(aStyle),
        iHandle(0),
        iNativeHandle(0),
        iVisible(false),
        iCapturingMouse(false),
        iNonClientCapturing(false),
        iReady(false),
        iRawInputMouseButtonEventExtraInfo{},
        iClickedWidgetPart(widget_part::Nowhere),
        iSystemMenuOpen(false)
    {
        install_creation_hook(*this);
        iHandle = SDL_CreateWindow(
            aWindowTitle.c_str(),
            0,
            0,
            aVideoMode.width(),
            aVideoMode.height(),
            SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | convert_style(aStyle));
        if (iHandle == nullptr)
            throw failed_to_create_window(SDL_GetError());
        init();
        int w, h;
        SDL_GetWindowSize(iHandle, &w, &h);
        iExtents = basic_size<int>{ w, h };

        if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
            show((aStyle & window_style::NoActivate) != window_style::NoActivate);

        SDL_SetWindowFullscreen(iHandle, SDL_WINDOW_FULLSCREEN);

        iReady = true;
    }

    sdl_window::sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, sdl_window& aParent, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
        opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
        iBasicServices(aBasicServices),
        iParent(&aParent),
        iStyle(aStyle),
        iHandle(0),
        iNativeHandle(0),
        iVisible(false),
        iCapturingMouse(false),
        iNonClientCapturing(false),
        iReady(false),
        iRawInputMouseButtonEventExtraInfo{},
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
        if (iHandle == nullptr)
            throw failed_to_create_window(SDL_GetError());
        init();
        int w, h;
        SDL_GetWindowSize(iHandle, &w, &h);
        iExtents = basic_size<int>{ w, h };

        if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
            show((aStyle & window_style::NoActivate) != window_style::NoActivate);

        iReady = true;
    }

    sdl_window::sdl_window(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, sdl_window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
        opengl_window(aRenderingEngine, aSurfaceManager, aWindow),
        iBasicServices(aBasicServices),
        iParent(&aParent),
        iStyle(aStyle),
        iHandle(0),
        iNativeHandle(0),
        iVisible(false),
        iCapturingMouse(false),
        iNonClientCapturing(false),
        iReady(false),
        iRawInputMouseButtonEventExtraInfo{},
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
        if (iHandle == nullptr)
            throw failed_to_create_window(SDL_GetError());
        init();
        int w, h;
        SDL_GetWindowSize(iHandle, &w, &h);
        iExtents = basic_size<int>{ w, h };

        if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
            show((aStyle & window_style::NoActivate) != window_style::NoActivate);

        iReady = true;
    }

    sdl_window::~sdl_window()
    {
        close(true);
    }

    void* sdl_window::target_handle() const
    {
        return iHandle;
    }

    bool sdl_window::has_parent() const
    {
        return iParent != nullptr;
    }

    const i_native_window& sdl_window::parent() const
    {
        if (has_parent())
            return *iParent;
        throw no_parent();
    }

    i_native_window& sdl_window::parent()
    {
        return const_cast<i_native_window&>(const_cast<const sdl_window*>(this)->parent());
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
        if (iNativeHandle == nullptr && !is_destroyed())
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
#ifdef WIN32
        RECT rc;
        GetWindowRect(static_cast<HWND>(native_handle()), &rc);
        x = rc.left;
        y = rc.top;
#else
        SDL_GetWindowPosition(iHandle, &x, &y);
#endif
        return basic_point<int>{ x, y };
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

    bool sdl_window::can_render() const
    {
        return visible() && opengl_window::can_render();
    }

    std::unique_ptr<i_graphics_context> sdl_window::create_graphics_context() const
    {
        return std::unique_ptr<i_graphics_context>(new opengl_graphics_context(*this));
    }

    std::unique_ptr<i_graphics_context> sdl_window::create_graphics_context(const i_widget& aWidget) const
    {
        return std::unique_ptr<i_graphics_context>(new opengl_graphics_context(*this, aWidget));
    }

    void sdl_window::close(bool aForce)
    {
        if (iHandle != nullptr && is_alive() && (aForce || surface_window().native_window_can_close()))
        {
#ifdef WIN32
            DestroyWindow(static_cast<HWND>(native_handle()));
#else
            // todo
            SDL_DestroyWindow(iHandle);
#endif
        }
    }

    bool sdl_window::visible() const
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

    double sdl_window::opacity() const
    {
        float result = 1.0f;
        SDL_GetWindowOpacity(iHandle, &result);
        return static_cast<double>(result);
    }
    
    void sdl_window::set_opacity(double aOpacity)
    {
        SDL_SetWindowOpacity(iHandle, static_cast<float>(aOpacity));
    }

    double sdl_window::transparency() const
    {
        return 1.0 - opacity();
    }

    void sdl_window::set_transparency(double aTransparency) 
    {
        set_opacity(1.0 - aTransparency);
    }

    bool sdl_window::is_active() const
    {
        return SDL_GetKeyboardFocus() == iHandle;
    }

    void sdl_window::activate()
    {
        if (!enabled())
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

    bool sdl_window::is_fullscreen() const
    {
        return (SDL_GetWindowFlags(iHandle) & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN;
    }

    bool sdl_window::enabled() const
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
                push_event(window_event(window_event_type::Enabled));
            else
                push_event(window_event(window_event_type::Disabled));
        }
#endif
    }

    bool sdl_window::is_capturing() const
    {
        return iCapturingMouse;
    }

    // If window being captured doesn't have SDL "keyboard focus" then SDL_CaptureMouse will behave badly.
    class focus_hack
    {
    public:
        focus_hack(sdl_window& aWindow) : iWindow{ aWindow }
        {
            iFocusWindow = sKeyboardFocus;
            sKeyboardFocus = static_cast<SDL_Window*>(iWindow.handle());
        }
        ~focus_hack()
        {
            sKeyboardFocus = iFocusWindow;
        }
    private:
        static SDL_Window*& sKeyboardFocus;
        sdl_window& iWindow;
        SDL_Window* iFocusWindow;
    };

    // Serious hack this that may tie us to specific SDL version; can't think of a non-hacky alternative that doesn't involve forking SDL or bypassing SDL mouse events...
    SDL_Window*& focus_hack::sKeyboardFocus = *reinterpret_cast<SDL_Window**>(const_cast<Uint8*>(SDL_GetKeyboardState(0)) - sizeof(Uint16) - sizeof(SDL_Window*));

    void sdl_window::set_capture()
    {
        if (!iCapturingMouse)
        {
            iCapturingMouse = true;
            iNonClientCapturing = false;
            {
                focus_hack fh{ *this };
                SDL_CaptureMouse(SDL_TRUE);
            }
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
            iNonClientCapturing = false;
#ifdef WIN32
            ReleaseCapture();
#endif
            {
                focus_hack fh{ *this };
                SDL_CaptureMouse(SDL_FALSE);
            }
        }
    }

    void sdl_window::non_client_set_capture()
    {
        if (!iCapturingMouse)
        {
            iCapturingMouse = true;
            iNonClientCapturing = true;
#ifndef WIN32
            {
                focus_hack fh{ *this };
                SDL_CaptureMouse(SDL_TRUE);
            }
#endif
        }
    }

    void sdl_window::non_client_release_capture()
    {
        if (iCapturingMouse)
        {
            iCapturingMouse = false;
            iNonClientCapturing = false;
#ifndef WIN32
            {
                focus_hack fh{ *this };
                SDL_CaptureMouse(SDL_FALSE);
            }
#endif
        }
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
            DWORD style = GetWindowLong(iWindow, GWL_STYLE);
            if ((style & iStyle) == iStyle)
                SetWindowLong(iWindow, GWL_STYLE, style & ~iStyle);
            else
                iStyle = 0;
        }
        ~suppress_style()
        {
            DWORD style = GetWindowLong(iWindow, GWL_STYLE);
            SetWindowLong(iWindow, GWL_STYLE, style | iStyle);
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
        bool const CUSTOM_DECORATION = (self.surface_window().style() & window_style::TitleBar) == window_style::TitleBar;
        switch (msg)
        {
        case WM_NCMOUSEMOVE:
        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONUP:
        case WM_NCLBUTTONDBLCLK:
        case WM_NCRBUTTONDOWN:
        case WM_NCRBUTTONUP:
        case WM_NCRBUTTONDBLCLK:
        case WM_NCMBUTTONDOWN:
        case WM_NCMBUTTONUP:
        case WM_NCMBUTTONDBLCLK:
        case WM_NCXBUTTONDOWN:
        case WM_NCXBUTTONUP:
        case WM_NCXBUTTONDBLCLK:
            if (CUSTOM_DECORATION)
            {
                key_modifiers_e modifiers = KeyModifier_NONE;
                if (GetKeyState(VK_SHIFT) >> 1)
                    modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_SHIFT);
                if (GetKeyState(VK_CONTROL) >> 1)
                    modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_CTRL);
                if (GetKeyState(VK_MENU) >> 1)
                    modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_ALT);
                POINT winPt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
                ScreenToClient(hwnd, &winPt);
                point pt{ basic_point<LONG>{winPt.x, winPt.y} };
                switch (msg)
                {
                case WM_NCMOUSEMOVE:
                    if (!self.non_client_entered())
                        self.push_event(window_event{ window_event_type::NonClientEnter, pt });
                    self.handle_event(non_client_mouse_event{ mouse_event_type::Moved, pt, modifiers });
                    break;
                case WM_NCLBUTTONDOWN:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonClicked, pt, mouse_button::Left, modifiers });
                    break;
                case WM_NCLBUTTONUP:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonReleased, pt, mouse_button::Left, modifiers });
                    break;
                case WM_NCLBUTTONDBLCLK:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, mouse_button::Left, modifiers });
                    break;
                case WM_NCRBUTTONDOWN:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonClicked, pt, mouse_button::Right, modifiers });
                    break;
                case WM_NCRBUTTONUP:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonReleased, pt, mouse_button::Right, modifiers });
                    break;
                case WM_NCRBUTTONDBLCLK:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, mouse_button::Right, modifiers });
                    break;
                case WM_NCMBUTTONDOWN:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonClicked, pt, mouse_button::Middle, modifiers });
                    break;
                case WM_NCMBUTTONUP:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonReleased, pt, mouse_button::Middle, modifiers });
                    break;
                case WM_NCMBUTTONDBLCLK:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, mouse_button::Middle, modifiers });
                    break;
                case WM_NCXBUTTONDOWN:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonClicked, pt, HIWORD(wparam) == XBUTTON1 ? mouse_button::X1 : mouse_button::X2, modifiers });
                    break;
                case WM_NCXBUTTONUP:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonReleased, pt, HIWORD(wparam) == XBUTTON1 ? mouse_button::X1 : mouse_button::X2, modifiers });
                    break;
                case WM_NCXBUTTONDBLCLK:
                    self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, HIWORD(wparam) == XBUTTON1 ? mouse_button::X1 : mouse_button::X2, modifiers });
                    break;
                }
            }
            break;
        }
        switch (msg)
        {
        case WM_NCCREATE:
            {
                auto cs = reinterpret_cast<CREATESTRUCT*>(lparam);
                if (sHandleMap[hwnd]->has_parent())
                    cs->hwndParent = reinterpret_cast<HWND>(sHandleMap[hwnd]->parent().native_handle());
                result = wndproc(hwnd, msg, wparam, lparam);
            }
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
            if (CUSTOM_DECORATION)
            {
                if (wparam == SC_CLOSE)
                {
                    self.handle_event(window_event{ window_event_type::Close });
                    return 0;
                }
            }
            result = wndproc(hwnd, msg, wparam, lparam);
            break;
        case WM_NCACTIVATE:
            {
                suppress_style ss{ hwnd, WS_VISIBLE };
                result = wndproc(hwnd, msg, wparam, lparam);
            }
            break;
        case WM_SETTEXT:
            result = wndproc(hwnd, msg, wparam, lparam);
            self.native_window::set_title_text(neolib::utf16_to_utf8(std::u16string{ (const char16_t*)lparam }));
            self.handle_event(window_event{ window_event_type::TitleTextChanged });
            break;
        case WM_NCPAINT:
            if (CUSTOM_DECORATION)
                result = 0;
            else
                result = wndproc(hwnd, msg, wparam, lparam);
            break;
        case WM_PAINT:
            ValidateRect(hwnd, NULL);
            self.handle_event(window_event(window_event_type::Paint));
            result = 0;
            break;
        case WM_SYSCHAR:
            result = wndproc(hwnd, msg, wparam, lparam);
            {
                char16_t characterCode = static_cast<char16_t>(wparam);
                std::string text = neolib::utf16_to_utf8(std::u16string(&characterCode, 1));
                self.push_event(keyboard_event(keyboard_event_type::SysTextInput, text));
            }
            break;
        case WM_CHAR:
            result = wndproc(hwnd, msg, wparam, lparam);
            {
                // SDL doesn't send characters with ASCII value < 32 (e.g. tab) so we do it properly here...
                char16_t characterCode = static_cast<char16_t>(wparam);
                std::string text = neolib::utf16_to_utf8(std::u16string(&characterCode, 1));
                uint8_t ch = static_cast<uint8_t>(text[0]);
                switch (ch)
                {
                case '\t':
                case '\n':
                case '\r':
                    self.push_event(keyboard_event(keyboard_event_type::TextInput, text));
                    break;
                default:
                    break;
                }
            }
            break;
        case WM_SETCURSOR:
            if (!self.iNonClientCapturing)
                result = wndproc(hwnd, msg, wparam, lparam);
            else
                result = TRUE;
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
        case WM_INPUT:
            {
                key_modifiers_e modifiers = KeyModifier_NONE;
                if (wparam & MK_SHIFT)
                    modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_SHIFT);
                if (wparam & MK_CONTROL)
                    modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_CTRL);
                if (GetKeyState(VK_MENU) >> 1)
                    modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_ALT);
                if (msg != WM_INPUT)
                    self.push_mouse_button_event_extra_info(modifiers);
                else
                    self.raw_input_mouse_button_event_extra_info(modifiers);
                result = wndproc(hwnd, msg, wparam, lparam);
            }
            break;
        case WM_NCLBUTTONDOWN:
        case WM_NCRBUTTONDOWN:
        case WM_NCMBUTTONDOWN:
            self.surface_window().native_window_dismiss_children(); // call this before default wndproc (which enters its own NC drag message loop)
            if (CUSTOM_DECORATION)
            {
                switch (wparam)
                {
                case HTCAPTION:
                    {
                        POINT pt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
                        ScreenToClient(hwnd, &pt);
                        self.iClickedWidgetPart = self.surface_window().native_window_hit_test(basic_point<LONG>{ pt.x, pt.y });
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
                        basic_rect<int> rectTitleBar = self.surface_window().native_window_widget_part_rect(widget_part::NonClientTitleBar) + self.surface_position();
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
            self.set_destroying();
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
                auto margins = self.border_thickness();
                if (pt.x < margins.left) hit |= left;
                if (pt.x > rc.right - margins.right) hit |= right;
                if (pt.y < margins.top) hit |= top;
                if (pt.y > rc.bottom - margins.bottom) hit |= bottom;
                if (hit & top && hit & left) result = HTTOPLEFT;
                else if (hit & top && hit & right) result = HTTOPRIGHT;
                else if (hit & bottom && hit & left) result = HTBOTTOMLEFT;
                else if (hit & bottom && hit & right) result = HTBOTTOMRIGHT;
                else if (hit & left) result = HTLEFT;
                else if (hit & top) result = HTTOP;
                else if (hit & right) result = HTRIGHT;
                else if (hit & bottom) result = HTBOTTOM;
                if (result == HTCLIENT)
                    result = convert_widget_part(self.surface_window().native_window_hit_test(basic_point<LONG>{ pt.x, pt.y }));
            }        
            else
                result = wndproc(hwnd, msg, wparam, lparam);
            break;
        case WM_NCMOUSELEAVE:
            if (self.surface_window().has_capturing_widget())
                self.surface_window().release_capture(self.surface_window().capturing_widget());
            else
                self.release_capture();
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
                    AdjustWindowRectEx(&referenceWindowRect, GetWindowLong(hwnd, GWL_STYLE), false, GetWindowLong(hwnd, GWL_EXSTYLE));
                    self.iExtents += size{
                        basic_size<LONG>{ referenceClientRect.right - referenceClientRect.left, referenceClientRect.bottom - referenceClientRect.top } -
                        basic_size<LONG>{ referenceWindowRect.right - referenceWindowRect.left, referenceWindowRect.bottom - referenceWindowRect.top } };
                }
                result = wndproc(hwnd, msg, wparam, lparam);
                self.handle_event(window_event(window_event_type::Resizing, self.iExtents));
            }
            break;
        case WM_NCDESTROY:
            {
                self.set_destroyed();
                sHandleMap.erase(mapEntry);
                result = wndproc(hwnd, msg, wparam, lparam);
            }
            break;
        case WM_MOUSEACTIVATE:
            if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE)
                result = MA_NOACTIVATE;
            else
                result = wndproc(hwnd, msg, wparam, lparam);
            break;
        case WM_CAPTURECHANGED:
            if (self.surface_window().has_capturing_widget())
                self.surface_window().release_capture(self.surface_window().capturing_widget());
            else
                self.release_capture();
            result = wndproc(hwnd, msg, wparam, lparam);
            break;
        case WM_DPICHANGED:
            self.surface_window().handle_dpi_changed();
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
//        if ((iStyle & window_style::DropShadow) == window_style::DropShadow)
//            SetClassLongPtr(hwnd, GCL_STYLE, GetClassLongPtr(hwnd, GCL_STYLE) | CS_DROPSHADOW); // doesn't work well with OpenGL
        DWORD existingStyle = GetWindowLong(hwnd, GWL_STYLE);
        DWORD newStyle = existingStyle;
        if ((iStyle & window_style::DropShadow) == window_style::DropShadow)
            newStyle |= WS_EX_TOPMOST;
        if ((iStyle & window_style::NoDecoration) == window_style::NoDecoration || (iStyle & window_style::TitleBar) == window_style::TitleBar)
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
            SetWindowLong(hwnd, GWL_STYLE, newStyle);
        DWORD existingExtendedStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        DWORD newExtendedStyle = existingExtendedStyle;
        if ((iStyle & window_style::NoActivate) == window_style::NoActivate)
            newExtendedStyle |= (WS_EX_NOACTIVATE | WS_EX_TOPMOST);
        if ((iStyle & window_style::TitleBar) == window_style::TitleBar)
            newExtendedStyle |= WS_EX_WINDOWEDGE;
        if (newExtendedStyle != existingExtendedStyle)
            SetWindowLong(hwnd, GWL_EXSTYLE, newExtendedStyle);
        if (iParent != nullptr)
            SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, static_cast<LONG>(reinterpret_cast<std::intptr_t>(iParent->native_handle())));
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
                push_event(window_event{ window_event_type::Close });
                break;
            case SDL_WINDOWEVENT_RESIZED:
                iExtents = basic_size<decltype(aEvent.window.data1)>{ aEvent.window.data1, aEvent.window.data2 };
                push_event(window_event{ window_event_type::Resized, iExtents });
                break;
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                iExtents = basic_size<decltype(aEvent.window.data1)>{ aEvent.window.data1, aEvent.window.data2 };
                push_event(window_event{ window_event_type::SizeChanged, iExtents });
                break;
            case SDL_WINDOWEVENT_MINIMIZED:
                push_event(window_event{ window_event_type::Iconized });
                invalidate(surface_size());
                render(true);
                break;
            case SDL_WINDOWEVENT_MAXIMIZED:
                push_event(window_event{ window_event_type::Maximized });
                invalidate(surface_size());
                render(true);
                break;
            case SDL_WINDOWEVENT_RESTORED:
                push_event(window_event{ window_event_type::Restored });
                invalidate(surface_size());
                render(true);
                break;
            case SDL_WINDOWEVENT_ENTER:
                push_event(window_event{ window_event_type::Enter, point{ basic_point<int>{ aEvent.window.data1, aEvent.window.data2 } } });
                break;
            case SDL_WINDOWEVENT_LEAVE:
                push_event(window_event{ window_event_type::Leave });
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                push_event(window_event{ window_event_type::FocusGained });
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                SDL_ResetMouse();
                iMouseButtonEventExtraInfo.clear();
                push_event(window_event{ window_event_type::FocusLost });
                break;
            case SDL_WINDOWEVENT_EXPOSED:
                invalidate(surface_size());
                render(true);
                break;
            }
            break;
        case SDL_MOUSEWHEEL:
            push_event(
                mouse_event{
                    mouse_event_type::WheelScrolled,
                    delta{ static_cast<coordinate>(aEvent.wheel.x), static_cast<coordinate>(aEvent.wheel.y) },
                    static_cast<mouse_wheel>(
                        (aEvent.wheel.y != 0 ? mouse_wheel::Vertical : mouse_wheel::None) | (aEvent.wheel.x != 0 ? mouse_wheel::Horizontal : mouse_wheel::None)),
                    pop_mouse_button_event_extra_info() });
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (aEvent.button.clicks == 1)
                push_event(
                    mouse_event{
                        mouse_event_type::ButtonClicked,
                        point{ static_cast<coordinate>(aEvent.button.x), static_cast<coordinate>(aEvent.button.y) },
                        convert_mouse_button(aEvent.button.button),
                        pop_mouse_button_event_extra_info() });

            else
                push_event(
                    mouse_event{
                        mouse_event_type::ButtonDoubleClicked,
                        point{ static_cast<coordinate>(aEvent.button.x), static_cast<coordinate>(aEvent.button.y) },
                        convert_mouse_button(aEvent.button.button),
                        pop_mouse_button_event_extra_info() });
            break;
        case SDL_MOUSEBUTTONUP:
            push_event(
                mouse_event{
                    mouse_event_type::ButtonReleased,
                    point{ static_cast<coordinate>(aEvent.button.x), static_cast<coordinate>(aEvent.button.y) },
                    convert_mouse_button(aEvent.button.button),
                    pop_mouse_button_event_extra_info() });
            break;
        case SDL_MOUSEMOTION:
            surface_window().as_window().window_manager().update_mouse_cursor(surface_window().as_window());
            push_event(
                mouse_event{ 
                    mouse_event_type::Moved,
                    point{ static_cast<coordinate>(aEvent.motion.x), static_cast<coordinate>(aEvent.motion.y) },
                    convert_mouse_button(aEvent.motion.state) });
            break;
        case SDL_KEYDOWN:
            push_event(keyboard_event{ keyboard_event_type::KeyPressed, sdl_keyboard::from_sdl_scan_code(aEvent.key.keysym.scancode), static_cast<key_code_e>(aEvent.key.keysym.sym), static_cast<key_modifiers_e>(aEvent.key.keysym.mod) });
            break;
        case SDL_KEYUP:
            push_event(keyboard_event{ keyboard_event_type::KeyReleased, sdl_keyboard::from_sdl_scan_code(aEvent.key.keysym.scancode), static_cast<key_code_e>(aEvent.key.keysym.sym), static_cast<key_modifiers_e>(aEvent.key.keysym.mod) });
            break;
        case SDL_TEXTEDITING:
            /* todo */
            break;
        case SDL_TEXTINPUT:
            push_event(keyboard_event{ keyboard_event_type::TextInput, std::string{ aEvent.text.text } });
            break;
        default:
            break;
        }
    }

    void sdl_window::set_destroying()
    {
        opengl_window::set_destroying();
        release_capture();
        surface_window().native_window_closing();
    }
        
    void sdl_window::set_destroyed()
    {
#ifdef WIN32
        SDL_DestroyWindow(iHandle);
#endif
        opengl_window::set_destroyed();
        iNativeHandle = 0;
        surface_window().native_window_closed();
    }

    margins sdl_window::border_thickness() const
    {
        if ((surface_window().style() & window_style::Resize) == window_style::Resize)
        {
#ifdef WIN32
            RECT borderThickness;
            SetRectEmpty(&borderThickness);
            AdjustWindowRectEx(&borderThickness, GetWindowLong(static_cast<HWND>(native_handle()), GWL_STYLE) | WS_THICKFRAME & ~WS_CAPTION, FALSE, NULL);
            borderThickness.left *= -1;
            borderThickness.top *= -1;
            iBorderThickness = basic_margins<LONG>{ borderThickness.left, borderThickness.top, borderThickness.right, borderThickness.bottom };
#else
            iBorderThickness = margins{ 6.0, 6.0, 6.0, 6.0 };
#endif
            iBorderThickness = iBorderThickness + service<i_app>().current_style().margins();
        }
        else
            iBorderThickness = margins{ 1.0, 1.0, 1.0, 1.0 };
        return iBorderThickness;
    }

    void sdl_window::push_mouse_button_event_extra_info(key_modifiers_e aKeyModifiers)
    {
        iMouseButtonEventExtraInfo.push_back(aKeyModifiers);
    }

    void sdl_window::raw_input_mouse_button_event_extra_info(key_modifiers_e aKeyModifiers)
    {
        iRawInputMouseButtonEventExtraInfo = aKeyModifiers;
    }

    key_modifiers_e sdl_window::pop_mouse_button_event_extra_info()
    {
        if (!iMouseButtonEventExtraInfo.empty())
        {
            auto next = iMouseButtonEventExtraInfo.front();
            iMouseButtonEventExtraInfo.pop_front();
            return next;
        }
        else
            return iRawInputMouseButtonEventExtraInfo;
    }

    void sdl_window::display()
    {
        if (rendering_engine().double_buffering())
            SDL_GL_SwapWindow(iHandle);
        else
            glCheck(glFinish());
    }
}