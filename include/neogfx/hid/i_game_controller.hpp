// i_game_controller.hpp
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

#include <neogfx/core/numerical.hpp>
#include <neogfx/hid/i_hid_device.hpp>
#include <neogfx/hid/i_keyboard.hpp>

namespace neogfx
{
    enum class game_player : std::uint32_t
    {
        One     = 0x00000001,
        Two     = 0x00000002,
        Three   = 0x00000003,
        Four    = 0x00000004
    };

    typedef std::uint32_t game_controller_port;

    enum class game_controller_button : std::uint64_t
    {
        None                = 0x0000000000000000,
        A                   = 0x0000000000000001,
        B                   = 0x0000000000000002,
        X                   = 0x0000000000000004,
        Y                   = 0x0000000000000008,
        C                   = X,
        D                   = Y,
        Cross               = A,
        Circle              = B,
        Square              = X,
        Triangle            = Y,
        Green               = A,
        Red                 = B,
        Blue                = X,
        Yellow              = Y,
        LeftShoulder        = 0x0000000000000010,
        RightShoulder       = 0x0000000000000020,
        LeftTrigger         = 0x0000000000000040,
        RightTrigger        = 0x0000000000000080,
        DirectionalPadUp    = 0x0000000000000100,
        DirectionalPadDown  = 0x0000000000000200,
        DirectionalPadLeft  = 0x0000000000000400,
        DirectionalPadRight = 0x0000000000000800,
        LeftThumb           = 0x0000000000001000,
        LeftThumbUp         = 0x0000000000002000,
        LeftThumbDown       = 0x0000000000004000,
        LeftThumbLeft       = 0x0000000000008000,
        LeftThumbRight      = 0x0000000000010000,
        LeftThumbUpLeft     = 0x0000000000020000,
        LeftThumbUpRight    = 0x0000000000040000,
        LeftThumbDownLeft   = 0x0000000000080000,
        LeftThumbDownRight  = 0x0000000000100000,
        RightThumb          = 0x0000000000200000,
        RightThumbUp        = 0x0000000000400000,
        RightThumbDown      = 0x0000000000800000,
        RightThumbLeft      = 0x0000000001000000,
        RightThumbRight     = 0x0000000002000000,
        RightThumbUpLeft    = 0x0000000004000000,
        RightThumbUpRight   = 0x0000000008000000,
        RightThumbDownLeft  = 0x0000000010000000,
        RightThumbDownRight = 0x0000000020000000,
        Start               = 0x0000000040000000,
        Back                = 0x0000000080000000,
        Function1           = 0x0000000100000000,
        Function2           = 0x0000000200000000,
        Function3           = 0x0000000400000000,
        Function4           = 0x0000000800000000,
        Function5           = 0x0000001000000000,
        Function6           = 0x0000002000000000,
        Function7           = 0x0000004000000000,
        Function8           = 0x0000008000000000,
        Function9           = 0x0000010000000000,
        Function10          = 0x0000020000000000,
        Function11          = 0x0000040000000000,
        Function12          = 0x0000080000000000,
        Function13          = 0x0000100000000000,
        Function14          = 0x0000200000000000,
        Function15          = 0x0000400000000000,
        Function16          = 0x0000800000000000,
        Function17          = 0x0001000000000000,
        Function18          = 0x0002000000000000,
        Function19          = 0x0004000000000000,
        Function20          = 0x0008000000000000,
        Function21          = 0x0010000000000000,
        Function22          = 0x0020000000000000,
        Function23          = 0x0040000000000000,
        Function24          = 0x0080000000000000,
        Function25          = 0x0100000000000000,
        Function26          = 0x0200000000000000,
        Function27          = 0x0400000000000000,
        Function28          = 0x0800000000000000,
        Function29          = 0x1000000000000000,
        Function30          = 0x2000000000000000,
        Function31          = 0x4000000000000000,
        Function32          = 0x8000000000000000
    };

    typedef std::uint32_t game_controller_button_ordinal;

    typedef std::uint32_t game_controller_pov_ordinal ;

    class i_game_controller : public i_hid_device
    {
    public:
        typedef i_game_controller abstract_type;
    public:
        declare_event(raw_button_pressed, game_controller_button_ordinal, key_modifiers_e)
        declare_event(raw_button_released, game_controller_button_ordinal, key_modifiers_e)
        declare_event(button_pressed, game_controller_button, key_modifiers_e)
        declare_event(button_released, game_controller_button, key_modifiers_e)
        declare_event(button_repeat, game_controller_button, key_modifiers_e)
        declare_event(left_trigger_moved, double, key_modifiers_e)
        declare_event(right_trigger_moved, double, key_modifiers_e)
        declare_event(left_thumb_moved, const vec2&, key_modifiers_e)
        declare_event(right_thumb_moved, const vec2&, key_modifiers_e)
        declare_event(pov_moved, game_controller_pov_ordinal , const vec2&, key_modifiers_e)
        declare_event(stick_moved, const vec3&, key_modifiers_e)
        declare_event(stick_rotated, const vec3&, key_modifiers_e)
        declare_event(slider_moved, const vec2&, key_modifiers_e)
    public:
        struct player_not_assigned : std::logic_error { player_not_assigned() : std::logic_error{ "neogfx::i_game_controller::player_not_assigned" } {} };
        struct unknown_port : std::logic_error { unknown_port() : std::logic_error{ "neogfx::i_game_controller::unknown_port" } {} };
    public:
        virtual bool player_assigned() const = 0;
        virtual game_player player() const = 0;
        virtual void assign_player(game_player aPlayer) = 0;
        virtual void unassign_player() = 0;
    public:
        virtual bool have_port() const = 0;
        virtual game_controller_port port() const = 0;
        virtual void set_port(game_controller_port aPort) = 0;
        virtual void clear_port() = 0;
    public:
        virtual bool needs_calibrating() const = 0;
    public:
        virtual bool is_button_pressed(game_controller_button_ordinal aButtonOrdinal) const = 0;
        virtual bool is_button_pressed(game_controller_button aButton) const = 0;
        virtual double left_trigger_position() const = 0;
        virtual double right_trigger_position() const = 0;
        virtual const vec2& left_thumb_position() const = 0;
        virtual const vec2& right_thumb_position() const = 0;
        virtual const vec3& stick_position() const = 0;
        virtual const vec3& stick_rotation() const = 0;
        virtual const vec2& slider_position() const = 0;
    public:
        virtual uint32_t button_count() const = 0;
        virtual bool button_mapped(game_controller_button aButton) const = 0;
        virtual bool button_mapped(game_controller_button_ordinal aButtonOrdinal) const = 0;
        virtual game_controller_button_ordinal button_to_button_ordinal(game_controller_button aButton) const = 0;
        virtual game_controller_button button_ordinal_to_button(game_controller_button_ordinal aButtonOrdinal) const = 0;
    };
}