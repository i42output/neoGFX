// game_controller.cpp
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
#include <neogfx/hid/game_controller.hpp>
#include <neogfx/hid/i_game_controllers.hpp>

namespace neogfx
{
    game_controller::game_controller(hid_device_subclass aSubclass, const hid_device_uuid& aProductId, const hid_device_uuid& aInstanceId, const button_map_type& aButtonMap) :
        hid_device<i_game_controller>{ hid_device_type::Input, hid_device_class::GameController, aSubclass, aProductId, aInstanceId },
        iUpdater{ service<async_task>(), [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            update_state();
        }, 50u },
        iButtonMap{ aButtonMap }
    {
    }

    bool game_controller::player_assigned() const
    {
        return !!iPlayer;
    }

    game_player game_controller::player() const
    {
        if (iPlayer)
            return *iPlayer;
        throw player_not_assigned();
    }

    void game_controller::assign_player(game_player aPlayer)
    {
        if (iPlayer != aPlayer)
        {
            if (service<i_game_controllers>().have_controller_for(aPlayer))
                service<i_game_controllers>().controller_for(aPlayer).unassign_player();
            iPlayer = aPlayer;
        }
    }

    void game_controller::unassign_player()
    {
        iPlayer = {};
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

    void game_controller::clear_port()
    {
        iPort = {};
    }

    bool game_controller::needs_calibrating() const
    {
        return button_count() == 0u;
    }

    bool game_controller::is_button_pressed(game_controller_button_ordinal aButtonOrdinal) const
    {
        return iButtonState[aButtonOrdinal - 1u];
    }

    bool game_controller::is_button_pressed(game_controller_button aButton) const
    {
        return is_button_pressed(button_to_button_ordinal(aButton));
    }

    double game_controller::left_trigger_position() const
    {
        return iLeftTriggerPosition;
    }

    double game_controller::right_trigger_position() const
    {
        return iRightTriggerPosition;
    }

    const vec2& game_controller::left_thumb_position() const
    {
        return iLeftThumbPosition;
    }

    const vec2& game_controller::right_thumb_position() const
    {
        return iRightThumbPosition;
    }

    const vec3& game_controller::stick_position() const
    {
        return iStickPosition;
    }

    const vec3& game_controller::stick_rotation() const
    {
        return iStickRotation;
    }

    const vec2& game_controller::slider_position() const
    {
        return iSliderPosition;
    }

    uint32_t game_controller::button_count() const
    {
        return static_cast<uint32_t>(iButtonMap.size());
    }

    bool game_controller::button_mapped(game_controller_button aButton) const
    {
        return iButtonMap.right.find(aButton) != iButtonMap.right.end();
    }

    game_controller_button_ordinal game_controller::button_to_button_ordinal(game_controller_button aButton) const
    {
        auto existing = iButtonMap.right.find(aButton);
        if (existing != iButtonMap.right.end())
            return existing->get_left();
        throw button_not_found();
    }

    game_controller_button game_controller::button_ordinal_to_button(game_controller_button_ordinal aButtonOrdinal) const
    {
        auto existing = iButtonMap.left.find(aButtonOrdinal);
        if (existing != iButtonMap.left.end())
            return existing->get_right();
        throw button_not_found();
    }

    game_controller::button_map_type& game_controller::button_map()
    {
        return iButtonMap;
    }

    void game_controller::set_button_state(game_controller_button_ordinal aButtonOrdinal, bool aIsPressed)
    {
        if (iButtonState[aButtonOrdinal - 1u] != aIsPressed)
        {
            iButtonState[aButtonOrdinal - 1u] = aIsPressed;
            if (aIsPressed)
                ButtonPressed.trigger(button_ordinal_to_button(aButtonOrdinal), service<i_keyboard>().modifiers());
            else
                ButtonReleased.trigger(button_ordinal_to_button(aButtonOrdinal), service<i_keyboard>().modifiers());
        }
    }

    void game_controller::set_button_state(game_controller_button aButton, bool aIsPressed)
    {
        set_button_state(button_to_button_ordinal(aButton), aIsPressed);
    }

    void game_controller::set_left_trigger_position(double aPosition)
    {
        if (iLeftTriggerPosition != aPosition)
        {
            iLeftTriggerPosition = aPosition;
            LeftTriggerMoved.trigger(aPosition, service<i_keyboard>().modifiers());
        }
    }

    void game_controller::set_right_trigger_position(double aPosition)
    {
        if (iRightTriggerPosition != aPosition)
        {
            iRightTriggerPosition = aPosition;
            RightTriggerMoved.trigger(aPosition, service<i_keyboard>().modifiers());
        }
    }

    void game_controller::set_left_thumb_position(const vec2& aPosition)
    {
        if (iLeftThumbPosition != aPosition)
        {
            iLeftThumbPosition = aPosition;
            LeftThumbMoved.trigger(aPosition, service<i_keyboard>().modifiers());
        }
    }

    void game_controller::set_right_thumb_position(const vec2& aPosition)
    {
        if (iRightThumbPosition != aPosition)
        {
            iRightThumbPosition = aPosition;
            RightThumbMoved.trigger(aPosition, service<i_keyboard>().modifiers());
        }
    }

    void game_controller::set_stick_position(const vec3& aPosition)
    {
        if (iStickPosition != aPosition)
        {
            iStickPosition = aPosition;
            StickMoved.trigger(aPosition, service<i_keyboard>().modifiers());
        }
    }

    void game_controller::set_stick_rotation(const vec3& aRotation)
    {
        if (iStickRotation != aRotation)
        {
            iStickRotation = aRotation;
            StickRotated.trigger(aRotation, service<i_keyboard>().modifiers());
        }
    }

    void game_controller::set_slider_position(const vec2& aPosition)
    {
        if (iSliderPosition != aPosition)
        {
            iSliderPosition = aPosition;
            SliderMoved.trigger(aPosition, service<i_keyboard>().modifiers());
        }
    }
}