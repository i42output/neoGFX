// i_hid_devices.hpp
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

#include <neolib/core/i_vector.hpp>

#include <neogfx/hid/i_hid_device.hpp>

namespace neogfx
{
    typedef hid_device_uuid hid_device_class_uuid;
    typedef hid_device_uuid hid_device_subclass_uuid;

    class i_hid_devices : public i_service
    {
    public:
        declare_event(device_connected, i_hid_device&)
        declare_event(device_disconnected, i_hid_device&)
    public:
        typedef neolib::i_vector<i_ref_ptr<i_hid_device>> device_list;
    public:
        virtual ~i_hid_devices() = default;
    public:
        virtual void enumerate_devices() = 0;
        virtual const device_list& devices() const = 0;
        virtual void add_device(i_hid_device& aDevice) = 0;
        virtual void remove_device(i_hid_device& aDevice) = 0;
    public:
        virtual hid_device_class device_class(const hid_device_class_uuid& aClassUuid) const = 0;
        virtual hid_device_subclass device_subclass(const hid_device_subclass_uuid& aClassUuid) const = 0;
        virtual const i_string& product_name(hid_device_class aClass, const hid_device_uuid& aProductId) const = 0;
    public:
        template <typename Device>
        ref_ptr<Device> add_device()
        {
            auto newDevice = make_ref<Device>();
            add_device(*newDevice);
            return newDevice;
        }
    public:
        static uuid const& iid() { static uuid const sIid{ 0xc97d8f86, 0xa1db, 0x4c8b, 0x8531, { 0x90, 0xba, 0x5b, 0x34, 0x27, 0x47 } }; return sIid; }
    };
}