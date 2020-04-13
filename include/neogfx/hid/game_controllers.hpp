// game_controllers.hpp
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
#include <neolib/vector.hpp>
#include <neogfx/hid/i_game_controllers.hpp>

namespace neogfx
{
    class game_controllers : public i_game_controllers
    {
    public:
        define_declared_event(ControllerConnected, controller_connected, i_game_controller&)
        define_declared_event(ControllerDisconnected, controller_disconnected, i_game_controller&)
    public:
        typedef neolib::vector<ref_ptr<i_game_controller>> controller_list;
    public:
        game_controllers();
    public:
        const controller_list& controllers() const override;
        controller_list::iterator add_device(const ref_ptr<i_game_controller>& aController);
        controller_list::iterator remove_device(const ref_ptr<i_game_controller>& aController);
    public:
        template <typename Controller, typename... Args>
        ref_ptr<Controller> add_controller(Args&&... aArgs)
        {
            auto newController = make_ref<Controller>(std::forward<Args>(aArgs)...);
            add_device(newController);
            return newController;
        }
    private:
        controller_list iControllers;
    };
}