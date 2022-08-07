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

    void terminal::resized()
    {
        base_type::resized();

        auto const oldTerminalSize = terminal_size();
        auto const oldBufferPos = buffer_pos();
        
        iTerminalSize.cx = std::max(1, static_cast<dimension_type>(client_rect(false).cx / character_extents().cx));
        iTerminalSize.cy = std::max(1, static_cast<dimension_type>(client_rect(false).cy / character_extents().cy));
        
        if (iPrimaryBuffer.scrollingRegion)
        {
            // todo: this is just a guess...
            if (iPrimaryBuffer.scrollingRegion.value().top >= terminal_size().cy)
                iPrimaryBuffer.scrollingRegion.value().top += (terminal_size().cy - oldTerminalSize.cy);
            iPrimaryBuffer.scrollingRegion.value().bottom += (terminal_size().cy - oldTerminalSize.cy);
        }
        if (iAlternateBuffer.scrollingRegion)
        {
            // todo: this is just a guess...
            if (iAlternateBuffer.scrollingRegion.value().top >= terminal_size().cy)
                iAlternateBuffer.scrollingRegion.value().top += (terminal_size().cy - oldTerminalSize.cy);
            iAlternateBuffer.scrollingRegion.value().bottom += (terminal_size().cy - oldTerminalSize.cy);
        }

        auto const yDelta = terminal_size().cy - oldTerminalSize.cy;
        if (yDelta < 0 && active_buffer().lines.size() > terminal_size().cy)
        {
            auto overflow = std::min(static_cast<dimension_type>(active_buffer().lines.size()), -yDelta);
            if (active_buffer().scrollingRegion)
            {
                auto eraseStart = std::next(active_buffer().lines.begin(), std::max(0, buffer_origin().y - overflow));
                auto eraseEnd = std::next(eraseStart, std::min(overflow, static_cast<dimension_type>(std::distance(eraseStart, active_buffer().lines.end()))));
                active_buffer().lines.erase(eraseStart, eraseEnd);
            }
            else
                set_buffer_origin(buffer_origin() + point_type{ 0, overflow });
        }
        else if (yDelta > 0 && active_buffer().lines.size() > terminal_size().cy)
        {
            set_buffer_origin(buffer_origin() + point_type{ 0, std::max(0, buffer_origin().y - yDelta) });
        }

        if (yDelta > 0 && buffer_origin().y + terminal_size().cy > active_buffer().lines.size())
            set_buffer_origin(buffer_origin().with_y(
                std::max(0, static_cast<dimension_type>(active_buffer().lines.size()) - terminal_size().cy)));

        if (!set_cursor_pos(oldBufferPos - buffer_origin()))
            update_cursor();

        if (terminal_size() != oldTerminalSize)
            TerminalResized.trigger(terminal_size());

        set_ideal_size(padding().size() + character_extents() * size { terminal_size() } +
            size{ effective_frame_width() } + size{ vertical_scrollbar().width(), horizontal_scrollbar().width() });
    }

    rect terminal::scroll_area() const
    {
        auto scrollAreaExtents = size{ iBufferSize.with_cy(static_cast<dimension_type>(active_buffer().lines.size())) };
        if (scrollAreaExtents.cy < iTerminalSize.cy + buffer_origin().y && buffer_origin().y > 0)
            scrollAreaExtents.cy = iTerminalSize.cy + buffer_origin().y;
        return rect{ point{}, scrollAreaExtents * character_extents() };
    }

    size terminal::scroll_page() const
    {
        return client_rect(false).extents();
    }

    bool terminal::use_scrollbar_container_updater() const
    {
        return false;
    }

    void terminal::paint(i_graphics_context& aGc) const
    {
        base_type::paint(aGc);

        auto const& cr = client_rect(false);
        auto const& tl = cr.top_left();
        auto const& ce = character_extents();

        scoped_scissor ss{ aGc, cr };

        scalar y = -vertical_scrollbar().position();

        for (auto const& line : active_buffer().lines)
        {
            if (line.glyphs == std::nullopt)
            {
                line.glyphs = aGc.to_glyph_text(line.text,
                    [&](std::size_t n) -> neogfx::font
                    {
                        return n < line.attributes.size() ? font(line.attributes[n].style) : normal_font();
                    });
                for (auto& g : *line.glyphs)
                    g.advance.cx = static_cast<float>(ce.cx);
            }
            if (y + ce.cy >= cr.top() && y < cr.bottom())
            {
                thread_local text_attribute_spans attributes;
                attributes.clear();
                for (auto const& g : *line.glyphs)
                {
                    auto ink = line.attributes[g.source.first].ink;
                    auto paper = line.attributes[g.source.first].paper;
                    if (line.attributes[g.source.first].reverse)
                        std::swap(ink, paper);
                    optional_text_effect effect;
                    if (iTextAttributes)
                    {
                        effect = iTextAttributes->effect();
                        if (effect->type() == text_effect_type::Glow)
                        {
                            if (std::holds_alternative<color>(effect->color()))
                                effect->set_color(ink.to_hsv().with_saturation(ink.to_hsv().saturation() * 0.7).to_rgb<color>());
                            ink = ink.to_hsv().with_saturation(ink.to_hsv().saturation() * 0.4).to_rgb<color>();
                        }
                    }
                    attributes.add(g.source.first, ink, paper, effect);
                }
                aGc.draw_glyphs(tl + point{ 0, y }, *line.glyphs, attributes);
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
        set_ideal_size(padding().size() + character_extents() * size { iTerminalSize } +
            size{ effective_frame_width() } + size{ vertical_scrollbar().width(), horizontal_scrollbar().width() });
        iPrimaryBuffer.cursor.set_width(character_extents().cx);
        iAlternateBuffer.cursor.set_width(character_extents().cx);
    }

    void terminal::set_text_attributes(optional_text_attributes const& aTextAttributes)
    {
        iTextAttributes = aTextAttributes;
        update();
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
        case ScanCode_ESCAPE:
            Input.trigger("\x1B"_s);
            break;
        case ScanCode_LEFT:
            if (active_buffer().ansiMode)
            {
                if (!active_buffer().cursorKeysMode)
                    Input.trigger("\x1B[D"_s);
                else
                    Input.trigger("\x1BOD"_s);
            }
            else
                Input.trigger("\x1B""D"_s);
            handled = true;
            break;
        case ScanCode_RIGHT:
            if (active_buffer().ansiMode)
            {
                if (!active_buffer().cursorKeysMode)
                    Input.trigger("\x1B[C"_s);
                else
                    Input.trigger("\x1BOC"_s);
            }
            else
                Input.trigger("\x1B""C"_s);
            handled = true;
            break;
        case ScanCode_UP:
            if (active_buffer().ansiMode)
            {
                if (!active_buffer().cursorKeysMode)
                    Input.trigger("\x1B[A"_s);
                else
                    Input.trigger("\x1BOA"_s);
            }
            else
                Input.trigger("\x1B""A"_s);
            handled = true;
            break;
        case ScanCode_DOWN:
            if (active_buffer().ansiMode)
            {
                if (!active_buffer().cursorKeysMode)
                    Input.trigger("\x1B[B"_s);
                else
                    Input.trigger("\x1BOB"_s);
            }
            else
                Input.trigger("\x1B""B"_s);
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

    terminal::size_type terminal::terminal_size() const
    {
        return iTerminalSize;
    }

    void terminal::output(i_string const& aOutput)
    {
        // todo: apply a bit of functional decomposition to this function which is getting a tad long...

        neolib::scoped_flag sf{ iOutputting };

        auto utf32 = neolib::utf8_to_utf32(aOutput.to_std_string_view());
        for (auto ch : utf32)
        {
#if 0 // for debugging purposes...
            if (ch >= U' ')
                std::cout << (char) ch << std::flush;
            else
                std::cout << "\\x" << std::hex << std::uppercase << (std::uint32_t)ch << std::flush;
#endif
            // todo: harden against security exploits due to dodgy input
            if (iEscapeSequence)
            {
                *iEscapeSequence += static_cast<char>(ch);
                switch(iEscapeSequence.value()[0])
                {
                case 'M':
                    if (!active_buffer().scrollingRegion || cursor_pos().y > active_buffer().scrollingRegion.value().top)
                        set_cursor_pos(cursor_pos().with_y(cursor_pos().y - 1));
                    else
                    {
                        if (!active_buffer().scrollingRegion)
                        {
                            active_buffer().lines.erase(std::next(active_buffer().lines.begin(), buffer_origin().y + iTerminalSize.cy - 1));
                            active_buffer().lines.insert(std::next(active_buffer().lines.begin(), buffer_origin().y), buffer_line{});
                        }
                        else
                        {
                            active_buffer().lines.erase(std::next(active_buffer().lines.begin(), buffer_origin().y + active_buffer().scrollingRegion.value().bottom));
                            active_buffer().lines.insert(std::next(active_buffer().lines.begin(), buffer_origin().y + active_buffer().scrollingRegion.value().top), buffer_line{});
                        }
                    }
                    iEscapeSequence = std::nullopt;
                    break;
                case '=':
                    active_buffer().keypadMode = keypad_mode::Application;
                    iEscapeSequence = std::nullopt;
                    break;
                case '>':
                    active_buffer().keypadMode = keypad_mode::Numeric;
                    iEscapeSequence = std::nullopt;
                    break;
                case '(': // todo
                    if (iEscapeSequence.value().size() > 1)
                    {
                        switch (iEscapeSequence.value()[1])
                        {
                        case 'B':
                            active_buffer().characterSet = character_set::USASCII;
                            break;
                        case '0':
                            active_buffer().characterSet = character_set::DECSpecial;
                            break;
                        // todo
                        default:
                            active_buffer().characterSet = character_set::Unknown;
                            break;
                        }
                        iEscapeSequence = std::nullopt;
                    }
                    break;
                case '%': // todo 
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
                        auto const paramsInput = iEscapeSequence.value().substr(1, iEscapeSequence.value().size() - 2);
                        auto const delim = ";"s;
                        std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>> result;
                        auto params = neolib::tokens(paramsInput.begin(), paramsInput.end(), delim.begin(), delim.end(), result, 0, false);
                        if (result.size() >= 2 && 
                            (std::string_view{ result[0].first, result[0].second } == "0" || std::string_view{ result[0].first, result[0].second } == "2"))
                        {
                            if (root().as_widget().is_parent_of(*this) || root().client_widget().is_parent_of(*this))
                                root().set_title_text(string{ std::string_view{ result[1].first, paramsInput.end() } });
                        }
                        else
                            service<debug::logger>() << "Unsupported escape sequence: " << iEscapeSequence.value().substr(0, iEscapeSequence.value().length() - 1) << endl;
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
                            set_cursor_pos(cursor_pos().with_x(cursor_pos().x - (cursor_pos().x % active_buffer().defaultTabStop)), false);
                            break;
                        case U'd':
                            try
                            {
                                set_cursor_pos(cursor_pos().with_y(params.empty() ? 0 : std::stoi(params[0]) - 1), false);
                            }
                            catch (...) {}
                            break;
                        case U'G':
                            try
                            {
                                set_cursor_pos(cursor_pos().with_x(params.empty() ? 0 : std::stoi(params[0]) - 1), false);
                            }
                            catch (...) {}
                            break;
                        case U'S':
                            try
                            {
                                auto lines = (params.empty() ? 1 : std::stoi(params[0]));
                                while (lines--)
                                {
                                    active_buffer().lines.erase(std::next(active_buffer().lines.begin(), buffer_origin().y));
                                    (void)line(buffer_origin().y + iTerminalSize.cy - 1);
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
                                    active_buffer().lines.erase(std::next(active_buffer().lines.begin(), buffer_origin().y + iTerminalSize.cy - 1));
                                    active_buffer().lines.insert(std::next(active_buffer().lines.begin(), buffer_origin().y), buffer_line{});
                                }
                            }
                            catch (...) {}
                            break;
                        case U'b':
                            try
                            {
                                coordinate_type n = params.empty() ? 1 : std::stoi(params[0]);
                                auto& line = terminal::line(buffer_pos().y);
                                auto repChar = line.text.at(buffer_pos().x - 1);
                                auto repAttribute = line.attributes.at(buffer_pos().x - 1);
                                while (n--)
                                    output_character(repChar, repAttribute);
                            }
                            catch (...) {}
                            break;
                        case U'r':
                            if (params.empty())
                                active_buffer().scrollingRegion = std::nullopt;
                            else
                            {
                                try
                                {
                                    scrolling_region sr;
                                    sr.top = std::stoi(params.at(0)) - 1;
                                    sr.bottom = std::stoi(params.at(1)) - 1;
                                    active_buffer().scrollingRegion = sr;
                                }
                                catch (...) {}
                            }
                            break;
                        case U'X':
                            try
                            {
                                coordinate_type const n = params.empty() ? 1 : std::stoi(params[0]);
                                auto& line = terminal::line(buffer_pos().y);
                                if (!line.text.empty())
                                {
                                    coordinate_type const start = std::min(static_cast<coordinate_type>(line.text.size()), buffer_pos().x);
                                    coordinate_type const end = std::min(static_cast<coordinate_type>(line.text.size()), start + n);
                                    line.text.erase(std::next(line.text.begin(), start), std::next(line.text.begin(), end));
                                    line.attributes.erase(std::next(line.attributes.begin(), start), std::next(line.attributes.begin(), end));
                                    line.text.insert(std::next(line.text.begin(), start), end - start, U' ');
                                    line.attributes.insert(std::next(line.attributes.begin(), start), end - start, default_attribute());
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
                                if (active_buffer().scrollingRegion)
                                {
                                    top = active_buffer().scrollingRegion.value().top;
                                    bottom = active_buffer().scrollingRegion.value().bottom;
                                }
                                top += buffer_origin().y;
                                bottom += buffer_origin().y;
                                while (lines--)
                                {
                                    auto inserted = active_buffer().lines.insert(std::next(active_buffer().lines.begin(), buffer_pos().y), buffer_line{});
                                    inserted->text.reserve(iBufferSize.cx);
                                    inserted->attributes.reserve(iBufferSize.cx);
                                    active_buffer().lines.erase(std::next(active_buffer().lines.begin(), bottom + 1));
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
                        case U'h':
                            if (!params.empty())
                            {
                                if (params[0] == "?25")
                                    cursor().show();
                                else if (params[0] == "?6")
                                    active_buffer().originMode = true;
                                else if (params[0] == "?7")
                                    active_buffer().autoWrap = true;
                                else if (params[0] == "?1049")
                                    enable_alternate_buffer();
                                else if (params[0] == "?2004")
                                    active_buffer().bracketedPaste = true;
                                else if (params[0] == "?1")
                                    active_buffer().cursorKeysMode = true;
                                else if (params[0] == "?2")
                                    active_buffer().ansiMode = true;
                            }
                            break;
                        case U'l':
                            if (!params.empty())
                            {
                                if (params[0] == "?25")
                                    cursor().hide();
                                else if (params[0] == "?7")
                                    active_buffer().autoWrap = false;
                                else if (params[0] == "?6")
                                    active_buffer().originMode = false;
                                else if (params[0] == "?1049")
                                    disable_alternate_buffer();
                                else if (params[0] == "?2004")
                                    active_buffer().bracketedPaste = false;
                                else if (params[0] == "?1")
                                    active_buffer().cursorKeysMode = false;
                                else if (params[0] == "?2")
                                    active_buffer().ansiMode = false;
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
                                active_buffer().attribute = std::nullopt;
                            else
                            {
                                if (params[0][0] == '>')
                                {
                                    service<debug::logger>() << "Unsupported CSI escape sequence: " << iEscapeSequence.value() << endl;
                                }
                                else
                                {
                                    for (auto const& param : params)
                                    {
                                        auto code = 0;
                                        try { code = std::stoi(param); }
                                        catch (...) {}
                                        if (code == 0)
                                            active_buffer().attribute = std::nullopt;
                                        else if (code == 1)
                                        {
                                            if (!active_buffer().attribute)
                                                active_buffer().attribute.emplace(default_attribute());
                                            active_buffer().attribute.value().style |= font_style::Bold;
                                            active_buffer().attribute.value().style &= ~font_style::Normal;
                                        }
                                        else if (code == 3)
                                        {
                                            if (!active_buffer().attribute)
                                                active_buffer().attribute.emplace(default_attribute());
                                            active_buffer().attribute.value().style |= font_style::Italic;
                                            active_buffer().attribute.value().style &= ~font_style::Normal;
                                        }
                                        else if (code == 7)
                                        {
                                            if (!active_buffer().attribute)
                                                active_buffer().attribute.emplace(default_attribute());
                                            active_buffer().attribute.value().reverse = true;
                                        }
                                        else if (code == 9)
                                        {
                                            if (!active_buffer().attribute)
                                                active_buffer().attribute.emplace(default_attribute());
                                            active_buffer().attribute.value().style |= font_style::Strike;
                                        }
                                        else if (code == 22)
                                        {
                                            if (active_buffer().attribute)
                                            {
                                                active_buffer().attribute.value().style &= ~font_style::Bold;
                                                if ((active_buffer().attribute.value().style & font_style::Italic) == font_style::Invalid)
                                                    active_buffer().attribute.value().style |= font_style::Normal;
                                            }
                                        }
                                        else if (code == 23)
                                        {
                                            if (active_buffer().attribute)
                                            {
                                                active_buffer().attribute.value().style &= ~font_style::Italic;
                                                if ((active_buffer().attribute.value().style & font_style::Bold) == font_style::Invalid)
                                                    active_buffer().attribute.value().style |= font_style::Normal;
                                            }
                                        }
                                        else if (code == 27)
                                        {
                                            if (active_buffer().attribute)
                                                active_buffer().attribute.value().reverse = false;
                                        }
                                        else if (code == 29)
                                        {
                                            if (active_buffer().attribute)
                                                active_buffer().attribute.value().style &= ~font_style::Strike;
                                        }
                                        else if ((code >= 30 && code <= 37) || (code >= 40 && code <= 47) ||
                                            (code >= 90 && code <= 97) || (code >= 100 && code <= 107))
                                        {
                                            if (!active_buffer().attribute)
                                                active_buffer().attribute.emplace(default_attribute());
                                            if ((code >= 30 && code <= 37) || (code >= 90 && code <= 97))
                                                active_buffer().attribute.value().ink = attribute_color(code);
                                            else
                                                active_buffer().attribute.value().paper = attribute_color(code);
                                        }
                                        else if (code == 38 || code == 48)
                                        {
                                            try
                                            {
                                                if (!active_buffer().attribute)
                                                    active_buffer().attribute.emplace(default_attribute());
                                                auto subcode = std::stoi(params.at(1));
                                                if (subcode == 5)
                                                {
                                                    (code == 38 ? active_buffer().attribute.value().ink : active_buffer().attribute.value().paper) =
                                                        attribute_color_8bit(std::stoi(params.at(2)));
                                                }
                                                else if (subcode == 2)
                                                {
                                                    auto r = std::stoi(params.at(2));
                                                    auto g = std::stoi(params.at(3));
                                                    auto b = std::stoi(params.at(4));
                                                    (code == 38 ? active_buffer().attribute.value().ink : active_buffer().attribute.value().paper) =
                                                        attribute_color_24bit(r, g, b);
                                                }
                                            }
                                            catch (...) {}
                                            break;
                                        }
                                        else if (code == 39)
                                        {
                                            if (active_buffer().attribute)
                                                active_buffer().attribute.value().ink = color::White;
                                        }
                                        else if (code == 49)
                                        {
                                            if (active_buffer().attribute)
                                                active_buffer().attribute.value().paper = color::Black;
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
                                if (active_buffer().originMode && active_buffer().scrollingRegion)
                                    row += active_buffer().scrollingRegion.value().top;
                                if (!params.empty())
                                    try { row = std::stoi(params[0]); if (params.size() >= 2) col = std::stoi(params[1]); } catch (...) {}
                                set_cursor_pos({ col - 1, row - 1 }, false);
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
                                    erase_in_display(buffer_origin(), buffer_pos());
                                    break;
                                case 2:
                                    erase_in_display(buffer_origin(), to_buffer_pos(iTerminalSize));
                                    break;
                                case 3:
                                    active_buffer().lines.clear();
                                    set_buffer_origin({});
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
                                    line.text.erase(line.text.begin(), std::next(line.text.begin(), std::min(static_cast<coordinate_type>(line.text.size()), buffer_pos().x + 1)));
                                    line.attributes.erase(line.attributes.begin(), std::next(line.attributes.begin(), std::min(static_cast<coordinate_type>(line.attributes.size()), buffer_pos().x + 1)));
                                    line.text.insert(line.text.begin(), buffer_pos().x + 1, U' ');
                                    line.attributes.insert(line.attributes.begin(), buffer_pos().x + 1, default_attribute());
                                    line.glyphs = std::nullopt;
                                    break;
                                case 2:
                                    line.text.clear();
                                    line.attributes.clear();
                                    line.glyphs = std::nullopt;
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
                                line.text.insert(line.text.end(), n, U' ');
                                line.attributes.insert(line.attributes.end(), n, attribute{});
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
                    set_cursor_pos(cursor_pos().with_x(cursor_pos().x + active_buffer().defaultTabStop - (cursor_pos().x % active_buffer().defaultTabStop)));
                    break;
                case U'\r':
                    set_cursor_pos(cursor_pos().with_x(0));
                    break;
                case U'\n':
                    if (!active_buffer().scrollingRegion || cursor_pos().y < active_buffer().scrollingRegion.value().bottom)
                        set_cursor_pos(cursor_pos().with_y(cursor_pos().y + 1));
                    else
                    {
                        active_buffer().lines.erase(std::next(active_buffer().lines.begin(), active_buffer().scrollingRegion.value().top + buffer_origin().y));
                        active_buffer().lines.insert(std::next(active_buffer().lines.begin(), active_buffer().scrollingRegion.value().bottom + buffer_origin().y), buffer_line{});
                    }
                    break;
                case U'\0':
                    break;
                case U'\b':
                    if (cursor_pos().x > 0)
                        set_cursor_pos(cursor_pos().with_x(cursor_pos().x - 1));
                    break;
                default:
                    if (ch >= U'\x20' && ch != U'\x7F')
                        output_character(to_unicode(ch), active_buffer().attribute);
                    else
                    {
                        std::ostringstream oss;
                        oss << "Unsupported control char: 0x" << std::hex << std::uppercase << static_cast<std::uint32_t>(ch);
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
        return active_buffer().cursor;
    }

    void terminal::init()
    {
        vertical_scrollbar().set_style(vertical_scrollbar().style() | scrollbar_style::AlwaysVisible);
        horizontal_scrollbar().set_style(scrollbar_style::None);
        set_ideal_size(padding().size() + character_extents() * size { iTerminalSize } +
            size{ effective_frame_width() } + size{ vertical_scrollbar().width(), horizontal_scrollbar().width() });
        iPrimaryBuffer.cursor.set_style(cursor_style::Xor);
        iPrimaryBuffer.cursor.set_width(character_extents().cx);
        iAlternateBuffer.cursor.set_style(cursor_style::Xor);
        iAlternateBuffer.cursor.set_width(character_extents().cx);

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

    terminal::buffer& terminal::active_buffer() const
    {
        return *iActiveBuffer;
    }

    void terminal::enable_alternate_buffer()
    {
        iActiveBuffer = &iAlternateBuffer;
        iAlternateBuffer = {};
        iAlternateBuffer.cursor.set_style(cursor_style::Xor);
        iAlternateBuffer.cursor.set_width(character_extents().cx);
        set_cursor_pos({});
        update_cursor();
    }

    void terminal::disable_alternate_buffer()
    {
        iActiveBuffer = &iPrimaryBuffer;
        update_cursor();
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
        if (active_buffer().attribute)
            return active_buffer().attribute.value();
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
        auto lineStart = std::min(aBufferPosStart.y, static_cast<coordinate_type>(active_buffer().lines.size()) - 1);
        auto lineEnd = std::min(aBufferPosEnd.y, static_cast<coordinate_type>(active_buffer().lines.size()));
        if (aBufferPosStart.x > 0)
        {
            ++lineStart;
            auto& line = terminal::line(aBufferPosStart.y);
            line.text.erase(std::next(line.text.begin(), aBufferPosStart.x), line.text.end());
            line.attributes.erase(std::next(line.attributes.begin(), aBufferPosStart.x), line.attributes.end());
        }
        if (aBufferPosEnd.x < terminal_size().cx)
        {
            --lineEnd;
            auto& line = terminal::line(aBufferPosStart.y);
            auto const eol = std::min(static_cast<coordinate_type>(line.text.size()), aBufferPosStart.y != aBufferPosEnd.y ? aBufferPosEnd.x : aBufferPosEnd.x - aBufferPosStart.x);
            line.text.erase(line.text.begin(), std::next(line.text.begin(), eol));
            line.attributes.erase(line.attributes.begin(), std::next(line.attributes.begin(), eol));
        }
        auto eraseLineStart = std::next(active_buffer().lines.begin(), lineStart);
        auto eraseLineEnd = std::next(active_buffer().lines.begin(), lineEnd);
        active_buffer().lines.erase(eraseLineStart, eraseLineEnd);
    }

    char32_t terminal::to_unicode(char32_t aCharacter) const
    {
        switch (active_buffer().characterSet)
        {
        case character_set::USASCII:
            return aCharacter;
        case character_set::DECSpecial:
            {
                static std::unordered_map<char32_t, char32_t> set
                {
                    { U'\x60', U'\x25C6' }, { U'\x61', U'\x2592' }, { U'\x62', U'\x2409' }, { U'\x63', U'\x240C' },
                    { U'\x64', U'\x240D' }, { U'\x65', U'\x240A' }, { U'\x66', U'\x00B0' }, { U'\x67', U'\x00B1' },
                    { U'\x68', U'\x2424' }, { U'\x69', U'\x240B' }, { U'\x6A', U'\x2518' }, { U'\x6B', U'\x2510' },
                    { U'\x6C', U'\x250C' }, { U'\x6D', U'\x2514' }, { U'\x6E', U'\x253C' }, { U'\x6F', U'\x23BA' },
                    { U'\x70', U'\x23BB' }, { U'\x71', U'\x2500' }, { U'\x72', U'\x23BC' }, { U'\x73', U'\x23BD' },
                    { U'\x74', U'\x251C' }, { U'\x75', U'\x2524' }, { U'\x76', U'\x2534' }, { U'\x77', U'\x252C' },
                    { U'\x78', U'\x2502' }, { U'\x79', U'\x2264' }, { U'\x7A', U'\x2265' }, { U'\x7B', U'\x03C0' },
                    { U'\x7C', U'\x2260' }, { U'\x7D', U'\x00A3' }, { U'\x7E', U'\x00B7' }
                };
                auto existing = set.find(aCharacter);
                if (existing != set.end())
                    return existing->second;
            }
            return U' ';
        default:
            {
                std::ostringstream oss;
                oss << "Unsupported character set, char: '" << std::string(1, static_cast<char>(aCharacter)) << "' (0x" << std::hex << std::uppercase << static_cast<std::uint32_t>(aCharacter) << ")";
                service<debug::logger>() << oss.str() << endl;
            }
            return U'\xFFFD';
        }
    }

    terminal::buffer_line& terminal::line(coordinate_type aLine)
    {
        auto oldBufferSize = active_buffer().lines.size();
        auto const desiredBufferSize = aLine + 1;

        while (active_buffer().lines.size() < desiredBufferSize)
        {
            active_buffer().lines.emplace_back();
            active_buffer().lines.back().text.reserve(iBufferSize.cx);
            active_buffer().lines.back().attributes.reserve(iBufferSize.cx);
        }

        if (active_buffer().lines.size() > iBufferSize.cy)
            active_buffer().lines.erase(active_buffer().lines.begin(), std::next(active_buffer().lines.begin(), active_buffer().lines.size() - iBufferSize.cy));

        if (active_buffer().lines.size() - buffer_origin().y > iTerminalSize.cy)
            set_buffer_origin( buffer_origin() + 
                point_type{ 0, (static_cast<coordinate_type>(active_buffer().lines.size() - oldBufferSize)) });

        return active_buffer().lines[aLine];
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

    void terminal::output_character(char32_t aCharacter, std::optional<attribute> const& aAttribute)
    {
        if (cursor_pos().x == iTerminalSize.cx && active_buffer().autoWrap &&
            (!active_buffer().scrollingRegion || cursor_pos().y + 1 < active_buffer().scrollingRegion->bottom))
            set_cursor_pos({ 0, cursor_pos().y + 1 });
        character(buffer_pos()) = aCharacter;
        if (aAttribute)
        {
            auto& line = terminal::line(buffer_pos().y);
            line.attributes.at(buffer_pos().x) = aAttribute.value();
        }
        line(buffer_pos().y).glyphs = std::nullopt;
        set_cursor_pos(cursor_pos().with_x(cursor_pos().x + 1));
    }

    terminal::point_type terminal::buffer_origin() const
    {
        return active_buffer().bufferOrigin;
    }

    void terminal::set_buffer_origin(terminal::point_type aBufferOrigin)
    {
        active_buffer().bufferOrigin = aBufferOrigin;
    }

    terminal::point_type terminal::buffer_pos() const
    {
        return to_buffer_pos(cursor_pos());
    }

    terminal::point_type terminal::to_buffer_pos(point_type aCursorPos) const
    {
        return buffer_origin() + aCursorPos;
    }

    terminal::point_type terminal::cursor_pos() const
    {
        return active_buffer().cursorPos.value();
    }

    bool terminal::set_cursor_pos(point_type aCursorPos, bool aExtendBuffer)
    {
        if (aExtendBuffer || aCursorPos.y < terminal_size().cy)
            (void)line(to_buffer_pos(aCursorPos).y);

        aCursorPos.y = std::max(0, std::min(aCursorPos.y, terminal_size().cy - 1));
        aCursorPos.x = std::max(0, std::min(aCursorPos.x, terminal_size().cx));

        if (active_buffer().cursorPos == aCursorPos)
            return false;

        active_buffer().cursorPos = aCursorPos;
        if (!iOutputting)
        {
            update_cursor();
        }

        return true;
    }

    void terminal::update_cursor()
    {
        cursor().set_position(iTerminalSize.cx * cursor_pos().y + cursor_pos().x);
        update_scrollbar_visibility();
        make_cursor_visible();
        update();
    }
    
    rect terminal::cursor_rect() const
    {
        return rect{ client_rect(false).top_left() + point{buffer_pos()} *character_extents() - point{0.0, vertical_scrollbar().position()}, character_extents()};
    }

    void terminal::make_cursor_visible(bool aToBufferOrigin)
    {
        if (aToBufferOrigin)
            vertical_scrollbar().set_position(padding().top + buffer_origin().y * character_extents().cy);
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