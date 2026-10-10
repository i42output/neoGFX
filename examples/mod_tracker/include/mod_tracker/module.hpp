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

#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace mod_tracker
{
    // ProTracker-family MOD files: the 31-sample formats (M.K., FLTn, nCHN, nnCH, ...) and the
    // original 15-sample Ultimate Soundtracker layout, which has no signature at all.

    constexpr std::uint32_t ROWS_PER_PATTERN = 64u;
    constexpr std::uint32_t PERIOD_TABLE_NOTES = 36u; // C-1 .. B-3
    constexpr std::uint32_t NO_NOTE = 0xFFu;

    struct module_load_error : std::runtime_error
    {
        module_load_error(std::string const& aReason) : std::runtime_error{ "mod_tracker: " + aReason } {}
    };

    struct sample
    {
        std::string name;
        std::vector<std::int8_t> data;
        std::int8_t finetune = 0;           // -8 .. 7
        std::uint8_t volume = 0;            // 0 .. 64
        std::uint32_t loopStart = 0u;       // in bytes
        std::uint32_t loopLength = 0u;      // in bytes; a loop of two bytes or fewer means "no loop"

        bool looped() const
        {
            return loopLength > 2u;
        }
        std::uint32_t loop_end() const
        {
            return loopStart + loopLength;
        }
    };

    struct cell
    {
        std::uint16_t period = 0u;          // Amiga period, 0 for no note
        std::uint8_t sample = 0u;           // 1-based sample number, 0 for none
        std::uint8_t effect = 0u;           // 0x0 .. 0xF
        std::uint8_t parameter = 0u;
        std::uint8_t note = NO_NOTE;        // index into the period table, NO_NOTE if the period isn't in it

        bool has_effect() const
        {
            return effect != 0u || parameter != 0u;
        }
    };

    struct pattern
    {
        std::vector<cell> cells;            // ROWS_PER_PATTERN rows of channel_count() cells
    };

    struct module_data
    {
        std::string title;
        std::string signature;              // empty for 15-sample modules
        std::uint32_t channels = 4u;
        std::vector<sample> samples;        // 15 or 31; sample number n is samples[n - 1]
        std::vector<std::uint8_t> orders;   // the song: one pattern number per position
        std::uint8_t restart = 0u;
        std::vector<pattern> patterns;

        cell const& at(std::uint32_t aPattern, std::uint32_t aRow, std::uint32_t aChannel) const
        {
            return patterns[aPattern].cells[aRow * channels + aChannel];
        }
    };

    module_data load_module(std::span<std::byte const> aData);
    module_data load_module(std::string const& aPath);

    // the ProTracker period table for finetune 0, C-1 .. B-3
    std::array<std::uint16_t, PERIOD_TABLE_NOTES> const& base_periods();
    // the period of aNote (0 .. 35) at aFinetune (-8 .. 7)
    std::uint16_t note_period(std::uint32_t aNote, std::int32_t aFinetune);
    // the note nearest to aPeriod at aFinetune, clamped to the table
    std::uint32_t period_note(std::uint32_t aPeriod, std::int32_t aFinetune);

    // tracker-style text for one cell: "C-2 01 A0F", with dots for empty fields
    std::string note_text(cell const& aCell);
    std::string sample_text(cell const& aCell);
    std::string effect_text(cell const& aCell);
}
