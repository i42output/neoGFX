// i_hid_device.hpp
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
#include <neolib/core/reference_counted.hpp>
#include <neogfx/hid/i_hid_device.hpp>
#include <neogfx/hid/i_hid_devices.hpp>

namespace neogfx
{
    template <typename HidInterface>
    class hid_device : public reference_counted<HidInterface>
    {
    public:
        define_declared_event(Enabled, enabled)
        define_declared_event(Disabled, disabled)
    public:
        typedef HidInterface abstract_type;
    public:
        hid_device(hid_device_type aType, hid_device_class aClass, hid_device_subclass aSubclass, const hid_device_uuid& aProductId = {}, const hid_device_uuid& aInstanceId = {}) :
            iType{ aType },
            iClass{ aClass },
            iSubclass{ aSubclass },
            iProductId{ aProductId },
            iInstanceId{ aInstanceId },
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
        hid_device_uuid product_id() const override
        {
            return iProductId;
        }
        hid_device_uuid instance_id() const override
        {
            return iInstanceId;
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
    public:
        const i_string& product_name() const override
        {
            return service<i_hid_devices>().product_name(device_class(), product_id());
        }
    private:
        hid_device_type const iType;
        hid_device_class const iClass;
        hid_device_subclass const iSubclass;
        hid_device_uuid const iProductId;
        hid_device_uuid const iInstanceId;
        bool iEnabled;
    };
}