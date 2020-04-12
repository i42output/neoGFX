// i_hid_device.hpp
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
#include <neolib/reference_counted.hpp>
#include <neogfx/hid/i_hid_device.hpp>

namespace neogfx
{
    template <typename HidInterface>
    class hid_device : public neolib::reference_counted<HidInterface>
    {
    public:
        define_declared_event(Enabled, enabled)
        define_declared_event(Disabled, disabled)
    public:
        hid_device(hid_device_type aType, hid_device_class aClass, hid_device_subclass aSubclass, const i_string& aName) : 
            iType{ aType },
            iClass{ aClass },
            iSubclass{ aSubclass },
            iName{ aName },
            iEnabled{ true }
        {
        }
    public:
        hid_device_type device_type() const override
        {
            return iType;
        }
        hid_device_class device_class() const override
        {
            return iClass;
        }
        hid_device_subclass device_subclass() const override
        {
            return iSubclass;
        }
        const i_string& device_name() const override
        {
            return iName;
        }
        bool is_enabled() const override
        {
            return iEnabled;
        }
        void enable() override
        {
            if (!iEnabled)
            {
                iEnabled = true;
                Enabled.trigger();
            }
        }
        void disable() override
        {
            if (iEnabled)
            {
                iEnabled = false;
                Disabled.trigger();
            }
        }
    private:
        hid_device_type const iType;
        hid_device_class const iClass;
        hid_device_subclass const iSubclass;
        string const iName;
        bool iEnabled;
    };
}