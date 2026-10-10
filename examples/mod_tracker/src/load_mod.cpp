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

/*
Portions of this file (the MOD loader (OpenMPT's soundlib/Load_mod.cpp)) are derived from OpenMPT
(https://openmpt.org/).

OpenMPT is distributed under the BSD 3-Clause License:

Copyright (c) 2004-2026, OpenMPT Project Developers and Contributors
Copyright (c) 1997-2003, Olivier Lapicque
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the OpenMPT project nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// ProTracker-family MOD files: the 31-sample formats (M.K., FLTn, nCHN, nnCH, ...) and the original
// 15-sample Ultimate Soundtracker layout, which has no signature at all.

#include <mod_tracker/module.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <optional>

#include "loader.hpp"

namespace mod_tracker::detail
{
    namespace
    {
        constexpr std::size_t TITLE_LENGTH = 20u;
        constexpr std::size_t SAMPLE_HEADER_SIZE = 30u;
        constexpr std::size_t SAMPLE_NAME_LENGTH = 22u;
        constexpr std::size_t ORDER_TABLE_SIZE = 128u;
        constexpr std::size_t CELL_SIZE = 4u;
        constexpr std::uint32_t ROWS = 64u;

        // ProTracker's periods from octave 0 (as FastTracker 2 numbers them) to octave 6; C-1 (period 856
        // in ProTracker's own numbering) is the third row
        constexpr std::uint16_t PERIODS[7u * 12u] =
        {
            2 * 1712, 2 * 1616, 2 * 1524, 2 * 1440, 2 * 1356, 2 * 1280, 2 * 1208, 2 * 1140, 2 * 1076, 2 * 1016, 2 * 960, 2 * 906,
            1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 907,
            856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
            428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
            214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
            107, 101, 95, 90, 85, 80, 75, 71, 67, 63, 60, 56,
            53, 50, 47, 45, 42, 40, 37, 35, 33, 31, 30, 28
        };

        std::optional<std::uint32_t> channels_from_signature(std::string const& aSignature)
        {
            if (aSignature.size() != 4u)
                return {};
            if (aSignature == "M.K." || aSignature == "M!K!" || aSignature == "M&K!" || aSignature == "N.T." ||
                aSignature == "FLT4" || aSignature == "4CHN")
                return 4u;
            if (aSignature == "FLT8" || aSignature == "OCTA" || aSignature == "OKTA" || aSignature == "CD81")
                return 8u;
            auto const digit = [](char aCh) { return aCh >= '0' && aCh <= '9'; };
            // nCHN: 1 to 9 channels
            if (digit(aSignature[0]) && aSignature.substr(1) == "CHN" && aSignature[0] != '0')
                return static_cast<std::uint32_t>(aSignature[0] - '0');
            // nnCH: 10 to 32 channels
            if (digit(aSignature[0]) && digit(aSignature[1]) && aSignature.substr(2) == "CH")
            {
                auto const channels = static_cast<std::uint32_t>((aSignature[0] - '0') * 10 + (aSignature[1] - '0'));
                if (channels >= 1u && channels <= 32u)
                    return channels;
            }
            // TDZn: TakeTracker, 1 to 3 channels
            if (aSignature.substr(0, 3) == "TDZ" && digit(aSignature[3]) && aSignature[3] != '0')
                return static_cast<std::uint32_t>(aSignature[3] - '0');
            return {};
        }

        // the note nearest to a period, as ProTracker's own period table names it
        std::uint8_t period_note(std::uint32_t aPeriod)
        {
            if (aPeriod == 0u || aPeriod == 0xFFFu)
                return NOTE_NONE;
            std::uint32_t note = static_cast<std::uint32_t>(std::size(PERIODS)) + 23u + NOTE_MIN;
            for (std::uint32_t index = 0u; index < std::size(PERIODS); ++index)
            {
                if (aPeriod >= PERIODS[index])
                {
                    if (aPeriod != PERIODS[index] && index != 0u)
                    {
                        auto const p1 = static_cast<std::int32_t>(PERIODS[index - 1u]);
                        auto const p2 = static_cast<std::int32_t>(PERIODS[index]);
                        auto const period = static_cast<std::int32_t>(aPeriod);
                        if (p1 - period < period - p2)
                        {
                            note = index + 23u + NOTE_MIN;
                            break;
                        }
                    }
                    note = index + 24u + NOTE_MIN;
                    break;
                }
            }
            return static_cast<std::uint8_t>(std::min<std::uint32_t>(note, NOTE_MAX));
        }

        bool amiga_note(std::uint8_t aNote)
        {
            return !is_note(aNote) || (aNote >= NOTE_MIDDLE_C - 12u && aNote < NOTE_MIDDLE_C + 24u);
        }

        struct raw_cell
        {
            cell decoded;
            std::uint8_t command;
            std::uint8_t parameter;
        };

        raw_cell decode_cell(reader const& aFile, std::size_t aOffset)
        {
            auto const b0 = aFile.u8(aOffset);
            auto const b1 = aFile.u8(aOffset + 1u);
            auto const b2 = aFile.u8(aOffset + 2u);
            auto const b3 = aFile.u8(aOffset + 3u);
            raw_cell result;
            result.decoded.note = period_note(((b0 & 0x0Fu) << 8u) | b1);
            result.decoded.instrument = static_cast<std::uint8_t>((b0 & 0xF0u) | (b2 >> 4u));
            result.command = static_cast<std::uint8_t>(b2 & 0x0Fu);
            result.parameter = b3;
            return result;
        }
    }

    module_data load_mod(reader const& aFile)
    {
        // 31-sample modules carry a signature at offset 1080; without one this is (at best) a 15-sample module
        std::size_t sampleCount = 31u;
        std::string signature;
        std::optional<std::uint32_t> channels;
        if (aFile.has(1080u, 4u))
        {
            for (std::size_t index = 0u; index < 4u; ++index)
                signature.push_back(static_cast<char>(aFile.u8(1080u + index)));
            channels = channels_from_signature(signature);
        }
        if (channels == std::nullopt)
        {
            sampleCount = 15u;
            signature.clear();
            channels = 4u;
        }

        module_data result;
        result.format = module_format::MOD;
        result.formatName = sampleCount == 15u ? "Soundtracker" : "ProTracker";
        result.channels = *channels;
        result.signature = signature;
        result.title = aFile.text(0u, TITLE_LENGTH);
        result.playBehaviour = default_behaviours(module_format::MOD);
        result.minimumPeriod = 14 * 4;
        result.maximumPeriod = 3424 * 4;
        result.samplePreAmp = std::clamp<std::uint32_t>(256u / result.channels, 32u, 128u);
        setup_amiga_panning(result);
        bool const isMK = (signature == "M.K.");
        bool const isSoundtracker = (sampleCount == 15u);

        std::size_t offset = TITLE_LENGTH;
        result.samples.resize(sampleCount);
        std::vector<std::uint32_t> sampleLengths(sampleCount);
        bool hasEmptySampleWithVolume = false;
        bool hasLongSamples = false;
        for (std::size_t index = 0u; index < sampleCount; ++index, offset += SAMPLE_HEADER_SIZE)
        {
            auto& s = result.samples[index];
            s.name = aFile.text(offset, SAMPLE_NAME_LENGTH);
            auto const length = static_cast<std::uint32_t>(aFile.u16be(offset + 22u)) * 2u;
            sampleLengths[index] = length;
            if (length > 0xFFFFu)
                hasLongSamples = true;
            auto const finetune = static_cast<std::uint8_t>(aFile.u8(offset + 24u) & 0x0Fu);
            s.finetune = static_cast<std::int8_t>(finetune << 4u);
            auto const volume = aFile.u8(offset + 25u);
            // a 15-sample module has no signature to identify it, so be strict about what else is plausible
            if (isSoundtracker && volume > 64u)
                throw module_load_error{ "not a MOD file" };
            s.volume = static_cast<std::uint16_t>(std::min<std::uint8_t>(volume, 64u) * 4u);
            if (length == 0u && volume != 0u)
                hasEmptySampleWithVolume = true;
            auto loopStart = static_cast<std::uint32_t>(aFile.u16be(offset + 26u)) * 2u;
            auto const loopLength = static_cast<std::uint32_t>(aFile.u16be(offset + 28u)) * 2u;
            // a loop start that is wrong in words but right in bytes (as Soundtracker stored it)
            if (loopLength > 2u && loopStart + loopLength > length && loopStart / 2u + loopLength <= length)
                loopStart /= 2u;
            s.length = (length == 2u ? 0u : length);
            if (s.length != 0u)
            {
                s.loopStart = std::min(loopStart, s.length - 1u);
                s.loopEnd = loopStart + loopLength;
                if (s.loopStart > s.loopEnd || s.loopEnd < 4u || s.loopEnd - s.loopStart < 4u)
                    s.loopStart = s.loopEnd = 0u;
                // a tiny loop at the very start of a long sample in a 4-channel module is most likely broken
                if (s.loopEnd <= 8u && s.loopStart == 0u && s.length > s.loopEnd && result.channels == 4u)
                    s.loopEnd = 0u;
                if (s.loopEnd > s.loopStart)
                    s.loop = loop_type::Forward;
            }
            s.c5speed = 8363u;
        }

        auto const songLength = std::min<std::size_t>(aFile.u8(offset), ORDER_TABLE_SIZE);
        auto const restartPosition = aFile.u8(offset + 1u);
        offset += 2u;
        if (songLength == 0u)
            throw module_load_error{ "the song is empty" };
        // ProTracker counts every entry in the order table, not just those in the song, when working out
        // how many patterns are stored
        std::uint32_t patternCount = 0u;
        bool songEnded = false;
        for (std::size_t index = 0u; index < ORDER_TABLE_SIZE; ++index)
        {
            auto const order = aFile.u8(offset + index);
            // 0xFF ("---") ends the song and 0xFE ("+++") is skipped: markers written by some PC trackers
            if (index < songLength && !songEnded)
            {
                if (order == 0xFFu && !result.orders.empty())
                    songEnded = true;
                else if (order != 0xFEu && order != 0xFFu)
                    result.orders.push_back(order);
            }
            if (order < 128u)
                patternCount = std::max<std::uint32_t>(patternCount, order + 1u);
        }
        if (result.orders.empty())
            throw module_load_error{ "the song is empty" };
        result.restart = (restartPosition < result.orders.size() && !(restartPosition == 0x78u && result.channels == 4u)) ? restartPosition : 0u;
        offset += ORDER_TABLE_SIZE;
        if (!isSoundtracker)
            offset += 4u; // the signature
        if (isSoundtracker && patternCount > 64u)
            throw module_load_error{ "not a MOD file" };

        // Startrekker's FLT8 stores each 8-channel pattern as a pair of 4-channel patterns (channels 1-4
        // then 5-8), and its order list counts in those halves
        bool const pairedPatterns = (signature == "FLT8");
        if (pairedPatterns)
            patternCount = (patternCount + 1u) & ~1u;
        auto const storedChannels = pairedPatterns ? 4u : result.channels;
        auto const patternSize = ROWS * storedChannels * CELL_SIZE;
        if (!aFile.has(offset, patternSize * patternCount))
            throw module_load_error{ "pattern data is truncated" };

        // first pass over the patterns, for what tells the trackers apart
        bool onlyAmigaNotes = true;
        bool isNoiseTracker = isMK && !hasEmptySampleWithVolume && !hasLongSamples;
        std::uint8_t maxPanning = 0u;
        bool leftPanning = false;
        bool extendedPanning = false;
        for (std::uint32_t p = 0u; p < patternCount; ++p)
        {
            std::uint32_t patternBreaks = 0u;
            for (std::uint32_t index = 0u; index < ROWS * storedChannels; ++index)
            {
                auto const raw = decode_cell(aFile, offset + (p * ROWS * storedChannels + index) * CELL_SIZE);
                if (!amiga_note(raw.decoded.note))
                    isNoiseTracker = onlyAmigaNotes = false;
                if ((raw.command > 0x06u && raw.command < 0x0Au) || (raw.command == 0x0Eu && raw.parameter > 0x01u) ||
                    (raw.command == 0x0Fu && raw.parameter > 0x1Fu) || (raw.command == 0x0Du && ++patternBreaks > 1u))
                    isNoiseTracker = false;
                if (raw.command == 0x08u)
                {
                    maxPanning = std::max(maxPanning, raw.parameter);
                    if (raw.parameter < 0x80u)
                        leftPanning = true;
                    else if (raw.parameter > 0x8Fu && raw.parameter != 0xA4u)
                        extendedPanning = true;
                }
                else if (raw.command == 0x0Eu && (raw.parameter & 0xF0u) == 0x80u)
                    maxPanning = std::max(maxPanning, static_cast<std::uint8_t>((raw.parameter & 0x0Fu) << 4u));
            }
        }
        constexpr std::uint8_t ENABLE_PANNING_THRESHOLD = 0x30u;
        bool const fix7BitPanning = leftPanning && !extendedPanning && maxPanning >= ENABLE_PANNING_THRESHOLD;

        std::vector<pattern> stored(patternCount);
        for (auto& p : stored)
        {
            p.rows = ROWS;
            p.cells.reserve(ROWS * storedChannels);
            for (std::uint32_t index = 0u; index < ROWS * storedChannels; ++index, offset += CELL_SIZE)
            {
                auto const raw = decode_cell(aFile, offset);
                auto c = raw.decoded;
                std::tie(c.command, c.parameter) = convert_mod_effect(raw.command, raw.parameter);
                // Soundtracker's pattern break had no parameter, nor did NoiseTracker's
                if (c.command == effect::PatternBreak && (isNoiseTracker || isSoundtracker))
                    c.parameter = 0u;
                else if (c.command == effect::Panning && fix7BitPanning)
                {
                    if (c.parameter == 0xA4u)
                    {
                        c.command = effect::ExtendedS3M;
                        c.parameter = 0x91u;
                    }
                    else
                        c.parameter = static_cast<std::uint8_t>(std::min(c.parameter * 2u, 255u));
                }
                p.cells.push_back(c);
            }
        }
        if (pairedPatterns)
        {
            for (std::uint32_t index = 0u; index < patternCount; index += 2u)
            {
                pattern merged;
                merged.rows = ROWS;
                merged.cells.reserve(ROWS * result.channels);
                for (std::uint32_t row = 0u; row < ROWS; ++row)
                    for (auto const* half : { &stored[index], &stored[index + 1u] })
                        merged.cells.insert(merged.cells.end(),
                            std::next(half->cells.begin(), row * storedChannels), std::next(half->cells.begin(), (row + 1u) * storedChannels));
                result.patterns.push_back(std::move(merged));
            }
            for (auto& order : result.orders)
                order = static_cast<std::uint16_t>(order / 2u);
        }
        else
            result.patterns = std::move(stored);

        // ProTracker itself, or something like it
        if (onlyAmigaNotes && (isMK || signature == "M!K!"))
        {
            result.amigaLimits = true;
            result.protrackerMode = true;
            result.playBehaviour.set(behaviour::MODSampleSwap);
            result.playBehaviour.set(behaviour::MODOutOfRangeNoteDelay);
            result.playBehaviour.set(behaviour::MODTempoOnSecondTick);
            // 8xx used only as sync markers
            if (maxPanning < ENABLE_PANNING_THRESHOLD)
            {
                result.playBehaviour.set(behaviour::MODIgnorePanning);
                if (restartPosition != 0x7Fu)
                    result.playBehaviour.set(behaviour::MODOneShotLoops);
            }
        }
        bool const genericMultiChannel = !isMK && signature != "M!K!" && signature != "M&K!" && signature != "N.T." &&
            signature.substr(0, 3) != "FLT" && !isSoundtracker && result.channels != 4u;
        if (genericMultiChannel || isMK || signature == "M!K!")
            result.playBehaviour.set(behaviour::FT2MODTremoloRampWaveform);
        if (isSoundtracker)
        {
            result.playBehaviour.set(behaviour::MODIgnorePanning);
            result.playBehaviour.set(behaviour::MODSampleSwap);
            result.minimumPeriod = 113 * 4;
            result.maximumPeriod = 856 * 4;
        }

        // sample data follows the patterns; a truncated file keeps whatever sample data it does have
        for (std::size_t index = 0u; index < sampleCount; ++index)
        {
            auto& s = result.samples[index];
            auto const storedLength = sampleLengths[index];
            if (s.length != 0u)
            {
                // ProTracker reads past the end of a sample whose loop overruns it, into the next sample
                auto frames = s.length;
                if (isMK && onlyAmigaNotes && !hasEmptySampleWithVolume)
                    frames = std::max(frames, s.loopEnd);
                decode_pcm(aFile.bytes(offset, frames), frames, sample_encoding{}, s);
            }
            offset += storedLength;
        }
        return result;
    }
}
