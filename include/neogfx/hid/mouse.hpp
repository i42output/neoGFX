// mouse.hpp
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

#include <neogfx/hid/hid_device.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include <neogfx/hid/i_mouse.hpp>

namespace neogfx
{
    class mouse;

    class mouse_grabber : public i_mouse_handler
    {
    public:
        mouse_grabber(mouse& aMouse);
    public:
        bool mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers) override;
    private:
        mouse& iMouse;
    };

    class mouse : public hid_device<i_mouse>
    {
    public:
        define_declared_event(ButtonPressed, button_pressed, mouse_button)
        define_declared_event(ButtonReleased, button_released, mouse_button)
    private:
        friend class mouse_grabber;
    public:
        mouse(const i_string& aName = string{ "Generic Mouse" });
    public:
        bool is_mouse_grabbed() const override;
        bool is_mouse_grabbed_by(i_mouse_handler& aMouseHandler) const override;
        bool is_front_grabber(i_mouse_handler& aMouseHandler) const override;
        void grab_mouse(i_mouse_handler& aMouseHandler) override;
        void ungrab_mouse(i_mouse_handler& aMouseHandler) override;
        i_mouse_handler& grabber() const override;
    private:
        mutable mouse_grabber iGrabber;
        std::deque<i_mouse_handler*> iGrabs;
    };
}