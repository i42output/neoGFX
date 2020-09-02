// windows_basic_services.cpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <ShellScalingApi.h>
#include <D2d1.h>
#pragma comment(lib, "Shcore.lib")

#include "3rdparty/tinyfiledialogs.h"

#include <neogfx/hid/display.hpp>
#include <neogfx/app/i_app.hpp>
#include "../../hid/native/windows_display.hpp"
#include "i_native_clipboard.hpp"
#include "windows_basic_services.hpp"

namespace neogfx
{
    namespace native::windows
    {
        BOOL CALLBACK count_display_monitors_proc(HMONITOR, HDC, LPRECT, LPARAM aDisplayCount)
        {
            ++*reinterpret_cast<uint32_t*>(aDisplayCount);
            return true;
        }

        BOOL CALLBACK enum_display_monitors_proc(HMONITOR aMonitor, HDC, LPRECT, LPARAM aDisplayList)
        {
            rect rectDisplay;
            MONITORINFO mi;
            mi.cbSize = sizeof(mi);
            GetMonitorInfo(aMonitor, &mi);
            basic_rect<LONG> monitorRect{ basic_point<LONG>{ mi.rcMonitor.left, mi.rcMonitor.top }, basic_size<LONG>{ mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top } };
            basic_rect<LONG> workAreaRect{ basic_point<LONG>{ mi.rcWork.left, mi.rcWork.top }, basic_size<LONG>{ mi.rcWork.right - mi.rcWork.left, mi.rcWork.bottom - mi.rcWork.top } };
            auto& displayList = *reinterpret_cast<std::vector<std::unique_ptr<i_display>>*>(aDisplayList);
            displayList.push_back(std::make_unique<native::windows::display>(static_cast<uint32_t>(displayList.size()), monitorRect, workAreaRect, reinterpret_cast<void*>(aMonitor), reinterpret_cast<void*>(GetDC(NULL))));
            return true;
        }

        basic_services::basic_services(i_async_task& aAppTask) :
            iAppTask{ aAppTask }
        {
            if (::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) == RPC_E_CHANGED_MODE)
                ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
        }

        basic_services::~basic_services()
        {
            ::CoUninitialize();
        }

        neogfx::platform basic_services::platform() const
        {
            return neogfx::platform::Windows;
        }

        i_async_task& basic_services::app_task()
        {
            return iAppTask;
        }

        void basic_services::system_beep()
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

        void basic_services::display_error_dialog(const std::string& aTitle, const std::string& aMessage, void* aParentWindowHandle) const
        {
            tinyfd_messageBox(aParentWindowHandle, aTitle.c_str(), aMessage.c_str(), "ok", "error", 1);
        }

        uint32_t basic_services::display_count() const
        {
            // todo: invalidate diplay count if monitor connected/disconnected
            if (iDisplayCount == std::nullopt)
            {
                iDisplayCount = 0u;
                EnumDisplayMonitors(NULL, NULL, &count_display_monitors_proc, reinterpret_cast<LPARAM>(&*iDisplayCount));
            }
            return *iDisplayCount;
        }

        i_display& basic_services::display(uint32_t aDisplayIndex) const
        {
            // todo: invalidate diplays if monitor connected/disconnected
            if (iDisplays.size() != display_count())
            {
                iDisplays.clear();
                EnumDisplayMonitors(NULL, NULL, &enum_display_monitors_proc, reinterpret_cast<LPARAM>(&iDisplays));
            }
            if (aDisplayIndex >= iDisplays.size())
                throw bad_display_index();
            return *iDisplays[aDisplayIndex];
        }

        class clipboard : public i_native_clipboard
        {
        public:
            bool has_text() const override
            {
                return ::IsClipboardFormatAvailable(CF_UNICODETEXT) && !text().empty();
            }
            std::string text() const override
            {
                std::string result;
                if (::IsClipboardFormatAvailable(CF_UNICODETEXT))
                {
                    if (::OpenClipboard(NULL))
                    {
                        auto const hMem = GetClipboardData(CF_UNICODETEXT);
                        if (hMem)
                        {
                            auto const src = reinterpret_cast<const char16_t*>(::GlobalLock(hMem));
                            result = neolib::utf16_to_utf8(src);
                            ::GlobalUnlock(hMem);
                        }
                        else
                        {
                            ::CloseClipboard();
                            throw failed_to_get_clipboard_data();
                        }
                        ::CloseClipboard();
                    }
                    else
                        throw failed_to_open_clipboard();
                }
                return result;
            }
            void set_text(const std::string& aText) override
            {
                if (::OpenClipboard(NULL))
                {
                    auto const src = neolib::utf8_to_utf16(aText);
                    auto const hMem = ::GlobalAlloc(GMEM_MOVEABLE, (src.size() + 1) * sizeof(char16_t));
                    if (hMem)
                    {
                        auto dst = reinterpret_cast<char16_t*>(::GlobalLock(hMem));
                        if (dst)
                        {
                            dst = std::copy(src.begin(), src.end(), dst);
                            *dst = '\0';
                            ::GlobalUnlock(hMem);
                        }
                        ::EmptyClipboard();
                        if (!SetClipboardData(CF_UNICODETEXT, hMem))
                        {
                            ::CloseClipboard();
                            throw failed_to_set_clipboard_data();
                        }
                    }
                    ::CloseClipboard();
                }
                else
                    throw failed_to_open_clipboard();
            }
            bool has_image() const override
            {
                return ::IsClipboardFormatAvailable(CF_DIB);
            }
            neogfx::image image() const override
            {
                neogfx::image result{ 1.0, texture_sampling::Nearest };
                if (::IsClipboardFormatAvailable(CF_DIB))
                {
                    if (::OpenClipboard(NULL))
                    {
                        auto const hMem = GetClipboardData(CF_DIB);
                        if (hMem)
                        {
                            // todo: this is a naive implementation not yet supporting all DIB formats...
                            auto const& bitmapInfo = *reinterpret_cast<BITMAPINFO const*>(::GlobalLock(hMem));
                            result.resize(basic_size<LONG>{ bitmapInfo.bmiHeader.biWidth, std::abs(bitmapInfo.bmiHeader.biHeight) });
                            int32_t const cx = static_cast<int32_t>(result.extents().cx);
                            int32_t const cy = static_cast<int32_t>(result.extents().cy);
                            auto const memStart = reinterpret_cast<RGBQUAD const*>(reinterpret_cast<std::byte const*>(&bitmapInfo) + 
                                bitmapInfo.bmiHeader.biSize) + 3;
                            for (int32_t y = 0; y < cy; ++y)
                            {
                                auto src = memStart + (bitmapInfo.bmiHeader.biHeight >= 0 ? (cy - y - 1) * cx : y * cx);
                                auto dst = static_cast<avec4u8*>(result.data()) + y * cx;
                                for (int32_t x = 0; x < cx; ++x)
                                {
                                    *dst++ = { src->rgbRed, src->rgbGreen, src->rgbBlue, 0xFF };
                                    ++src;
                                }
                            }
                            ::GlobalUnlock(hMem);
                        }
                        else
                        {
                            ::CloseClipboard();
                            throw failed_to_get_clipboard_data();
                        }
                        ::CloseClipboard();
                    }
                    else
                        throw failed_to_open_clipboard();
                }
                return std::move(result);
            }
            void set_image(const neogfx::image& aImage) override
            {
                throw unsupported_clipboard_operation(); // todo
#if 0 // todo
                if (::OpenClipboard(NULL))
                {
                    auto const src = static_cast<std::byte const*>(aImage.cdata());
                    auto const hDIB = nullptr; // todo
                    auto const hMem = ::GlobalAlloc(GMEM_MOVEABLE, ::GlobalSize(hDIB));
                    if (hMem)
                    {
                        auto dst = reinterpret_cast<std::byte*>(::GlobalLock(hMem));
                        if (dst)
                        {
                            dst = std::copy(src, src + aImage.size(), dst);
                            ::GlobalUnlock(hMem);
                        }
                        ::EmptyClipboard();
                        if (!SetClipboardData(CF_DIB, hMem))
                        {
                            ::CloseClipboard();
                            throw failed_to_set_clipboard_data();
                        }
                    }
                    ::CloseClipboard();
                }
                else
                    throw failed_to_open_clipboard();
#endif // todo
            }
        };

        bool basic_services::has_system_clipboard() const
        {
            return true;
        }

        i_native_clipboard& basic_services::system_clipboard()
        {
            static clipboard sSystemClipboard;
            return sSystemClipboard;
        }

        bool basic_services::has_system_menu_bar() const
        {
            return false;
        }

        i_shared_menu_bar& basic_services::system_menu_bar()
        {
            throw no_system_menu_bar();
        }
    }
}