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
    game_controller::game_controller(game_controller_port aPort, const i_string& aName) :
        hid_device<i_game_controller>{ hid_device_type::Input, hid_device_class::GameController, hid_device_subclass::Gamepad, aName }, iPort{ aPort }
    {
    }

    game_controller_port game_controller::port() const
    {
        return iPort;
    }
}