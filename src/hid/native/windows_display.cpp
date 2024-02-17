// windows_display.cpp
/*
  neogfx C++ App/Game Engine
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

#include <neogfx/neogfx.hpp>

#include <boost/lexical_cast.hpp>
#include <ShellScalingApi.h>
#include <D2d1.h>
#pragma comment(lib, "Shcore.lib")

#include "windows_display.hpp"

namespace neogfx
{
    namespace native::windows
    {
        display::display(uint32_t aIndex, const neogfx::rect& aRect, const neogfx::rect& aDesktopRect, void* aNativeDisplayHandle, void* aNativeDeviceContextHandle) :
            neogfx::display{ aIndex, aRect, aDesktopRect }, 
            iNativeDisplayHandle{ aNativeDisplayHandle },
            iNativeDeviceContextHandle{ aNativeDeviceContextHandle }
        {
            update_dpi();
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
        }

        display::~display()
        {
            ReleaseDC(NULL, reinterpret_cast<HDC>(iNativeDeviceContextHandle));
        }

        void display::update_dpi()
        {
            UINT dpiX = 0;
            UINT dpiY = 0;
            auto ret = ::GetDpiForMonitor(reinterpret_cast<HMONITOR>(iNativeDisplayHandle), MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
            if (ret != S_OK)
                throw failed_to_get_monitor_dpi();
            iPixelDensityDpi = basic_size<UINT>(dpiX, dpiY);
        }

        bool display::is_fullscreen() const
        {
            return !!iFullscreenDisplaySettings;
        }

        const video_mode& display::fullscreen_video_mode() const
        {
            if (is_fullscreen())
                return iFullscreenDisplaySettings->first;
            throw fullscreen_not_active();
        }

        void display::enter_fullscreen(const video_mode& aVideoMode)
        {
            DEVMODE Mode = {};
            // todo: multiple monitors
            if (!::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &Mode))
                throw failed_to_enter_fullscreen{ "enum display settings" };
            if (!is_fullscreen())
                iDesktopDisplaySettings = Mode;
            Mode.dmBitsPerPel = aVideoMode.bits_per_pixel();
            Mode.dmPelsWidth = aVideoMode.resolution().cx;
            Mode.dmPelsHeight = aVideoMode.resolution().cy;
            Mode.dmSize = sizeof(Mode);
            Mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
            if (is_fullscreen() && 
                iFullscreenDisplaySettings->second.dmBitsPerPel == Mode.dmBitsPerPel && 
                iFullscreenDisplaySettings->second.dmPelsWidth == Mode.dmPelsWidth &&
                iFullscreenDisplaySettings->second.dmPelsHeight == Mode.dmPelsHeight)
                return;
            auto result = ::ChangeDisplaySettings(&Mode, CDS_FULLSCREEN);
            if (result != DISP_CHANGE_SUCCESSFUL)
                throw failed_to_enter_fullscreen{ "change display settings, code: " + std::to_string(result) };
            iFullscreenDisplaySettings = { video_mode{Mode.dmPelsWidth, Mode.dmPelsHeight, Mode.dmBitsPerPel }, Mode };
        }

        void display::leave_fullscreen()
        {
            if (!is_fullscreen())
                throw fullscreen_not_active();
            auto result = ::ChangeDisplaySettings(&*iDesktopDisplaySettings, 0);
            if (result != DISP_CHANGE_SUCCESSFUL)
                throw failed_to_leave_fullscreen{ "change display settings, code: " + std::to_string(result) };
            iDesktopDisplaySettings = std::nullopt;
            iFullscreenDisplaySettings = std::nullopt;
        }

        color_space display::color_space() const
        {
            return neogfx::color_space::sRGB;
        }

        color display::read_pixel(const point& aPosition) const
        {
            auto clr = ::GetPixel(reinterpret_cast<HDC>(iNativeDisplayHandle), static_cast<int>(aPosition.x), static_cast<int>(aPosition.y));
            return color{ GetRValue(clr), GetGValue(clr), GetBValue(clr) };
        }
    }
}