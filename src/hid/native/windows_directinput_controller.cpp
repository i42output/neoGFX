// windows_directinput_controller.cpp
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
#include <neogfx/hid/i_game_controllers.hpp>
#include "windows_directinput_controller.hpp"

namespace neogfx
{
    namespace native::windows
    {
        directinput_controller::directinput_controller(IDirectInputDevice8* aDevice,  hid_device_subclass aSubclass, const hid_device_uuid& aProductId, const hid_device_uuid& aInstanceId) :
            game_controller{ aSubclass, aProductId, aInstanceId, directinput_button_map(aProductId) }, iDevice{ aDevice }
        {
            iDevice->SetDataFormat(&c_dfDIJoystick2);
        }

        directinput_controller::~directinput_controller()
        {
            iDevice->Release();
        }

        void directinput_controller::update_state()
        {
            DIJOYSTATE2 djs = {};
            if (FAILED(iDevice->GetDeviceState(sizeof(djs), &djs)))
                return;
            // todo
        }

        const directinput_controller::button_map_type& directinput_controller::directinput_button_map(const hid_device_uuid& aProductId)
        {
            thread_local std::map<hid_device_uuid, button_map_type> tMapCache;
            if (tMapCache.find(aProductId) == tMapCache.end())
            {
                auto const& map = service<i_game_controllers>().button_map(aProductId);
                for (auto const& mapping : map)
                    tMapCache[aProductId].insert(button_map_type::value_type{ mapping.first(), mapping.second() });
            }
            return tMapCache[aProductId];
        }
    }
}