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
#include <neolib/task/thread.hpp>
#include <neolib/app/i_power.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/scrollable_widget.ipp>
#include <neogfx/gui/widget/terminal.hpp>

namespace neogfx
{
    template class scrollable_widget<framed_widget<widget<i_terminal>>>;

    terminal::terminal() : 
        iTerminalSize{ 80u, 25u },
        iCursorAnimationStartTime{ neolib::thread::program_elapsed_ms() },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } }
    {
        init();
    }

    terminal::terminal(i_widget& aParent) : 
        base_type{ aParent, scrollbar_style::Normal, frame_style::NoFrame },
        iTerminalSize{ 80u, 25u },
        iBufferSize{ iTerminalSize },
        iCursorPos{},
        iCursorAnimationStartTime{ neolib::thread::program_elapsed_ms() },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } }
    {
        init();
    }

    terminal::terminal(i_layout& aLayout) :
        base_type{ aLayout, scrollbar_style::Normal, frame_style::NoFrame },
        iTerminalSize{ 80u, 25u },
        iBufferSize{ iTerminalSize },
        iCursorPos{},
        iCursorAnimationStartTime{ neolib::thread::program_elapsed_ms() },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } }
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

        auto const& cr = client_rect(false);
        scalar y = -vertical_scrollbar().position();
        for (auto const& line : iBuffer)
        {
            if (line.glyphs == std::nullopt)
                line.glyphs = aGc.to_glyph_text(line.text, font());
            if (y + line.glyphs->extents().cy >= cr.top() && y < cr.bottom())
            {
                scalar x = 0.0;
                for (auto const& g : *line.glyphs)
                {
                    aGc.draw_glyph(point{ x, y }, *line.glyphs, g, { line.attributes[g.source.first].ink, line.attributes[g.source.first].paper });
                    x += g.advance.cx;
                }
            }
            y += line.glyphs->extents().cy;
        }

        if (has_focus())
            draw_cursor(aGc);
    }

    color terminal::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole))
            return base_type::palette_color(aColorRole);
        if (aColorRole == color_role::Background)
            return palette_color(color_role::Void);
        return base_type::palette_color(aColorRole);
    }

    void terminal::set_font(optional_font const& aFont)
    {
        base_type::set_font(aFont);
    }

    focus_policy terminal::focus_policy() const
    {
        return neogfx::focus_policy::StrongFocus;
    }

    void terminal::focus_gained(focus_reason aFocusReason)
    {
        base_type::focus_gained(aFocusReason);
        neolib::service<neolib::i_power>().register_activity();
        iCursorAnimationStartTime = neolib::thread::program_elapsed_ms();
        update();
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
        neolib::service<neolib::i_power>().register_activity();

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
        neolib::service<neolib::i_power>().register_activity();

        return base_type::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
    }

    bool terminal::key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        return base_type::key_released(aScanCode, aKeyCode, aKeyModifiers);
    }

    bool terminal::text_input(i_string const& aText)
    {
        Input.trigger(aText);
        output(aText); // todo: remove
        return true;
    }

    void terminal::output(i_string const& aOutput)
    {
        auto utf32 = neolib::utf8_to_utf32(aOutput.to_std_string_view());
        for (auto ch : utf32)
        {
            auto cursorPos = iCursorPos;
            if (iBuffer.size() <= cursorPos.y)
                iBuffer.resize(cursorPos.y + 1u);
            if (ch == U'\n' || ch == U'\r')
            {
                cursorPos.x = 0u;
                ++cursorPos.y;
            }
            else
            {
                if (iBuffer[cursorPos.y].text.size() <= cursorPos.x)
                    iBuffer[cursorPos.y].text.resize(cursorPos.x + 1u);
                iBuffer[cursorPos.y].text[cursorPos.x] = ch;
                if (iBuffer[cursorPos.y].attributes.size() <= cursorPos.x)
                    iBuffer[cursorPos.y].attributes.resize(cursorPos.x + 1u, { color::White, color::Black });
                iBuffer[cursorPos.y].glyphs = std::nullopt;
                ++cursorPos.x;
                if (cursorPos.x == iTerminalSize.cx)
                {
                    cursorPos.x = 0u;
                    ++cursorPos.y;
                }
            }
            if (cursorPos != iCursorPos)
                set_cursor_pos(cursorPos);
        }
        update();
    }

    cursor& terminal::cursor() const
    {
        return iCursor;
    }

    void terminal::init()
    {
        vertical_scrollbar().set_style(vertical_scrollbar().style() | scrollbar_style::AlwaysVisible);
        horizontal_scrollbar().set_style(scrollbar_style::None);
        set_ideal_size(size{ font().max_advance(), font().height() } * size { iTerminalSize } +
            size{ effective_frame_width() } + size{ vertical_scrollbar().width(), horizontal_scrollbar().width() });
        cursor().set_style(cursor_style::Xor);
        cursor().set_width(font().max_advance());

        iSink += neolib::service<neolib::i_power>().green_mode_entered([this]()
            {
                if (has_focus())
                    update(cursor_rect());
            });
        iSink += cursor().PositionChanged([this]()
            {
                iCursorAnimationStartTime = neolib::thread::program_elapsed_ms();
                update();
            });
        iSink += cursor().AnchorChanged([this]()
            {
                update();
            });
        iSink += cursor().AppearanceChanged([this]()
            {
                update();
            });

    }

    void terminal::animate()
    {
        if (neolib::service<neolib::i_power>().green_mode_active())
            return;
        if (has_focus())
            update(cursor_rect());
    }

    void terminal::set_cursor_pos(cursor_pos aCursorPos)
    {
        aCursorPos.x = std::min(aCursorPos.x, iTerminalSize.cx - 1u);
        aCursorPos.y = std::min(aCursorPos.y, iTerminalSize.cy - 1u);
        if (iCursorPos != aCursorPos)
        {
            iCursorPos = aCursorPos;
            cursor().set_position(iTerminalSize.cx * iCursorPos.y + iCursorPos.x);
            update();
        }
    }
    
    rect terminal::cursor_rect() const
    {
        size const cursorSize{ font().max_advance(), font().height() };
        return rect{ point{ iCursorPos } * cursorSize, cursorSize };

    }
    
    void terminal::draw_cursor(i_graphics_context& aGc) const
    {
        auto elapsedTime_ms = (neolib::thread::program_elapsed_ms() - iCursorAnimationStartTime);
        auto const flashInterval_ms = cursor().flash_interval().count();
        auto const normalizedFrameTime = (elapsedTime_ms % flashInterval_ms) / ((flashInterval_ms - 1) * 1.0);
        auto const cursorAlpha = neolib::service<neolib::i_power>().green_mode_active() ? 1.0 : partitioned_ease(easing::InvertedInOutQuint, easing::InOutQuint, normalizedFrameTime);
        auto cursorColor = cursor().color();
        if (cursorColor == neolib::none)
            cursorColor = service<i_app>().current_style().palette().default_text_color_for_widget(*this);
        if (cursor().style() == cursor_style::Xor)
        {
            aGc.push_logical_operation(logical_operation::Xor);
            aGc.fill_rect(cursor_rect(), (cursorAlpha >= 0.5 ? color::White : color::Black));
            aGc.pop_logical_operation();
        }
        else if (std::holds_alternative<color>(cursorColor))
        {
            aGc.fill_rect(cursor_rect(), static_variant_cast<const color&>(cursorColor).with_combined_alpha(cursorAlpha));
        }
        else if (std::holds_alternative<gradient>(cursorColor))
        {
            aGc.fill_rect(cursor_rect(), static_variant_cast<const gradient&>(cursorColor).with_combined_alpha(cursorAlpha));
        }

    }

}