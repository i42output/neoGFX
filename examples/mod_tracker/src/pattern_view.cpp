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

#include <mod_tracker/pattern_view.hpp>

namespace mod_tracker
{
    namespace
    {
        ng::color const BACKGROUND{ 14, 17, 22 };
        ng::color const BEAT_BACKGROUND{ 22, 26, 34 };
        ng::color const PLAYHEAD_BACKGROUND{ 44, 56, 92 };
        ng::color const HEADER_BACKGROUND{ 28, 32, 42 };
        ng::color const ROW_NUMBER{ 110, 118, 135 };
        ng::color const BEAT_ROW_NUMBER{ 170, 178, 195 };
        ng::color const NOTE{ 235, 238, 245 };
        ng::color const SAMPLE{ 255, 196, 92 };
        ng::color const EFFECT{ 118, 200, 255 };
        ng::color const EMPTY{ 62, 68, 82 };
        ng::color const ORDER_TEXT{ 150, 158, 175 };
        ng::color const ORDER_CURRENT{ 255, 255, 255 };
        ng::color const METER_LOW{ 60, 200, 110 };
        ng::color const METER_HIGH{ 255, 120, 60 };
        ng::color const MUTED{ 210, 72, 72 };

        // how quickly a channel meter falls back after a peak, per frame
        constexpr float METER_DECAY = 0.88f;

        // columns of text per channel in each layout, including the gap after it
        constexpr std::uint32_t FULL_CELL_COLUMNS = 12u;     // "C-2 01 A0F  "
        constexpr std::uint32_t COMPACT_CELL_COLUMNS = 8u;   // "C-2 01  "
        constexpr std::uint32_t MINIMAL_CELL_COLUMNS = 4u;   // "C-2 "
        constexpr std::uint32_t ROW_NUMBER_COLUMNS = 4u;     // "00  "

        std::string digits(std::uint32_t aValue, std::uint32_t aDigits)
        {
            std::string result(aDigits, '0');
            for (auto digit = result.rbegin(); digit != result.rend() && aValue != 0u; ++digit, aValue /= 10u)
                *digit = static_cast<char>('0' + aValue % 10u);
            return result;
        }
    }

    pattern_view::pattern_view(ng::i_layout& aLayout, player& aPlayer) :
        ng::widget<>{ aLayout },
        iPlayer{ aPlayer },
        iFont{ "Consolas", ng::font_style::Normal, 11.0 }
    {
        set_size_policy(ng::size_constraint::Expanding);
        set_minimum_size(ng::size{ 480.0, 320.0 });
    }

    void pattern_view::refresh()
    {
        if (!iPlayer.loaded())
        {
            if (iRow != std::nullopt)
            {
                iRow = std::nullopt;
                iMeters.clear();
                update();
            }
            return;
        }
        auto const& song = iPlayer.song();
        auto const row = iPlayer.current_row();
        bool changed = (iRow != row);
        iRow = row;
        // a channel's meter jumps to the loudest the channel has been since the last frame and then decays,
        // so that short notes are still seen; nothing moves while the song isn't playing
        iMeters.resize(song.channels, 0.0f);
        bool const playing = (iPlayer.current_state() == player::state::Playing);
        for (std::uint32_t channel = 0u; channel < song.channels; ++channel)
        {
            auto const level = playing ? iPlayer.take_level(channel) : 0.0f;
            auto const meter = std::max(level, iMeters[channel] * METER_DECAY);
            if (std::abs(meter - iMeters[channel]) > 0.002f)
            {
                iMeters[channel] = meter < 0.01f ? 0.0f : meter;
                changed = true;
            }
        }
        if (changed)
            update();
    }

    void pattern_view::paint(ng::i_graphics_context& aGc) const
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

        if (!iPlayer.loaded() || iRow == std::nullopt)
        {
            std::string const hint = "Open a ProTracker module to begin";
            auto const extents = aGc.text_extent(ng::string{ hint }, iFont);
            draw(ng::point{ clientRect.center().x - extents.cx / 2.0, clientRect.center().y - lineHeight / 2.0 }, hint, ORDER_TEXT);
            return;
        }

        auto const& m = iPlayer.song_module();
        auto const& song = iPlayer.song();
        auto const& current = song.rows[*iRow];

        // order list: the pattern played at each position, the current position highlighted
        iOrderListRect = ng::rect{ clientRect.top_left(), ng::size{ clientRect.width(), lineHeight } };
        // pattern numbers go to three digits only in the largest multi-channel modules
        auto const patternDigits = m.patterns.size() > 100u ? 3u : 2u;
        iOrderCellWidth = charWidth * (patternDigits + 1u);
        aGc.fill_rect(iOrderListRect, HEADER_BACKGROUND);
        auto const ordersShown = static_cast<std::uint32_t>(std::max(1.0, std::floor((clientRect.width() - charWidth) / iOrderCellWidth)));
        iFirstOrderShown = current.orderPosition >= ordersShown / 2u ? current.orderPosition - ordersShown / 2u : 0u;
        if (iFirstOrderShown + ordersShown > m.orders.size())
            iFirstOrderShown = m.orders.size() > ordersShown ? static_cast<std::uint32_t>(m.orders.size()) - ordersShown : 0u;
        for (std::uint32_t order = iFirstOrderShown; order < m.orders.size() && order < iFirstOrderShown + ordersShown; ++order)
        {
            ng::point const position{ clientRect.x + charWidth / 2.0 + (order - iFirstOrderShown) * iOrderCellWidth, clientRect.y };
            if (order == current.orderPosition)
                aGc.fill_rect(ng::rect{ position - ng::point{ charWidth / 2.0, 0.0 }, ng::size{ iOrderCellWidth, lineHeight } }, PLAYHEAD_BACKGROUND);
            draw(position, digits(m.orders[order], patternDigits), order == current.orderPosition ? ORDER_CURRENT : ORDER_TEXT);
        }

        // pick the widest layout the channels fit in
        auto const available = clientRect.width() / charWidth - ROW_NUMBER_COLUMNS;
        auto const cellColumns =
            m.channels * FULL_CELL_COLUMNS <= available ? FULL_CELL_COLUMNS :
            m.channels * COMPACT_CELL_COLUMNS <= available ? COMPACT_CELL_COLUMNS : MINIMAL_CELL_COLUMNS;
        auto const cellWidth = charWidth * cellColumns;
        auto const patternLeft = clientRect.x + charWidth * ROW_NUMBER_COLUMNS;

        // channel headers, each with a level meter
        ng::rect const headerRect{ ng::point{ clientRect.x, iOrderListRect.bottom() }, ng::size{ clientRect.width(), lineHeight } };
        iChannelHeaderRect = headerRect;
        iChannelLeft = patternLeft;
        iChannelWidth = cellWidth;
        aGc.fill_rect(headerRect, HEADER_BACKGROUND.darker(0x08));
        for (std::uint32_t channel = 0u; channel < m.channels; ++channel)
        {
            ng::point const position{ patternLeft + channel * cellWidth, headerRect.y };
            // the channel number, then its meter in the rest of the column (all meter when space is short)
            auto const label = (cellColumns == MINIMAL_CELL_COLUMNS ? std::string{} : "Ch" + std::to_string(channel + 1u));
            auto const labelColumns = label.empty() ? 0.0 : static_cast<double>(label.size() + 1u);
            auto const meterHeight = std::max(2.0, std::floor(lineHeight * 0.3));
            ng::rect const meterRect{
                position + ng::point{ charWidth * labelColumns, std::floor((lineHeight - meterHeight) / 2.0) },
                ng::size{ cellWidth - charWidth * (labelColumns + 1.0), meterHeight } };
            bool const muted = iPlayer.muted(channel);
            draw(position, label, muted ? MUTED : ORDER_TEXT);
            aGc.fill_rect(meterRect, muted ? MUTED.darker(0x80) : BACKGROUND);
            auto const level = channel < iMeters.size() && !muted ? iMeters[channel] : 0.0f;
            if (level > 0.0f)
                aGc.fill_rect(ng::rect{ meterRect.top_left(), ng::size{ meterRect.width() * level, meterRect.height() } },
                    ng::mix(METER_LOW, METER_HIGH, static_cast<double>(level)));
        }

        // the pattern, scrolling past the playhead row in the middle
        ng::rect const patternRect{ ng::point{ clientRect.x, headerRect.bottom() }, ng::point{ clientRect.right(), clientRect.bottom() } };
        auto const playheadY = patternRect.y + std::floor((patternRect.height() - lineHeight) / 2.0 / lineHeight) * lineHeight;
        auto const rowsAbove = static_cast<std::int32_t>((playheadY - patternRect.y) / lineHeight);
        auto const rowsBelow = static_cast<std::int32_t>((patternRect.bottom() - playheadY) / lineHeight);
        for (std::int32_t offset = -rowsAbove; offset < rowsBelow; ++offset)
        {
            auto const row = static_cast<std::int32_t>(current.row) + offset;
            if (row < 0 || row >= static_cast<std::int32_t>(ROWS_PER_PATTERN))
                continue;
            auto const y = playheadY + offset * lineHeight;
            bool const beat = (row % 4 == 0);
            if (offset == 0)
                aGc.fill_rect(ng::rect{ ng::point{ clientRect.x, y }, ng::size{ clientRect.width(), lineHeight } }, PLAYHEAD_BACKGROUND);
            else if (beat)
                aGc.fill_rect(ng::rect{ ng::point{ clientRect.x, y }, ng::size{ clientRect.width(), lineHeight } }, BEAT_BACKGROUND);
            draw(ng::point{ clientRect.x + charWidth, y }, digits(static_cast<std::uint32_t>(row), 2u), beat ? BEAT_ROW_NUMBER : ROW_NUMBER);
            for (std::uint32_t channel = 0u; channel < m.channels; ++channel)
            {
                auto const& c = m.at(current.pattern, static_cast<std::uint32_t>(row), channel);
                ng::point const position{ patternLeft + channel * cellWidth, y };
                draw(position, note_text(c), c.period != 0u ? NOTE : EMPTY);
                if (cellColumns == MINIMAL_CELL_COLUMNS)
                    continue;
                draw(position + ng::point{ charWidth * 4.0, 0.0 }, sample_text(c), c.sample != 0u ? SAMPLE : EMPTY);
                if (cellColumns == COMPACT_CELL_COLUMNS)
                    continue;
                draw(position + ng::point{ charWidth * 7.0, 0.0 }, effect_text(c), c.has_effect() ? EFFECT : EMPTY);
            }
        }
    }

    ng::focus_policy pattern_view::focus_policy() const
    {
        return ng::focus_policy::StrongFocus;
    }

    bool pattern_view::key_pressed(ng::scan_code_e aScanCode, ng::key_code_e aKeyCode, ng::key_modifier aKeyModifier)
    {
        if (!iPlayer.loaded())
            return widget<>::key_pressed(aScanCode, aKeyCode, aKeyModifier);
        auto const order = static_cast<std::int32_t>(iPlayer.song().rows[iPlayer.current_row()].orderPosition);
        switch (aKeyCode)
        {
        case ng::key_code_e::KeyCode_SPACE:
            if (iPlayer.current_state() == player::state::Playing)
                iPlayer.pause();
            else
                iPlayer.play();
            return true;
        case ng::key_code_e::KeyCode_LEFT:
            iPlayer.seek_order(order - 1);
            return true;
        case ng::key_code_e::KeyCode_RIGHT:
            iPlayer.seek_order(order + 1);
            return true;
        case ng::key_code_e::KeyCode_UP:
            iPlayer.seek_relative(-1);
            return true;
        case ng::key_code_e::KeyCode_DOWN:
            iPlayer.seek_relative(1);
            return true;
        case ng::key_code_e::KeyCode_PAGEUP:
            iPlayer.seek_relative(-16);
            return true;
        case ng::key_code_e::KeyCode_PAGEDOWN:
            iPlayer.seek_relative(16);
            return true;
        case ng::key_code_e::KeyCode_HOME:
            iPlayer.seek(0u);
            return true;
        default:
            return widget<>::key_pressed(aScanCode, aKeyCode, aKeyModifier);
        }
    }

    bool pattern_view::mouse_wheel_scrolled(ng::mouse_wheel aWheel, const ng::point& aPosition, ng::delta aDelta, ng::key_modifier aKeyModifier)
    {
        if (!iPlayer.loaded() || aWheel != ng::mouse_wheel::Vertical || aDelta.dy == 0.0)
            return widget<>::mouse_wheel_scrolled(aWheel, aPosition, aDelta, aKeyModifier);
        iPlayer.seek_relative(aDelta.dy > 0.0 ? -1 : 1);
        return true;
    }

    void pattern_view::mouse_button_clicked(ng::mouse_button aButton, const ng::point& aPosition, ng::key_modifier aKeyModifier)
    {
        widget<>::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        if (aButton != ng::mouse_button::Left || !iPlayer.loaded())
            return;
        if (iOrderListRect.contains(aPosition) && iOrderCellWidth > 0.0)
        {
            auto const column = static_cast<std::int32_t>(std::floor((aPosition.x - iOrderListRect.x) / iOrderCellWidth));
            iPlayer.seek_order(static_cast<std::int32_t>(iFirstOrderShown) + column);
        }
        else if (iChannelHeaderRect.contains(aPosition) && iChannelWidth > 0.0 && aPosition.x >= iChannelLeft)
        {
            auto const channel = static_cast<std::uint32_t>(std::floor((aPosition.x - iChannelLeft) / iChannelWidth));
            if (channel < iPlayer.song_module().channels)
            {
                iPlayer.set_muted(channel, !iPlayer.muted(channel));
                update();
            }
        }
    }
}
