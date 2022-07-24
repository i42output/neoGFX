// terminal.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2022 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/scrollable_widget.hpp>
#include <neogfx/gui/widget/cursor.hpp>
#include <neogfx/gui/widget/i_terminal.hpp>

namespace neogfx
{
    class terminal : public scrollable_widget<framed_widget<widget<i_terminal>>>
    {
    public:
        define_declared_event(Input, input, i_string const&)
    private:
        typedef scrollable_widget<framed_widget<widget<i_terminal>>> base_type;
    private:
        struct attribute
        {
            color ink;
            color paper;
            bool reverse = false;
            bool blink = false;
            font_style style = font_style::Normal;
        };
        struct buffer_line
        {
            std::u32string text;
            mutable optional_glyph_text glyphs;
            std::vector<attribute> attributes;
        };
        typedef basic_size<std::int32_t> size_type;
        typedef basic_point<std::int32_t> point_type;
        typedef point_type::coordinate_type coordinate_type;
    public:
        terminal();
        terminal(i_widget& aParent);
        terminal(i_layout& aLayout);
        ~terminal(); 
    public:
        neogfx::size_policy size_policy() const override;
    public:
        void paint(i_graphics_context& aGc) const override;
        color palette_color(color_role aColorRole) const override;
    public:
        void set_font(optional_font const& aFont) override;
    public:
        neogfx::focus_policy focus_policy() const override;
        void focus_gained(focus_reason aFocusReason) override;
        void focus_lost(focus_reason aFocusReason) override;
    public:
        void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
        void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
        neogfx::mouse_cursor mouse_cursor() const override;
    public:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool text_input(i_string const& aText) override;
    public:
        void output(i_string const& aOutput) override;
        neogfx::cursor& cursor() const;
    private:
        void init();
        using base_type::font;
        neogfx::font const& font(font_style aStyle) const;
        neogfx::font const& normal_font() const;
        neogfx::font const& bold_font() const;
        neogfx::font const& italic_font() const;
        neogfx::font const& bold_italic_font() const;
        size character_extents() const;
        void animate();
        void erase_in_display(point_type const& aBufferPosStart, point_type const& aBufferPosEnd);
        buffer_line& line(coordinate_type aLine);
        char32_t& character(point_type const& aBufferPos);
        point_type buffer_pos() const;
        point_type to_buffer_pos(point_type aCursorPos) const;
        point_type cursor_pos() const;
        void set_cursor_pos(point_type aCursorPos);
        void update_cursor();
        rect cursor_rect() const;
        void make_cursor_visible(bool aToBufferOrigin = true);
        void draw_cursor(i_graphics_context& aGc) const;
    private:
        size_type iTerminalSize;
        size_type iBufferSize;
        mutable optional_font iNormalFont;
        mutable optional_font iBoldFont;
        mutable optional_font iItalicFont;
        mutable optional_font iBoldItalicFont;
        mutable optional_size iCharacterExtents;
        point_type iBufferOrigin;
        std::optional<point_type> iCursorPos;
        std::vector<buffer_line> iBuffer;
        std::optional<attribute> iAttribute;
        std::optional<std::string> iEscapeSequence;
        bool iAutoWrap = true;
        bool iBracketedPaste = false;
        mutable neogfx::cursor iCursor;
        mutable bool iOutputting = false;
        uint64_t iCursorAnimationStartTime;
        widget_timer iAnimator;
        sink iSink;
    };
}