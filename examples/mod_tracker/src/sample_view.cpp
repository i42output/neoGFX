/*
neogfx C++ App/Game Engine - Examples - MOD Tracker
Copyright(C) 2026 Leigh Johnston

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

#include <mod_tracker/mod_tracker.hpp>

#include <algorithm>
#include <cmath>
#include <string>

#include <neogfx/gfx/i_graphics_context.hpp>

#include <mod_tracker/sample_view.hpp>

namespace mod_tracker
{
    namespace
    {
        ng::color const BACKGROUND{ 14, 17, 22 };
        ng::color const HEADER_BACKGROUND{ 28, 32, 42 };
        ng::color const HEADER_TEXT{ 150, 158, 175 };
        ng::color const NUMBER{ 255, 196, 92 };
        ng::color const NAME{ 200, 205, 215 };
        ng::color const EMPTY{ 62, 68, 82 };
        ng::color const GLOW{ 60, 120, 200 };

        constexpr float GLOW_DECAY = 0.90f;
        constexpr std::uint32_t VIEW_COLUMNS = 34u;
    }

    sample_view::sample_view(ng::i_layout& aLayout, player& aPlayer) :
        ng::widget<>{ aLayout },
        iPlayer{ aPlayer },
        iFont{ "Consolas", ng::font_style::Normal, 11.0 }
    {
        set_size_policy(ng::size_policy{ ng::size_constraint::Minimum, ng::size_constraint::Expanding });
        set_minimum_size(ng::size{ std::ceil(iFont.max_advance() * VIEW_COLUMNS), 320.0 });
    }

    void sample_view::refresh()
    {
        if (!iPlayer.loaded())
        {
            if (iRow != std::nullopt)
            {
                iRow = std::nullopt;
                iGlow.clear();
                update();
            }
            return;
        }
        auto const& m = iPlayer.song_module();
        auto const row = iPlayer.current_row();
        bool changed = (iRow != row);
        iGlow.resize(m.instrument_mode() ? m.instruments.size() : m.samples.size(), 0.0f);
        for (auto& glow : iGlow)
            if (glow > 0.0f)
            {
                glow = glow * GLOW_DECAY < 0.02f ? 0.0f : glow * GLOW_DECAY;
                changed = true;
            }
        // an instrument (or sample) lights up when a row that strikes it starts playing
        if (iRow != row && iPlayer.current_state() == player::state::Playing)
        {
            auto const& current = iPlayer.song().rows[row];
            for (std::uint32_t channel = 0u; channel < m.channels; ++channel)
            {
                auto const& c = m.at(current.pattern, current.row, channel);
                if (c.instrument != 0u && c.instrument <= iGlow.size() && is_note(c.note))
                    iGlow[c.instrument - 1u] = 1.0f;
            }
        }
        iRow = row;
        if (changed)
            update();
    }

    void sample_view::paint(ng::i_graphics_context& aGc) const
    {
        auto const clientRect = client_rect();
        aGc.fill_rect(clientRect, BACKGROUND);

        auto const charWidth = aGc.text_extent(ng::string{ "0" }, iFont).cx;
        auto const lineHeight = std::ceil(iFont.height() * 1.25);
        auto const textOffset = std::floor((lineHeight - iFont.height()) / 2.0);
        auto draw = [&](ng::point const& aPosition, std::string const& aText, ng::color const& aColor)
            {
                aGc.draw_text(aPosition + ng::point{ 0.0, textOffset }, ng::string{ aText }, iFont, ng::text_format{ aColor });
            };

        // two header lines, to line up with the pattern view's order list and channel headers
        ng::rect const headerRect{ clientRect.top_left(), ng::size{ clientRect.width(), lineHeight * 2.0 } };
        aGc.fill_rect(headerRect, HEADER_BACKGROUND);
        if (!iPlayer.loaded())
            return;
        auto const& m = iPlayer.song_module();
        draw(headerRect.top_left() + ng::point{ charWidth, 0.0 }, m.title.empty() ? std::string{ "(untitled)" } : m.title, NAME);
        auto const format = m.format == module_format::MOD ? (m.signature.empty() ? std::string{ "Soundtracker" } : m.signature) : m.formatName;
        draw(headerRect.top_left() + ng::point{ charWidth, lineHeight },
            format + (m.container.empty() ? std::string{} : " (" + m.container + ")") + ", " + std::to_string(m.channels) + " channels", HEADER_TEXT);

        // the instruments, or the samples if the module has no instruments
        auto const count = m.instrument_mode() ? m.instruments.size() : m.samples.size();
        for (std::size_t index = 0u; index < count; ++index)
        {
            ng::point const position{ clientRect.x + charWidth, headerRect.bottom() + static_cast<double>(index) * lineHeight };
            if (position.y > clientRect.bottom())
                break;
            auto const glow = index < iGlow.size() ? iGlow[index] : 0.0f;
            if (glow > 0.0f)
                aGc.fill_rect(ng::rect{ ng::point{ clientRect.x, position.y }, ng::size{ clientRect.width(), lineHeight } },
                    ng::mix(BACKGROUND, GLOW, static_cast<double>(glow)));
            bool empty = true;
            std::string name;
            if (m.instrument_mode())
            {
                auto const& ins = m.instruments[index];
                name = ins.name;
                for (auto const smp : ins.keyboard)
                    if (smp != 0u && smp <= m.samples.size() && m.samples[smp - 1u].has_data())
                    {
                        empty = false;
                        break;
                    }
            }
            else
            {
                name = m.samples[index].name;
                empty = !m.samples[index].has_data();
            }
            cell numbered;
            numbered.instrument = static_cast<std::uint8_t>(index + 1u);
            draw(position, instrument_text(numbered), empty ? EMPTY : NUMBER);
            draw(position + ng::point{ charWidth * 3.0, 0.0 }, name, empty ? EMPTY : NAME);
        }
    }
}
