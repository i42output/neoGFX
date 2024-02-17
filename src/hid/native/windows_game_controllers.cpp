// windows_game_controllers.cpp
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

#include "windows_game_controllers.hpp"
#include "windows_xinput_controller.hpp"
#include "windows_directinput_controller.hpp"

#define SAFE_RELEASE(X) if (X) { X->Release(); X = NULL; }

namespace neogfx
{
    namespace native::windows
    {
        extern neolib::uuid GUID_to_uuid(const GUID& aGuid);
            
        bool game_controllers::is_xinput_controller(const GUID& aProductId) const
        {
            // Code based on https://docs.microsoft.com/en-us/windows/win32/xinput/xinput-and-directinput
            //-----------------------------------------------------------------------------
            // Enum each PNP device using WMI and check each device ID to see if it contains 
            // "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
            // Unfortunately this information can not be found by just using DirectInput 
            //-----------------------------------------------------------------------------
            IWbemClassObject* pDevices[20] = {};

            BSTR                    bstrNamespace = NULL;
            BSTR                    bstrDeviceID = NULL;
            BSTR                    bstrClassName = NULL;
            DWORD                   uReturned = 0;
            bool                    bIsXinputDevice = false;
            UINT                    iDevice = 0;
            VARIANT                 var;
            HRESULT                 hr;

            // Create WMI
            if (iWbemLocator == nullptr)
            {
                hr = CoCreateInstance(__uuidof(WbemLocator),
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    __uuidof(IWbemLocator),
                    (LPVOID*)&iWbemLocator);
                if (FAILED(hr) || iWbemLocator == NULL)
                    goto LCleanup;
            }

            bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2"); if (bstrNamespace == NULL) goto LCleanup;
            bstrClassName = SysAllocString(L"Win32_PNPEntity");   if (bstrClassName == NULL) goto LCleanup;
            bstrDeviceID = SysAllocString(L"DeviceID");          if (bstrDeviceID == NULL)  goto LCleanup;

            // Connect to WMI
            if (iWbemServices == nullptr)
            {
                hr = iWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L,
                    0L, NULL, NULL, &iWbemServices);
                if (FAILED(hr) || iWbemServices == NULL)
                    goto LCleanup;
            }

            if (iEnumDevices == nullptr)
            {
                // Switch security level to IMPERSONATE. 
                CoSetProxyBlanket(iWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                    RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

                hr = iWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &iEnumDevices);
                if (FAILED(hr) || iEnumDevices == NULL)
                    goto LCleanup;
            }

            // Loop over all devices
            hr = iEnumDevices->Reset();
            if (FAILED(hr))
                goto LCleanup;
            for (;; )
            {
                // Get 20 at a time
                hr = iEnumDevices->Next(10000, 20, pDevices, &uReturned);
                if (FAILED(hr))
                    goto LCleanup;
                if (uReturned == 0)
                    break;

                for (iDevice = 0; iDevice < uReturned; iDevice++)
                {
                    // For each device, get its device ID
                    hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
                    if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL)
                    {
                        // Check if the device ID contains "IG_".  If it does, then it's an XInput device
                            // This information can not be found from DirectInput 
                        if (wcsstr(var.bstrVal, L"IG_"))
                        {
                            // If it does, then get the VID/PID from var.bstrVal
                            DWORD dwPid = 0, dwVid = 0;
                            WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
                            if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1)
                                dwVid = 0;
                            WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
                            if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1)
                                dwPid = 0;

                            // Compare the VID/PID to the DInput device
                            DWORD dwVidPid = MAKELONG(dwVid, dwPid);
                            if (dwVidPid == aProductId.Data1)
                            {
                                bIsXinputDevice = true;
                                goto LCleanup;
                            }
                        }
                    }
                    SAFE_RELEASE(pDevices[iDevice]);
                }
            }

        LCleanup:
            if (bstrNamespace)
                SysFreeString(bstrNamespace);
            if (bstrDeviceID)
                SysFreeString(bstrDeviceID);
            if (bstrClassName)
                SysFreeString(bstrClassName);
            for (iDevice = 0; iDevice < 20; iDevice++)
                SAFE_RELEASE(pDevices[iDevice]);

            return bIsXinputDevice;
        }

        BOOL CALLBACK game_controllers::EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
        {
            auto& self = *reinterpret_cast<game_controllers*>(pContext);
            IDirectInputDevice8* directinputDevice;
            auto const deviceProductId = GUID_to_uuid(pdidInstance->guidProduct);
            auto const deviceInstanceId = GUID_to_uuid(pdidInstance->guidInstance);
            auto const deviceProductName = neolib::utf16_to_utf8(reinterpret_cast<const char16_t*>(pdidInstance->tszProductName));
            self.iEnumerationResults.push_back(deviceInstanceId);
            self.iProductNames[deviceProductId] = deviceProductName;
            for (auto const& existingDevice : service<i_game_controllers>().controllers())
                if (existingDevice->instance_id() == deviceInstanceId)
                    return DIENUM_CONTINUE;
            if (SUCCEEDED(self.iDirectInput->CreateDevice(pdidInstance->guidInstance, &directinputDevice, NULL)))
            {
                try
                {
                    if (self.is_xinput_controller(pdidInstance->guidProduct))
                    {
                        service<i_game_controllers>().add_controller<xinput_controller>(
                            directinputDevice,
                            hid_device_subclass::Gamepad,
                            deviceProductId,
                            deviceInstanceId);
                    }
                    else
                    {
                        service<i_game_controllers>().add_controller<directinput_controller>(
                            directinputDevice,
                            hid_device_subclass::Gamepad,
                            deviceProductId,
                            deviceInstanceId);
                    }
                }
                catch (...)
                {
                    directinputDevice->Release();
                    throw;
                }
            }
            return DIENUM_CONTINUE;
        }

        game_controllers::game_controllers() :
            iUpdater{ service<i_async_task>(), [this](neolib::callback_timer& aTimer)
            {
                aTimer.again();
                if (iEnumerationRequested)
                    do_enumerate_controllers();
            }, std::chrono::milliseconds{ 500 } },
            iDirectInput{ nullptr }
        {
            auto result = ::CoCreateInstance(CLSID_DirectInput8, NULL, CLSCTX_INPROC_SERVER, IID_IDirectInput8, (LPVOID*)&iDirectInput);
            if (FAILED(result))
                throw direct_input_error("CoCreateInstance");
            result = iDirectInput->Initialize(GetModuleHandle(NULL), DIRECTINPUT_VERSION);
            if (FAILED(result))
                throw direct_input_error("IDirectInput::Initialize");
        }

        game_controllers::~game_controllers()
        {
            SAFE_RELEASE(iEnumDevices);
            SAFE_RELEASE(iWbemLocator);
            SAFE_RELEASE(iWbemServices);
            iDirectInput->Release();
        }

        void game_controllers::enumerate_controllers()
        {
            iEnumerationRequested = true;
        }

        const i_string& game_controllers::product_name(const hid_device_uuid& aProductId) const
        {
            static const string sUnknownProductName = "Generic Game Controller";
            auto existing = iProductNames.find(aProductId);
            if (existing != iProductNames.end())
                return existing->second();
            return sUnknownProductName;
        }

        void game_controllers::do_enumerate_controllers()
        {
            iEnumerationRequested = false;

            std::set<DWORD> connectedXinputPorts;
            connectedXinputPorts.clear();
            for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
            {
                XINPUT_STATE state = {};
                if (::XInputGetState(i, &state) == ERROR_SUCCESS)
                    connectedXinputPorts.insert(i);
            }

            iEnumerationResults.clear();
            iDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);
            for (auto existing = controllers().begin(); existing != controllers().end();)
            {
                auto found = std::find_if(iEnumerationResults.begin(), iEnumerationResults.end(),
                    [&](auto&& er) { return er == (**existing).instance_id(); });
                if (found != iEnumerationResults.end())
                {
                    auto& controller = **existing++;
                    bool const isXinputController = (dynamic_cast<xinput_controller*>(&controller) != nullptr);
                    if (isXinputController)
                    {
                        if (connectedXinputPorts.size() == 1)
                            controller.set_port(*connectedXinputPorts.begin());
                        else
                        {
                            controller.clear_port();
                            ControllerCalibrationRequired.trigger(controller);
                        }
                    }
                    else
                        controller.clear_port();
                }
                else
                    existing = remove_device(**existing);
            }

            if (!controllers().empty() && !have_controller_for(game_player::One))
            {
                for (auto& controller : controllers())
                {
                    if (controller->needs_calibrating())
                        continue;
                    bool const isXinputController = (dynamic_cast<const xinput_controller*>(&*controller) != nullptr);
                    if (isXinputController)
                        controller->assign_player(game_player::One);
                }
                if (!have_controller_for(game_player::One) && !controllers()[0]->needs_calibrating())
                    controllers()[0]->assign_player(game_player::One);
            }
        }
    }
}