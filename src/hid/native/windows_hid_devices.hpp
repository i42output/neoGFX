// windows_hid_devices.hpp
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
#include <neogfx/hid/hid_devices.hpp>

namespace neogfx
{
    namespace native::windows
    {
        class hid_devices : public neogfx::hid_devices
        {
        public:
            hid_devices();
            ~hid_devices();
        public:
            void enumerate_devices() override;
        public:
            hid_device_class device_class(const hid_device_class_uuid& aClassUuid) const override;
            hid_device_subclass device_subclass(const hid_device_subclass_uuid& aClassUuid) const override;
        private:
            HWND iHidHelperWindow;
            HDEVNOTIFY iHidHelperNotifyHandle;
        };
    }
}