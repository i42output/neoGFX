// game_controller.hpp
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

#include <boost/bimap.hpp>

#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/hid/hid_device.hpp>
#include <neogfx/hid/i_game_controller.hpp>

namespace neogfx
{
    class game_controller : public hid_device<i_game_controller>
    {
    public:
        define_declared_event(RawButtonPressed, raw_button_pressed, game_controller_button_ordinal, key_modifiers_e)
        define_declared_event(RawButtonReleased, raw_button_released, game_controller_button_ordinal, key_modifiers_e)
        define_declared_event(ButtonPressed, button_pressed, game_controller_button, key_modifiers_e)
        define_declared_event(ButtonReleased, button_released, game_controller_button, key_modifiers_e)
        define_declared_event(ButtonRepeat, button_repeat, game_controller_button, key_modifiers_e)
        define_declared_event(LeftTriggerMoved, left_trigger_moved, double, key_modifiers_e)
        define_declared_event(RightTriggerMoved, right_trigger_moved, double, key_modifiers_e)
        define_declared_event(LeftThumbMoved, left_thumb_moved, const vec2&, key_modifiers_e)
        define_declared_event(RightThumbMoved, right_thumb_moved, const vec2&, key_modifiers_e)
        define_declared_event(PovMoved, pov_moved, game_controller_pov_ordinal , const vec2&, key_modifiers_e)
        define_declared_event(StickMoved, stick_moved, const vec3&, key_modifiers_e)
        define_declared_event(StickRotated, stick_rotated, const vec3&, key_modifiers_e)
        define_declared_event(SliderMoved, slider_moved, const vec2&, key_modifiers_e)
    public:
        struct button_not_found : std::logic_error { button_not_found() : std::logic_error{ "neogfx::game_controller::button_not_found" } {} };
    public:
        static constexpr game_controller_button_ordinal MAX_BUTTONS = 128u;
        static constexpr game_controller_pov_ordinal MAX_POVS = 4u;
    public:
        typedef boost::bimap<game_controller_button_ordinal, game_controller_button> button_map_type;
    public:
        game_controller(hid_device_subclass aSubclass, const hid_device_uuid& aProductId, const hid_device_uuid& aInstanceId, const button_map_type& aButtonMap = {});
    public:
        bool player_assigned() const override;
        game_player player() const override;
        void assign_player(game_player aPlayer) override;
        void unassign_player() override;    
    public:
        bool have_port() const override;
        game_controller_port port() const override;
        void set_port(game_controller_port aPort) override;
        void clear_port() override;
    public:
        bool needs_calibrating() const override;
    public:
        bool is_button_pressed(game_controller_button_ordinal aButtonOrdinal) const override;
        bool is_button_pressed(game_controller_button aButton) const override;
        double left_trigger_position() const override;
        double right_trigger_position() const override;
        const vec2& left_thumb_position() const override;
        const vec2& right_thumb_position() const override;
        const vec3& stick_position() const override;
        const vec3& stick_rotation() const override;
        const vec2& slider_position() const override;
    public:
        uint32_t button_count() const override;
        bool button_mapped(game_controller_button aButton) const override;
        bool button_mapped(game_controller_button_ordinal aButtonOrdinal) const override;
        game_controller_button_ordinal button_to_button_ordinal(game_controller_button aButton) const override;
        game_controller_button button_ordinal_to_button(game_controller_button_ordinal aButtonOrdinal) const override;
    protected:
        virtual void update_state() = 0;
    protected:
        button_map_type& button_map();
        void set_button_state(game_controller_button_ordinal aButtonOrdinal, bool aIsPressed);
        void set_button_state(game_controller_button aButton, bool aIsPressed);
        void set_left_trigger_position(double aPosition);
        void set_right_trigger_position(double aPosition);
        void set_left_thumb_position(const vec2& aPosition);
        void set_right_thumb_position(const vec2& aPosition);
        void set_pov_position(game_controller_pov_ordinal aPov, const vec2& aPosition);
        void set_stick_position(const vec3& aPosition);
        void set_stick_rotation(const vec3& aRotation);
        void set_slider_position(const vec2& aPosition);
    private:
        neolib::callback_timer iUpdater;
        std::optional<game_player> iPlayer;
        std::optional<game_controller_port> iPort;
        button_map_type iButtonMap;
        std::array<bool, MAX_BUTTONS> iButtonState = {};
        double iLeftTriggerPosition = 0.0;
        double iRightTriggerPosition = 0.0;
        vec2 iLeftThumbPosition;
        vec2 iRightThumbPosition;
        std::array<vec2, MAX_POVS> iPovPosition = {};
        vec3 iStickPosition;
        vec3 iStickRotation;
        vec2 iSliderPosition;
    };
}