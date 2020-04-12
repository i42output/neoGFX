// hid_devices.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/hid/hid_devices.hpp>

namespace neogfx
{
    hid_devices::hid_devices()
    {
    }

    hid_devices::~hid_devices()
    {
    }

    const hid_devices::device_list& hid_devices::devices() const
    {
        return iDevices;
    }

    hid_devices::device_list::iterator hid_devices::add_device(const neolib::ref_ptr<i_hid_device>& aDevice)
    {
        auto newDevice = iDevices.insert(iDevices.end(), aDevice);
        DeviceConnected.trigger(*aDevice);
        return newDevice;
    }

    hid_devices::device_list::iterator hid_devices::remove_device(const neolib::ref_ptr<i_hid_device>& aDevice)
    {
        neolib::ref_ptr<i_hid_device> detached{ aDevice };
        auto existing = std::find(iDevices.begin(), iDevices.end(), detached);
        if (existing != iDevices.end())
        {
            auto next = iDevices.erase(existing);
            DeviceDisconnected.trigger(*detached);
            return next;
        }
        return iDevices.end();
    }
}