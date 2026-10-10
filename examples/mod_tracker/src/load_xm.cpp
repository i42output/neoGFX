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
Portions of this file (the XM loader (OpenMPT's soundlib/Load_xm.cpp)) are derived from OpenMPT
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

// FastTracker 2 extended modules: up to 32 channels, patterns of up to 256 rows, instruments made of
// several samples with volume and panning envelopes, and linear or Amiga frequency slides.

#include <mod_tracker/module.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <optional>
#include <string_view>

#include "loader.hpp"

namespace mod_tracker::detail
{
    namespace
    {
        constexpr std::size_t INSTRUMENT_HEADER_SIZE = 263u;
        constexpr std::size_t SAMPLE_HEADER_SIZE = 40u;

        // the trackers an XM may have been saved with, as far as can be told
        constexpr std::uint32_t VER_UNKNOWN = 0x000u;
        constexpr std::uint32_t VER_OLD_MODPLUG = 0x001u;
        constexpr std::uint32_t VER_NEW_MODPLUG = 0x002u;
        constexpr std::uint32_t VER_MODPLUG_BIDI_FLAG = 0x004u;
        constexpr std::uint32_t VER_OPENMPT = 0x008u;
        constexpr std::uint32_t VER_CONFIRMED = 0x010u;
        constexpr std::uint32_t VER_FT2_GENERIC = 0x020u;
        constexpr std::uint32_t VER_FT2_CLONE = 0x040u;
        constexpr std::uint32_t VER_PLAYERPRO = 0x080u;
        constexpr std::uint32_t VER_DIGITRAKKER = 0x100u;
        constexpr std::uint32_t VER_EMPTY_ORDERS = 0x200u;

        // "1.28.03.00" as OpenMPT writes its version: each part in hexadecimal
        std::uint32_t parse_openmpt_version(std::string const& aText)
        {
            std::uint32_t result = 0u;
            std::uint32_t part = 0u;
            std::uint32_t parts = 0u;
            for (auto ch : aText + ".")
            {
                if (ch == '.')
                {
                    result = (result << 8u) | (part & 0xFFu);
                    part = 0u;
                    if (++parts == 4u)
                        break;
                }
                else if (ch >= '0' && ch <= '9')
                    part = part * 16u + static_cast<std::uint32_t>(ch - '0');
                else if (ch >= 'A' && ch <= 'F')
                    part = part * 16u + static_cast<std::uint32_t>(ch - 'A' + 10);
                else if (ch >= 'a' && ch <= 'f')
                    part = part * 16u + static_cast<std::uint32_t>(ch - 'a' + 10);
                else
                    break;
            }
            while (parts < 4u)
            {
                result <<= 8u;
                ++parts;
            }
            return result;
        }

        void read_envelope(reader const& aFile, std::size_t aPoints, std::uint8_t aCount, std::uint8_t aFlags,
            std::uint8_t aSustain, std::uint8_t aLoopStart, std::uint8_t aLoopEnd, envelope& aEnvelope)
        {
            auto const count = std::min<std::uint8_t>(aCount, 12u);
            aEnvelope.points.resize(count);
            for (std::uint32_t index = 0u; index < count; ++index)
            {
                auto& point = aEnvelope.points[index];
                point.tick = aFile.u16le(aPoints + index * 4u);
                point.value = static_cast<std::uint8_t>(std::min<std::uint16_t>(aFile.u16le(aPoints + index * 4u + 2u), 64u));
                // some editors only saved the low byte of the position
                if (index > 0u && point.tick < aEnvelope.points[index - 1u].tick && (point.tick & 0xFF00u) == 0u)
                {
                    point.tick = static_cast<std::uint16_t>(point.tick | (aEnvelope.points[index - 1u].tick & 0xFF00u));
                    if (point.tick < aEnvelope.points[index - 1u].tick)
                        point.tick = static_cast<std::uint16_t>(point.tick + 0x100u);
                }
            }
            aEnvelope.enabled = (aFlags & 0x01u) != 0u && !aEnvelope.points.empty();
            if (aSustain < 12u)
            {
                aEnvelope.sustain = (aFlags & 0x02u) != 0u;
                aEnvelope.sustainStart = aEnvelope.sustainEnd = aSustain;
            }
            if (aLoopEnd < 12u && aLoopEnd >= aLoopStart)
            {
                aEnvelope.loop = (aFlags & 0x04u) != 0u;
                aEnvelope.loopStart = aLoopStart;
                aEnvelope.loopEnd = aLoopEnd;
            }
        }
    }

    bool is_xm(reader const& aFile)
    {
        return aFile.has(0u, 80u) && aFile.magic(0u, "Extended Module: ") && aFile.u16le(58u) >= 0x0102u && aFile.u16le(58u) <= 0x0104u &&
            aFile.u16le(68u) != 0u && aFile.u16le(68u) <= 128u;
    }

    module_data load_xm(reader const& aFile)
    {
        module_data result;
        result.format = module_format::XM;
        result.formatName = "FastTracker 2";
        result.title = aFile.text(17u, 20u);
        auto const trackerName = aFile.text(38u, 20u);
        auto const version = aFile.u16le(58u);
        auto const headerSize = aFile.u32le(60u);
        auto const orderCount = aFile.u16le(64u);
        auto const restartPosition = aFile.u16le(66u);
        result.channels = std::min<std::uint32_t>(aFile.u16le(68u), MAX_CHANNELS);
        auto const patternCount = aFile.u16le(70u);
        auto const instrumentCount = aFile.u16le(72u);
        auto const flags = aFile.u16le(74u);
        auto const speed = aFile.u16le(76u);
        auto const tempo = aFile.u16le(78u);
        if (version < 0x0104u)
            throw module_load_error{ "XM files older than version 1.04 aren't supported" };

        result.playBehaviour = default_behaviours(module_format::XM);
        result.mixLevels = mix_levels::Compatible;

        // Which tracker saved it decides how it's played (ModPlug Tracker's XMs, for one, don't play as
        // FastTracker 2 does), and is told by the tell-tale signs each leaves, as OpenMPT reads them.
        std::uint32_t madeWith = VER_UNKNOWN;
        std::uint32_t lastSaved = 0u;
        auto const made_with = [&](std::uint32_t aFlags) { return (madeWith & aFlags) != 0u; };
        auto const raw_equals = [&](std::size_t aOffset, std::string_view aText)
            {
                for (std::size_t index = 0u; index < aText.size(); ++index)
                    if (aFile.u8(aOffset + index) != static_cast<std::uint8_t>(aText[index]))
                        return false;
                return true;
            };
        std::array<char, 20u> songName = {};
        for (std::size_t index = 0u; index < songName.size(); ++index)
            songName[index] = static_cast<char>(aFile.u8(17u + index));
        std::string_view const songNameView{ songName.data(), songName.size() };
        if (raw_equals(38u, "FastTracker v2.00   ") && headerSize == 276u)
        {
            if (auto const firstNull = songNameView.find('\0'); firstNull != std::string_view::npos)
            {
                // FastTracker 2 pads the title with spaces; others use null characters
                if (restartPosition != 0u)
                    madeWith = VER_FT2_CLONE | VER_NEW_MODPLUG | VER_EMPTY_ORDERS;
                else if (firstNull == songNameView.size() - 1u)
                    madeWith = VER_FT2_CLONE | VER_NEW_MODPLUG | VER_PLAYERPRO | VER_EMPTY_ORDERS;
                else if (songNameView.find_first_not_of(' ', firstNull + 1u) == std::string_view::npos)
                    madeWith = VER_PLAYERPRO | VER_CONFIRMED;
                else
                    madeWith = VER_FT2_CLONE | VER_NEW_MODPLUG | VER_EMPTY_ORDERS;
            }
            else if (restartPosition != 0u)
                madeWith = VER_FT2_GENERIC | VER_NEW_MODPLUG;
            else
                madeWith = VER_FT2_GENERIC | VER_NEW_MODPLUG | VER_PLAYERPRO;
        }
        else if (raw_equals(38u, "FastTracker v 2.00  "))
            madeWith = VER_OLD_MODPLUG;
        else
        {
            madeWith = VER_UNKNOWN | VER_CONFIRMED;
            result.tracker = trackerName;
            if (raw_equals(38u, "OpenMPT "))
                madeWith = VER_OPENMPT | VER_CONFIRMED | VER_EMPTY_ORDERS;
            else if (raw_equals(38u, "MilkyTracker "))
            {
                // MilkyTracker has used FastTracker 2's pan law since it started saving its version
                if (!raw_equals(50u, "        "))
                    result.mixLevels = mix_levels::CompatibleFT2;
            }
            else if (raw_equals(38u, "Fasttracker II clone"))
                madeWith = VER_FT2_GENERIC | VER_CONFIRMED;
            else if (raw_equals(38u, std::string_view{ "MadTracker 2.0\0", 15u }))
            {
                result.playBehaviour.reset(behaviour::FT2PortaNoNote);
                result.playBehaviour.reset(behaviour::FT2Arpeggio);
            }
            else if (raw_equals(38u, std::string_view{ "Skale Tracker\0", 14u }) || raw_equals(38u, std::string_view{ "Sk@le Tracker\0", 14u }))
            {
                result.playBehaviour.reset(behaviour::FT2ST3OffsetOutOfRange);
                result.playBehaviour.reset(behaviour::FT2Arpeggio);
            }
            else if (raw_equals(38u, "*Converted ") && raw_equals(52u, "-File*"))
                madeWith = VER_DIGITRAKKER | VER_CONFIRMED;
        }
        result.minimumPeriod = 1;
        result.maximumPeriod = 31999;
        result.linearSlides = (flags & 0x0001u) != 0u;
        result.extendedFilterRange = (flags & 0x1000u) != 0u;
        if (result.extendedFilterRange && made_with(VER_NEW_MODPLUG))
            madeWith = VER_FT2_CLONE | VER_NEW_MODPLUG | VER_CONFIRMED | VER_EMPTY_ORDERS;
        if (speed != 0u)
            result.speed = speed;
        if (tempo != 0u)
            result.tempo = std::clamp<double>(tempo, 32.0, 512.0);
        result.channelSettings.assign(result.channels, channel_settings{});
        result.reset_macros();

        for (std::uint32_t index = 0u; index < std::min<std::uint32_t>(orderCount, 256u); ++index)
            result.orders.push_back(aFile.u8(80u + index));
        if (result.orders.empty() && !made_with(VER_EMPTY_ORDERS))
            result.orders.push_back(0u);
        result.restart = restartPosition < result.orders.size() ? restartPosition : 0u;

        // patterns
        std::size_t offset = 60u + headerSize;
        result.patterns.resize(patternCount);
        for (auto& p : result.patterns)
        {
            auto const patternHeaderSize = aFile.u32le(offset);
            if (patternHeaderSize < 8u)
                throw module_load_error{ "bad XM pattern header" };
            auto rows = static_cast<std::uint32_t>(aFile.u16le(offset + 5u));
            auto const packedSize = aFile.u16le(offset + 7u);
            if (rows == 0u)
                rows = 64u;
            rows = std::min(rows, 1024u);
            offset += patternHeaderSize;
            p.rows = rows;
            p.cells.assign(static_cast<std::size_t>(rows) * result.channels, cell{});
            std::size_t const end = offset + packedSize;
            cursor data{ aFile, offset };
            auto const fileChannels = static_cast<std::uint32_t>(aFile.u16le(68u));
            if (packedSize != 0u)
            {
                for (std::uint32_t row = 0u; row < rows; ++row)
                    for (std::uint32_t channel = 0u; channel < fileChannels; ++channel)
                    {
                        if (data.offset() >= end || !data.can_read(1u))
                            break;
                        cell dummy;
                        cell& c = channel < result.channels ? p.cells[row * result.channels + channel] : dummy;
                        auto info = data.u8();
                        std::uint8_t note = 0u;
                        std::uint8_t volume = 0u;
                        std::uint8_t command = 0u;
                        std::uint8_t parameter = 0u;
                        if ((info & 0x80u) != 0u)
                        {
                            if ((info & 0x01u) != 0u)
                                note = data.u8();
                        }
                        else
                        {
                            note = info;
                            info = 0xFFu;
                        }
                        if ((info & 0x02u) != 0u)
                            c.instrument = data.u8();
                        if ((info & 0x04u) != 0u)
                            volume = data.u8();
                        if ((info & 0x08u) != 0u)
                            command = data.u8();
                        if ((info & 0x10u) != 0u)
                            parameter = data.u8();
                        if (note == 97u)
                            c.note = NOTE_OFF;
                        else if (note > 0u && note < 97u)
                            c.note = static_cast<std::uint8_t>(note + 12u);
                        if ((command | parameter) != 0u)
                            std::tie(c.command, c.parameter) = convert_mod_effect(command, parameter);
                        if (c.instrument == 0xFFu)
                            c.instrument = 0u;
                        if (volume >= 0x10u && volume <= 0x50u)
                        {
                            c.volumeCommand = volume_command::Volume;
                            c.volume = static_cast<std::uint8_t>(volume - 0x10u);
                        }
                        else if (volume >= 0x60u)
                        {
                            static constexpr volume_command sCommands[] =
                            {
                                volume_command::VolumeSlideDown, volume_command::VolumeSlideUp, volume_command::FineVolumeDown, volume_command::FineVolumeUp,
                                volume_command::VibratoSpeed, volume_command::VibratoDepth, volume_command::Panning, volume_command::PanSlideLeft,
                                volume_command::PanSlideRight, volume_command::TonePortamento
                            };
                            c.volumeCommand = sCommands[(volume - 0x60u) >> 4u];
                            c.volume = static_cast<std::uint8_t>(volume & 0x0Fu);
                            if (c.volumeCommand == volume_command::Panning)
                                c.volume = static_cast<std::uint8_t>(c.volume * 4u);
                        }
                    }
            }
            offset = end;
        }

        // instruments, each followed by its sample headers and then their data
        std::uint8_t sampleReserved = 0u;
        std::int32_t lastInstrumentType = -1;
        std::int32_t lastSampleReserved = -1;
        std::int64_t lastSampleHeaderSize = -1;
        bool instrumentWithSamplesEncountered = false;
        result.instruments.resize(instrumentCount);
        for (auto& ins : result.instruments)
        {
            if (!aFile.has(offset, 4u))
                break;
            auto size = static_cast<std::size_t>(aFile.u32le(offset));
            if (size == 0u)
                size = INSTRUMENT_HEADER_SIZE;
            // the header may be shorter than the full structure, the rest being zeroes
            std::vector<std::byte> header(std::max(size, INSTRUMENT_HEADER_SIZE), std::byte{ 0 });
            auto const available = aFile.bytes(offset, size);
            std::copy(available.begin(), available.end(), header.begin());
            reader const h{ std::span<std::byte const>{ header } };
            offset += size;
            ins.name = h.text(4u, 22u);
            auto const sampleCount = h.u16le(27u);
            auto const rawSize = h.u32le(0u);
            auto const instrumentType = h.u8(26u);
            auto const sampleHeaderSize = h.u32le(29u);
            constexpr std::uint8_t ENVELOPE_LOOP = 0x04u;
            if (madeWith == VER_OLD_MODPLUG)
            {
                madeWith |= VER_CONFIRMED;
                if (rawSize == 245u)
                {
                    lastSaved = 0x010000A5u;
                    result.tracker = "ModPlug Tracker 1.0 alpha";
                }
                else if (rawSize == 263u)
                {
                    lastSaved = 0x010000B3u;
                    result.tracker = "ModPlug Tracker 1.0 beta";
                }
                else
                    madeWith = VER_UNKNOWN | VER_CONFIRMED;
            }
            else if (sampleCount == 0u)
            {
                // empty instruments make the tracker easy to tell
                if (rawSize == 263u && sampleHeaderSize == 0u && made_with(VER_NEW_MODPLUG))
                    madeWith |= VER_CONFIRMED;
                else if (rawSize != 29u && made_with(VER_DIGITRAKKER))
                    madeWith &= ~VER_DIGITRAKKER;
                else if (made_with(VER_FT2_CLONE | VER_FT2_GENERIC) && rawSize != 33u)
                    madeWith = VER_UNKNOWN;
                if (rawSize != 33u)
                    madeWith &= ~VER_PLAYERPRO;
                else if (sampleHeaderSize > SAMPLE_HEADER_SIZE && made_with(VER_PLAYERPRO))
                {
                    if (instrumentWithSamplesEncountered || (lastSampleHeaderSize != -1 && sampleHeaderSize != lastSampleHeaderSize))
                        madeWith = VER_PLAYERPRO | VER_CONFIRMED;
                    lastSampleHeaderSize = sampleHeaderSize;
                }
            }
            if (lastInstrumentType == -1)
                lastInstrumentType = instrumentType;
            else if (lastInstrumentType != instrumentType && made_with(VER_FT2_GENERIC))
            {
                // FastTracker 2 writes the same junk as every instrument's type
                madeWith &= ~VER_FT2_GENERIC;
                madeWith |= VER_FT2_CLONE;
            }
            if (sampleCount > 0u)
            {
                instrumentWithSamplesEncountered = true;
                if ((h.u8(241u) | h.u8(242u) | h.u16le(243u) | h.u8(247u)) != 0u)
                    madeWith &= ~(VER_OLD_MODPLUG | VER_NEW_MODPLUG | VER_PLAYERPRO);
                if (rawSize != 263u || instrumentType != 0u)
                    madeWith &= ~VER_PLAYERPRO;
                if (!made_with(VER_CONFIRMED) && made_with(VER_PLAYERPRO))
                {
                    if (((h.u8(233u) & ENVELOPE_LOOP) == 0u && h.u8(228u) == 0xFFu && h.u8(229u) == 0xFFu) ||
                        ((h.u8(234u) & ENVELOPE_LOOP) == 0u && h.u8(231u) == 0xFFu && h.u8(232u) == 0xFFu))
                    {
                        madeWith |= VER_CONFIRMED;
                        madeWith &= ~VER_NEW_MODPLUG;
                    }
                }
            }
            ins.fadeOut = h.u16le(239u);
            read_envelope(h, 129u, h.u8(225u), h.u8(233u), h.u8(227u), h.u8(228u), h.u8(229u), ins.volumeEnvelope);
            read_envelope(h, 177u, h.u8(226u), h.u8(234u), h.u8(230u), h.u8(231u), h.u8(232u), ins.panningEnvelope);
            if (sampleCount == 0u)
                continue;
            auto const firstSample = static_cast<std::uint32_t>(result.samples.size());
            for (std::uint32_t note = 0u; note < 96u; ++note)
            {
                auto const s = h.u8(33u + note);
                ins.keyboard[note + 12u] = s < sampleCount ? static_cast<std::uint16_t>(firstSample + s + 1u) : 0u;
            }
            auto const vibratoType = h.u8(235u);
            auto const vibratoSweep = h.u8(236u);
            auto const vibratoDepth = h.u8(237u);
            auto const vibratoRate = h.u8(238u);
            struct stored_sample
            {
                std::uint32_t bytes;
                sample_encoding encoding;
            };
            std::vector<stored_sample> stored;
            for (std::uint32_t index = 0u; index < sampleCount; ++index, offset += SAMPLE_HEADER_SIZE)
            {
                sample s;
                auto const bytes = aFile.u32le(offset);
                auto loopStart = aFile.u32le(offset + 4u);
                auto loopLength = aFile.u32le(offset + 8u);
                s.volume = static_cast<std::uint16_t>(std::min<std::uint8_t>(aFile.u8(offset + 12u), 64u) * 4u);
                s.finetune = aFile.i8(offset + 13u);
                auto const sampleFlags = aFile.u8(offset + 14u);
                s.pan = aFile.u8(offset + 15u);
                s.relativeNote = aFile.i8(offset + 16u);
                auto const reserved = aFile.u8(offset + 17u);
                s.name = aFile.text(offset + 18u, 22u);
                sampleReserved |= reserved;
                if (reserved != 0u && reserved != 0xADu)
                    madeWith &= ~(VER_OLD_MODPLUG | VER_NEW_MODPLUG | VER_OPENMPT);
                if (lastSampleReserved == -1)
                    lastSampleReserved = reserved;
                else if (lastSampleReserved != reserved)
                    madeWith &= ~VER_PLAYERPRO;
                if (aFile.u8(offset + 15u) != 128u)
                    madeWith &= ~VER_PLAYERPRO;
                if ((aFile.u8(offset + 13u) & 0x0Fu) != 0u && aFile.u8(offset + 13u) != 127u)
                    madeWith &= ~VER_PLAYERPRO;
                if (made_with(VER_FT2_GENERIC | VER_FT2_CLONE) && made_with(VER_NEW_MODPLUG | VER_PLAYERPRO) && !made_with(VER_CONFIRMED))
                {
                    // FastTracker 2 stores the length of the sample's name here
                    bool nonSpace = false;
                    if (reserved <= 22u)
                        for (std::size_t index = reserved; index < 22u; ++index)
                            if (aFile.u8(offset + 18u + index) != ' ')
                                nonSpace = true;
                    if (reserved > 22u || nonSpace)
                    {
                        madeWith &= ~VER_FT2_GENERIC;
                        madeWith |= VER_FT2_CLONE | VER_CONFIRMED;
                    }
                }
                if ((sampleFlags & 3u) == 3u && made_with(VER_NEW_MODPLUG))
                    madeWith |= VER_MODPLUG_BIDI_FLAG;
                s.vibratoType = static_cast<vibrato_type>(std::min<std::uint8_t>(vibratoType, 4u));
                s.vibratoSweep = vibratoSweep;
                s.vibratoDepth = vibratoDepth;
                s.vibratoRate = vibratoRate;
                sample_encoding encoding;
                encoding.delta = true;
                std::uint32_t length = bytes;
                if ((sampleFlags & 0x10u) != 0u)
                {
                    encoding.width = sample_encoding::bits::Sixteen;
                    length /= 2u;
                    loopStart /= 2u;
                    loopLength /= 2u;
                }
                if ((sampleFlags & 0x20u) != 0u)
                {
                    encoding.stereo = true;
                    length /= 2u;
                    loopStart /= 2u;
                    loopLength /= 2u;
                }
                s.length = length;
                s.loopStart = loopStart;
                s.loopEnd = loopStart + loopLength;
                if ((sampleFlags & 0x03u) != 0u && s.loopEnd > s.loopStart)
                    s.loop = (sampleFlags & 0x02u) != 0u ? loop_type::PingPong : loop_type::Forward;
                if (reserved == 0xADu && (sampleFlags & 0x30u) == 0u)
                    encoding.width = sample_encoding::bits::ThirtyTwo; // ModPlug ADPCM: not supported, played as silence
                result.samples.push_back(std::move(s));
                stored.push_back(stored_sample{ bytes, encoding });
            }
            for (std::uint32_t index = 0u; index < sampleCount; ++index)
            {
                auto& s = result.samples[firstSample + index];
                auto const& info = stored[index];
                std::size_t bytes = info.bytes;
                if (info.encoding.width == sample_encoding::bits::ThirtyTwo)
                {
                    // ModPlug's ADPCM: a table of 16 deltas, then a four-bit index into it for each sample
                    bytes = 16u + (info.bytes + 1u) / 2u;
                    s.data.assign(s.length, 0.0f);
                    std::int8_t delta = 0;
                    for (std::uint32_t frame = 0u; frame < s.length && aFile.has(offset + 16u + frame / 2u, 1u); ++frame)
                    {
                        auto const packed = aFile.u8(offset + 16u + frame / 2u);
                        auto const index = (frame & 1u) == 0u ? (packed & 0x0Fu) : (packed >> 4u);
                        delta = static_cast<std::int8_t>(delta + aFile.i8(offset + index));
                        s.data[frame] = static_cast<float>(delta) / 128.0f;
                    }
                }
                else if (s.length != 0u)
                {
                    auto const loop = s.loop;
                    auto const loopStart = s.loopStart;
                    auto const loopEnd = s.loopEnd;
                    decode_pcm(aFile.bytes(offset, bytes), s.length, info.encoding, s);
                    s.loop = loop;
                    s.loopStart = loopStart;
                    s.loopEnd = loopEnd;
                }
                offset += bytes;
            }
        }
        if (sampleReserved == 0u && made_with(VER_NEW_MODPLUG) && songNameView.find('\0') != std::string_view::npos)
            madeWith |= VER_CONFIRMED;

        // what ModPlug Tracker and OpenMPT add after the samples
        bool hasMidiConfig = false;
        auto const chunk = [&](char const* aMagic) -> std::optional<std::pair<std::size_t, std::size_t>>
            {
                if (!aFile.magic(offset, aMagic) || !aFile.has(offset + 4u, 4u))
                    return {};
                auto const size = aFile.u32le(offset + 4u);
                std::pair<std::size_t, std::size_t> const result{ offset + 8u, size };
                offset += 8u + size;
                return result;
            };
        if (chunk("text"))
        {
            madeWith |= VER_CONFIRMED;
            madeWith &= ~VER_PLAYERPRO;
        }
        if (auto const midi = chunk("MIDI"))
        {
            // the global macros, then SFx, then Zxx, 32 characters each
            auto const read_macro = [&](std::size_t aIndex) -> std::optional<std::string>
                {
                    if ((aIndex + 1u) * 32u > midi->second)
                        return {};
                    std::string result;
                    for (std::size_t index = 0u; index < 32u; ++index)
                    {
                        auto const ch = static_cast<char>(aFile.u8(midi->first + aIndex * 32u + index));
                        if (ch == '\0')
                            break;
                        if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'z'))
                            result.push_back(ch);
                    }
                    return result;
                };
            for (std::size_t index = 0u; index < 16u; ++index)
                if (auto const macro = read_macro(9u + index))
                    result.parameteredMacros[index] = *macro;
            for (std::size_t index = 0u; index < 128u; ++index)
                if (auto const macro = read_macro(25u + index))
                    result.fixedMacros[index] = *macro;
            hasMidiConfig = true;
            madeWith |= VER_CONFIRMED;
            madeWith &= ~VER_PLAYERPRO;
        }
        if (chunk("PNAM"))
        {
            madeWith |= VER_CONFIRMED;
            madeWith &= ~VER_PLAYERPRO;
        }
        if (chunk("CNAM"))
        {
            madeWith |= VER_CONFIRMED;
            madeWith &= ~VER_PLAYERPRO;
        }
        {
            // mix plugins (not played), up to OpenMPT's extensions
            auto const start = offset;
            while (aFile.has(offset, 9u))
            {
                if (aFile.magic(offset, "IMPI") || aFile.magic(offset, "IMPS") || aFile.magic(offset, "XTPM") || aFile.magic(offset, "STPM") ||
                    !aFile.has(offset + 8u, aFile.u32le(offset + 4u)))
                    break;
                offset += 8u + aFile.u32le(offset + 4u);
            }
            if (offset != start)
            {
                madeWith |= VER_CONFIRMED;
                madeWith &= ~VER_PLAYERPRO;
            }
        }
        if (made_with(VER_CONFIRMED))
        {
            if (made_with(VER_MODPLUG_BIDI_FLAG))
            {
                lastSaved = 0x01110000u;
                result.tracker = "ModPlug Tracker 1.0 - 1.11";
            }
            else if (made_with(VER_NEW_MODPLUG))
            {
                lastSaved = 0x01160000u;
                result.tracker = made_with(VER_PLAYERPRO) ? "ModPlug Tracker 1.0 - 1.16 / PlayerPRO" : "ModPlug Tracker 1.0 - 1.16";
            }
            else if (made_with(VER_PLAYERPRO))
                result.tracker = "PlayerPRO";
        }
        if (raw_equals(38u, "OpenMPT "))
        {
            lastSaved = parse_openmpt_version(aFile.text(46u, 12u));
            madeWith = VER_OPENMPT | VER_CONFIRMED;
            result.mixLevels = lastSaved < 0x01220719u ? mix_levels::Compatible : mix_levels::CompatibleFT2;
        }
        if (lastSaved != 0u && !made_with(VER_OPENMPT))
        {
            result.mixLevels = mix_levels::Original;
            result.playBehaviour.reset();
        }
        if (made_with(VER_FT2_GENERIC))
        {
            result.mixLevels = mix_levels::CompatibleFT2;
            // FastTracker 2's soft volume ramping
            result.playBehaviour.set(behaviour::FT2VolumeRamping);
            // FastTracker 2 lets any letter be typed as an effect, Zxx included: they mean nothing
            if (!hasMidiConfig)
            {
                for (auto& macro : result.parameteredMacros)
                    macro.clear();
                for (auto& macro : result.fixedMacros)
                    macro.clear();
            }
        }
        if (result.tracker.empty())
            result.tracker = made_with(VER_FT2_GENERIC) ? "FastTracker 2 or compatible" : "Unknown";
        bool openmptMade = false;
        if (!result.instruments.empty() && aFile.magic(offset, "XTPM"))
        {
            openmptMade = true;
            offset = read_instrument_extensions(aFile, offset, result.instruments);
        }
        if (aFile.magic(offset, "STPM"))
        {
            openmptMade = true;
            read_song_extensions(aFile, offset + 4u, result);
        }
        if (result.openmptVersion != 0u)
            lastSaved = result.openmptVersion;
        if (openmptMade && lastSaved < 0x01170000u)
            lastSaved = 0x01170000u;
        if (lastSaved >= 0x01170000u)
            result.tracker = "OpenMPT";
        // old versions allowed --- and +++ in the order list
        if (lastSaved != 0u && lastSaved < 0x01220202u)
        {
            std::erase_if(result.orders, [&](std::uint16_t aOrder) { return aOrder == 0xFEu && aOrder >= result.patterns.size(); });
            for (auto& order : result.orders)
                if (order == 0xFFu && order >= result.patterns.size())
                    order = module_data::ORDER_END;
        }
        result.openmptVersion = lastSaved;
        apply_openmpt_upgrades(result);
        return result;
    }
}
