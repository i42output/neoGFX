// windows_directinput_controller.cpp
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
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/i_game_controllers.hpp>
#include "windows_directinput_controller.hpp"

namespace neogfx
{
    namespace native::windows
    {
        directinput_controller::directinput_controller(IDirectInputDevice8* aDevice,  hid_device_subclass aSubclass, const hid_device_uuid& aProductId, const hid_device_uuid& aInstanceId) :
            game_controller{ aSubclass, aProductId, aInstanceId, directinput_button_map(aProductId) }, iDevice{ aDevice }
        {
            if (FAILED(iDevice->SetCooperativeLevel(static_cast<HWND>(service<i_basic_services>().helper_window_handle()), DISCL_EXCLUSIVE | DISCL_BACKGROUND)))
                throw failed_to_initialise();
            if (FAILED(iDevice->SetDataFormat(&c_dfDIJoystick2)))
                throw failed_to_initialise();
            if (FAILED(iDevice->Acquire()))
                throw failed_to_initialise();
        }

        directinput_controller::~directinput_controller()
        {
            iDevice->Unacquire();
            iDevice->Release();
        }

        void directinput_controller::update_state()
        {
            DIJOYSTATE2 djs = {};
            auto const result = iDevice->GetDeviceState(sizeof(djs), &djs);
            if (FAILED(result))
                return;
            for (game_controller_button_ordinal button = 1u; button <= std::min<game_controller_button_ordinal>(MAX_BUTTONS, sizeof(djs.rgbButtons) / sizeof(djs.rgbButtons[0])); ++button)
                set_button_state(button, djs.rgbButtons[button - 1u]);
            auto round_to = [](double n, double precision = 1.0)
            {
                return std::round(n / precision) * precision;
            };
            for (game_controller_pov_ordinal  pov = 1u; pov <= std::min<game_controller_pov_ordinal>(MAX_POVS, sizeof(djs.rgdwPOV) / sizeof(djs.rgdwPOV[0])); ++pov)
                set_pov_position(pov, djs.rgdwPOV[pov - 1u] == -1 ?
                    vec2{} :
                    vec2{ 
                        round_to(std::sin(djs.rgdwPOV[pov - 1u] / 100.0 * math::pi<double>() / 180.0), 0.00001),
                        round_to(std::cos(djs.rgdwPOV[pov - 1u] / 100.0 * math::pi<double>() / 180.0), 0.00001) });
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