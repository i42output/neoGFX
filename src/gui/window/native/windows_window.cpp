// windows_window.cpp
// Parts of this source file based on Simple DirectMedia Layer, Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>
/*
  neogfx C++ GUI Library
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.

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
/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <neogfx/neogfx.hpp>
#include <thread>
#include <neolib/string_utf.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/app/i_app.hpp>
#include "../../../gfx/native/opengl.hpp"
#include "../../../gfx/native/opengl_rendering_context.hpp"
#include "../../../hid/native/windows_keyboard.hpp"
#include "../../../hid/native/windows_mouse.hpp"
#include "windows_window.hpp"

#include <Windowsx.h>
#include <Dwmapi.h>
#include <Uxtheme.h>
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Uxtheme.lib")
#include <GL/glew.h>
#include <GL/wglew.h>

namespace neogfx
{
    namespace native::windows
    {
        extern std::string GetLastErrorText();

        extern const std::wstring sWindowClassName = L"neoGFX::Window";

        window* window::sNewWindow;

        DWORD window::convert_style(window_style aStyle)
        {   
            DWORD result = 0u;
            if ((aStyle & window_style::NativeTitleBar) != window_style::NativeTitleBar)
                result |= WS_POPUP;
            if ((aStyle & window_style::Resize) == window_style::Resize)
                result |= (WS_THICKFRAME | WS_MAXIMIZEBOX);
            if ((aStyle & window_style::Fullscreen) == window_style::Fullscreen)
                result |= WS_POPUP;
            return result;
        }

        LRESULT convert_widget_part(widget_part aWidgetPart)
        {
            switch (aWidgetPart)
            {
            case widget_part::TitleBar:
            case widget_part::Grab:
                return HTCAPTION;
            case widget_part::Border:
                return HTBORDER;
            case widget_part::BorderLeft:
                return HTLEFT;
            case widget_part::BorderTopLeft:
                return HTTOPLEFT;
            case widget_part::BorderTop:
                return HTTOP;
            case widget_part::BorderTopRight:
                return HTTOPRIGHT;
            case widget_part::BorderRight:
                return HTRIGHT;
            case widget_part::BorderBottomRight:
                return HTBOTTOMRIGHT;
            case widget_part::BorderBottom:
                return HTBOTTOM;
            case widget_part::BorderBottomLeft:
                return HTBOTTOMLEFT;
            case widget_part::GrowBox:
                return HTGROWBOX;
            case widget_part::CloseButton:
                return HTCLOSE;
            case widget_part::MaximizeButton:
                return HTMAXBUTTON;
            case widget_part::MinimizeButton:
                return HTMINBUTTON;
            case widget_part::RestoreButton:
                return HTMAXBUTTON;
            case widget_part::Menu:
                return HTMENU;
            case widget_part::SystemMenu:
                return HTSYSMENU;
            case widget_part::Nowhere:
                return HTNOWHERE;
            case widget_part::NowhereError:
                return HTERROR;
            case widget_part::VerticalScrollbar:
            case widget_part::HorizontalScrollbar:
            case widget_part::Client:
            case widget_part::NonClient:
            case widget_part::NonClientOther:
            default:
                return HTCLIENT;
            }
        }

        std::map<void*, window*> sHandleMap;

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle) :
            opengl_window{ aRenderingEngine, aSurfaceManager, aWindow },
            iParent{},
            iStyle{ aStyle },
            iHandle{},
            iHdc{},
            iVisible{ false },
            iMouseEntered{ false },
            iCapturingMouse{ false },
            iNonClientCapturing{ false },
            iReady{ false },
            iClickedWidgetPart{ widget_part::Nowhere },
            iSystemMenuOpen{ false }
        {
            sNewWindow = this;

            iHandle = ::CreateWindow(
                sWindowClassName.c_str(),
                reinterpret_cast<LPCWSTR>(neolib::utf8_to_utf16(aWindowTitle).c_str()),
                convert_style(aStyle),
                0,
                0,
                aVideoMode.resolution().cx,
                aVideoMode.resolution().cy,
                NULL,
                NULL,
                GetModuleHandle(NULL),
                0);
            if (iHandle == nullptr)
                throw failed_to_create_window(GetLastErrorText());

            init();

            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
            opengl_window{ aRenderingEngine, aSurfaceManager, aWindow },
            iParent{},
            iStyle{ aStyle },
            iHandle{},
            iHdc{},
            iVisible{ false },
            iMouseEntered{ false },
            iCapturingMouse{ false },
            iNonClientCapturing{ false },
            iReady{ false },
            iClickedWidgetPart{ widget_part::Nowhere },
            iSystemMenuOpen{ false }
        {
            sNewWindow = this;

            iHandle = ::CreateWindow(
                sWindowClassName.c_str(),
                reinterpret_cast<LPCWSTR>(neolib::utf8_to_utf16(aWindowTitle).c_str()),
                convert_style(aStyle),
                aSurfaceManager.display(0).desktop_rect().as<int>().centre().x - aDimensions.cx / 2,
                aSurfaceManager.display(0).desktop_rect().as<int>().centre().y - aDimensions.cy / 2,
                aDimensions.cx,
                aDimensions.cy,
                NULL,
                NULL,
                GetModuleHandle(NULL),
                0);
            if (iHandle == nullptr)
                throw failed_to_create_window(GetLastErrorText());

            init();

            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
            opengl_window{ aRenderingEngine, aSurfaceManager, aWindow },
            iParent{},
            iStyle(aStyle),
            iHandle{},
            iHdc{},
            iVisible{ false },
            iMouseEntered{ false },
            iCapturingMouse{ false },
            iNonClientCapturing{ false },
            iReady{ false },
            iClickedWidgetPart{ widget_part::Nowhere },
            iSystemMenuOpen{ false }
        {
            sNewWindow = this;

            iHandle = ::CreateWindow(
                sWindowClassName.c_str(),
                reinterpret_cast<LPCWSTR>(neolib::utf8_to_utf16(aWindowTitle).c_str()),
                convert_style(aStyle),
                aPosition.x,
                aPosition.y,
                aDimensions.cx,
                aDimensions.cy,
                NULL,
                NULL,
                GetModuleHandle(NULL),
                0);
            if (iHandle == nullptr)
                throw failed_to_create_window(GetLastErrorText());

            init();

            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle) :
            opengl_window{ aRenderingEngine, aSurfaceManager, aWindow },
            iParent{ &aParent },
            iStyle{ aStyle },
            iHandle{},
            iHdc{},
            iVisible{ false },
            iMouseEntered{ false },
            iCapturingMouse{ false },
            iNonClientCapturing{ false },
            iReady{ false },
            iClickedWidgetPart{ widget_part::Nowhere },
            iSystemMenuOpen{ false }
        {
            sNewWindow = this;

            iHandle = ::CreateWindow(
                sWindowClassName.c_str(),
                reinterpret_cast<LPCWSTR>(neolib::utf8_to_utf16(aWindowTitle).c_str()),
                convert_style(aStyle),
                0,
                0,
                aVideoMode.resolution().cx,
                aVideoMode.resolution().cy,
                static_cast<HWND>(aParent.handle()),
                NULL,
                GetModuleHandle(NULL),
                0);
            if (iHandle == nullptr)
                throw failed_to_create_window(GetLastErrorText());

            init();

            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
            opengl_window{ aRenderingEngine, aSurfaceManager, aWindow },
            iParent{ &aParent },
            iStyle{ aStyle },
            iHandle{},
            iHdc{},
            iVisible{ false },
            iMouseEntered{ false },
            iCapturingMouse{ false },
            iNonClientCapturing{ false },
            iReady{ false },
            iClickedWidgetPart{ widget_part::Nowhere },
            iSystemMenuOpen{ false }
        {
            sNewWindow = this;

            iHandle = ::CreateWindow(
                sWindowClassName.c_str(),
                reinterpret_cast<LPCWSTR>(neolib::utf8_to_utf16(aWindowTitle).c_str()),
                convert_style(aStyle),
                aSurfaceManager.display(0).desktop_rect().as<int>().centre().x - aDimensions.cx / 2,
                aSurfaceManager.display(0).desktop_rect().as<int>().centre().y - aDimensions.cy / 2,
                aDimensions.cx,
                aDimensions.cy,
                static_cast<HWND>(aParent.handle()),
                NULL,
                GetModuleHandle(NULL),
                0);
            if (iHandle == nullptr)
                throw failed_to_create_window(GetLastErrorText());

            init();

            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, const std::string& aWindowTitle, window_style aStyle) :
            opengl_window{ aRenderingEngine, aSurfaceManager, aWindow },
            iParent{ &aParent },
            iStyle{ aStyle },
            iHandle{},
            iHdc{},
            iPixelFormat{},
            iVisible{ false },
            iMouseEntered{ false },
            iCapturingMouse{ false },
            iNonClientCapturing{ false },
            iReady{ false },
            iClickedWidgetPart{ widget_part::Nowhere },
            iSystemMenuOpen{ false }
        {
            sNewWindow = this;

            iHandle = ::CreateWindow(
                sWindowClassName.c_str(),
                reinterpret_cast<LPCWSTR>(neolib::utf8_to_utf16(aWindowTitle).c_str()),
                convert_style(aStyle),
                aPosition.x,
                aPosition.y,
                aDimensions.cx,
                aDimensions.cy,
                static_cast<HWND>(aParent.handle()),
                NULL,
                GetModuleHandle(NULL),
                0);
            if (iHandle == nullptr)
                throw failed_to_create_window(GetLastErrorText());

            init();

            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::~window()
        {
            close(true);
        }

        void* window::target_handle() const
        {
            return iHandle;
        }

        void* window::target_device_handle() const
        {
            return iHdc;
        }

        i_rendering_engine::pixel_format_t window::pixel_format() const
        {
            if (iPixelFormat == std::nullopt)
                iPixelFormat = service<i_rendering_engine>().set_pixel_format(*this);
            return *iPixelFormat;
        }

        bool window::has_parent() const
        {
            return iParent != nullptr;  
        }

        void window::activate_target() const
        {
            opengl_window::activate_target();
            if (iFirstActivation)
            {
                iFirstActivation = false;
                ::InvalidateRect(iHandle, NULL, true);
            }
        }

        const i_native_window& window::parent() const
        {
            if (has_parent())
                return *iParent;
            throw no_parent();
        }

        i_native_window& window::parent()
        {
            return const_cast<i_native_window&>(to_const(*this).parent());
        }

        bool window::initialising() const
        {
            return !iReady;
        }

        void window::initialisation_complete()
        {
            iReady = true;
        }

        void* window::handle() const
        {
            return iHandle;
        }

        void* window::native_handle() const
        {
            return iHandle;
        }

        point window::surface_position() const
        {
            if (!iPosition)
            {
                RECT rc;
                ::GetWindowRect(iHandle, &rc);
                iPosition.emplace(rc.right - rc.left, rc.bottom - rc.top);
            }
            return *iPosition;
        }

        void window::move_surface(const point& aPosition)
        {
            ::SetWindowPos(iHandle, HWND_NOTOPMOST, static_cast<int>(aPosition.x), static_cast<int>(aPosition.y), 0, 0, SWP_NOCOPYBITS | SWP_NOSIZE | SWP_NOACTIVATE);
        }

        size window::surface_size() const
        {
            if (!iExtents)
            {
                RECT rc;
                ::GetWindowRect(iHandle, &rc);
                iExtents.emplace(rc.right - rc.left, rc.bottom - rc.top);
            }
            return *iExtents;
        }

        void window::resize_surface(const size& aSize)
        {
            ::SetWindowPos(iHandle, HWND_NOTOPMOST, 0, 0, static_cast<int>(aSize.cx), static_cast<int>(aSize.cy), SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOACTIVATE);
        }

        bool window::can_render() const
        {
            return visible() && opengl_window::can_render();
        }

        void window::render(bool aOOBRequest)
        {
            if (!can_render())
                return;
            if (!aOOBRequest)
                opengl_window::render(aOOBRequest);
            else if (has_invalidated_area())
            {
                auto const invalidatedArea = invalidated_area().as<LONG>();
                RECT const rect{ invalidatedArea.left(), invalidatedArea.top(), invalidatedArea.right(), invalidatedArea.bottom() };
                ::InvalidateRect(iHandle, &rect, true);
                validate();
                ::UpdateWindow(iHandle);
            }
        }

        std::unique_ptr<i_rendering_context> window::create_graphics_context(blending_mode aBlendingMode) const
        {
            return std::unique_ptr<i_rendering_context>(new opengl_rendering_context{ *this, aBlendingMode });
        }

        std::unique_ptr<i_rendering_context> window::create_graphics_context(const i_widget& aWidget, blending_mode aBlendingMode) const
        {
            return std::unique_ptr<i_rendering_context>(new opengl_rendering_context{ *this, aWidget, aBlendingMode });
        }

        void window::close(bool aForce)
        {
            if (iHandle != nullptr && is_alive() && (aForce || surface_window().native_window_can_close()))
            {
                set_destroying();
                ::ReleaseDC(iHandle, iHdc);
                ::DestroyWindow(iHandle);
            }
        }

        bool window::visible() const
        {
            return iVisible;
        }

        void window::show(bool aActivate)
        {
            iVisible = true;
            ::ShowWindow(iHandle, aActivate ? SW_SHOW : SW_SHOWNA);
        }

        void window::hide()
        {
            iVisible = false;
            ::ShowWindow(iHandle, SW_HIDE);
        }

        double window::opacity() const
        {
            BYTE alpha;
            if (::GetLayeredWindowAttributes(iHandle, NULL, &alpha, NULL))
                return alpha / 255.0;
            else
                throw failed_to_get_window_information(GetLastErrorText());
        }
    
        void window::set_opacity(double aOpacity)
        {
            if (aOpacity == 1.0) 
            {
                auto const exStyle = GetWindowLong(iHandle, GWL_EXSTYLE);
                if (exStyle & WS_EX_LAYERED)
                {
                    if (::SetWindowLong(iHandle, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED) == 0)
                        throw failed_to_set_window_attributes(GetLastErrorText());
                }
            }
            else 
            {
                auto const alpha = static_cast<BYTE>(aOpacity * 255.0);
                auto const exStyle = GetWindowLong(iHandle, GWL_EXSTYLE);
                if (!(exStyle & WS_EX_LAYERED))
                {
                    if (::SetWindowLong(iHandle, GWL_EXSTYLE, exStyle | WS_EX_LAYERED) == 0)
                        throw failed_to_set_window_attributes(GetLastErrorText());
                }
                if (::SetLayeredWindowAttributes(iHandle, 0, alpha, LWA_ALPHA) == 0)
                    throw failed_to_set_window_attributes(GetLastErrorText());
            }
        }

        double window::transparency() const
        {
            return 1.0 - opacity();
        }

        void window::set_transparency(double aTransparency) 
        {
            set_opacity(1.0 - aTransparency);
        }

        bool window::is_active() const
        {
            return ::GetForegroundWindow() == iHandle;
        }

        void window::activate()
        {
            if (!enabled())
                return;
            ::SetForegroundWindow(iHandle);
            ::SetWindowPos(iHandle, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }

        bool window::is_iconic() const
        {
            return ::GetWindowLong(iHandle, GWL_STYLE) & WS_MINIMIZE;
        }

        void window::iconize()
        {
            ::ShowWindow(iHandle, SW_MINIMIZE);
        }

        bool window::is_maximized() const
        {
            return GetWindowLong(iHandle, GWL_STYLE) & WS_MAXIMIZE;
        }

        void window::maximize()
        {
            ::ShowWindow(iHandle, SW_MAXIMIZE);
        }

        bool window::is_restored() const
        {
            return !is_iconic() && !is_maximized();
        }

        void window::restore()
        {
            ::ShowWindow(iHandle, SW_RESTORE);
        }

        bool window::is_fullscreen() const
        {
            return (iStyle & window_style::Fullscreen) == window_style::Fullscreen;
        }

        bool window::enabled() const
        {
            return ::IsWindowEnabled(iHandle);
        }

        void window::enable(bool aEnable)
        {
            bool wasEnabled = (::IsWindowEnabled(iHandle) == TRUE);
            ::EnableWindow(iHandle, aEnable);
            if (wasEnabled != aEnable)
            {
                if (aEnable)
                    push_event(window_event(window_event_type::Enabled));
                else
                    push_event(window_event(window_event_type::Disabled));
            }
        }

        bool window::is_capturing() const
        {
            return iCapturingMouse;
        }

        void window::set_capture()
        {
            if (!iCapturingMouse)
            {
                iCapturingMouse = true;
                iNonClientCapturing = false;
                service<i_mouse>().capture(surface_window());
            }
        }

        void window::release_capture()
        {
            if (iCapturingMouse)
            {
                iCapturingMouse = false;
                iNonClientCapturing = false;
                service<i_mouse>().release_capture();
            }
        }

        void window::non_client_set_capture()
        {
            if (!iCapturingMouse)
            {
                iCapturingMouse = true;
                iNonClientCapturing = true;
                service<i_mouse>().capture_raw(surface_window());
            }
        }

        void window::non_client_release_capture()
        {
            if (iCapturingMouse)
            {
                iCapturingMouse = false;
                iNonClientCapturing = false;
                service<i_mouse>().release_capture();
            }
        }

        void window::set_title_text(const std::string& aTitleText)
        {
            ::SetWindowText(iHandle, reinterpret_cast<LPCWSTR>(neolib::utf8_to_utf16(aTitleText).c_str()));
        }

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

        LRESULT CALLBACK window::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            if (msg == WM_NCCREATE)
            {
                auto newWindow = new_window();
                if (newWindow != nullptr)
                {
                    sHandleMap[hwnd] = newWindow;
                    sHandleMap[hwnd]->iHandle = hwnd;
                }
            }
            auto const wndproc = DefWindowProc;
            auto const mapEntry = sHandleMap.find(hwnd);
            if (mapEntry == sHandleMap.end())
                return DefWindowProc(hwnd, msg, wparam, lparam);
            auto& self = *mapEntry->second;
            LRESULT result = 0;
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
                    POINT winPt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
                    ScreenToClient(hwnd, &winPt);
                    point pt{ basic_point<LONG>{winPt.x, winPt.y} };
                    switch (msg)
                    {
                    case WM_NCMOUSEMOVE:
                        if (!self.non_client_entered())
                            self.push_event(window_event{ window_event_type::NonClientEnter, pt });
                        self.push_event(non_client_mouse_event{ mouse_event_type::Moved, pt, {}, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCLBUTTONDOWN:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonClicked, pt, mouse_button::Left, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCLBUTTONUP:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonReleased, pt, mouse_button::Left, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCLBUTTONDBLCLK:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, mouse_button::Left, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCRBUTTONDOWN:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonClicked, pt, mouse_button::Right, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCRBUTTONUP:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonReleased, pt, mouse_button::Right, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCRBUTTONDBLCLK:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, mouse_button::Right, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCMBUTTONDOWN:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonClicked, pt, mouse_button::Middle, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCMBUTTONUP:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonReleased, pt, mouse_button::Middle, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCMBUTTONDBLCLK:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, mouse_button::Middle, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCXBUTTONDOWN:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonClicked, pt, HIWORD(wparam) == XBUTTON1 ? mouse_button::X1 : mouse_button::X2, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCXBUTTONUP:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonReleased, pt, HIWORD(wparam) == XBUTTON1 ? mouse_button::X1 : mouse_button::X2, service<i_keyboard>().modifiers() });
                        break;
                    case WM_NCXBUTTONDBLCLK:
                        self.handle_event(non_client_mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, HIWORD(wparam) == XBUTTON1 ? mouse_button::X1 : mouse_button::X2, service<i_keyboard>().modifiers() });
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
            case WM_CLOSE:
                self.handle_event(window_event{ window_event_type::Close });
                result = wndproc(hwnd, msg, wparam, lparam);
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
                if (!self.initialising())
                {
                    RECT rect;
                    if (::GetUpdateRect(hwnd, &rect, FALSE))
                    {
                        PAINTSTRUCT ps;
                        ::BeginPaint(self.iHandle, &ps);
                        self.invalidate(neogfx::rect{ basic_point<LONG>{ ps.rcPaint.left, ps.rcPaint.top }, basic_size<LONG>{ ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top } });
                        self.handle_event(window_event{ window_event_type::Paint });
                        ::EndPaint(self.iHandle, &ps);
                    }
                }
                else
                    ::ValidateRect(hwnd, NULL);
                result = 0;
                break;
            case WM_TIMER:
                ::InvalidateRect(hwnd, NULL, FALSE);
                ::UpdateWindow(hwnd);
                result = 0;
                break;
            case WM_INPUTLANGCHANGE:
                service<i_keyboard>().update_keymap();
                break;
            case WM_SYSCHAR:
                result = wndproc(hwnd, msg, wparam, lparam);
                {
                    char16_t characterCode = static_cast<char16_t>(wparam);
                    std::string text = neolib::utf16_to_utf8(std::u16string(&characterCode, 1));
                    self.push_event(keyboard_event{ keyboard_event_type::SysTextInput, text });
                }
                break;
            case WM_UNICHAR:
                if (wparam == UNICODE_NOCHAR)
                {
                    result = 1;
                    break;
                }
                // fall through
            case WM_CHAR:
                {
                    char16_t characterCode = static_cast<char16_t>(wparam);
                    std::string text = neolib::utf16_to_utf8(std::u16string(&characterCode, 1));
                    if (!text.empty() && (text.size() > 1 || text[0] >= ' ' || std::isspace(text[0])))
                        self.push_event(keyboard_event{ keyboard_event_type::TextInput, text });
                }
                break;
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                self.push_event(
                    keyboard_event{
                        keyboard_event_type::KeyPressed,
                        keyboard::scan_code_from_message(lparam, wparam),
                        service<i_keyboard>().scan_code_to_key_code(keyboard::scan_code_from_message(lparam, wparam)),
                        service<i_keyboard>().modifiers()
                    });
                break;
            case WM_SYSKEYUP:
            case WM_KEYUP:
                self.push_event(
                    keyboard_event{ 
                        keyboard_event_type::KeyReleased, 
                        keyboard::scan_code_from_message(lparam, wparam),
                        service<i_keyboard>().scan_code_to_key_code(keyboard::scan_code_from_message(lparam, wparam)),
                        service<i_keyboard>().modifiers()
                    });
                break;
            case WM_SETCURSOR:
                if (!self.iNonClientCapturing && LOWORD(lparam) == HTCLIENT)
                {
                    service<i_window_manager>().update_mouse_cursor(self.surface_window().as_window());
                    result = TRUE;
                }
                else
                    result = wndproc(hwnd, msg, wparam, lparam);
                break;
            case WM_MOUSEMOVE:
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
                    POINT winPt = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
                    point pt{ basic_point<LONG>{winPt.x, winPt.y} };
                    switch (msg)
                    {
                    case WM_MOUSEMOVE:
                        if (!self.iMouseEntered)
                        {
                            self.iMouseEntered = true;
                            TRACKMOUSEEVENT trackMouseEvent;
                            trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
                            trackMouseEvent.dwFlags = TME_LEAVE;
                            trackMouseEvent.hwndTrack = hwnd;
                            TrackMouseEvent(&trackMouseEvent);
                            self.handle_event(window_event{ window_event_type::Enter, pt });
                        }
                        self.push_event(mouse_event{ mouse_event_type::Moved, pt, mouse::button_from_message(wparam), mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_LBUTTONDOWN:
                        self.push_event(mouse_event{ mouse_event_type::ButtonClicked, pt, mouse_button::Left, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_LBUTTONUP:
                        self.push_event(mouse_event{ mouse_event_type::ButtonReleased, pt, mouse_button::Left, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_LBUTTONDBLCLK:
                        self.push_event(mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, mouse_button::Left, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_RBUTTONDOWN:
                        self.push_event(mouse_event{ mouse_event_type::ButtonClicked, pt, mouse_button::Right, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_RBUTTONUP:
                        self.push_event(mouse_event{ mouse_event_type::ButtonReleased, pt, mouse_button::Right, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_RBUTTONDBLCLK:
                        self.push_event(mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, mouse_button::Right, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_MBUTTONDOWN:
                        self.push_event(mouse_event{ mouse_event_type::ButtonClicked, pt, mouse_button::Middle, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_MBUTTONUP:
                        self.push_event(mouse_event{ mouse_event_type::ButtonReleased, pt, mouse_button::Middle, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_MBUTTONDBLCLK:
                        self.push_event(mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, mouse_button::Middle, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_XBUTTONDOWN:
                        self.push_event(mouse_event{ mouse_event_type::ButtonClicked, pt, HIWORD(wparam) == XBUTTON1 ? mouse_button::X1 : mouse_button::X2, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_XBUTTONUP:
                        self.push_event(mouse_event{ mouse_event_type::ButtonReleased, pt, HIWORD(wparam) == XBUTTON1 ? mouse_button::X1 : mouse_button::X2, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_XBUTTONDBLCLK:
                        self.push_event(mouse_event{ mouse_event_type::ButtonDoubleClicked, pt, HIWORD(wparam) == XBUTTON1 ? mouse_button::X1 : mouse_button::X2, mouse::modifiers_from_message(wparam) });
                        break;
                    case WM_MOUSEWHEEL:
                        self.push_event(mouse_event{ mouse_event_type::WheelScrolled, pt, mouse_wheel::Vertical, mouse::modifiers_from_message(LOWORD(wparam)), neogfx::basic_delta<int16_t>{ 0, static_cast<int16_t>(HIWORD(wparam)) } });
                        break;
                    case WM_MOUSEHWHEEL:
                        self.push_event(mouse_event{ mouse_event_type::WheelScrolled, pt, mouse_wheel::Horizontal, mouse::modifiers_from_message(LOWORD(wparam)), neogfx::basic_delta<int16_t>{ static_cast<int16_t>(HIWORD(wparam)), 0 } });
                        break;
                    }
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
                        self.iClickedWidgetPart = widget_part::SystemMenu;
                        result = 0;
                        break;
                    case HTMINBUTTON:
                        self.iClickedWidgetPart = widget_part::MinimizeButton;
                        result = 0;
                        break;
                    case HTMAXBUTTON:
                        self.iClickedWidgetPart = widget_part::MaximizeButton;
                        result = 0;
                        break;
                    case HTCLOSE:
                        self.iClickedWidgetPart = widget_part::CloseButton;
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
                        if (msg == WM_NCRBUTTONUP && self.iClickedWidgetPart == widget_part::TitleBar)
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
                        if (self.iClickedWidgetPart == widget_part::SystemMenu)
                        {
                            self.iSystemMenuOpen = true;
                            basic_rect<int> rectTitleBar = self.surface_window().native_window_widget_part_rect(widget_part::TitleBar) + self.surface_position();
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
                        if (self.iClickedWidgetPart == widget_part::MinimizeButton)
                            PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, lparam);
                        result = 0;
                        break;
                    case HTMAXBUTTON:
                        if (self.iClickedWidgetPart == widget_part::MaximizeButton)
                            PostMessage(hwnd, WM_SYSCOMMAND, IsMaximized(hwnd) ? SC_RESTORE : SC_MAXIMIZE, lparam);
                        result = 0;
                        break;
                    case HTCLOSE:
                        if (self.iClickedWidgetPart == widget_part::CloseButton)
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
            case WM_MOUSELEAVE:
                if (self.iMouseEntered)
                {
                    self.iMouseEntered = false;
                    self.handle_event(window_event{ window_event_type::Leave });
                }
                break;
            case WM_DESTROY:
                if (self.is_alive())
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
            case WM_INPUT:
                {
                    HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(lparam);
                    RAWINPUT inp;
                    UINT size;
                    GetRawInputData(hRawInput, RID_INPUT, &inp, &size, sizeof(RAWINPUTHEADER));
                    if (inp.header.dwType == RIM_TYPEMOUSE) 
                    {
                        if (service<i_mouse>().capture_type() == mouse_capture_type::Raw) 
                        {
                            POINT pt;
                            RECT hwndRect;
                            HWND currentHnd;
                            GetCursorPos(&pt);
                            currentHnd = WindowFromPoint(pt);
                            ScreenToClient(hwnd, &pt);
                            GetClientRect(hwnd, &hwndRect);
                            if (currentHnd != hwnd || pt.x < 0 || pt.y < 0 || pt.x > hwndRect.right || pt.y > hwndRect.right) 
                                self.push_event(mouse_event{ mouse_event_type::Moved, basic_point<LONG>{ pt.x, pt.y }, service<i_mouse>().button_state(), service<i_keyboard>().modifiers() });
                            if (inp.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
                                self.push_event(mouse_event{ mouse_event_type::ButtonClicked, basic_point<LONG>{ pt.x, pt.y }, mouse_button::Left, service<i_keyboard>().modifiers() });
                            if (inp.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
                                self.push_event(mouse_event{ mouse_event_type::ButtonReleased, basic_point<LONG>{ pt.x, pt.y }, mouse_button::Left, service<i_keyboard>().modifiers() });
                            if (inp.data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
                                self.push_event(mouse_event{ mouse_event_type::ButtonClicked, basic_point<LONG>{ pt.x, pt.y }, mouse_button::Middle, service<i_keyboard>().modifiers() });
                            if (inp.data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
                                self.push_event(mouse_event{ mouse_event_type::ButtonReleased, basic_point<LONG>{ pt.x, pt.y }, mouse_button::Middle, service<i_keyboard>().modifiers() });
                            if (inp.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
                                self.push_event(mouse_event{ mouse_event_type::ButtonClicked, basic_point<LONG>{ pt.x, pt.y }, mouse_button::Right, service<i_keyboard>().modifiers() });
                            if (inp.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
                                self.push_event(mouse_event{ mouse_event_type::ButtonReleased, basic_point<LONG>{ pt.x, pt.y }, mouse_button::Right, service<i_keyboard>().modifiers() });
                            if (inp.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
                                self.push_event(mouse_event{ mouse_event_type::ButtonClicked, basic_point<LONG>{ pt.x, pt.y }, mouse_button::X1, service<i_keyboard>().modifiers() });
                            if (inp.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
                                self.push_event(mouse_event{ mouse_event_type::ButtonReleased, basic_point<LONG>{ pt.x, pt.y }, mouse_button::X1, service<i_keyboard>().modifiers() });
                            if (inp.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
                                self.push_event(mouse_event{ mouse_event_type::ButtonClicked, basic_point<LONG>{ pt.x, pt.y }, mouse_button::X2, service<i_keyboard>().modifiers() });
                            if (inp.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
                                self.push_event(mouse_event{ mouse_event_type::ButtonReleased, basic_point<LONG>{ pt.x, pt.y }, mouse_button::X2, service<i_keyboard>().modifiers() });
                        }
                    }
                }
                break;
            case WM_WINDOWPOSCHANGED:
                {
                    RECT rect;
                    ::GetWindowRect(hwnd, &rect);
                    self.iPosition.emplace(rect.left, rect.top);
                    self.iExtents.emplace(rect.right - rect.left, rect.bottom - rect.top);
                    self.push_event(window_event{ window_event_type::Moved, *self.iPosition });
                    self.push_event(window_event{ window_event_type::Resized, *self.iExtents });
                    if (!self.initialising())
                    {
                        ::InvalidateRect(hwnd, NULL, FALSE);
                        ::UpdateWindow(hwnd);
                    }
                }
                break;
            case WM_ENTERSIZEMOVE:
                ::SetTimer(hwnd, 1, USER_TIMER_MINIMUM, NULL);
                result = wndproc(hwnd, msg, wparam, lparam);
                break;
            case WM_EXITSIZEMOVE:
                ::KillTimer(hwnd, 1);
                result = wndproc(hwnd, msg, wparam, lparam);
                break;
            case WM_SIZE:
                switch (wparam) 
                {
                case SIZE_MAXIMIZED:
                    self.push_event(window_event{ window_event_type::Maximized, *self.iPosition });
                    break;
                case SIZE_MINIMIZED:
                    self.push_event(window_event{ window_event_type::Iconized, *self.iPosition });
                    break;
                default:
                    self.push_event(window_event{ window_event_type::Restored, *self.iPosition });
                    break;
                }
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
                        *self.iExtents += size{
                            basic_size<LONG>{ referenceClientRect.right - referenceClientRect.left, referenceClientRect.bottom - referenceClientRect.top } -
                            basic_size<LONG>{ referenceWindowRect.right - referenceWindowRect.left, referenceWindowRect.bottom - referenceWindowRect.top } };
                    }
                    result = wndproc(hwnd, msg, wparam, lparam);
                    self.handle_event(window_event(window_event_type::Resizing, self.surface_size()));
                }
                break;
            case WM_NCDESTROY:
                {
                    self.set_destroyed();
                    sHandleMap.erase(mapEntry);
                    result = wndproc(hwnd, msg, wparam, lparam);
                }
                break;
            case WM_ACTIVATE:
                {
                    BOOL minimized = HIWORD(wparam);
                    if (!minimized && (LOWORD(wparam) != WA_INACTIVE))
                        self.push_event(window_event{ window_event_type::FocusGained });
                    else
                        self.push_event(window_event{ window_event_type::FocusLost });
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

        window* window::new_window()
        {
            auto newWindow = sNewWindow;
            sNewWindow = nullptr;
            return newWindow;
        }

        void window::init()
        {
            sHandleMap[native_handle()] = this;

            iHdc = ::GetDC(iHandle);

            std::u16string title;
            auto const titleLength = GetWindowTextLength(iHandle);
            title.resize(titleLength + 1);
            GetWindowText(iHandle, reinterpret_cast<LPWSTR>(&title[0]), titleLength + 1);
            title.resize(titleLength);
            native_window::set_title_text(neolib::utf16_to_utf8(title));

            HWND hwnd = iHandle;
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

            ::SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }

         void window::set_destroying()
        {
            opengl_window::set_destroying();
            release_capture();
            surface_window().native_window_closing();
        }
        
        void window::set_destroyed()
        {
            opengl_window::set_destroyed();
            iHandle = 0;
            surface_window().native_window_closed();
        }

        margins window::border_thickness() const
        {
            if ((surface_window().style() & window_style::Resize) == window_style::Resize)
            {
                RECT borderThickness;
                ::SetRectEmpty(&borderThickness);
                ::AdjustWindowRectEx(&borderThickness, GetWindowLong(iHandle, GWL_STYLE) | WS_THICKFRAME & ~WS_CAPTION, FALSE, NULL);
                borderThickness.left *= -1;
                borderThickness.top *= -1;
                iBorderThickness = basic_margins<LONG>{ borderThickness.left, borderThickness.top, borderThickness.right, borderThickness.bottom };
                iBorderThickness += service<i_app>().current_style().margins();
            }
            else
                iBorderThickness = margins{ 1.0, 1.0, 1.0, 1.0 };
            return iBorderThickness;
        }

        void window::display()
        {
            if (rendering_engine().double_buffering())
                ::SwapBuffers(static_cast<HDC>(iHdc));
            else
                glCheck(glDrawBuffer(GL_FRONT));
        }

    }
}