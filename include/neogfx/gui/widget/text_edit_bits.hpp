// text_edit_bits.hpp
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
    enum class text_edit_caps : std::uint32_t
    {
        None                    = 0x00000000,

        SingleLine              = 0x00000001,
        MultiLine               = 0x00000002,
        GrowLines               = SingleLine | MultiLine,

        OverwriteMode           = 0x00000010,

        Password                = 0x00000100,
        ShowPassword            = 0x00000200,

        NonPrintableWhitespace  = 0x00001000,
        ParseURIs               = 0x00002000,
        TranslateEmoticons      = 0x00004000,

        OnlyAccept              = 0x00010000,

        LINES_MASK              = SingleLine | MultiLine
    };

    enum class text_edit_line_ending : std::uint32_t
    {
        Lf              = 0,
        CrLf            = 1,
        LfCr            = 2,
        AutomaticLf     = 3,
        AutomaticCrLf   = 4,
        AutomaticLfCr   = 5
    };

    inline bool is_automatic(text_edit_line_ending aLineEnding)
    {
        switch (aLineEnding)
        {
        case text_edit_line_ending::Lf:
        case text_edit_line_ending::CrLf:
        case text_edit_line_ending::LfCr:
        default:
            return false;
        case text_edit_line_ending::AutomaticLf:
        case text_edit_line_ending::AutomaticCrLf:
        case text_edit_line_ending::AutomaticLfCr:
            return true;
        }
    }
}

begin_declare_enum(neogfx::text_edit_caps)
declare_enum_string(neogfx::text_edit_caps, SingleLine)
declare_enum_string(neogfx::text_edit_caps, MultiLine)
declare_enum_string(neogfx::text_edit_caps, GrowLines)
declare_enum_string(neogfx::text_edit_caps, OverwriteMode)
declare_enum_string(neogfx::text_edit_caps, Password)
declare_enum_string(neogfx::text_edit_caps, ShowPassword)
declare_enum_string(neogfx::text_edit_caps, ParseURIs)
declare_enum_string(neogfx::text_edit_caps, OnlyAccept)
end_declare_enum(neogfx::text_edit_caps)

namespace neogfx
{
    inline text_edit_caps operator~(text_edit_caps aLhs)
    {
        return static_cast<text_edit_caps>(~static_cast<std::uint32_t>(aLhs));
    }

    inline text_edit_caps operator&(text_edit_caps aLhs, text_edit_caps aRhs)
    {
        return static_cast<text_edit_caps>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline text_edit_caps operator|(text_edit_caps aLhs, text_edit_caps aRhs)
    {
        return static_cast<text_edit_caps>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }
}
