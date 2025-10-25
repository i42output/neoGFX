// mouse.cpp
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

#include <neogfx/hid/mouse.hpp>

namespace neogfx
{
    mouse_grabber::mouse_grabber(mouse& aMouse) : iMouse(aMouse)
    {
    }

    bool mouse_grabber::mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifier aKeyModifier)
    {
        for (auto& g : iMouse.iGrabs)
            if (g->mouse_wheel_scrolled(aWheel, aPosition, aDelta, aKeyModifier))
                return true;
        return false;
    }

    mouse::mouse(const i_string& aName) :
        hid_device<i_mouse>{ hid_device_type::Input, hid_device_class::Mouse, hid_device_subclass::Mouse },
        iGrabber{ *this }
    {
    }

    bool mouse::is_mouse_grabbed() const
    {
        return !iGrabs.empty();
    }

    bool mouse::is_mouse_grabbed_by(i_mouse_handler& aMouseHandler) const
    {
        return std::find(iGrabs.begin(), iGrabs.end(), &aMouseHandler) != iGrabs.end();
    }

    bool mouse::is_front_grabber(i_mouse_handler& aMouseHandler) const
    {
        return !iGrabs.empty() && &**iGrabs.begin() == &aMouseHandler;
    }

    void mouse::grab_mouse(i_mouse_handler& aMouseHandler)
    {
        if (is_mouse_grabbed_by(aMouseHandler))
            throw already_grabbed();
        iGrabs.push_front(&aMouseHandler);
    }

    void mouse::ungrab_mouse(i_mouse_handler& aMouseHandler)
    {
        auto grab = std::find(iGrabs.begin(), iGrabs.end(), &aMouseHandler);
        if (grab != iGrabs.end())
            iGrabs.erase(grab);
    }

    i_mouse_handler& mouse::grabber() const
    {
        return iGrabber;
    }
}