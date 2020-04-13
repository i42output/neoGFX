// game_controller.hpp
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
#include <boost/bimap.hpp>
#include <neogfx/hid/hid_device.hpp>
#include <neogfx/hid/i_game_controller.hpp>

namespace neogfx
{
    class game_controller : public hid_device<i_game_controller>
    {
    public:
        define_declared_event(ButtonPressed, button_pressed, game_controller_button, key_modifiers_e)
        define_declared_event(ButtonReleased, button_released, game_controller_button, key_modifiers_e)
        define_declared_event(ButtonRepeat, button_repeat, game_controller_button, key_modifiers_e)
        define_declared_event(LeftTriggerMoved, left_trigger_moved, double, key_modifiers_e)
        define_declared_event(RightTriggerMoved, right_trigger_moved, double, key_modifiers_e)
        define_declared_event(LeftThumbMoved, left_thumb_moved, const vec2&, key_modifiers_e)
        define_declared_event(RightThumbMoved, right_thumb_moved, const vec2&, key_modifiers_e)
        define_declared_event(StickMoved, stick_moved, const vec3&, key_modifiers_e)
        define_declared_event(StickRotated, stick_rotated, const vec3&, key_modifiers_e)
        define_declared_event(SliderMoved, slider_moved, const vec2&, key_modifiers_e)
    public:
        struct button_not_found : std::logic_error { button_not_found() : std::logic_error{ "neogfx::game_controller::button_not_found" } {} };
    private:
        typedef boost::bimap<game_controller_button_index, game_controller_button> button_map_type;
    public:
        game_controller(game_controller_port aPort, hid_device_subclass aSubclass = hid_device_subclass::Gamepad, const i_string& aName = string{ "Generic Game Controller" });
    public:
        game_controller_port port() const override;
    public:
        uint32_t button_count() const override;
        game_controller_button_index button_to_button_index(game_controller_button aButton) const override;
        game_controller_button button_index_to_button(game_controller_button_index aButtonIndex) const override;
    public:
        game_controller_port iPort;
        button_map_type iButtonMap;
    };
}