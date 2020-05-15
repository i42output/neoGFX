// game_controllers.cpp
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
#include <neogfx/hid/hid_devices.hpp>
#include <neogfx/hid/game_controllers.hpp>

namespace neogfx
{
    game_controllers::game_controllers()
    {
    }

    const game_controllers::controller_list& game_controllers::controllers() const
    {
        return iControllers;
    }

    bool game_controllers::have_controller_for(game_player aPlayer) const
    {
        for (auto const& controller : controllers())
            if ((*controller).player_assigned() && (*controller).player() == aPlayer)
                return true;
        return false;
    }

    i_game_controller& game_controllers::controller_for(game_player aPlayer) const
    {
        for (auto const& controller : controllers())
            if ((*controller).player_assigned() && (*controller).player() == aPlayer)
                return *controller;
        throw unassigned_player();
    }

    const game_controllers::button_map_type& game_controllers::button_map(const hid_device_uuid& aProductId) const
    {
        // todo
        return iButtonMaps[aProductId];
    }

    game_controllers::controller_list::iterator game_controllers::add_device(const ref_ptr<i_game_controller>& aController)
    {
        auto newController = iControllers.insert(iControllers.end(), aController);
        service<hid_devices>().add_device(aController);
        ControllerConnected.trigger(*aController);
        return newController;
    }

    game_controllers::controller_list::iterator game_controllers::remove_device(const ref_ptr<i_game_controller>& aController)
    {
        neolib::ref_ptr<i_game_controller> detached{ aController };
        auto existing = std::find(iControllers.begin(), iControllers.end(), detached);
        if (existing != iControllers.end())
        {
            auto next = iControllers.erase(existing);
            ControllerDisconnected.trigger(*detached);
            service<hid_devices>().remove_device(*detached);
            return next;
        }
        return iControllers.end();
    }
}