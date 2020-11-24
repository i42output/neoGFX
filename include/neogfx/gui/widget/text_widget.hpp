// text_widget.hpp
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
#include "widget.hpp"

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
        return static_cast<text_widget_flags>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr text_widget_flags operator&(text_widget_flags aLhs, text_widget_flags aRhs)
    {
        return static_cast<text_widget_flags>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    class text_widget : public widget<>
    {
    public:
        define_event(TextChanged, text_changed)
        define_event(TextGeometryChanged, text_geometry_changed)
    public:
        text_widget(std::string const& aText = std::string{}, text_widget_type aType = text_widget_type::SingleLine, text_widget_flags aFlags = text_widget_flags::None);
        text_widget(i_widget& aParent, std::string const& aText = std::string{}, text_widget_type aType = text_widget_type::SingleLine, text_widget_flags aFlags = text_widget_flags::None);
        text_widget(i_layout& aLayout, std::string const& aText = std::string{}, text_widget_type aType = text_widget_type::SingleLine, text_widget_flags aFlags = text_widget_flags::None);
        ~text_widget();
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
    public:
        void paint(i_graphics_context& aGc) const override;
    public:
        void set_font(optional_font const& aFont) override;
    public:
        bool visible() const override;
    public:
        std::string const& text() const;
        void set_text(std::string const& aText);
        void set_size_hint(const size_hint& aSizeHint);
        bool multi_line() const;
        text_widget_flags flags() const;
        void set_flags(text_widget_flags aFlags);
        neogfx::alignment alignment() const;
        void set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout = true);
        bool has_text_color() const;
        color text_color() const;
        void set_text_color(const optional_color& aTextColor);
        bool has_text_appearance() const;
        neogfx::text_appearance text_appearance() const;
        void set_text_appearance(const optional_text_appearance& aTextAppearance);
    protected:
        size text_extent() const;
        size size_hint_extent() const;
    private:
        void init();
        const neogfx::glyph_text& glyph_text() const;
    private:
        std::string iText;
        mutable neogfx::glyph_text iGlyphText;
        mutable optional_size iTextExtent;
        size_hint iSizeHint;
        mutable optional_size iSizeHintExtent;
        text_widget_type iType;
        text_widget_flags iFlags;
        neogfx::alignment iAlignment;
        optional_text_appearance iTextAppearance;
        sink iSink;
    };
}