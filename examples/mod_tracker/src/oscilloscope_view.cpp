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

#include <neogfx/gfx/i_graphics_context.hpp>

#include <mod_tracker/oscilloscope_view.hpp>

namespace mod_tracker
{
    namespace
    {
        ng::color const BACKGROUND{ 14, 17, 22 };
        ng::color const HEADER_BACKGROUND{ 28, 32, 42 };
        ng::color const HEADER_TEXT{ 150, 158, 175 };
        ng::color const GRID{ 30, 35, 46 };
        ng::color const AXIS{ 46, 53, 68 };
        ng::color const TRACE{ 80, 200, 160 };

        // the trace is 1024 frames (23 ms at 44.1 kHz) long; it starts at the latest rising zero crossing that
        // leaves a whole trace after it, looked for in the frames before that (up to 70 ms before the playhead)
        constexpr std::size_t WINDOW_FRAMES = 4096u;
        constexpr std::size_t TRACE_FRAMES = 1024u;
        // a crossing only counts if the signal was at least this far below zero in the frames just before it,
        // so that noise around zero doesn't trigger
        constexpr float TRIGGER_ARM = 0.01f;
        constexpr std::size_t TRIGGER_ARM_FRAMES = 32u;
    }

    oscilloscope_view::oscilloscope_view(ng::i_layout& aLayout, player& aPlayer, std::uint32_t aChannel, std::string const& aTitle) :
        ng::widget<>{ aLayout },
        iPlayer{ aPlayer },
        iChannel{ aChannel },
        iTitle{ aTitle },
        iFont{ "Consolas", ng::font_style::Normal, 9.0 },
        iSamples(WINDOW_FRAMES)
    {
        set_size_policy(ng::size_policy{ ng::size_constraint::Expanding, ng::size_constraint::Minimum });
        set_minimum_size(ng::size{ 200.0, 150.0 });
    }

    void oscilloscope_view::refresh()
    {
        if (!iPlayer.output_samples(iChannel, iSamples.data(), WINDOW_FRAMES))
        {
            if (iLive)
            {
                iLive = false;
                update();
            }
            return;
        }
        iLive = true;
        // free running if nothing triggers
        auto const latest = WINDOW_FRAMES - TRACE_FRAMES;
        iStart = latest;
        for (auto frame = latest; frame > TRIGGER_ARM_FRAMES; --frame)
            if (iSamples[frame - 1u] < 0.0f && iSamples[frame] >= 0.0f &&
                *std::min_element(std::next(iSamples.begin(), static_cast<std::ptrdiff_t>(frame - TRIGGER_ARM_FRAMES)),
                    std::next(iSamples.begin(), static_cast<std::ptrdiff_t>(frame))) < -TRIGGER_ARM)
            {
                iStart = frame;
                break;
            }
        update();
    }

    void oscilloscope_view::paint(ng::i_graphics_context& aGc) const
    {
        auto const clientRect = client_rect();
        aGc.fill_rect(clientRect, BACKGROUND);

        auto const charWidth = aGc.text_extent(ng::string{ "0" }, iFont).cx;
        auto const lineHeight = std::ceil(iFont.height() * 1.25);
        auto const textOffset = std::floor((lineHeight - iFont.height()) / 2.0);

        ng::rect const headerRect{ clientRect.top_left(), ng::size{ clientRect.width(), lineHeight } };
        aGc.fill_rect(headerRect, HEADER_BACKGROUND);
        aGc.draw_text(headerRect.top_left() + ng::point{ charWidth, textOffset }, ng::string{ iTitle }, iFont, ng::text_format{ HEADER_TEXT });

        auto const left = std::floor(clientRect.x + charWidth);
        auto const top = std::floor(headerRect.bottom() + lineHeight / 2.0);
        auto const right = std::floor(clientRect.x + clientRect.width() - charWidth);
        auto const bottom = std::floor(clientRect.bottom() - lineHeight / 2.0);
        if (right - left < 8.0 || bottom - top < 8.0)
            return;
        ng::rect const plot{ ng::point{ left, top }, ng::size{ right - left, bottom - top } };
        auto const middle = std::floor(plot.y + plot.height() / 2.0);
        auto const halfHeight = plot.height() / 2.0;

        // full scale at the top and bottom, half scale between, and a division every 2 ms (at 44.1 kHz)
        for (auto const level : { -1.0, -0.5, 0.5, 1.0 })
            aGc.fill_rect(ng::rect{ ng::point{ plot.x, std::min(std::floor(middle - level * halfHeight), plot.bottom() - 1.0) }, ng::size{ plot.width(), 1.0 } }, GRID);
        for (std::size_t division = 1u; division * 88u < TRACE_FRAMES; ++division)
            aGc.fill_rect(ng::rect{ ng::point{ std::floor(plot.x + plot.width() * (division * 88.0) / TRACE_FRAMES), plot.y }, ng::size{ 1.0, plot.height() } }, GRID);
        aGc.fill_rect(ng::rect{ ng::point{ plot.x, middle }, ng::size{ plot.width(), 1.0 } }, AXIS);
        if (!iLive)
            return;

        // one vertical span per pixel column, from the lowest to the highest sample it covers (and the last
        // sample of the column before, so that the trace joins up)
        auto const y_of = [&](float aSample)
            {
                return middle - std::clamp(static_cast<double>(aSample), -1.0, 1.0) * halfHeight;
            };
        auto const columns = static_cast<std::size_t>(plot.width());
        for (std::size_t column = 0u; column < columns; ++column)
        {
            auto const first = iStart + column * TRACE_FRAMES / columns;
            auto const last = std::max(iStart + (column + 1u) * TRACE_FRAMES / columns, first + 1u);
            auto low = iSamples[first > iStart ? first - 1u : first];
            auto high = low;
            for (auto frame = first; frame < last; ++frame)
            {
                low = std::min(low, iSamples[frame]);
                high = std::max(high, iSamples[frame]);
            }
            auto const y1 = std::floor(y_of(high));
            auto const y2 = std::floor(y_of(low)) + 1.0;
            aGc.fill_rect(ng::rect{ ng::point{ plot.x + column, y1 }, ng::size{ 1.0, y2 - y1 } }, TRACE);
        }
    }
}
