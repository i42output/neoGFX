// font_widget.hpp
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
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gui/widget/framed_widget.hpp>
#include <neogfx/gui/widget/drop_list.hpp>
#include <neogfx/gui/widget/button.hpp>

namespace neogfx
{
    enum class font_widget_style : std::uint32_t
    {
        Dialog      = 0x00000001,
        DropList    = 0x00000002
    };

    inline constexpr font_widget_style operator|(font_widget_style lhs, font_widget_style rhs)
    {
        return static_cast<font_widget_style>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
    }

    inline constexpr font_widget_style operator&(font_widget_style lhs, font_widget_style rhs)
    {
        return static_cast<font_widget_style>(static_cast<std::uint32_t>(lhs) & static_cast<std::uint32_t>(rhs));
    }

    inline constexpr font_widget_style operator~(font_widget_style lhs)
    {
        return static_cast<font_widget_style>(~static_cast<std::uint32_t>(lhs));
    }

    class font_widget : public framed_widget<>
    {
    public:
        define_event(SelectionChanged, selection_changed)
    private:
        class font_picker : public button<>
        {
        public:
            font_picker(font_widget& aParent);
        private:
            size minimum_size(optional_size const& aAvailableSpace) const override;
        private:
            void paint(i_graphics_context& aGc) const override;
        private:
            glyph_text const& sample_text() const;
        private:
            font_widget& iParent;
            mutable std::optional<glyph_text> iSampleText;
        };
    public:
        font_widget(const neogfx::font& aCurrentFont = neogfx::font{}, font_widget_style aStyle = font_widget_style::Dialog);
        font_widget(const neogfx::font& aCurrentFont, const text_format& aCurrentTextFormat, font_widget_style aStyle = font_widget_style::Dialog);
        font_widget(i_widget& aParent, const neogfx::font& aCurrentFont = neogfx::font{}, font_widget_style aStyle = font_widget_style::Dialog);
        font_widget(i_widget& aParent, const neogfx::font& aCurrentFont, const text_format& aCurrentTextFormat, font_widget_style aStyle = font_widget_style::Dialog);
        font_widget(i_layout& aLayout, const neogfx::font& aCurrentFont = neogfx::font{}, font_widget_style aStyle = font_widget_style::Dialog);
        font_widget(i_layout& aLayout, const neogfx::font& aCurrentFont, const text_format& aCurrentTextFormat, font_widget_style aStyle = font_widget_style::Dialog);
        ~font_widget();
    public:
        neogfx::font current_font() const;
        neogfx::font selected_font() const;
        bool has_format() const;
        neogfx::text_format current_format() const;
        neogfx::text_format selected_format() const;
        void select_font(const neogfx::font& aFont);
        void select_format(const text_format& aTextFormat);
    private:
        void init();
        void update_selected_font(const i_widget& aUpdatingWidget);
        void update_selected_format(const i_widget& aUpdatingWidget);
    private:
        sink iSink;
        bool iUpdating;
        font_widget_style iStyle;
        neogfx::font iCurrentFont;
        neogfx::font iSelectedFont;
        std::optional<text_format> iCurrentTextFormat;
        std::optional<text_format> iSelectedTextFormat;
        horizontal_layout iLayout0;
        std::optional<drop_list> iFamilyPicker;
        std::optional<drop_list> iStylePicker;
        std::optional<drop_list> iSizePicker;
        std::optional<font_picker> iFontPicker;
    };
}