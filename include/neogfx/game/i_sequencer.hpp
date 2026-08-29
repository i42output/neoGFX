// i_sequencer.hpp
/*
  neolib C++ App/Game Engine
  Copyright (c)  2026 Leigh Johnston.  All Rights Reserved.

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

#include <neolib/core/i_jar.hpp>

namespace neogfx
{
    namespace game
    {
        using sequencer_track_id = neolib::cookie;
        using optional_sequencer_track_id = std::optional<sequencer_track_id>;

        class i_sequencer : public i_service
        {
        public:
            virtual sequencer_track_id create_track() = 0;
        public:
            static uuid const& iid() { static uuid const sIid{ 0xa2fe8afe, 0x4483, 0x4b66, 0x8ee4, { 0x70, 0xd2, 0xf, 0x4e, 0xfc, 0xd0 } }; return sIid; }
        };
    }
}