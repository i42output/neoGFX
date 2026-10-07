// label_bits.hpp
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
    enum class label_placement : std::uint32_t
    {
        Text                        = 0x00000001,
        Image                       = 0x00000002,
        Spacer                      = 0x00000004,
        Horizontal                  = 0x00000010,
        Vertical                    = 0x00000020,
        TextBeforeImage             = Text | Image | 0x00000100,
        ImageBeforeText             = Text | Image | 0x00000200,
        TextHorizontal              = Text | Horizontal,
        TextVertical                = Text | Vertical,
        ImageHorizontal             = Image | Horizontal,
        ImageVertical               = Image | Vertical,
        TextImageHorizontal         = TextBeforeImage | Horizontal,
        TextImageVertical           = TextBeforeImage | Vertical,
        ImageTextHorizontal         = ImageBeforeText | Horizontal,
        ImageTextVertical           = ImageBeforeText | Vertical,
        TextSpacerImageHorizontal   = TextBeforeImage | Spacer | Horizontal,
        TextSpacerImageVertical     = TextBeforeImage | Spacer | Vertical,
        ImageSpacerTextHorizontal   = ImageBeforeText | Spacer | Horizontal,
        ImageSpacerTextVertical     = ImageBeforeText | Spacer | Vertical
    };

    inline constexpr label_placement operator|(label_placement aLhs, label_placement aRhs)
    {
        return static_cast<label_placement>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr label_placement operator&(label_placement aLhs, label_placement aRhs)
    {
        return static_cast<label_placement>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }
}

begin_declare_enum(neogfx::label_placement)
declare_enum_string(neogfx::label_placement, TextHorizontal)
declare_enum_string(neogfx::label_placement, TextVertical)
declare_enum_string(neogfx::label_placement, ImageHorizontal)
declare_enum_string(neogfx::label_placement, ImageVertical)
declare_enum_string(neogfx::label_placement, TextImageHorizontal)
declare_enum_string(neogfx::label_placement, TextImageVertical)
declare_enum_string(neogfx::label_placement, ImageTextHorizontal)
declare_enum_string(neogfx::label_placement, ImageTextVertical)
declare_enum_string(neogfx::label_placement, TextSpacerImageHorizontal)
declare_enum_string(neogfx::label_placement, TextSpacerImageVertical)
declare_enum_string(neogfx::label_placement, ImageSpacerTextHorizontal)
declare_enum_string(neogfx::label_placement, ImageSpacerTextVertical)
end_declare_enum(neogfx::label_placement)
