// windows_game_controllers.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <vector>
#include <initguid.h>
#include <wbemidl.h>
#include <oleauto.h>
#include <d3d11_1.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <xinput.h>
#pragma comment(lib, "Xinput.lib")
#include <neolib/map.hpp>
#include <neolib/timer.hpp>
#include <neogfx/hid/game_controllers.hpp>

namespace neogfx
{
    namespace native::windows
    {
        struct direct_input_error : std::runtime_error { direct_input_error(const std::string& aReason) : std::runtime_error{ "neogfx::native::windows::direct_input_error: " + aReason } {} };

        class game_controllers : public neogfx::game_controllers
        {
        private:
            struct joystick_device
            {
                DIDEVICEINSTANCE instance;
                IDirectInputDevice* device;
            };
            typedef std::vector<joystick_device> joystick_device_list;
        public:
            game_controllers();
            ~game_controllers();
        public:
            void enumerate_controllers() override;
        public:
            const i_string& product_name(const hid_device_uuid& aProductId) const override;
            const hid_device_uuid& product_database_id(const hid_device_uuid& aProductId) const override;
        private:
            void do_enumerate_controllers();
            bool is_xinput_controller(const GUID& aProductId) const;
            static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
        private:
            neolib::callback_timer iUpdater;
            bool iEnumerationRequested = false;
            mutable IWbemLocator* iWbemLocator = nullptr;
            mutable IEnumWbemClassObject* iEnumDevices = nullptr;
            mutable IWbemServices* iWbemServices = nullptr;
            IDirectInput8* iDirectInput = nullptr;
            std::vector<hid_device_uuid> iEnumerationResults;
            neolib::map<hid_device_uuid, string> iProductNames;
            neolib::map<hid_device_uuid, hid_device_uuid> iProductDatabaseIds;
        };
    }
}