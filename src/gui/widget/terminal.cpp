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
        iTerminalSize{ 80, 25 },
        iBufferSize{ 80, 250 },
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
        iTerminalSize{ 80, 25 },
        iBufferSize{ 80, 250 },
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
        iTerminalSize{ 80, 25 },
        iBufferSize{ 80, 250 },
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

        bool handled = true;
        switch (aScanCode)
        {
        case ScanCode_LEFT:
            set_cursor_pos(cursor_pos().with_x(cursor_pos().x - 1));
            break;
        case ScanCode_RIGHT:
            set_cursor_pos(cursor_pos().with_x(cursor_pos().x + 1));
            break;
        case ScanCode_UP:
            set_cursor_pos(cursor_pos().with_y(cursor_pos().y - 1));
            break;
        case ScanCode_DOWN:
            set_cursor_pos(cursor_pos().with_y(cursor_pos().y + 1));
            break;
        default:
            handled = base_type::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
            break;
        }
        return handled;
    }

    bool terminal::key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        return base_type::key_released(aScanCode, aKeyCode, aKeyModifiers);
    }

    bool terminal::text_input(i_string const& aText)
    {
        Input.trigger(aText);
        if (aText == "\r"_s)
            Input.trigger("\n"_s);
        return true;
    }

    namespace
    {
        color attribute_color(std::int32_t aCode)
        {
            switch (aCode)
            {
            case 30:
            case 40:
                return color::Black;
            case 31:
            case 41:
                return color::Red;
            case 32:
            case 42:
                return color::Green;
            case 33:
            case 43:
                return color::Yellow;
            case 34:
            case 44:
                return color::Blue;
            case 35:
            case 45:
                return color::Magenta;
            case 36:
            case 46:
                return color::Cyan;
            case 37:
            case 47:
            default:
                return color::White;
            }
        };
    }

    void terminal::output(i_string const& aOutput)
    {
        neolib::scoped_flag sf{ iOutputting };
        auto utf32 = neolib::utf8_to_utf32(aOutput.to_std_string_view());
        for (auto ch : utf32)
        {
            auto cursorPos = cursor_pos();
            if (iEscapeSequence)
            {
                *iEscapeSequence += static_cast<char>(ch);
                switch(iEscapeSequence.value()[0])
                {
                case '[':
                    // todo
                    if (iEscapeSequence.value().size() > 1 && ch >= U'\x40' && ch <= U'\x7E')
                    {
                        auto params = neolib::tokens(iEscapeSequence.value().substr(1, iEscapeSequence.value().size() - 2), ";"s, 0, false);
                        switch (ch)
                        {
                        case U'm':
                            {
                                for (auto const& param : params)
                                {
                                    auto code = 0;
                                    try { code = std::stoi(param); } catch (...) {}
                                    if (code == 0)
                                        iAttribute = std::nullopt;
                                    else if ((code >= 30 && code <= 37) || (code >= 40 && code <= 47))
                                    {
                                        if (!iAttribute)
                                            iAttribute.emplace(color::White, color::Black);
                                        if (code <= 37)
                                            iAttribute.value().ink = attribute_color(code);
                                        else
                                            iAttribute.value().paper = attribute_color(code);
                                    }
                                }
                            }
                            break;
                        }
                        iEscapeSequence = std::nullopt;
                    }
                    break;
                default:
                    // todo
                    iEscapeSequence = std::nullopt;
                    break;
                }
                continue;
            }
            else if (ch == U'\r')
                cursorPos.x = 0;
            else if (ch == U'\n')
            {
                if (cursorPos.x != 0 || cursorPos.y == 0 || !iBuffer[cursorPos.y - 1].eol)
                    ++cursorPos.y;
                else
                    iBuffer[cursorPos.y - 1].eol = false;
            }
            else if (ch == U'\0')
                continue;
            else if (ch == U'\x1B')
                iEscapeSequence.emplace();
            else
            {
                if (iBuffer[cursorPos.y].text.size() <= cursorPos.x)
                    iBuffer[cursorPos.y].text.resize(cursorPos.x + 1);
                iBuffer[cursorPos.y].text[cursorPos.x] = ch;
                if (iBuffer[cursorPos.y].attributes.size() <= cursorPos.x)
                    iBuffer[cursorPos.y].attributes.resize(cursorPos.x + 1, iAttribute ? iAttribute.value() : attribute{ color::White, color::Black });
                iBuffer[cursorPos.y].glyphs = std::nullopt;
                ++cursorPos.x;
                if (cursorPos.x == iTerminalSize.cx)
                {
                    iBuffer[cursorPos.y].eol = true;
                    cursorPos.x = 0;
                    ++cursorPos.y;
                }
            }
            if (cursorPos != cursor_pos())
                set_cursor_pos(cursorPos, true);
        }
        update_cursor();
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

        set_cursor_pos({}, true);
    }

    void terminal::animate()
    {
        if (neolib::service<neolib::i_power>().green_mode_active())
            return;
        if (has_focus())
            update(cursor_rect());
    }

    terminal::point_type terminal::cursor_pos() const
    {
        return iCursorPos.value();
    }

    void terminal::set_cursor_pos(point_type aCursorPos, bool aExtendBuffer)
    {
        if (aCursorPos.y >= iBuffer.size() && aCursorPos.y <= iBufferSize.cy && aExtendBuffer)
        {
            iBuffer.resize(aCursorPos.y + 1);
            iBuffer.back().text.reserve(iBufferSize.cx);
            iBuffer.back().attributes.reserve(iBufferSize.cx);
        }
        if (iBuffer.size() > iBufferSize.cy)
            iBuffer.erase(iBuffer.begin(), std::next(iBuffer.begin(), iBuffer.size() - iBufferSize.cy));
        aCursorPos.y = std::max(0, std::min(aCursorPos.y, static_cast<size_type::coordinate_type>(iBuffer.size() - 1)));
        aCursorPos.x = std::max(0, std::min(aCursorPos.x, static_cast<size_type::coordinate_type>(iBuffer[aCursorPos.y].text.size())));
        if (iCursorPos != aCursorPos)
        {
            iCursorPos = aCursorPos;
            if (!iOutputting)
            {
                update_cursor();
            }
        }
    }

    void terminal::update_cursor()
    {
        cursor().set_position(iTerminalSize.cx * iCursorPos->y + iCursorPos->x);
        vertical_scrollbar().set_maximum(iBuffer.size() * font().height());
        make_cursor_visible();
        update();
    }
    
    rect terminal::cursor_rect() const
    {
        size const cursorSize{ font().max_advance(), font().height() };
        return rect{ point{ cursor_pos() } * cursorSize - point{ 0.0, vertical_scrollbar().position() }, cursorSize};

    }

    void terminal::make_cursor_visible()
    {
        auto const& cr = cursor_rect();
        if (cr.bottom() <= 0.0)
            vertical_scrollbar().set_position(vertical_scrollbar().position() + cr.top());
        else if (cr.top() >= vertical_scrollbar().page())
            vertical_scrollbar().set_position(vertical_scrollbar().position() + cr.bottom() - vertical_scrollbar().page());
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