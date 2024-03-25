// hid_devices.cpp
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

#include <neogfx/hid/hid_devices.hpp>
#include <neogfx/hid/i_game_controllers.hpp>

namespace neogfx
{
    hid_devices::hid_devices()
    {
    }

    hid_devices::~hid_devices()
    {
    }

    void hid_devices::enumerate_devices()
    {
        // todo: keyboard and mouse connection/disconnection
        service<i_game_controllers>().enumerate_controllers();
    }

    const hid_devices::device_list& hid_devices::devices() const
    {
        return iDevices;
    }

    void hid_devices::add_device(i_hid_device& aDevice)
    {
        auto newDevice = iDevices.insert(iDevices.end(), ref_ptr<i_hid_device>(aDevice));
        DeviceConnected.trigger(aDevice);
    }

    void hid_devices::remove_device(i_hid_device& aDevice)
    {
        ref_ptr<i_hid_device> detached{ aDevice };
        auto existing = std::find(iDevices.begin(), iDevices.end(), detached);
        if (existing != iDevices.end())
        {
            auto next = iDevices.erase(existing);
            DeviceDisconnected.trigger(*detached);
        }
    }

    const i_string& hid_devices::product_name(hid_device_class aClass, const hid_device_uuid& aProductId) const
    {
        static const string sUnknownProductName = "Generic HID Device";
        switch (aClass)
        {
        case hid_device_class::GameController:
            return service<i_game_controllers>().product_name(aProductId);
        default:
            return sUnknownProductName;
        }
    }
}