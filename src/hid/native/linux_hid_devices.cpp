// linux_hid_devices.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include "linux_hid_devices.hpp"

namespace neogfx
{
    namespace native::linux
    {
        hid_devices::hid_devices()
        {
        }

        hid_devices::~hid_devices()
        {
        }

        void hid_devices::enumerate_devices()
        {
            neogfx::hid_devices::enumerate_devices();
        }

        hid_device_class hid_devices::device_class(const hid_device_class_uuid&) const
        {
            return hid_device_class::Unknown;
        }

        hid_device_subclass hid_devices::device_subclass(const hid_device_subclass_uuid&) const
        {
            return hid_device_subclass::Unknown;
        }
    }
}
