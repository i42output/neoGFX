// terminal.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/scrollable_widget.ipp>
#include <neogfx/gui/widget/terminal.hpp>

namespace neogfx
{
    template class scrollable_widget<framed_widget<widget<i_terminal>>>;

    terminal::terminal() : 
        iTerminalSize{ 80u, 25u }
    {
        init();
    }

    terminal::terminal(i_widget& aParent) : 
        base_type{ aParent, scrollbar_style::Normal, frame_style::NoFrame },
        iTerminalSize{ 80u, 25u }
    {
        init();
    }

    terminal::terminal(i_layout& aLayout) :
        base_type{ aLayout, scrollbar_style::Normal, frame_style::NoFrame },
        iTerminalSize{ 80u, 25u }
    {
        init();
    }

    terminal::~terminal()
    {
    }

    size_policy terminal::size_policy() const
    {
        return base_type::size_policy();
    }

    void terminal::paint(i_graphics_context& aGc) const
    {
        base_type::paint(aGc);

        aGc.draw_text(point{}, "12345678901234567890123456789012345678901234567890123456789012345678901234567890"_s, font(), color::White);
    }

    color terminal::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole))
            return base_type::palette_color(aColorRole);
        if (aColorRole == color_role::Background)
            return palette_color(color_role::Base);
        return base_type::palette_color(aColorRole);
    }

    void terminal::set_font(optional_font const& aFont)
    {
        base_type::set_font(aFont);
    }

    void terminal::focus_gained(focus_reason aFocusReason)
    {
        base_type::focus_gained(aFocusReason);
    }

    void terminal::focus_lost(focus_reason aFocusReason)
    {
        base_type::focus_lost(aFocusReason);
    }

    void terminal::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
    }

    void terminal::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
    }

    void terminal::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        base_type::mouse_button_released(aButton, aPosition);
    }

    void terminal::mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_moved(aPosition, aKeyModifiers);
    }

    void terminal::mouse_entered(const point& aPosition)
    {
        base_type::mouse_entered(aPosition);
    }

    void terminal::mouse_left()
    {
        base_type::mouse_left();
    }

    mouse_cursor terminal::mouse_cursor() const
    {
        return base_type::mouse_cursor();
    }

    bool terminal::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        return base_type::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
    }

    bool terminal::key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        return base_type::key_released(aScanCode, aKeyCode, aKeyModifiers);
    }

    bool terminal::text_input(i_string const& aText)
    {
        return base_type::text_input(aText);
    }

    void terminal::init()
    {
        vertical_scrollbar().set_style(vertical_scrollbar().style() | scrollbar_style::AlwaysVisible);
        horizontal_scrollbar().set_style(scrollbar_style::None);
        set_ideal_size(size{ font().max_advance(), font().height() } * size { iTerminalSize } +
            size{ effective_frame_width() } + size{ vertical_scrollbar().width(), horizontal_scrollbar().width() });
    }
}