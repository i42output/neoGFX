// game_controller.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/hid/game_controller.hpp>

namespace neogfx
{
    game_controller::game_controller(hid_device_subclass aSubclass, hid_device_uuid aProductId, hid_device_uuid aInstanceId) :
        hid_device<i_game_controller>{ hid_device_type::Input, hid_device_class::GameController, aSubclass, aProductId, aInstanceId }
    {
    }

    bool game_controller::have_port() const
    {
        return !!iPort;
    }

    game_controller_port game_controller::port() const
    {
        if (iPort)
            return *iPort;
        throw unknown_port();
    }

    void game_controller::set_port(game_controller_port aPort)
    {
        iPort = aPort;
    }

    uint32_t game_controller::button_count() const
    {
        return static_cast<uint32_t>(iButtonMap.size());
    }

    game_controller_button_index game_controller::button_to_button_index(game_controller_button aButton) const
    {
        auto existing = iButtonMap.right.find(aButton);
        if (existing != iButtonMap.right.end())
            return existing->get_left();
        throw button_not_found();
    }

    game_controller_button game_controller::button_index_to_button(game_controller_button_index aButtonIndex) const
    {
        auto existing = iButtonMap.left.find(aButtonIndex);
        if (existing != iButtonMap.left.end())
            return existing->get_right();
        throw button_not_found();
    }
}