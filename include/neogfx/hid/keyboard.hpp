// keyboard.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <deque>

#include <neogfx/hid/hid_device.hpp>
#include <neogfx/hid/i_keyboard.hpp>

namespace neogfx
{
    class keyboard;

    class keyboard_grabber : public i_keyboard_handler
    {
    public:
        keyboard_grabber(keyboard& aKeyboard);
    public:
        bool is_filter_processing_event() const;
        bool is_grabber_processing_event() const;
    public:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) override;
        bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) override;
        bool text_input(i_string const& aText) override;
        bool sys_text_input(i_string const& aText) override;
    private:
        keyboard& iKeyboard;
        bool iFilterProcessingEvent = false;
        bool iGrabberProcessingEvent = false;
    };

    class keyboard : public hid_device<i_keyboard>
    {
    public:
        define_declared_event(KeyPressed, key_pressed, scan_code_e, key_code_e, key_modifier)
        define_declared_event(KeyReleased, key_released, scan_code_e, key_code_e, key_modifier)
        define_declared_event(TextInput, text_input, i_string const&)
        define_declared_event(SysTextInput, sys_text_input, i_string const&)
        define_declared_event(InputLanguageChanged, input_language_changed)
    private:
        friend class keyboard_grabber;
    public:
        keyboard(const i_string& aName = string{ "Generic Keyboard" });
    public:
        key_modifier event_modifiers() const override;
        void set_event_modifiers(key_modifier aModifiers) override;
        void clear_event_modifiers() override;
    public:
        void filter_keyboard(i_keyboard_handler& aKeyboardHandler, bool aFront = true) override;
        void unfilter_keyboard(i_keyboard_handler& aKeyboardHandler) override;
        bool is_filter_processing_event() const override;
        bool is_keyboard_grabbed() const override;
        bool is_keyboard_grabbed_by(i_keyboard_handler& aKeyboardHandler) const override;
        bool is_front_grabber(i_keyboard_handler& aKeyboardHandler) const override;
        void grab_keyboard(i_keyboard_handler& aKeyboardHandler, bool aFront = true) override;
        void ungrab_keyboard(i_keyboard_handler& aKeyboardHandler) override;
        i_keyboard_handler& grabber() const override;
        bool is_grabber_processing_event() const override;
    private:
        std::deque<i_keyboard_handler*> iFilters;
        std::deque<i_keyboard_handler*> iGrabs;
        mutable keyboard_grabber iGrabber;
        std::optional<key_modifier> iEventModifiers;
    };
}