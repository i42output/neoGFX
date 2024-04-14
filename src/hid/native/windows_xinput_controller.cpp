// windows_xinput_controller.cpp
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

#include <boost/bimap.hpp>

#include "windows_xinput_controller.hpp"

namespace neogfx
{
    namespace native::windows
    {
        template <typename L, typename R>
        inline boost::bimap<L, R> make_bimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list)
        {
            return boost::bimap<L, R>(list.begin(), list.end());
        }

        xinput_controller::xinput_controller(IDirectInputDevice8* aDevice,  hid_device_subclass aSubclass, const hid_device_uuid& aProductId, const hid_device_uuid& aInstanceId) :
            game_controller{ aSubclass, aProductId, aInstanceId, xinput_button_map() }, iDevice{ aDevice }
        {
        }

        xinput_controller::~xinput_controller()
        {
            iDevice->Release();
        }

        void xinput_controller::update_state()
        {
            if (!have_port())
                return;
            static const std::unordered_map<int, game_controller_button> keymap =
            {
                { VK_PAD_A, game_controller_button::A },
                { VK_PAD_B, game_controller_button::B },
                { VK_PAD_X, game_controller_button::X },
                { VK_PAD_Y, game_controller_button::Y },
                { VK_PAD_RSHOULDER, game_controller_button::RightShoulder },
                { VK_PAD_LSHOULDER, game_controller_button::LeftShoulder },
                { VK_PAD_LTRIGGER, game_controller_button::LeftTrigger },
                { VK_PAD_RTRIGGER, game_controller_button::RightTrigger },
                { VK_PAD_DPAD_UP, game_controller_button::DirectionalPadUp },
                { VK_PAD_DPAD_DOWN, game_controller_button::DirectionalPadDown },
                { VK_PAD_DPAD_LEFT, game_controller_button::DirectionalPadLeft },
                { VK_PAD_DPAD_RIGHT, game_controller_button::DirectionalPadRight },
                { VK_PAD_START, game_controller_button::Start },
                { VK_PAD_BACK, game_controller_button::Back },
                { VK_PAD_LTHUMB_PRESS, game_controller_button::LeftThumb },
                { VK_PAD_RTHUMB_PRESS, game_controller_button::RightThumb },
                { VK_PAD_LTHUMB_UP, game_controller_button::LeftThumbUp },
                { VK_PAD_LTHUMB_DOWN, game_controller_button::LeftThumbDown },
                { VK_PAD_LTHUMB_RIGHT, game_controller_button::LeftThumbRight },
                { VK_PAD_LTHUMB_LEFT, game_controller_button::LeftThumbLeft },
                { VK_PAD_LTHUMB_UPLEFT, game_controller_button::LeftThumbUpLeft },
                { VK_PAD_LTHUMB_UPRIGHT, game_controller_button::LeftThumbUpRight },
                { VK_PAD_LTHUMB_DOWNRIGHT, game_controller_button::LeftThumbDownRight },
                { VK_PAD_LTHUMB_DOWNLEFT, game_controller_button::LeftThumbDownLeft },
                { VK_PAD_RTHUMB_UP, game_controller_button::RightThumbUp },
                { VK_PAD_RTHUMB_DOWN, game_controller_button::RightThumbDown },
                { VK_PAD_RTHUMB_RIGHT, game_controller_button::RightThumbRight },
                { VK_PAD_RTHUMB_LEFT, game_controller_button::RightThumbLeft },
                { VK_PAD_RTHUMB_UPLEFT, game_controller_button::RightThumbUpLeft },
                { VK_PAD_RTHUMB_UPRIGHT, game_controller_button::RightThumbUpRight },
                { VK_PAD_RTHUMB_DOWNRIGHT, game_controller_button::RightThumbDownRight },
                { VK_PAD_RTHUMB_DOWNLEFT, game_controller_button::RightThumbDownLeft }
            };
            XINPUT_KEYSTROKE key;
            if (::XInputGetKeystroke(port(), 0, &key) == ERROR_SUCCESS)
            {
                auto existing = keymap.find(key.VirtualKey);
                if (existing != keymap.end())
                {
                    set_button_state(existing->second, key.Flags & XINPUT_KEYSTROKE_KEYDOWN);
                    if (key.Flags & XINPUT_KEYSTROKE_REPEAT)
                        ButtonRepeat.trigger(existing->second, service<i_keyboard>().modifiers());
                }
            }
            XINPUT_STATE state = {};
            if (::XInputGetState(port(), &state) == ERROR_SUCCESS)
            {
                set_button_state(game_controller_button::DirectionalPadUp, state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
                set_button_state(game_controller_button::DirectionalPadDown, state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                set_button_state(game_controller_button::DirectionalPadLeft, state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                set_button_state(game_controller_button::DirectionalPadRight, state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                set_button_state(game_controller_button::Start, state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
                set_button_state(game_controller_button::Back, state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
                set_button_state(game_controller_button::LeftThumb, state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
                set_button_state(game_controller_button::RightThumb, state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
                set_button_state(game_controller_button::LeftShoulder, state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                set_button_state(game_controller_button::RightShoulder, state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                set_button_state(game_controller_button::A, state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
                set_button_state(game_controller_button::B, state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
                set_button_state(game_controller_button::X, state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
                set_button_state(game_controller_button::Y, state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
                set_left_trigger_position(state.Gamepad.bLeftTrigger / 255.0);
                set_right_trigger_position(state.Gamepad.bRightTrigger / 255.0);
                set_left_thumb_position(
                    vec2{
                        std::abs(state.Gamepad.sThumbLX) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? state.Gamepad.sThumbLX / 32767.0 : 0.0,
                        std::abs(state.Gamepad.sThumbLY) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? state.Gamepad.sThumbLY / 32767.0 : 0.0 });
                set_right_thumb_position(
                    vec2{
                        std::abs(state.Gamepad.sThumbRX) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? state.Gamepad.sThumbRX / 32767.0 : 0.0,
                        std::abs(state.Gamepad.sThumbRY) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? state.Gamepad.sThumbRY / 32767.0 : 0.0 });
                set_stick_position(left_thumb_position());
                set_stick_rotation(right_thumb_position());
            }
        }

        const xinput_controller::button_map_type& xinput_controller::xinput_button_map()
        {
            static auto const sXinputButtonMap = make_bimap<game_controller_button_ordinal, game_controller_button>(
                {
                    { 1u, game_controller_button::A },
                    { 2u, game_controller_button::B },
                    { 3u, game_controller_button::X },
                    { 4u, game_controller_button::Y },
                    { 5u, game_controller_button::LeftShoulder },
                    { 6u, game_controller_button::RightShoulder },
                    { 7u, game_controller_button::Back },
                    { 8u, game_controller_button::Start },
                    { 9u, game_controller_button::LeftThumb },
                    { 10u, game_controller_button::RightThumb },
                    // todo: what should the mapping of the rest of these actually be? see what DirectInput says.
                    { 11u, game_controller_button::LeftTrigger },
                    { 12u, game_controller_button::RightTrigger },
                    { 13u, game_controller_button::DirectionalPadUp },
                    { 14u, game_controller_button::DirectionalPadDown },
                    { 15u, game_controller_button::DirectionalPadLeft },
                    { 16u, game_controller_button::DirectionalPadRight },
                    { 17u, game_controller_button::LeftThumbUp },
                    { 18u, game_controller_button::LeftThumbDown },
                    { 19u, game_controller_button::LeftThumbRight },
                    { 20u, game_controller_button::LeftThumbLeft },
                    { 21u, game_controller_button::LeftThumbUpLeft },
                    { 22u, game_controller_button::LeftThumbUpRight },
                    { 23u, game_controller_button::LeftThumbDownRight },
                    { 24u, game_controller_button::LeftThumbDownLeft },
                    { 25u, game_controller_button::RightThumbUp },
                    { 26u, game_controller_button::RightThumbDown },
                    { 27u, game_controller_button::RightThumbRight },
                    { 28u, game_controller_button::RightThumbLeft },
                    { 29u, game_controller_button::RightThumbUpLeft },
                    { 30u, game_controller_button::RightThumbUpRight },
                    { 31u, game_controller_button::RightThumbDownRight },
                    { 32u, game_controller_button::RightThumbDownLeft }
                });
            return sXinputButtonMap;
        }
    }
}