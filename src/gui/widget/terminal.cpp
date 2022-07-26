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
#include <neogfx/app/i_basic_services.hpp>
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
        auto const& ce = character_extents();

        scalar y = -vertical_scrollbar().position();

        for (auto const& line : iBuffer)
        {
            if (line.glyphs == std::nullopt)
                line.glyphs = aGc.to_glyph_text(line.text, 
                    [&](std::size_t n) -> neogfx::font 
                    { 
                        return n < line.attributes.size() ? font(line.attributes[n].style) : normal_font();
                    });
            if (y + ce.cy >= cr.top() && y < cr.bottom())
            {
                scalar x = 0.0;
                for (auto const& g : *line.glyphs)
                {
                    auto ink = line.attributes[g.source.first].ink;
                    auto paper = line.attributes[g.source.first].paper;
                    if (line.attributes[g.source.first].reverse)
                        std::swap(ink, paper);
                    aGc.draw_glyph(point{ x, y }, *line.glyphs, g, { ink, paper });
                    x += ce.cx;
                }
            }
            y += ce.cy;
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
        iNormalFont = std::nullopt;
        iBoldFont = std::nullopt;
        iItalicFont = std::nullopt;
        iBoldItalicFont = std::nullopt;
        iCharacterExtents = std::nullopt;
        set_ideal_size(character_extents() * size { iTerminalSize } +
            size{ effective_frame_width() } + size{ vertical_scrollbar().width(), horizontal_scrollbar().width() });
        cursor().set_width(character_extents().cx);
    }

    focus_policy terminal::focus_policy() const
    {
        return neogfx::focus_policy::StrongFocus | focus_policy::ConsumeTabKey;
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

        bool handled = false;

        if (aScanCode >= ScanCode_A && aScanCode <= ScanCode_Z && (aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE)
        {
            Input.trigger(string{ 1, static_cast<char>(aScanCode - ScanCode_A) + '\x01' });
            handled = true;
        }

        switch (aScanCode)
        {
        case ScanCode_LEFT:
            Input.trigger("\x1B[D"_s);
            handled = true;
            break;
        case ScanCode_RIGHT:
            Input.trigger("\x1B[C"_s);
            handled = true;
            break;
        case ScanCode_UP:
            Input.trigger("\x1B[A"_s);
            handled = true;
            break;
        case ScanCode_DOWN:
            Input.trigger("\x1B[B"_s);
            handled = true;
            break;
        case ScanCode_BACKSPACE:
            Input.trigger("\x7F"_s);
            handled = true;
            break;
        default:
            if (!handled)
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
                return color{ 170, 0, 0 };
            case 32:
            case 42:
                return color{ 0, 170, 0 };
            case 33:
            case 43:
                return color{ 170, 85, 0 };
            case 34:
            case 44:
                return color{ 0, 0, 170 };
            case 35:
            case 45:
                return color{ 170, 0, 170 };
            case 36:
            case 46:
                return color{ 0, 170, 170 };
            case 37:
            case 47:
                return color{ 170, 170, 170 };
            case 90:
            case 100:
                return color{ 85, 85, 85 };
            case 91:
            case 101:
                return color{ 255, 85, 85 };
            case 92:
            case 102:
                return color{ 85, 255, 85 };
            case 93:
            case 103:
                return color{ 255, 255, 85 };
            case 94:
            case 104:
                return color{ 85, 85, 255 };
            case 95:
            case 105:
                return color{ 255, 85, 255 };
            case 96:
            case 106:
                return color{ 85, 255, 255 };
            case 97:
            case 107:
                return color{ 255, 255, 255 };
            default:
                return color{ 255, 255, 255 };
            }
        };

        color attribute_color_8bit(std::int32_t code)
        {
            static color const table[256]
            {
                "#000000"s,"#800000"s,"#008000"s,"#808000"s,"#000080"s,"#800080"s,"#008080"s,"#c0c0c0"s,
                "#808080"s,"#ff0000"s,"#00ff00"s,"#ffff00"s,"#0000ff"s,"#ff00ff"s,"#00ffff"s,"#ffffff"s,
                "#000000"s,"#00005f"s,"#000087"s,"#0000af"s,"#0000d7"s,"#0000ff"s,"#005f00"s,"#005f5f"s,
                "#005f87"s,"#005faf"s,"#005fd7"s,"#005fff"s,"#008700"s,"#00875f"s,"#008787"s,"#0087af"s,
                "#0087d7"s,"#0087ff"s,"#00af00"s,"#00af5f"s,"#00af87"s,"#00afaf"s,"#00afd7"s,"#00afff"s,
                "#00d700"s,"#00d75f"s,"#00d787"s,"#00d7af"s,"#00d7d7"s,"#00d7ff"s,"#00ff00"s,"#00ff5f"s,
                "#00ff87"s,"#00ffaf"s,"#00ffd7"s,"#00ffff"s,"#5f0000"s,"#5f005f"s,"#5f0087"s,"#5f00af"s,
                "#5f00d7"s,"#5f00ff"s,"#5f5f00"s,"#5f5f5f"s,"#5f5f87"s,"#5f5faf"s,"#5f5fd7"s,"#5f5fff"s,
                "#5f8700"s,"#5f875f"s,"#5f8787"s,"#5f87af"s,"#5f87d7"s,"#5f87ff"s,"#5faf00"s,"#5faf5f"s,
                "#5faf87"s,"#5fafaf"s,"#5fafd7"s,"#5fafff"s,"#5fd700"s,"#5fd75f"s,"#5fd787"s,"#5fd7af"s,
                "#5fd7d7"s,"#5fd7ff"s,"#5fff00"s,"#5fff5f"s,"#5fff87"s,"#5fffaf"s,"#5fffd7"s,"#5fffff"s,
                "#870000"s,"#87005f"s,"#870087"s,"#8700af"s,"#8700d7"s,"#8700ff"s,"#875f00"s,"#875f5f"s,
                "#875f87"s,"#875faf"s,"#875fd7"s,"#875fff"s,"#878700"s,"#87875f"s,"#878787"s,"#8787af"s,
                "#8787d7"s,"#8787ff"s,"#87af00"s,"#87af5f"s,"#87af87"s,"#87afaf"s,"#87afd7"s,"#87afff"s,
                "#87d700"s,"#87d75f"s,"#87d787"s,"#87d7af"s,"#87d7d7"s,"#87d7ff"s,"#87ff00"s,"#87ff5f"s,
                "#87ff87"s,"#87ffaf"s,"#87ffd7"s,"#87ffff"s,"#af0000"s,"#af005f"s,"#af0087"s,"#af00af"s,
                "#af00d7"s,"#af00ff"s,"#af5f00"s,"#af5f5f"s,"#af5f87"s,"#af5faf"s,"#af5fd7"s,"#af5fff"s,
                "#af8700"s,"#af875f"s,"#af8787"s,"#af87af"s,"#af87d7"s,"#af87ff"s,"#afaf00"s,"#afaf5f"s,
                "#afaf87"s,"#afafaf"s,"#afafd7"s,"#afafff"s,"#afd700"s,"#afd75f"s,"#afd787"s,"#afd7af"s,
                "#afd7d7"s,"#afd7ff"s,"#afff00"s,"#afff5f"s,"#afff87"s,"#afffaf"s,"#afffd7"s,"#afffff"s,
                "#d70000"s,"#d7005f"s,"#d70087"s,"#d700af"s,"#d700d7"s,"#d700ff"s,"#d75f00"s,"#d75f5f"s,
                "#d75f87"s,"#d75faf"s,"#d75fd7"s,"#d75fff"s,"#d78700"s,"#d7875f"s,"#d78787"s,"#d787af"s,
                "#d787d7"s,"#d787ff"s,"#d7af00"s,"#d7af5f"s,"#d7af87"s,"#d7afaf"s,"#d7afd7"s,"#d7afff"s,
                "#d7d700"s,"#d7d75f"s,"#d7d787"s,"#d7d7af"s,"#d7d7d7"s,"#d7d7ff"s,"#d7ff00"s,"#d7ff5f"s,
                "#d7ff87"s,"#d7ffaf"s,"#d7ffd7"s,"#d7ffff"s,"#ff0000"s,"#ff005f"s,"#ff0087"s,"#ff00af"s,
                "#ff00d7"s,"#ff00ff"s,"#ff5f00"s,"#ff5f5f"s,"#ff5f87"s,"#ff5faf"s,"#ff5fd7"s,"#ff5fff"s,
                "#ff8700"s,"#ff875f"s,"#ff8787"s,"#ff87af"s,"#ff87d7"s,"#ff87ff"s,"#ffaf00"s,"#ffaf5f"s,
                "#ffaf87"s,"#ffafaf"s,"#ffafd7"s,"#ffafff"s,"#ffd700"s,"#ffd75f"s,"#ffd787"s,"#ffd7af"s,
                "#ffd7d7"s,"#ffd7ff"s,"#ffff00"s,"#ffff5f"s,"#ffff87"s,"#ffffaf"s,"#ffffd7"s,"#ffffff"s,
                "#080808"s,"#121212"s,"#1c1c1c"s,"#262626"s,"#303030"s,"#3a3a3a"s,"#444444"s,"#4e4e4e"s,
                "#585858"s,"#626262"s,"#6c6c6c"s,"#767676"s,"#808080"s,"#8a8a8a"s,"#949494"s,"#9e9e9e"s,
                "#a8a8a8"s,"#b2b2b2"s,"#bcbcbc"s,"#c6c6c6"s,"#d0d0d0"s,"#dadada"s,"#e4e4e4"s,"#eeeeee"s
            };
            return table[code % 256];
        }

        color attribute_color_24bit(std::int32_t r, std::int32_t g, std::int32_t b)
        {
            return color{ r, g, b };
        }
    }

    void terminal::output(i_string const& aOutput)
    {
        neolib::scoped_flag sf{ iOutputting };
        auto utf32 = neolib::utf8_to_utf32(aOutput.to_std_string_view());
        for (auto ch : utf32)
        {
            // todo: harden against security exploits due to dodgy input
            if (iEscapeSequence)
            {
                *iEscapeSequence += static_cast<char>(ch);
                switch(iEscapeSequence.value()[0])
                {
                case '%': // todo 
                case '(': // todo
                case ')': // todo
                case '*': // todo
                case '+': // todo
                case '-': // todo
                case '.': // todo
                case '/': // todo
                    if (iEscapeSequence.value().size() > 1)
                    {
                        service<debug::logger>() << "Unsupported escape sequence: " << iEscapeSequence.value() << endl;
                        iEscapeSequence = std::nullopt;
                    }
                    break;
                case ']':
                    // todo
                    if (iEscapeSequence.value().size() > 1 && ch == U'\a')
                    {
                        iEscapeSequence = std::nullopt;
                    }
                    break;
                case '[':
                    // todo
                    if (iEscapeSequence.value().size() > 1 && ch >= U'\x40' && ch <= U'\x7E')
                    {
                        //service<debug::logger>() << "CSI escape sequence: " << iEscapeSequence.value() << endl;
                        auto params = neolib::tokens(iEscapeSequence.value().substr(1, iEscapeSequence.value().size() - 2), ";"s, 0, false);
                        switch (ch)
                        {
                        case U'Z':
                            set_cursor_pos(cursor_pos().with_x(cursor_pos().x - (cursor_pos().x % iDefaultTabStop)));
                            break;
                        case U'd':
                            try
                            {
                                set_cursor_pos(cursor_pos().with_y(params.empty() ? 0 : std::stoi(params[0]) - 1));
                            }
                            catch (...) {}
                            break;
                        case U'G':
                            try
                            {
                                set_cursor_pos(cursor_pos().with_x(params.empty() ? 0 : std::stoi(params[0]) - 1));
                            }
                            catch (...) {}
                            break;
                        case U'S':
                            try
                            {
                                auto lines = (params.empty() ? 1 : std::stoi(params[0]));
                                while (lines--)
                                {
                                    iBuffer.erase(std::next(iBuffer.begin(), iBufferOrigin.y));
                                    (void)line(iBufferOrigin.y + iTerminalSize.cy - 1);
                                }
                            }
                            catch (...) {}
                            break;
                        case U'T':
                            try
                            {
                                auto lines = (params.empty() ? 1 : std::stoi(params[0]));
                                while (lines--)
                                {
                                    iBuffer.erase(std::next(iBuffer.begin(), iBufferOrigin.y + iTerminalSize.cy - 1));
                                    iBuffer.insert(std::next(iBuffer.begin(), iBufferOrigin.y), buffer_line{});
                                }
                            }
                            catch (...) {}
                            break;
                        case U'b':
                            try
                            {
                                // todo: wrap??
                                coordinate_type const n = params.empty() ? 1 : std::stoi(params[0]);
                                auto& line = terminal::line(buffer_pos().y);
                                line.text.insert(std::next(line.text.begin(), buffer_pos().x),
                                    n,
                                    line.text.at(buffer_pos().x - 1));
                                line.attributes.insert(std::next(line.attributes.begin(), buffer_pos().x),
                                    n,
                                    line.attributes.at(buffer_pos().x - 1));
                                line.glyphs = std::nullopt;
                                set_cursor_pos(cursor_pos().with_x(cursor_pos().x + n));
                            }
                            catch (...) {}
                            break;
                        case U'r':
                            if (params.empty())
                                iScrollingRegion = std::nullopt;
                            else
                            {
                                try
                                {
                                    scrolling_region sr;
                                    sr.top = std::stoi(params.at(0)) - 1;
                                    sr.bottom = std::stoi(params.at(1)) - 1;
                                    iScrollingRegion = sr;
                                }
                                catch (...) {}
                            }
                            break;
                        case U'X':
                            try
                            {
                                auto& line = terminal::line(buffer_pos().y);
                                if (!line.text.empty())
                                {
                                    line.text.erase(std::next(line.text.begin(), std::min(static_cast<coordinate_type>(line.text.size()) - 1, buffer_pos().x)), line.text.end());
                                    line.attributes.erase(std::next(line.attributes.begin(), std::min(static_cast<coordinate_type>(line.attributes.size()) - 1, buffer_pos().x)), line.attributes.end());
                                    line.glyphs = std::nullopt;
                                }
                            }
                            catch (...)
                            {
                            }
                            break;
                        case U'L':
                            {
                                coordinate_type lines = 1;
                                if (!params.empty())
                                    try { lines = std::stoi(params[0]); } catch (...) {}
                                coordinate_type top = 0;
                                coordinate_type bottom = iTerminalSize.cy - 1;
                                if (iScrollingRegion)
                                {
                                    top = iScrollingRegion.value().top;
                                    bottom = iScrollingRegion.value().bottom;
                                }
                                top += iBufferOrigin.y;
                                bottom += iBufferOrigin.y;
                                while (lines--)
                                {
                                    auto inserted = iBuffer.insert(std::next(iBuffer.begin(), buffer_pos().y), buffer_line{});
                                    inserted->text.reserve(iBufferSize.cx);
                                    inserted->attributes.reserve(iBufferSize.cx);
                                    iBuffer.erase(std::next(iBuffer.begin(), bottom + 1));
                                }
                                set_cursor_pos(cursor_pos().with_x(0));
                            }
                            break;
                        case U'c':
                            if (!params.empty())
                            {
                                if (params[0][0] == '>')
                                {
                                    int code = 0;
                                    try { code = std::stoi(params[0].substr(1)); } catch (...) {}
                                    if (code == 0)
                                        Input.trigger("\x1B[>0;0;0c"_s);
                                }
                            }
                            break;
                        case U't': // todo
                            break;
                        case U'h':
                            if (!params.empty())
                            {
                                if (params[0] == "?25")
                                    cursor().show();
                                else if (params[0] == "?7")
                                    iAutoWrap = true;
                                else if (params[0] == "?2004")
                                    iBracketedPaste = true;
                            }
                            break;
                        case U'l':
                            if (!params.empty())
                            {
                                if (params[0] == "?25")
                                    cursor().hide();
                                else if (params[0] == "?7")
                                    iAutoWrap = false;
                                else if (params[0] == "?2004")
                                    iBracketedPaste = false;
                            }
                            break;
                        case U'n':
                            if (!params.empty())
                            {
                                auto code = 0;
                                try { code = std::stoi(params[0]); }
                                catch (...) {}
                                if (code == 6)
                                {
                                    std::ostringstream oss;
                                    oss << "\x1B[" << cursor_pos().y + 1 << ";" << cursor_pos().x + 1 << "R";
                                    Input.trigger(string{oss.str()});
                                }
                            }
                            break;
                        case U'm':
                            if (params.empty())
                                iAttribute = std::nullopt;
                            else
                            {
                                if (params[0][0] == '>')
                                {
                                    // todo
                                }
                                else
                                {
                                    for (auto const& param : params)
                                    {
                                        auto code = 0;
                                        try { code = std::stoi(param); }
                                        catch (...) {}
                                        if (code == 0)
                                            iAttribute = std::nullopt;
                                        else if (code == 1)
                                        {
                                            if (!iAttribute)
                                                iAttribute.emplace(default_attribute());
                                            iAttribute.value().style |= font_style::Bold;
                                            iAttribute.value().style &= ~font_style::Normal;
                                        }
                                        else if (code == 3)
                                        {
                                            if (!iAttribute)
                                                iAttribute.emplace(default_attribute());
                                            iAttribute.value().style |= font_style::Italic;
                                            iAttribute.value().style &= ~font_style::Normal;
                                        }
                                        else if (code == 7)
                                        {
                                            if (!iAttribute)
                                                iAttribute.emplace(default_attribute());
                                            iAttribute.value().reverse = true;
                                        }
                                        else if (code == 9)
                                        {
                                            if (!iAttribute)
                                                iAttribute.emplace(default_attribute());
                                            iAttribute.value().style |= font_style::Strike;
                                        }
                                        else if (code == 22)
                                        {
                                            if (iAttribute)
                                            {
                                                iAttribute.value().style &= ~font_style::Bold;
                                                if ((iAttribute.value().style & font_style::Italic) == font_style::Invalid)
                                                    iAttribute.value().style |= font_style::Normal;
                                            }
                                        }
                                        else if (code == 23)
                                        {
                                            if (iAttribute)
                                            {
                                                iAttribute.value().style &= ~font_style::Italic;
                                                if ((iAttribute.value().style & font_style::Bold) == font_style::Invalid)
                                                    iAttribute.value().style |= font_style::Normal;
                                            }
                                        }
                                        else if (code == 27)
                                        {
                                            if (iAttribute)
                                                iAttribute.value().reverse = false;
                                        }
                                        else if (code == 29)
                                        {
                                            if (iAttribute)
                                                iAttribute.value().style &= ~font_style::Strike;
                                        }
                                        else if ((code >= 30 && code <= 37) || (code >= 40 && code <= 47) ||
                                            (code >= 90 && code <= 97) || (code >= 100 && code <= 107))
                                        {
                                            if (!iAttribute)
                                                iAttribute.emplace(default_attribute());
                                            if ((code >= 30 && code <= 37) || (code >= 90 && code <= 97))
                                                iAttribute.value().ink = attribute_color(code);
                                            else
                                                iAttribute.value().paper = attribute_color(code);
                                        }
                                        else if (code == 38 || code == 48)
                                        {
                                            try
                                            {
                                                if (!iAttribute)
                                                    iAttribute.emplace(default_attribute());
                                                auto subcode = std::stoi(params.at(1));
                                                if (subcode == 5)
                                                {
                                                    (code == 38 ? iAttribute.value().ink : iAttribute.value().paper) =
                                                        attribute_color_8bit(std::stoi(params.at(2)));
                                                }
                                                else if (subcode == 2)
                                                {
                                                    auto r = std::stoi(params.at(2));
                                                    auto g = std::stoi(params.at(3));
                                                    auto b = std::stoi(params.at(4));
                                                    (code == 38 ? iAttribute.value().ink : iAttribute.value().paper) =
                                                        attribute_color_24bit(r, g, b);
                                                }
                                            }
                                            catch (...) {}
                                            break;
                                        }
                                        else if (code == 39)
                                        {
                                            if (iAttribute)
                                                iAttribute.value().ink = color::White;
                                        }
                                        else if (code == 49)
                                        {
                                            if (iAttribute)
                                                iAttribute.value().paper = color::Black;
                                        }
                                        else
                                        {
                                            service<debug::logger>() << "Unknown CSI escape sequence: " << iEscapeSequence.value() << endl;
                                        }
                                    }
                                }
                            }
                            break;
                        case U'A':
                            {
                                std::int32_t n = 1;
                                if (!params.empty())
                                    try { n = std::stoi(params[0]); } catch (...) {}
                                set_cursor_pos(cursor_pos().with_y(cursor_pos().y - n));
                            }
                            break;
                        case U'B':
                            {
                                std::int32_t n = 1;
                                if (!params.empty())
                                    try { n = std::stoi(params[0]); } catch (...) {}
                                set_cursor_pos(cursor_pos().with_y(cursor_pos().y + n));
                            }
                            break;
                        case U'C':
                            {
                                std::int32_t n = 1;
                                if (!params.empty())
                                    try { n = std::stoi(params[0]); } catch (...) {}
                                set_cursor_pos(cursor_pos().with_x(cursor_pos().x + n));
                            }
                            break;
                        case U'D':
                            {
                                std::int32_t n = 1;
                                if (!params.empty())
                                    try { n = std::stoi(params[0]); } catch (...) {}
                                set_cursor_pos(cursor_pos().with_x(cursor_pos().x - n));
                            }
                            break;
                        case U'H':
                            {
                                std::int32_t row = 1;
                                std::int32_t col = 1;
                                if (!params.empty())
                                    try { row = std::stoi(params[0]); if (params.size() >= 2) col = std::stoi(params[1]); } catch (...) {}
                                set_cursor_pos({ col - 1, row - 1 });
                            }
                            break;
                        case U'J':
                            {
                                std::int32_t n = 0;
                                if (!params.empty())
                                    try { n = std::stoi(params[0]); } catch (...) {}
                                switch (n)
                                {
                                case 0:
                                    erase_in_display(buffer_pos(), to_buffer_pos(iTerminalSize));
                                    break;
                                case 1:
                                    erase_in_display(iBufferOrigin, buffer_pos());
                                    break;
                                case 2:
                                    erase_in_display(iBufferOrigin, to_buffer_pos(iTerminalSize));
                                    break;
                                case 3:
                                    iBuffer.clear();
                                    iBufferOrigin = {};
                                    set_cursor_pos({});
                                    break;
                                }
                            }
                            break;
                        case U'K':
                            {
                                std::int32_t n = 0;
                                if (!params.empty())
                                    try { n = std::stoi(params[0]); } catch (...) {}
                                auto& line = terminal::line(buffer_pos().y);
                                switch (n)
                                {
                                case 0:
                                    if (!line.text.empty())
                                    {
                                        line.text.erase(std::next(line.text.begin(), std::min(static_cast<coordinate_type>(line.text.size()), buffer_pos().x)), line.text.end());
                                        line.attributes.erase(std::next(line.attributes.begin(), std::min(static_cast<coordinate_type>(line.attributes.size()), buffer_pos().x)), line.attributes.end());
                                        line.glyphs = std::nullopt;
                                    }
                                    break;
                                case 1:
                                    line.text.erase(line.text.begin(), std::next(line.text.begin(), std::min(static_cast<coordinate_type>(line.text.size()), buffer_pos().x - 1)));
                                    line.attributes.erase(line.attributes.begin(), std::next(line.attributes.begin(), std::min(static_cast<coordinate_type>(line.attributes.size()), buffer_pos().x - 1)));
                                    line.glyphs = std::nullopt;
                                    set_cursor_pos(cursor_pos().with_x(0));
                                    break;
                                case 2:
                                    line.text.clear();
                                    line.attributes.clear();
                                    line.glyphs = std::nullopt;
                                    set_cursor_pos(cursor_pos().with_x(0));
                                    break;
                                }
                            }
                            break;
                        case U'P':
                            {
                                std::int32_t n = 1;
                                if (!params.empty())
                                    try { n = std::stoi(params[0]); } catch (...) {}
                                auto& line = terminal::line(buffer_pos().y);
                                line.text.erase(std::next(line.text.begin(), buffer_pos().x), std::next(line.text.begin(), buffer_pos().x + n));
                                line.attributes.erase(std::next(line.attributes.begin(), buffer_pos().x), std::next(line.attributes.begin(), buffer_pos().x + n));
                                line.glyphs = std::nullopt;
                            }
                            break;
                        default:
                            service<debug::logger>() << "Unknown CSI escape sequence: " << iEscapeSequence.value() << endl;
                            break;
                        }
                        iEscapeSequence = std::nullopt;
                    }
                    break;
                default:
                    // todo
                    service<debug::logger>() << "Unknown escape sequence: " << iEscapeSequence.value() << endl;
                    iEscapeSequence = std::nullopt;
                    break;
                }
            }
            else if (ch == U'\x1B')
            {
                iEscapeSequence.emplace();
            }
            else 
            {
                switch (ch)
                {
                case U'\a':
                    service<i_basic_services>().system_beep();
                    break;
                case U'\t':
                    set_cursor_pos(cursor_pos().with_x(cursor_pos().x + iDefaultTabStop - (cursor_pos().x % iDefaultTabStop)));
                    break;
                case U'\r':
                    set_cursor_pos(cursor_pos().with_x(0));
                    break;
                case U'\n':
                    if (!iScrollingRegion || cursor_pos().y < iScrollingRegion.value().bottom)
                        set_cursor_pos(cursor_pos().with_y(cursor_pos().y + 1));
                    else
                    {
                        iBuffer.erase(std::next(iBuffer.begin(), iScrollingRegion.value().top + iBufferOrigin.y));
                        iBuffer.insert(std::next(iBuffer.begin(), iScrollingRegion.value().bottom + iBufferOrigin.y), buffer_line{});
                    }
                    break;
                case U'\0':
                    break;
                case U'\b':
                    if (cursor_pos().x > 0)
                        set_cursor_pos(cursor_pos().with_x(cursor_pos().x - 1));
                    break;
                default:
                    if (ch >= U'\x20' && ch < U'\x7F')
                    {
                        if (cursor_pos().x == iTerminalSize.cx && iAutoWrap)
                            set_cursor_pos({ 0, cursor_pos().y + 1 });
                        character(buffer_pos()) = ch;
                        line(buffer_pos().y).glyphs = std::nullopt;
                        set_cursor_pos(cursor_pos().with_x(cursor_pos().x + 1));
                    }
                    else
                    {
                        std::ostringstream oss;
                        oss << "Unknown control char: 0x" << std::hex << std::uppercase << static_cast<std::uint32_t>(ch);
                        service<debug::logger>() << oss.str() << endl;
                    }
                    break;
                }
            }
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
        set_ideal_size(character_extents() * size{ iTerminalSize } +
            size{ effective_frame_width() } + size{ vertical_scrollbar().width(), horizontal_scrollbar().width() });
        cursor().set_style(cursor_style::Xor);
        cursor().set_width(character_extents().cx);

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

        set_cursor_pos({});
    }

    font const& terminal::font(font_style aStyle) const
    {
        switch (aStyle & (font_style::Normal | font_style::Bold | font_style::Italic))
        {
        case font_style::Normal:
            return normal_font();
        case font_style::Bold:
            return bold_font();
        case font_style::Italic:
            return italic_font();
        case font_style::Bold | font_style::Italic:
            return bold_italic_font();
        default:
            return normal_font();
        }
    }
    
    font const& terminal::normal_font() const
    {
        if (iNormalFont == std::nullopt)
            iNormalFont = font();
        return iNormalFont.value();
    }

    font const& terminal::bold_font() const
    {
        if (iBoldFont == std::nullopt)
            iBoldFont = neogfx::font{ font().with_style_xor(font_style::Bold) };
        return iBoldFont.value();
    }

    font const& terminal::italic_font() const
    {
        if (iItalicFont == std::nullopt)
            iItalicFont = neogfx::font{ font().with_style_xor(font_style::Italic) };
        return iItalicFont.value();
    }

    font const& terminal::bold_italic_font() const
    {
        if (iBoldItalicFont == std::nullopt)
            iBoldItalicFont = neogfx::font{ font().with_style_xor(font_style::Italic | font_style::BoldItalic) };
        return iBoldItalicFont.value();
    }

    terminal::attribute terminal::default_attribute() const
    {
        return attribute{ color::White, color::Black };
    }

    terminal::attribute terminal::active_attribute() const
    {
        if (iAttribute)
            return iAttribute.value();
        return default_attribute();
    }

    size terminal::character_extents() const
    {
        if (iCharacterExtents == std::nullopt)
        {
            size result;
            result.cx = normal_font().max_advance();
            result.cx = std::min(result.cx, bold_font().max_advance());
            result.cx = std::min(result.cx, italic_font().max_advance());
            result.cx = std::min(result.cx, bold_italic_font().max_advance());
            result.cy = normal_font().height();
            result.cy = std::min(result.cy, bold_font().height());
            result.cy = std::min(result.cy, italic_font().height());
            result.cy = std::min(result.cy, bold_italic_font().height());
            iCharacterExtents = result;
        }
        return iCharacterExtents.value();
    }

    void terminal::animate()
    {
        if (neolib::service<neolib::i_power>().green_mode_active())
            return;
        if (has_focus())
            update(cursor_rect());
    }

    void terminal::erase_in_display(point_type const& aBufferPosStart, point_type const& aBufferPosEnd)
    {
        auto lineStart = std::min(aBufferPosStart.y, static_cast<coordinate_type>(iBuffer.size()) - 1);
        auto lineEnd = std::min(aBufferPosEnd.y, static_cast<coordinate_type>(iBuffer.size()));
        if (aBufferPosStart.x > 0)
        {
            ++lineStart;
            auto& line = terminal::line(aBufferPosStart.y);
            line.text.erase(std::next(line.text.begin(), aBufferPosStart.x), line.text.end());
            line.attributes.erase(std::next(line.attributes.begin(), aBufferPosStart.x), line.attributes.end());
        }
        if (aBufferPosEnd.x < iBufferSize.cx)
        {
            --lineEnd;
            auto& line = terminal::line(aBufferPosStart.y);
            auto const eol = std::min(static_cast<coordinate_type>(line.text.size()), aBufferPosStart.y != aBufferPosEnd.y ? aBufferPosEnd.x : aBufferPosEnd.x - aBufferPosStart.x);
            line.text.erase(line.text.begin(), std::next(line.text.begin(), eol));
            line.attributes.erase(line.attributes.begin(), std::next(line.attributes.begin(), eol));
        }
        auto eraseLineStart = std::next(iBuffer.begin(), lineStart);
        auto eraseLineEnd = std::next(iBuffer.begin(), lineEnd);
        iBuffer.erase(eraseLineStart, eraseLineEnd);
    }

    terminal::buffer_line& terminal::line(coordinate_type aLine)
    {
        auto oldBufferSize = iBuffer.size();
        auto const desiredBufferSize = aLine + 1;

        while (iBuffer.size() < desiredBufferSize)
        {
            iBuffer.emplace_back();;
            iBuffer.back().text.reserve(iBufferSize.cx);
            iBuffer.back().attributes.reserve(iBufferSize.cx);
        }

        if (iBuffer.size() > iBufferSize.cy)
            iBuffer.erase(iBuffer.begin(), std::next(iBuffer.begin(), iBuffer.size() - iBufferSize.cy));

        if (iBuffer.size() - iBufferOrigin.y > iTerminalSize.cy)
            iBufferOrigin.y += (static_cast<coordinate_type>(iBuffer.size() - oldBufferSize));

        return iBuffer[aLine];
    }

    char32_t& terminal::character(point_type const& aBufferPos)
    {
        auto& line = terminal::line(aBufferPos.y);
        if (line.text.size() <= aBufferPos.x)
            line.text.resize(aBufferPos.x + 1, U' ');
        if (line.attributes.size() <= aBufferPos.x)
            line.attributes.resize(aBufferPos.x + 1, default_attribute());
        line.attributes[aBufferPos.x] = active_attribute();
        return line.text[aBufferPos.x];
    }

    terminal::point_type terminal::buffer_pos() const
    {
        return to_buffer_pos(cursor_pos());
    }

    terminal::point_type terminal::to_buffer_pos(point_type aCursorPos) const
    {
        return iBufferOrigin + aCursorPos;
    }

    terminal::point_type terminal::cursor_pos() const
    {
        return iCursorPos.value();
    }

    void terminal::set_cursor_pos(point_type aCursorPos)
    {
        (void)line(to_buffer_pos(aCursorPos).y);

        aCursorPos.y = std::max(0, std::min(aCursorPos.y, iTerminalSize.cy - 1));
        aCursorPos.x = std::max(0, std::min(aCursorPos.x, iTerminalSize.cx));

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
        auto scrollAreaY = static_cast<coordinate_type>(iBuffer.size());
        if (scrollAreaY < iTerminalSize.cy + iBufferOrigin.y && iBufferOrigin.y > 0)
            scrollAreaY = iTerminalSize.cy + iBufferOrigin.y;
        vertical_scrollbar().set_maximum(scrollAreaY * character_extents().cy);
        make_cursor_visible();
        update();
    }
    
    rect terminal::cursor_rect() const
    {
        return rect{ point{ buffer_pos() } * character_extents() - point{0.0, vertical_scrollbar().position() }, character_extents() };
    }

    void terminal::make_cursor_visible(bool aToBufferOrigin)
    {
        if (aToBufferOrigin)
        {
            vertical_scrollbar().set_position(iBufferOrigin.y * character_extents().cy);
        }
        else
        {
            auto const& cr = cursor_rect();
            if (cr.bottom() <= 0.0)
                vertical_scrollbar().set_position(vertical_scrollbar().position() + cr.top());
            else if (cr.top() >= vertical_scrollbar().page())
                vertical_scrollbar().set_position(vertical_scrollbar().position() + cr.bottom() - vertical_scrollbar().page());
        }
    }
    
    void terminal::draw_cursor(i_graphics_context& aGc) const
    {
        if (cursor().hidden())
            return;
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