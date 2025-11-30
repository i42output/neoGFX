// keyboard.cpp
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

#include <neogfx/neogfx.hpp>

#include <neogfx/hid/keyboard.hpp>

namespace neogfx
{
    keyboard_grabber::keyboard_grabber(keyboard& aKeyboard) : iKeyboard(aKeyboard)
    {
    }

    bool keyboard_grabber::is_filter_processing_event() const
    {
        return iFiltering;
    }

    bool keyboard_grabber::is_processing_event() const
    {
        return iProcessingEvent;
    }

    bool keyboard_grabber::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier)
    {
        {
            neolib::scoped_flag sf{ iFiltering };
            for (auto& f : iKeyboard.iFilters)
                if (f->key_pressed(aScanCode, aKeyCode, aKeyModifier))
                    return true;
        }
        neolib::scoped_flag sf{ iProcessingEvent };
        for (auto& g : iKeyboard.iGrabs)
            if (g->key_pressed(aScanCode, aKeyCode, aKeyModifier))
                return true;
        return false;
    }

    bool keyboard_grabber::key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier)
    {
        {
            neolib::scoped_flag sf{ iFiltering };
            for (auto& f : iKeyboard.iFilters)
                if (f->key_released(aScanCode, aKeyCode, aKeyModifier))
                    return true;
        }
        neolib::scoped_flag sf{ iProcessingEvent };
        for (auto& g : iKeyboard.iGrabs)
            if (g->key_released(aScanCode, aKeyCode, aKeyModifier))
                return true;
        return false;
    }

    bool keyboard_grabber::text_input(i_string const& aText)
    {
        {
            neolib::scoped_flag sf{ iFiltering };
            for (auto& f : iKeyboard.iFilters)
                if (f->text_input(aText))
                    return true;
        }
        neolib::scoped_flag sf{ iProcessingEvent };
        for (auto& g : iKeyboard.iGrabs)
            if (g->text_input(aText))
                return true;
        return false;
    }

    bool keyboard_grabber::sys_text_input(i_string const& aText)
    {
        {
            neolib::scoped_flag sf{ iFiltering };
            for (auto& f : iKeyboard.iFilters)
                if (f->sys_text_input(aText))
                    return true;
        }
        neolib::scoped_flag sf{ iProcessingEvent };
        for (auto& g : iKeyboard.iGrabs)
            if (g->sys_text_input(aText))
                return true;
        return false;
    }

    keyboard::keyboard(const i_string& aName) :
        hid_device<i_keyboard>{ hid_device_type::Input, hid_device_class::Keyboard, hid_device_subclass::Keyboard }, 
        iGrabber { *this }
    {
    }

    key_modifier keyboard::event_modifiers() const
    {
        if (iEventModifiers)
            return *iEventModifiers;
        return modifiers();
    }

    void keyboard::set_event_modifiers(key_modifier aModifiers)
    {
        iEventModifiers = aModifiers;
    }

    void keyboard::clear_event_modifiers()
    {
        iEventModifiers = std::nullopt;
    }

    void keyboard::filter_keyboard(i_keyboard_handler& aKeyboardHandler, bool aFront)
    {
        if (aFront)
            iFilters.push_front(&aKeyboardHandler);
        else
            iFilters.push_back(&aKeyboardHandler);
    }

    void keyboard::unfilter_keyboard(i_keyboard_handler& aKeyboardHandler)
    {
        auto filter = std::find(iFilters.begin(), iFilters.end(), &aKeyboardHandler);
        if (filter != iFilters.end())
            iFilters.erase(filter);
    }

    bool keyboard::is_filter_processing_event() const
    {
        return iGrabber.is_filter_processing_event();
    }

    bool keyboard::is_keyboard_grabbed() const
    {
        return !iGrabs.empty();
    }

    bool keyboard::is_keyboard_grabbed_by(i_keyboard_handler& aKeyboardHandler) const
    {
        return std::find(iGrabs.begin(), iGrabs.end(), &aKeyboardHandler) != iGrabs.end();
    }

    bool keyboard::is_front_grabber(i_keyboard_handler& aKeyboardHandler) const
    {
        return !iGrabs.empty() && &**iGrabs.begin() == &aKeyboardHandler;
    }

    void keyboard::grab_keyboard(i_keyboard_handler& aKeyboardHandler, bool aFront)
    {
        if (is_keyboard_grabbed_by(aKeyboardHandler))
            throw already_grabbed();
        if (aFront)
            iGrabs.push_front(&aKeyboardHandler);
        else
            iGrabs.push_back(&aKeyboardHandler);
    }

    void keyboard::ungrab_keyboard(i_keyboard_handler& aKeyboardHandler)
    {
        auto grab = std::find(iGrabs.begin(), iGrabs.end(), &aKeyboardHandler);
        if (grab != iGrabs.end())
            iGrabs.erase(grab);
    }

    i_keyboard_handler& keyboard::grabber() const
    {
        return iGrabber;
    }

    bool keyboard::is_grabber_processing_event() const
    {
        return iGrabber.is_processing_event();
    }
}