// i_accessibility.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/core/event.hpp>

namespace neogfx
{
    class i_accessibility : public i_service
    {
    public:
        declare_event(settings_changed);
    public:
        virtual std::uint32_t text_cursor_width() const = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0x39198e2d, 0x9029, 0x445e, 0x9ad5, { 0xa6, 0xfb, 0xae, 0xd8, 0x5d, 0xfc } }; return sIid; }
    };
}