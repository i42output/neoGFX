// i_text_widget.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.
  
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

#include "i_widget.hpp"

namespace neogfx
{
    enum class text_widget_type
    {
        SingleLine,
        MultiLine
    };

    enum class text_widget_flags
    {
        None                = 0x00,
        HideOnEmpty         = 0x01,
        TakesSpaceWhenEmpty = 0x02,
        CutOff              = 0x04,
        UseEllipsis         = 0x08
    };

    inline constexpr text_widget_flags operator|(text_widget_flags aLhs, text_widget_flags aRhs)
    {
        return static_cast<text_widget_flags>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr text_widget_flags operator&(text_widget_flags aLhs, text_widget_flags aRhs)
    {
        return static_cast<text_widget_flags>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr text_widget_flags operator~(text_widget_flags aLhs)
    {
        return static_cast<text_widget_flags>(~static_cast<std::uint32_t>(aLhs));
    }

    class i_text_widget : public i_widget
    {
    public:
        declare_event(text_changed)
        declare_event(text_geometry_changed)
    public:
        virtual i_string const& text() const = 0;
        virtual void set_text(i_string const& aText) = 0;
        virtual void set_size_hint(const size_hint& aSizeHint) = 0;
        virtual bool multi_line() const = 0;
        virtual text_widget_flags flags() const = 0;
        virtual void set_flags(text_widget_flags aFlags) = 0;
        virtual neogfx::alignment alignment() const = 0;
        virtual void set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout = true) = 0;
        virtual bool has_text_color() const = 0;
        virtual color text_color() const = 0;
        virtual void set_text_color(const optional_color& aTextColor) = 0;
        virtual bool has_text_format() const = 0;
        virtual neogfx::text_format text_format() const = 0;
        virtual void set_text_format(const optional_text_format& aTextAppearance) = 0;
    };
}