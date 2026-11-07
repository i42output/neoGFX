// linux_game_controllers.cpp
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

#include "linux_game_controllers.hpp"

namespace neogfx
{
    namespace native::linux
    {
        game_controllers::game_controllers()
        {
        }

        game_controllers::~game_controllers()
        {
        }

        void game_controllers::enumerate_controllers()
        {
        }

        const i_string& game_controllers::product_name(const hid_device_uuid&) const
        {
            static const string sUnknown{ "Unknown Game Controller" };
            return sUnknown;
        }
    }
}
