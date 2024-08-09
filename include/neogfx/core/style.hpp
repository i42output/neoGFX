// style.hpp
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

namespace neogfx
{
    enum class border_style : std::uint32_t
    {
        Dotted,
        Dashed,
        Solid,
        Double,
        Groove,
        Ridge,
        Inset,
        Outset,
        None,
        Hidden
    };
}

begin_declare_enum(neogfx::border_style)
declare_enum_string_explicit(neogfx::border_style, Dotted, dotted)
declare_enum_string_explicit(neogfx::border_style, Dashed, dashed)
declare_enum_string_explicit(neogfx::border_style, Solid,  solid)
declare_enum_string_explicit(neogfx::border_style, Double, double)
declare_enum_string_explicit(neogfx::border_style, Groove, groove)
declare_enum_string_explicit(neogfx::border_style, Ridge, ridge)
declare_enum_string_explicit(neogfx::border_style, Inset, inset)
declare_enum_string_explicit(neogfx::border_style, Outset, outset)
declare_enum_string_explicit(neogfx::border_style, None, none)
declare_enum_string_explicit(neogfx::border_style, Hidden, hidden)
end_declare_enum(neogfx::border_style)
