// line.hpp
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

#include <neogfx/gfx/stipple.hpp>
#include <neogfx/core/style.hpp>

namespace neogfx
{
    enum class line_style : std::uint32_t
    {
        None,
        Solid,
        Dash,
        Dot,
        DashDot,
        DashDotDot,
        CustomDash  
    };

    using line_dash = stipple;

    inline line_style to_line_style(border_style aBorderStyle)
    {
        switch (aBorderStyle)
        {
        case border_style::Dotted:
            return line_style::Dot;
        case border_style::Dashed:
            return line_style::Dash;
        case border_style::Solid:
            return line_style::Solid;
        case border_style::Double:
            return line_style::Solid;
        case border_style::Groove:
            return line_style::Solid;
        case border_style::Ridge:
            return line_style::Solid;
        case border_style::Inset:
            return line_style::Solid;
        case border_style::Outset:
            return line_style::Solid;
        case border_style::None:
        case border_style::Hidden:
        default:
            return line_style::None;
        }
    }
}