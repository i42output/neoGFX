// slider_bits.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/i_enum.hpp>

namespace neogfx
{
    enum class slider_orientation : std::uint32_t
    {
        Horizontal,
        Vertical
    };
}

begin_declare_enum(neogfx::slider_orientation)
declare_enum_string(neogfx::slider_orientation, Horizontal)
declare_enum_string(neogfx::slider_orientation, Vertical)
end_declare_enum(neogfx::slider_orientation)
