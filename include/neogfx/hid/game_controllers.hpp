// game_controllers.hpp
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
#include <neolib/core/vector.hpp>
#include <neolib/core/map.hpp>
#include <neogfx/hid/i_game_controllers.hpp>

namespace neogfx
{
    uint32_t constexpr HARDWARE_BUS_USB         = 0x03;
    uint32_t constexpr HARDWARE_BUS_BLUETOOTH   = 0x05;

    class game_controllers : public i_game_controllers
    {
    public:
        define_declared_event(ControllerConnected, controller_connected, i_game_controller&)
        define_declared_event(ControllerDisconnected, controller_disconnected, i_game_controller&)
        define_declared_event(ControllerCalibrationRequired, controller_calibration_required, i_game_controller&)
    public:
        typedef neolib::vector<ref_ptr<i_game_controller>> controller_list;
        typedef neolib::map<game_controller_button_ordinal, game_controller_button> button_map_type;
    public:
        game_controllers();
    public:
        const controller_list& controllers() const override;
        bool have_controller_for(game_player aPlayer) const override;
        i_game_controller& controller_for(game_player aPlayer) const override;
    public:
        const button_map_type& button_map(const hid_device_uuid& aProductId) const override;
    public:
        abstract_t<neolib::vector<ref_ptr<i_game_controller>>>::iterator add_device(i_game_controller& aController) override;
        abstract_t<neolib::vector<ref_ptr<i_game_controller>>>::iterator remove_device(i_game_controller& aController) override;
    private:
        controller_list iControllers;
        mutable std::map<hid_device_uuid, button_map_type> iButtonMaps;
    };
}