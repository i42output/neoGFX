// drop_list_bits.hpp
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
    enum class drop_list_style : std::uint32_t
    {
        Normal              = 0x0000,
        Editable            = 0x0001,
        ListAlwaysVisible   = 0x0002,
        NoFilter            = 0x0004
    };
}

begin_declare_enum(neogfx::drop_list_style)
declare_enum_string(neogfx::drop_list_style, Normal)
declare_enum_string(neogfx::drop_list_style, Editable)
declare_enum_string(neogfx::drop_list_style, ListAlwaysVisible)
declare_enum_string(neogfx::drop_list_style, NoFilter)
end_declare_enum(neogfx::drop_list_style)

namespace neogfx
{
    inline drop_list_style operator|(drop_list_style aLhs, drop_list_style aRhs)
    {
        return static_cast<drop_list_style>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline drop_list_style operator&(drop_list_style aLhs, drop_list_style aRhs)
    {
        return static_cast<drop_list_style>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline drop_list_style operator~(drop_list_style aLhs)
    {
        return static_cast<drop_list_style>(~static_cast<std::uint32_t>(aLhs));
    }
}
