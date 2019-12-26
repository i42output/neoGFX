// sdl_basic_services.cpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#ifdef WIN32
#include <ShellScalingApi.h>
#include <D2d1.h>
#pragma comment(lib, "Shcore.lib")
#endif
#include <SDL_messagebox.h>
#include <SDL_clipboard.h>
#include <SDL_syswm.h>

#include <neogfx/hid/display.hpp>
#include <neogfx/app/i_app.hpp>
#include "i_native_clipboard.hpp"
#include "sdl_basic_services.hpp"

namespace neogfx
{
    window_placement window_placement::default_placement()
    {
        return window_placement{ service<i_basic_services>().display().rect().extents() };
    }

#ifdef WIN32
    BOOL CALLBACK enum_display_monitors_proc(HMONITOR aMonitor, HDC, LPRECT, LPARAM aDisplayList)
    {
        rect rectDisplay;
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfo(aMonitor, &mi);
        basic_rect<LONG> monitorRect{ basic_point<LONG>{ mi.rcMonitor.left, mi.rcMonitor.top }, basic_size<LONG>{ mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top } };
        basic_rect<LONG> workAreaRect{ basic_point<LONG>{ mi.rcWork.left, mi.rcWork.top }, basic_size<LONG>{ mi.rcWork.right - mi.rcWork.left, mi.rcWork.bottom - mi.rcWork.top } };
        auto& displayList = *reinterpret_cast<std::vector<std::unique_ptr<i_display>>*>(aDisplayList);
        displayList.push_back(std::make_unique<display>(static_cast<uint32_t>(displayList.size()), monitorRect, workAreaRect, reinterpret_cast<void*>(aMonitor), reinterpret_cast<void*>(GetDC(NULL))));
        return true;
    }
#endif

    display::display(uint32_t aIndex, const neogfx::rect& aRect, const neogfx::rect& aDesktopRect, void* aNativeDisplayHandle, void* aNativeDeviceContextHandle) :
        iIndex{ aIndex },
        iPixelDensityDpi{ 96.0, 96.0 },
        iRect{ aRect },
        iDesktopRect{ aDesktopRect },
        iSubpixelFormat{ subpixel_format::None },
        iNativeDisplayHandle{ aNativeDisplayHandle },
        iNativeDeviceContextHandle{ aNativeDeviceContextHandle }
    {
        update_dpi();
#ifdef WIN32
        HKEY hkey;
        if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, (L"SOFTWARE\\Microsoft\\Avalon.Graphics\\DISPLAY" + boost::lexical_cast<std::wstring>(index() + 1)).c_str(), 0, KEY_READ, &hkey) == ERROR_SUCCESS)
        {
            DWORD subpixelFormat = 0;
            DWORD cbValue = sizeof(subpixelFormat);
            if (RegQueryValueEx(hkey, L"PixelStructure", NULL, NULL, (LPBYTE)&subpixelFormat, &cbValue) == ERROR_SUCCESS)
            {
                switch (subpixelFormat)
                {
                case 1:
                    iSubpixelFormat = subpixel_format::RGBHorizontal;
                    break;
                case 2:
                    iSubpixelFormat = subpixel_format::BGRHorizontal;
                    break;
                }
            }
            ::RegCloseKey(hkey);
        }
        else
            iSubpixelFormat = subpixel_format::RGBHorizontal;
#endif
    }

    display::~display()
    {
#ifdef WIN32
        ReleaseDC(NULL, reinterpret_cast<HDC>(iNativeDeviceContextHandle));
#endif
    }

    bool display::high_dpi() const
    {
        return device_metrics().ppi() >= 150.0;
    }

    dimension display::dpi_scale_factor() const
    {
        return default_dpi_scale_factor(device_metrics().ppi());
    }

    bool display::device_metrics_available() const
    {
        return true;
    }

    const i_device_metrics& display::device_metrics() const
    {
        return *this;
    }

    uint32_t display::index() const
    {
        return iIndex;
    }

    const i_device_metrics& display::metrics() const
    {
        return *this;
    }

    void display::update_dpi()
    {
#ifdef WIN32
        UINT dpiX = 0;
        UINT dpiY = 0;
        auto ret = GetDpiForMonitor(reinterpret_cast<HMONITOR>(iNativeDisplayHandle), MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
        if (ret != S_OK)
            throw failed_to_get_monitor_dpi();
        iPixelDensityDpi = basic_size<UINT>(dpiX, dpiY);
#endif
    }

    rect display::rect() const
    {
        return iRect;
    }

    rect display::desktop_rect() const
    {
        return iDesktopRect;
    }

    window_placement display::default_window_placement() const
    {
        auto fullscreenResolution = service<i_app>().program_options().full_screen();
        if (fullscreenResolution != std::nullopt)
        {
            if (fullscreenResolution->first == 0)
                return window_placement{ video_mode{ rect().extents() } };
            else
                return window_placement{ video_mode{ fullscreenResolution->first, fullscreenResolution->second } };
        }
        return desktop_rect().deflate(desktop_rect().extents() * 0.167);
    }

    subpixel_format display::subpixel_format() const
    {
        return iSubpixelFormat;
    }

    bool display::metrics_available() const
    {
        return true;
    }

    size display::extents() const
    {
        return desktop_rect().extents();
    }

    dimension display::horizontal_dpi() const
    {
        return iPixelDensityDpi.cx;
    }

    dimension display::vertical_dpi() const
    {
        return iPixelDensityDpi.cy;
    }

    dimension display::ppi() const
    {
        return iPixelDensityDpi.magnitude() / std::sqrt(2.0);
    }

    dimension display::em_size() const
    {
        return 0;
    }

    colour display::read_pixel(const point& aPosition) const
    {
#ifdef WIN32
        auto clr = GetPixel(reinterpret_cast<HDC>(iNativeDisplayHandle), static_cast<int>(aPosition.x), static_cast<int>(aPosition.y));
        return colour{ GetRValue(clr), GetGValue(clr), GetBValue(clr) };
#else
        // todo
        return colour::Black;
#endif
    }

    sdl_basic_services::sdl_basic_services(neolib::async_task& aAppTask) :
        iAppTask{ aAppTask }
    {
    }

    neogfx::platform sdl_basic_services::platform() const
    {
        return neogfx::platform::Windows;
    }

    neolib::async_task& sdl_basic_services::app_task()
    {
        return iAppTask;
    }


    void sdl_basic_services::system_beep()
    {
#if defined(WIN32) 
        MessageBeep(MB_OK);
#elif defined(__APPLE__) 
        SysBeep(1);
#elif defined(SDL_VIDEO_DRIVER_X11) 
        SDL_SysWMInfo info;
        SDL_VERSION(&info.version);
        SDL_GetWMInfo(&info);
        XBell(info.info.x11.display, 100);
#else 
        std::cerr << '\a' << std::flush;
#endif 
    }

    void sdl_basic_services::display_error_dialog(const std::string& aTitle, const std::string& aMessage, void* aParentWindowHandle) const
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, aTitle.c_str(), aMessage.c_str(), static_cast<SDL_Window*>(aParentWindowHandle));
    }

    uint32_t sdl_basic_services::display_count() const
    {
        return SDL_GetNumVideoDisplays();
    }

    i_display& sdl_basic_services::display(uint32_t aDisplayIndex) const
    {
        if (iDisplays.size() != display_count())
        {
            iDisplays.clear();
#ifdef WIN32
            EnumDisplayMonitors(NULL, NULL, &enum_display_monitors_proc, reinterpret_cast<LPARAM>(&iDisplays));
#else
            for (int i = 0; i < display_count(); ++i)
            {
                SDL_Rect rectDisplayBounds;
                SDL_GetDisplayBounds(i, &rectDisplayBounds);
                rect rectDisplay{ point{ rectDisplayBounds.x, rectDisplayBounds.y }, size{ rectDisplayBounds.w, rectDisplayBounds.h } }
                iDisplays.push_back(std::make_unique<neogfx::display>(rectDisplay, rectDisplay, nullptr));
            }
#endif
        }
        if (aDisplayIndex >= iDisplays.size())
            throw bad_display_index();
        return *iDisplays[aDisplayIndex];
    }

    class sdl_clipboard : public i_native_clipboard
    {
    public:
        bool has_text() const override
        {
            return SDL_HasClipboardText() == SDL_TRUE;
        }
        std::string text() const override
        {
            char* clipboardText = SDL_GetClipboardText();
            if (clipboardText == NULL)
                return std::string{};
            std::string result{ clipboardText };
            SDL_free(clipboardText);
            return result;
        }
        void set_text(const std::string& aText) override
        {
            SDL_SetClipboardText(aText.c_str());
        }
    };

    bool sdl_basic_services::has_system_clipboard() const
    {
        return true;
    }

    i_native_clipboard& sdl_basic_services::system_clipboard()
    {
        static sdl_clipboard sSystemClipboard;
        return sSystemClipboard;
    }

    bool sdl_basic_services::has_system_menu_bar() const
    {
        return false;
    }

    i_shared_menu_bar& sdl_basic_services::system_menu_bar()
    {
        throw no_system_menu_bar();
    }
}