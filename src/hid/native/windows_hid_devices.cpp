// windows_hid_devices.cpp
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

#include <Dbt.h>
#include <initguid.h>
#include <Usbiodef.h>
#include <hidclass.h>

#include <neogfx/app/i_clipboard.hpp>
#include "windows_hid_devices.hpp"

namespace neogfx
{
    namespace native::windows
    {
        neolib::uuid GUID_to_uuid(const GUID& aGuid)
        {
            return neolib::uuid
            {
                aGuid.Data1,
                aGuid.Data2,
                aGuid.Data3,
                static_cast<std::uint16_t>(aGuid.Data4[0] * 0x100 + aGuid.Data4[1]),
                { aGuid.Data4[2], aGuid.Data4[3], aGuid.Data4[4], aGuid.Data4[5], aGuid.Data4[6], aGuid.Data4[7] }
            };
        }

        LRESULT hid_helper_wndproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            switch (message) 
            {
            case WM_DEVICECHANGE:
                switch (wParam) 
                {
                case DBT_DEVICEARRIVAL:
                    if (reinterpret_cast<DEV_BROADCAST_HDR*>(lParam)->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                    {
                        auto& devInfo = *reinterpret_cast<const DEV_BROADCAST_DEVICEINTERFACE*>(lParam);
                        if (devInfo.dbcc_classguid == GUID_DEVINTERFACE_HID)
                        {
#ifdef DEBUG_HID
                            std::cout << "HID device connected." << std::endl;
#endif
                            service<i_hid_devices>().enumerate_devices();
                        }
                    }
                    break;
                case DBT_DEVICEREMOVECOMPLETE:
                    if (reinterpret_cast<DEV_BROADCAST_HDR*>(lParam)->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
                    {
                        auto& devInfo = *reinterpret_cast<const DEV_BROADCAST_DEVICEINTERFACE*>(lParam);
                        if (devInfo.dbcc_classguid == GUID_DEVINTERFACE_HID)
                        {
#ifdef DEBUG_HID
                            std::cout << "HID device disconnected." << std::endl;
#endif
                            service<i_hid_devices>().enumerate_devices();
                        }
                    }
                    break;
                }
                return TRUE;
            case WM_CLIPBOARDUPDATE:
                service<i_clipboard>().updated().trigger();
                break;
            }
            return DefWindowProc(hwnd, message, wParam, lParam);
        }

        static const std::wstring sHidHelperWindowClassName = L"neoGFX::HID_Helper";

        hid_devices::hid_devices() : 
            iHidHelperWindow{ NULL }, iHidHelperNotifyHandle{ NULL }
        {
            WNDCLASSEX wndclass = {};
            wndclass.cbSize = sizeof(wndclass);
            wndclass.hInstance = ::GetModuleHandle(NULL);
            wndclass.lpszClassName = sHidHelperWindowClassName.c_str();
            wndclass.lpfnWndProc = hid_helper_wndproc;
            ::RegisterClassEx(&wndclass);
            iHidHelperWindow = ::CreateWindowEx(0, L"neoGFX::HID_Helper", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
            DEV_BROADCAST_DEVICEINTERFACE devBroadcast = {};
            devBroadcast.dbcc_size = sizeof(devBroadcast);
            devBroadcast.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            devBroadcast.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;
            iHidHelperNotifyHandle = ::RegisterDeviceNotification(iHidHelperWindow, &devBroadcast, DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
            ::AddClipboardFormatListener(iHidHelperWindow);
        }

        hid_devices::~hid_devices()
        {
            ::UnregisterDeviceNotification(iHidHelperNotifyHandle);
            ::DestroyWindow(iHidHelperWindow);
            ::UnregisterClass(sHidHelperWindowClassName.c_str(), ::GetModuleHandle(NULL));
        }

        void hid_devices::enumerate_devices()
        {
            neogfx::hid_devices::enumerate_devices();
        }

        hid_device_class hid_devices::device_class(const hid_device_class_uuid& aClassUuid) const
        {
            return hid_device_class::Unknown;
        }

        hid_device_subclass hid_devices::device_subclass(const hid_device_subclass_uuid& aClassUuid) const
        {
            return hid_device_subclass::Unknown;
        }
    }
}