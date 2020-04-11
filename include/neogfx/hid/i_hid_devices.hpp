// i_hid_devices.hpp
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
#include <neolib/i_vector.hpp>
#include <neogfx/hid/i_hid_device.hpp>

namespace neogfx
{
    class i_hid_devices
    {
    public:
        declare_event(device_connected, i_hid_device&)
        declare_event(device_disconnected, i_hid_device&)
    public:
        typedef neolib::i_vector<i_hid_device> device_list;
    public:
        virtual ~i_hid_devices() = default;
    public:
        virtual const device_list& devices() const = 0;
        virtual device_list& devices() = 0;
    public:
        virtual hid_device_class device_class(const hid_device_class_uuid& aClassUuid) const = 0;
        virtual hid_device_subclass device_subclass(const hid_device_class_uuid& aClassUuid) const = 0;
    };
}