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
Portions of this file (the MO3 loader (OpenMPT's soundlib/Load_mo3.cpp)) are derived from OpenMPT
(https://openmpt.org/). OpenMPT's MO3 loader is based on documentation and the decompression
routines from the open-source UNMO3 project (https://github.com/lclevy/unmo3); the modified
decompression code was relicensed to the BSD license with permission from Laurent Clevy.

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

// Un4seen MO3: an MOD, S3M, XM or IT module with its music data LZ-compressed and its samples compressed
// (losslessly, or as MP3 or Ogg Vorbis). The format is as documented by the UNMO3 project and read by
// OpenMPT.

#include <mod_tracker/module.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>

#include "loader.hpp"
#include "tables.hpp"

// Ogg Vorbis samples are decoded with stb_vorbis (public domain), built in a translation unit of its own
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_HEADER_ONLY
#include <stb/stb_vorbis.c>

namespace mod_tracker::detail
{
    namespace
    {
        constexpr std::size_t FILE_HEADER_SIZE = 422u;
        constexpr std::size_t ENVELOPE_SIZE = 106u;
        constexpr std::size_t INSTRUMENT_SIZE = 826u;
        constexpr std::size_t SAMPLE_SIZE = 41u;

        constexpr std::uint32_t FLAG_LINEAR_SLIDES = 0x0001u;
        constexpr std::uint32_t FLAG_S3M = 0x0002u;
        constexpr std::uint32_t FLAG_S3M_FAST_SLIDES = 0x0004u;
        constexpr std::uint32_t FLAG_MTM = 0x0008u;
        constexpr std::uint32_t FLAG_S3M_AMIGA_LIMITS = 0x0010u;
        constexpr std::uint32_t FLAG_MOD = 0x0080u;
        constexpr std::uint32_t FLAG_IT = 0x0100u;
        constexpr std::uint32_t FLAG_INSTRUMENT_MODE = 0x0200u;
        constexpr std::uint32_t FLAG_IT_COMPATIBLE_GXX = 0x0400u;
        constexpr std::uint32_t FLAG_IT_OLD_EFFECTS = 0x0800u;
        constexpr std::uint32_t FLAG_MODPLUG_MODE = 0x10000u;
        constexpr std::uint32_t FLAG_MOD_VBLANK = 0x80000u;
        constexpr std::uint32_t FLAG_HAS_PLUGINS = 0x100000u;
        constexpr std::uint32_t FLAG_EXTENDED_FILTER_RANGE = 0x200000u;

        constexpr std::uint16_t SAMPLE_16BIT = 0x01u;
        constexpr std::uint16_t SAMPLE_LOOP = 0x10u;
        constexpr std::uint16_t SAMPLE_PINGPONG_LOOP = 0x20u;
        constexpr std::uint16_t SAMPLE_SUSTAIN = 0x100u;
        constexpr std::uint16_t SAMPLE_PINGPONG_SUSTAIN = 0x200u;
        constexpr std::uint16_t SAMPLE_STEREO = 0x400u;
        constexpr std::uint16_t COMPRESSION_MPEG = 0x1000u;
        constexpr std::uint16_t COMPRESSION_OGG = 0x1000u | 0x2000u;
        constexpr std::uint16_t COMPRESSION_SHARED_OGG = 0x1000u | 0x2000u | 0x4000u;
        constexpr std::uint16_t COMPRESSION_DELTA = 0x2000u;
        constexpr std::uint16_t COMPRESSION_DELTA_PREDICTION = 0x4000u;
        constexpr std::uint16_t COMPRESSION_MASK = 0x1000u | 0x2000u | 0x4000u | 0x8000u;

        // the control bits of the LZ and sample compression: read most significant first, a byte at a
        // time, with a marker bit telling when the byte is used up
        class control_bits
        {
        public:
            control_bits(reader const& aFile, std::size_t aOffset) :
                iFile{ aFile }, iOffset{ aOffset }
            {
            }
        public:
            // false at the end of the data
            bool next(std::int32_t& aCarry)
            {
                iData = static_cast<std::uint16_t>(iData << 1u);
                aCarry = iData > 0xFFu ? 1 : 0;
                iData &= 0xFFu;
                if (iData == 0u)
                {
                    if (!iFile.has(iOffset, 1u))
                        return false;
                    iData = static_cast<std::uint16_t>((iFile.u8(iOffset++) << 1u) + 1u);
                    aCarry = iData > 0xFFu ? 1 : 0;
                    iData &= 0xFFu;
                }
                return true;
            }
            bool byte(std::uint8_t& aByte)
            {
                if (!iFile.has(iOffset, 1u))
                    return false;
                aByte = iFile.u8(iOffset++);
                return true;
            }
            std::size_t offset() const
            {
                return iOffset;
            }
        private:
            reader const& iFile;
            std::size_t iOffset;
            std::uint16_t iData = 0u;
        };

        // the music data: LZ compressed
        std::vector<std::byte> unpack(reader const& aFile, std::size_t aOffset, std::uint32_t aSize, std::size_t& aEnd)
        {
            std::vector<std::byte> result;
            result.reserve(aSize);
            control_bits bits{ aFile, aOffset };
            std::uint8_t first = 0u;
            if (aSize == 0u || !bits.byte(first))
                throw module_load_error{ "bad MO3 music data" };
            result.push_back(static_cast<std::byte>(first));
            std::int32_t carry = 0;
            std::int32_t strOffset = 0;
            // a length: a 1 bit, then the first of each pair of bits until the second is 0
            auto const decode_length = [&](std::int32_t& aLength)
                {
                    ++aLength;
                    do
                    {
                        if (!bits.next(carry))
                            return false;
                        // (no real length is anywhere near this long)
                        if (aLength >= 0x1000000)
                            return false;
                        aLength = aLength * 2 + carry;
                        if (!bits.next(carry))
                            return false;
                    } while (carry != 0);
                    return true;
                };
            while (result.size() < aSize)
            {
                if (!bits.next(carry))
                    break;
                if (carry == 0)
                {
                    // a literal byte
                    std::uint8_t literal = 0u;
                    if (!bits.byte(literal))
                        break;
                    result.push_back(static_cast<std::byte>(literal));
                    continue;
                }
                // a copy of what came before
                std::int32_t length = 0;
                std::int32_t lengthAdjust = 0;
                if (!decode_length(length))
                    break;
                length -= 3;
                if (length < 0)
                    ++length;       // the same offset as the last copy
                else
                {
                    std::uint8_t low = 0u;
                    if (!bits.byte(low) || length > 0x7FFFFF)
                        break;
                    strOffset = ~((length << 8) | low);
                    length = 0;
                    if (strOffset < -1280)
                        ++lengthAdjust;
                    ++lengthAdjust;
                    if (strOffset < -32000)
                        ++lengthAdjust;
                }
                if (strOffset >= 0 || -static_cast<std::int64_t>(result.size()) > strOffset)
                    break;
                if (length >= 0x1000000)
                    break;
                bool ok = bits.next(carry);
                length = length * 2 + carry;
                ok = ok && bits.next(carry);
                length = length * 2 + carry;
                if (!ok)
                    break;
                if (length == 0)
                {
                    if (!decode_length(length))
                        break;
                    if (length >= 0x1000000)
                        break;
                    length += 2;
                }
                length += lengthAdjust;
                if (length <= 0 || aSize - result.size() < static_cast<std::size_t>(length))
                    break;
                for (std::int32_t index = 0; index < length; ++index)
                    result.push_back(result[result.size() + strOffset]);
            }
            if (result.size() != aSize)
                throw module_load_error{ "bad MO3 music data" };
            aEnd = bits.offset();
            return result;
        }

        // losslessly compressed samples: variable-length deltas, optionally from a prediction
        template <typename Sample, typename Unsigned, int Shift, std::uint8_t DhInit>
        std::vector<Sample> unpack_delta(reader const& aFile, std::size_t aOffset, std::uint32_t aLength, std::uint32_t aChannels, bool aPrediction)
        {
            std::vector<Sample> result(static_cast<std::size_t>(aLength) * aChannels, 0);
            control_bits bits{ aFile, aOffset };
            std::uint8_t dh = DhInit;
            std::int32_t carry = 0;
            std::int32_t next = 0;
            Sample previous = 0;
            bool end = false;
            for (std::uint32_t channel = 0u; channel < aChannels && !end; ++channel)
            {
                for (std::uint32_t frame = 0u; frame < aLength; ++frame)
                {
                    Unsigned value = 0u;
                    // the first part of the delta
                    if constexpr (sizeof(Sample) == 2u)
                    {
                        if (dh < 5u)
                        {
                            do
                            {
                                if (!bits.next(carry)) { end = true; break; }
                                value = static_cast<Unsigned>((value << 1u) + carry);
                                if (!bits.next(carry)) { end = true; break; }
                                value = static_cast<Unsigned>((value << 1u) + carry);
                                if (!bits.next(carry)) { end = true; break; }
                            } while (carry != 0);
                        }
                        else
                        {
                            do
                            {
                                if (!bits.next(carry)) { end = true; break; }
                                value = static_cast<Unsigned>((value << 1u) + carry);
                                if (!bits.next(carry)) { end = true; break; }
                            } while (carry != 0);
                        }
                    }
                    else
                    {
                        do
                        {
                            if (!bits.next(carry)) { end = true; break; }
                            value = static_cast<Unsigned>((value << 1u) + carry);
                            if (!bits.next(carry)) { end = true; break; }
                        } while (carry != 0);
                    }
                    if (end)
                        break;
                    // the second part, and the sign
                    for (std::uint8_t remaining = dh; remaining > 0u; --remaining)
                    {
                        if (!bits.next(carry)) { end = true; break; }
                        value = static_cast<Unsigned>((value << 1u) + carry);
                    }
                    if (end)
                        break;
                    std::uint8_t cl = 1u;
                    if (value >= 4u)
                    {
                        cl = Shift;
                        while (((1u << cl) & value) == 0u && cl > 1u)
                            --cl;
                    }
                    dh = static_cast<std::uint8_t>((dh + cl) >> 1u);
                    bool const positive = (value & 1u) != 0u;
                    value = static_cast<Unsigned>(value >> 1u);
                    if (!positive)
                        value = static_cast<Unsigned>(~value);
                    if (!aPrediction)
                    {
                        value = static_cast<Unsigned>(value + static_cast<Unsigned>(previous));
                        result[static_cast<std::size_t>(frame) * aChannels + channel] = static_cast<Sample>(value);
                        previous = static_cast<Sample>(value);
                    }
                    else
                    {
                        auto const delta = static_cast<Sample>(value);
                        value = static_cast<Unsigned>(value + static_cast<Unsigned>(next));
                        auto const sampleValue = static_cast<Sample>(value);
                        result[static_cast<std::size_t>(frame) * aChannels + channel] = sampleValue;
                        next = sampleValue * 2 + (delta >> 1) - previous;
                        next = std::clamp<std::int32_t>(next, std::numeric_limits<Sample>::min(), std::numeric_limits<Sample>::max());
                        previous = sampleValue;
                    }
                }
            }
            return result;
        }

        effect translate_effect(std::uint8_t aCommand)
        {
            static constexpr effect TRANSLATION[] =
            {
                effect::None, effect::None, effect::None, effect::Arpeggio,
                effect::PortamentoUp, effect::PortamentoDown, effect::TonePortamento, effect::Vibrato,
                effect::TonePortamentoVolumeSlide, effect::VibratoVolumeSlide, effect::Tremolo, effect::Panning,
                effect::Offset, effect::VolumeSlide, effect::PositionJump, effect::Volume,
                effect::PatternBreak, effect::ExtendedMod, effect::Tempo, effect::Tremor,
                effect::None, effect::None, effect::GlobalVolume, effect::GlobalVolumeSlide,
                effect::KeyOff, effect::SetEnvelopePosition, effect::PanningSlide, effect::None,
                effect::Retrigger, effect::ExtraFinePortamento, effect::ExtraFinePortamento, effect::None,
                effect::None, effect::Speed, effect::VolumeSlide, effect::PortamentoDown,
                effect::PortamentoUp, effect::Tremor, effect::Retrigger, effect::FineVibrato,
                effect::ChannelVolume, effect::ChannelVolumeSlide, effect::PanningSlide, effect::ExtendedS3M,
                effect::Tempo, effect::GlobalVolumeSlide, effect::Panbrello, effect::Midi,
                effect::None, effect::None, effect::None, effect::Dummy,
                effect::None, effect::ExtendedParameter, effect::SmoothMidi, effect::DelayCut,
                effect::None, effect::None
            };
            return aCommand < std::size(TRANSLATION) ? TRANSLATION[aCommand] : effect::None;
        }

        void read_envelope(reader const& aFile, std::size_t aOffset, std::uint32_t aShift, bool aXM, envelope& aEnvelope)
        {
            auto const flags = aFile.u8(aOffset);
            aEnvelope.enabled = (flags & 0x01u) != 0u;
            aEnvelope.sustain = (flags & 0x02u) != 0u;
            aEnvelope.loop = (flags & 0x04u) != 0u;
            aEnvelope.filter = (flags & 0x10u) != 0u;
            aEnvelope.carry = (flags & 0x20u) != 0u;
            aEnvelope.points.resize(std::min<std::uint8_t>(aFile.u8(aOffset + 1u), 25u));
            aEnvelope.sustainStart = aFile.u8(aOffset + 2u);
            aEnvelope.sustainEnd = aXM ? aEnvelope.sustainStart : aFile.u8(aOffset + 3u);
            aEnvelope.loopStart = aFile.u8(aOffset + 4u);
            aEnvelope.loopEnd = aFile.u8(aOffset + 5u);
            for (std::size_t index = 0u; index < aEnvelope.points.size(); ++index)
            {
                auto& point = aEnvelope.points[index];
                point.tick = static_cast<std::uint16_t>(aFile.i16le(aOffset + 6u + index * 4u));
                if (index > 0u && point.tick < aEnvelope.points[index - 1u].tick)
                    point.tick = static_cast<std::uint16_t>(aEnvelope.points[index - 1u].tick + 1u);
                point.value = static_cast<std::uint8_t>(std::clamp(aFile.i16le(aOffset + 8u + index * 4u) >> aShift, 0, 64));
            }
        }

        std::string read_string(cursor& aData)
        {
            std::string result;
            while (aData.can_read(1u))
            {
                auto const ch = static_cast<char>(aData.u8());
                if (ch == '\0')
                    break;
                result.push_back(ch);
            }
            return result;
        }

        std::string hex(std::uint32_t aValue)
        {
            char text[3];
            std::snprintf(text, sizeof(text), "%02X", aValue & 0xFFu);
            return text;
        }
    }

    bool is_mo3(reader const& aFile)
    {
        return aFile.magic(0u, "MO3") && aFile.has(0u, 8u) && aFile.u8(3u) <= 5u && aFile.u32le(4u) > FILE_HEADER_SIZE && aFile.u32le(4u) < 0x20000000u;
    }

    module_data load_mo3(reader const& aFile)
    {
        auto const version = aFile.u8(3u);
        auto const musicSize = aFile.u32le(4u);
        std::size_t musicOffset = 8u;
        std::uint32_t compressedSize = 0u;
        if (version >= 5u)
        {
            compressedSize = aFile.u32le(8u);
            musicOffset = 12u;
        }
        std::size_t musicEnd = 0u;
        auto const music = unpack(aFile, musicOffset, musicSize, musicEnd);
        reader const m{ std::span<std::byte const>{ music } };
        cursor data{ m, 0u };

        module_data result;
        result.container = "MO3";
        result.title = read_string(data);
        read_string(data);  // the song message
        if (!data.can_read(FILE_HEADER_SIZE))
            throw module_load_error{ "bad MO3 header" };
        auto const header = data.offset();
        data.skip(FILE_HEADER_SIZE);
        auto const channels = m.u8(header);
        auto const orderCount = m.u16le(header + 1u);
        auto const restart = m.u16le(header + 3u);
        auto const patternCount = m.u16le(header + 5u);
        auto const trackCount = m.u16le(header + 7u);
        auto const instrumentCount = m.u16le(header + 9u);
        auto const sampleCount = m.u16le(header + 11u);
        auto const speed = m.u8(header + 13u);
        auto const tempo = m.u8(header + 14u);
        auto const flags = m.u32le(header + 15u);
        auto const globalVolume = m.u8(header + 19u);
        auto const sampleVolume = m.i8(header + 21u);
        if (channels == 0u || channels > MAX_CHANNELS || restart > orderCount)
            throw module_load_error{ "bad MO3 header" };

        if ((flags & FLAG_IT) != 0u)
        {
            result.format = module_format::IT;
            result.formatName = "Impulse Tracker";
        }
        else if ((flags & FLAG_S3M) != 0u)
        {
            result.format = module_format::S3M;
            result.formatName = "Scream Tracker 3";
        }
        else if ((flags & FLAG_MOD) != 0u)
        {
            result.format = module_format::MOD;
            result.formatName = "ProTracker";
        }
        else if ((flags & FLAG_MTM) != 0u)
            throw module_load_error{ "MO3 modules made from MultiTracker modules aren't supported" };
        else
        {
            result.format = module_format::XM;
            result.formatName = "FastTracker 2";
        }
        bool const isIT = result.format == module_format::IT;
        bool const isXM = result.format == module_format::XM;
        result.tracker = "MO3 v" + std::to_string(version);
        result.channels = channels;
        result.playBehaviour = default_behaviours(result.format);
        result.mixLevels = mix_levels::Compatible;
        result.minimumPeriod = 16;
        result.maximumPeriod = 32767;
        result.restart = restart;
        result.speed = speed != 0u ? speed : 6u;
        result.tempo = tempo != 0u ? tempo : 125u;
        result.linearSlides = (flags & FLAG_LINEAR_SLIDES) != 0u;
        result.amigaLimits = (flags & FLAG_S3M_AMIGA_LIMITS) != 0u && result.format == module_format::S3M;
        result.fastVolumeSlides = (flags & FLAG_S3M_FAST_SLIDES) != 0u && result.format == module_format::S3M;
        // (these two flags mean the opposite of what they say)
        result.itOldEffects = (flags & FLAG_IT_OLD_EFFECTS) == 0u && isIT;
        result.itCompatibleGxx = (flags & FLAG_IT_COMPATIBLE_GXX) == 0u && isIT;
        result.extendedFilterRange = (flags & FLAG_EXTENDED_FILTER_RANGE) != 0u;
        if ((flags & FLAG_MOD_VBLANK) != 0u)
            result.playBehaviour.set(behaviour::MODVBlankTiming);
        if (isIT)
            result.globalVolume = std::min<std::uint32_t>(globalVolume, 128u) * 2u;
        else if (result.format == module_format::S3M)
            result.globalVolume = std::min<std::uint32_t>(globalVolume, 64u) * 4u;
        if (sampleVolume < 0)
            result.samplePreAmp = static_cast<std::uint32_t>(sampleVolume + 52);
        else
            result.samplePreAmp = static_cast<std::uint32_t>(std::exp(sampleVolume * 3.1 / 20.0)) + 51u;

        result.channelSettings.assign(channels, channel_settings{});
        for (std::uint32_t index = 0u; index < channels; ++index)
        {
            auto& settings = result.channelSettings[index];
            if (isIT)
                settings.volume = std::min<std::uint8_t>(m.u8(header + 22u + index), 64u);
            if (!isXM)
            {
                auto const pan = m.u8(header + 86u + index);
                if (pan == 127u)
                    settings.surround = true;
                else if (pan == 255u)
                    settings.pan = 256u;
                else
                    settings.pan = pan;
            }
        }

        // MIDI macros, if any (otherwise the defaults)
        result.reset_macros();
        bool anyMacros = false;
        for (std::uint32_t index = 0u; index < 16u; ++index)
            anyMacros = anyMacros || m.u8(header + 150u + index) != 0u;
        for (std::uint32_t index = 0u; index < 128u; ++index)
            anyMacros = anyMacros || m.u8(header + 166u + index * 2u + 1u) != 0u;
        if (anyMacros)
        {
            for (std::uint32_t index = 0u; index < 16u; ++index)
            {
                auto const macro = m.u8(header + 150u + index);
                result.parameteredMacros[index] = macro != 0u ? "F0F0" + hex(macro - 1u) + "z" : std::string{};
            }
            for (std::uint32_t index = 0u; index < 128u; ++index)
            {
                auto const type = m.u8(header + 166u + index * 2u + 1u);
                auto const value = m.u8(header + 166u + index * 2u);
                result.fixedMacros[index] = type != 0u ? "F0F0" + hex(type - 1u) + hex(value) : std::string{};
            }
        }

        // orders; S3M and IT have separators
        bool const separators = !(result.format == module_format::MOD || isXM);
        for (std::uint32_t index = 0u; index < orderCount; ++index)
        {
            auto const order = data.u8();
            if (separators && order == 0xFFu)
                result.orders.push_back(module_data::ORDER_END);
            else if (separators && order == 0xFEu)
                result.orders.push_back(module_data::ORDER_SKIP);
            else
                result.orders.push_back(order);
        }

        // patterns: each channel of each is a track, and tracks are shared
        auto const trackAssignments = data.offset();
        data.skip(static_cast<std::size_t>(patternCount) * channels * 2u);
        auto const patternLengths = data.offset();
        data.skip(static_cast<std::size_t>(patternCount) * 2u);
        std::vector<std::pair<std::size_t, std::size_t>> tracks(trackCount);
        for (auto& track : tracks)
        {
            auto const length = data.u32le();
            if (length >= 0x200000u || !data.can_read(length))
                throw module_load_error{ "bad MO3 track" };
            track = { data.offset(), length };
            data.skip(length);
        }
        std::uint8_t noteOffset = isIT ? NOTE_MIN : static_cast<std::uint8_t>(12u + NOTE_MIN);
        bool onlyAmigaNotes = true;
        result.patterns.resize(patternCount);
        for (std::uint32_t patternIndex = 0u; patternIndex < patternCount; ++patternIndex)
        {
            auto& p = result.patterns[patternIndex];
            p.rows = std::max<std::uint32_t>(m.u16le(patternLengths + patternIndex * 2u), 1u);
            p.cells.assign(static_cast<std::size_t>(p.rows) * channels, cell{});
            for (std::uint32_t channel = 0u; channel < channels; ++channel)
            {
                auto const trackIndex = m.u16le(trackAssignments + (patternIndex * channels + channel) * 2u);
                if (trackIndex >= tracks.size())
                    continue;
                reader const track{ m.bytes(tracks[trackIndex].first, tracks[trackIndex].second) };
                std::size_t position = 0u;
                std::uint32_t row = 0u;
                while (row < p.rows && track.has(position, 1u))
                {
                    auto const event = track.u8(position++);
                    if (event == 0u)
                        break;
                    std::uint32_t const commands = event & 0x0Fu;
                    std::uint32_t const repeat = event >> 4u;
                    cell c;
                    for (std::uint32_t command = 0u; command < commands && track.has(position, 2u); ++command, position += 2u)
                    {
                        auto const code = track.u8(position);
                        auto const parameter = track.u8(position + 1u);
                        auto const set_effect = [&](effect aEffect, std::uint8_t aParameter)
                            {
                                c.command = aEffect;
                                c.parameter = aParameter;
                            };
                        auto const set_volume = [&](volume_command aCommand, std::uint8_t aValue)
                            {
                                c.volumeCommand = aCommand;
                                c.volume = aValue;
                            };
                        switch (code)
                        {
                        case 0x01u:
                            if (parameter < 120u)
                                c.note = static_cast<std::uint8_t>(parameter + noteOffset);
                            else if (parameter == 0xFFu)
                                c.note = NOTE_OFF;
                            else if (parameter == 0xFEu)
                                c.note = NOTE_CUT;
                            else
                                c.note = NOTE_FADE;
                            if (!is_note(c.note) || c.note < NOTE_MIDDLE_C - 12u || c.note >= NOTE_MIDDLE_C + 24u)
                                onlyAmigaNotes = false;
                            break;
                        case 0x02u:
                            c.instrument = static_cast<std::uint8_t>(parameter + 1u);
                            break;
                        case 0x06u:
                            if (c.volumeCommand == volume_command::None && isXM && (parameter & 0x0Fu) == 0u)
                            {
                                set_volume(volume_command::TonePortamento, static_cast<std::uint8_t>(parameter >> 4u));
                                break;
                            }
                            else if (c.volumeCommand == volume_command::None && isIT)
                            {
                                for (std::uint8_t index = 0u; index < 10u; ++index)
                                    if (IT_PORTAMENTO_VOLUME_COLUMN[index] == parameter)
                                    {
                                        set_volume(volume_command::TonePortamento, index);
                                        break;
                                    }
                                if (c.volumeCommand != volume_command::None)
                                    break;
                            }
                            set_effect(effect::TonePortamento, parameter);
                            break;
                        case 0x07u:
                            if (c.volumeCommand == volume_command::None && parameter < 10u && isIT)
                                set_volume(volume_command::VibratoDepth, parameter);
                            else
                                set_effect(effect::Vibrato, parameter);
                            break;
                        case 0x0Bu:
                            if (c.volumeCommand == volume_command::None)
                            {
                                if (isIT && parameter == 0xFFu)
                                {
                                    set_volume(volume_command::Panning, 64u);
                                    break;
                                }
                                if ((isIT && (parameter & 0x03u) == 0u) || (isXM && (parameter & 0x0Fu) == 0u))
                                {
                                    set_volume(volume_command::Panning, static_cast<std::uint8_t>(parameter / 4u));
                                    break;
                                }
                            }
                            set_effect(effect::Panning, parameter);
                            break;
                        case 0x0Fu:
                            if (result.format != module_format::MOD && c.volumeCommand == volume_command::None && parameter <= 64u)
                                set_volume(volume_command::Volume, parameter);
                            else
                                set_effect(effect::Volume, parameter);
                            break;
                        case 0x10u:
                            // decimal (stored as BCD) except in IT
                            set_effect(effect::PatternBreak, isIT ? parameter : static_cast<std::uint8_t>((parameter >> 4u) * 10u + (parameter & 0x0Fu)));
                            break;
                        case 0x12u:
                            set_effect(parameter < 0x20u ? effect::Speed : effect::Tempo, parameter);
                            break;
                        case 0x14u:
                        case 0x15u:
                            if ((parameter & 0xF0u) != 0u)
                                set_volume(code == 0x14u ? volume_command::VolumeSlideUp : volume_command::FineVolumeUp, static_cast<std::uint8_t>(parameter >> 4u));
                            else
                                set_volume(code == 0x14u ? volume_command::VolumeSlideDown : volume_command::FineVolumeDown, static_cast<std::uint8_t>(parameter & 0x0Fu));
                            break;
                        case 0x1Bu:
                            if ((parameter & 0xF0u) != 0u)
                                set_volume(volume_command::PanSlideRight, static_cast<std::uint8_t>(parameter >> 4u));
                            else
                                set_volume(volume_command::PanSlideLeft, static_cast<std::uint8_t>(parameter & 0x0Fu));
                            break;
                        case 0x1Du:
                            set_effect(effect::ExtraFinePortamento, static_cast<std::uint8_t>(0x10u | parameter));
                            break;
                        case 0x1Eu:
                            set_effect(effect::ExtraFinePortamento, static_cast<std::uint8_t>(0x20u | parameter));
                            break;
                        case 0x1Fu:
                        case 0x20u:
                            set_volume(code == 0x1Fu ? volume_command::VibratoSpeed : volume_command::VibratoDepth, parameter);
                            break;
                        case 0x22u:
                            // IT and S3M: K and L are a vibrato or tone portamento and a volume slide
                            if (c.command == effect::TonePortamento)
                                c.command = effect::TonePortamentoVolumeSlide;
                            else if (c.command == effect::Vibrato)
                                c.command = effect::VibratoVolumeSlide;
                            else
                                c.command = effect::VolumeSlide;
                            c.parameter = parameter;
                            break;
                        case 0x30u:
                            c.volume = static_cast<std::uint8_t>(parameter % 10u);
                            if (parameter < 10u)
                                c.volumeCommand = volume_command::FineVolumeUp;
                            else if (parameter < 20u)
                                c.volumeCommand = volume_command::FineVolumeDown;
                            else if (parameter < 30u)
                                c.volumeCommand = volume_command::VolumeSlideUp;
                            else if (parameter < 40u)
                                c.volumeCommand = volume_command::VolumeSlideDown;
                            break;
                        case 0x31u:
                        case 0x32u:
                            set_volume(code == 0x31u ? volume_command::PortamentoDown : volume_command::PortamentoUp, parameter);
                            break;
                        case 0x34u:
                            if (parameter >= 223u && parameter <= 232u)
                                set_volume(volume_command::Offset, static_cast<std::uint8_t>(parameter - 223u));
                            break;
                        default:
                            set_effect(translate_effect(code), parameter);
                            break;
                        }
                    }
                    auto const target = std::min(row + repeat, p.rows);
                    for (; row < target; ++row)
                        p.cells[static_cast<std::size_t>(row) * channels + channel] = c;
                }
            }
        }
        if (result.format == module_format::MOD && channels == 4u && onlyAmigaNotes)
            result.amigaLimits = true;

        // instruments (IT in sample mode still has their headers)
        bool const sampleMode = !isXM && (flags & FLAG_INSTRUMENT_MODE) == 0u;
        std::vector<std::array<std::uint8_t, 4u>> instrumentVibrato(isXM ? instrumentCount : 0u);
        result.instruments.resize(instrumentCount);
        for (std::uint32_t index = 0u; index < instrumentCount; ++index)
        {
            auto& ins = result.instruments[index];
            ins.name = read_string(data);
            if (version >= 5u)
                read_string(data);
            if (!data.can_read(INSTRUMENT_SIZE))
                throw module_load_error{ "bad MO3 instrument" };
            auto const offset = data.offset();
            data.skip(INSTRUMENT_SIZE);
            if (sampleMode)
                continue;
            if (isXM)
            {
                for (std::size_t note = 0u; note < 96u; ++note)
                    ins.keyboard[note + 12u] = static_cast<std::uint16_t>(m.u16le(offset + 4u + note * 4u + 2u) + 1u);
            }
            else
            {
                for (std::size_t note = 0u; note < 120u; ++note)
                {
                    ins.noteMap[note] = static_cast<std::uint8_t>(m.u16le(offset + 4u + note * 4u) + NOTE_MIN);
                    ins.keyboard[note] = static_cast<std::uint16_t>(m.u16le(offset + 4u + note * 4u + 2u) + 1u);
                }
            }
            read_envelope(m, offset + 484u, 0u, isXM, ins.volumeEnvelope);
            read_envelope(m, offset + 484u + ENVELOPE_SIZE, 0u, isXM, ins.panningEnvelope);
            read_envelope(m, offset + 484u + ENVELOPE_SIZE * 2u, 5u, isXM, ins.pitchEnvelope);
            if (isXM)
                for (std::size_t byte = 0u; byte < 4u; ++byte)
                    instrumentVibrato[index][byte] = m.u8(offset + 802u + byte);
            ins.fadeOut = m.u16le(offset + 806u);
            if (isIT)
                ins.globalVolume = static_cast<std::uint16_t>(std::min<std::uint8_t>(m.u8(offset + 812u), 128u) / 2u);
            auto const pan = m.u16le(offset + 813u);
            if (pan <= 256u)
                ins.pan = pan;
            ins.nna = static_cast<new_note_action>(std::min<std::uint8_t>(m.u8(offset + 815u), 3u));
            ins.pitchPanSeparation = m.i8(offset + 816u);
            ins.pitchPanCenter = m.u8(offset + 817u);
            ins.dct = static_cast<duplicate_check>(std::min<std::uint8_t>(m.u8(offset + 818u), 3u));
            ins.dca = static_cast<duplicate_action>(std::min<std::uint8_t>(m.u8(offset + 819u), 2u));
            ins.volumeSwing = static_cast<std::uint8_t>(std::min<std::uint16_t>(m.u16le(offset + 820u), 100u));
            ins.panSwing = static_cast<std::uint8_t>(std::min<std::uint16_t>(m.u16le(offset + 822u), 256u) / 4u);
            auto const cutoff = m.u8(offset + 824u);
            auto const resonance = m.u8(offset + 825u);
            if ((cutoff & 0x80u) != 0u)
                ins.cutoff = static_cast<std::uint8_t>(cutoff & 0x7Fu);
            if ((resonance & 0x80u) != 0u)
                ins.resonance = static_cast<std::uint8_t>(resonance & 0x7Fu);
        }
        if (sampleMode)
            result.instruments.clear();

        // samples: their headers here, their data after the music data
        struct stored_sample
        {
            std::uint16_t flags;
            std::int32_t compressedSize;
            std::uint16_t encoderDelay;
            std::int16_t sharedHeader;
            std::uint32_t length;
        };
        std::vector<stored_sample> stored;
        bool const frequencyIsHertz = version >= 5u || (flags & FLAG_LINEAR_SLIDES) == 0u;
        result.samples.resize(sampleCount);
        for (auto& s : result.samples)
        {
            s.name = read_string(data);
            if (version >= 5u)
                read_string(data);
            if (!data.can_read(SAMPLE_SIZE))
                throw module_load_error{ "bad MO3 sample" };
            auto const offset = data.offset();
            data.skip(SAMPLE_SIZE);
            auto const frequency = m.u32le(offset);
            if (isIT || result.format == module_format::S3M)
            {
                if (frequencyIsHertz)
                    s.c5speed = frequency;
                else
                    s.c5speed = static_cast<std::uint32_t>(std::lround(8363.0 * std::pow(2.0, static_cast<std::int32_t>(frequency + 1408u) / 1536.0)));
            }
            else
            {
                s.finetune = static_cast<std::int8_t>(static_cast<std::int8_t>(frequency) - 128);
                s.relativeNote = m.i8(offset + 4u);
            }
            s.volume = static_cast<std::uint16_t>(std::min<std::uint8_t>(m.u8(offset + 5u), 64u) * 4u);
            auto const pan = m.u16le(offset + 6u);
            if (pan <= 256u)
                s.pan = pan;
            s.length = m.u32le(offset + 8u);
            s.loopStart = m.u32le(offset + 12u);
            s.loopEnd = m.u32le(offset + 16u);
            auto const sampleFlags = m.u16le(offset + 20u);
            if ((sampleFlags & SAMPLE_LOOP) != 0u)
                s.loop = (sampleFlags & SAMPLE_PINGPONG_LOOP) != 0u ? loop_type::PingPong : loop_type::Forward;
            if ((sampleFlags & SAMPLE_SUSTAIN) != 0u)
                s.sustain = (sampleFlags & SAMPLE_PINGPONG_SUSTAIN) != 0u ? loop_type::PingPong : loop_type::Forward;
            static constexpr vibrato_type IT_VIBRATO[] = { vibrato_type::Sine, vibrato_type::RampDown, vibrato_type::Square, vibrato_type::Random,
                vibrato_type::RampUp, vibrato_type::Sine, vibrato_type::Sine, vibrato_type::Sine };
            s.vibratoType = IT_VIBRATO[m.u8(offset + 22u) & 7u];
            s.vibratoSweep = m.u8(offset + 23u);
            s.vibratoDepth = m.u8(offset + 24u);
            s.vibratoRate = m.u8(offset + 25u);
            if (isIT)
                s.globalVolume = std::min<std::uint8_t>(m.u8(offset + 26u), 64u);
            s.sustainStart = m.u32le(offset + 27u);
            s.sustainEnd = m.u32le(offset + 31u);
            stored_sample info{ sampleFlags, static_cast<std::int32_t>(m.u32le(offset + 35u)), m.u16le(offset + 39u), 0, s.length };
            if (version >= 5u && (sampleFlags & COMPRESSION_MASK) == COMPRESSION_SHARED_OGG)
                info.sharedHeader = static_cast<std::int16_t>(data.u16le());
            stored.push_back(info);
        }
        // XM: the instrument's auto-vibrato is its samples'
        if (isXM)
            for (std::size_t index = 0u; index < result.instruments.size(); ++index)
                for (auto const smp : result.instruments[index].keyboard)
                    if (smp >= 1u && smp <= result.samples.size())
                    {
                        auto& s = result.samples[smp - 1u];
                        s.vibratoType = static_cast<vibrato_type>(std::min<std::uint8_t>(instrumentVibrato[index][0], 4u));
                        s.vibratoSweep = instrumentVibrato[index][1];
                        s.vibratoDepth = instrumentVibrato[index][2];
                        s.vibratoRate = instrumentVibrato[index][3];
                    }

        // mix plugins (not played)
        if ((flags & FLAG_HAS_PLUGINS) != 0u && data.can_read(1u))
        {
            auto const pluginFlags = data.u8();
            if ((pluginFlags & 1u) != 0u)
                data.skip(static_cast<std::size_t>(channels) * 4u);
            while (data.can_read(1u))
            {
                if (data.u8() == 0u)
                    break;
                auto const length = data.u32le();
                if (!data.can_read(length))
                    throw module_load_error{ "bad MO3 plugin" };
                data.skip(length);
            }
        }

        // chunks: the tracker version, MIDI macros, and OpenMPT's extensions
        std::uint16_t cwtv = 0u;
        while (data.can_read(8u))
        {
            auto const id = data.u32le();
            auto const length = data.u32le();
            if (!data.can_read(length))
                throw module_load_error{ "bad MO3 chunk" };
            auto const start = data.offset();
            data.skip(length);
            if (id == 0x53524556u)          // "VERS"
            {
                if ((isIT || result.format == module_format::S3M) && length >= 2u)
                    cwtv = m.u16le(start);
            }
            else if (id == 0x49485250u)     // "PRHI"
            {
                if (length >= 1u)
                    result.rowsPerBeat = std::max<std::uint8_t>(m.u8(start), 1u);
            }
            else if (id == 0x4944494Du)     // "MIDI"
            {
                auto const read_macro = [&](std::size_t aIndex) -> std::string
                    {
                        std::string macro;
                        for (std::size_t index = 0u; index < 32u && (aIndex * 32u + index) < length; ++index)
                        {
                            auto const ch = static_cast<char>(m.u8(start + aIndex * 32u + index));
                            if (ch == '\0')
                                break;
                            if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'z'))
                                macro.push_back(ch);
                        }
                        return macro;
                    };
                for (std::size_t index = 0u; index < 16u; ++index)
                    result.parameteredMacros[index] = read_macro(9u + index);
                for (std::size_t index = 0u; index < 128u; ++index)
                    result.fixedMacros[index] = read_macro(25u + index);
            }
            else if (id == 0x54504D4Fu)     // "OMPT"
            {
                reader const chunk{ m.bytes(start, length) };
                std::size_t position = 0u;
                for (char const* names : { "PNAM", "CNAM" })
                    if (chunk.magic(position, names) && chunk.has(position + 4u, 4u))
                        position += 8u + chunk.u32le(position + 4u);
                position = read_instrument_extensions(chunk, position, result.instruments);
                if (chunk.magic(position, "STPM"))
                    read_song_extensions(chunk, position + 4u, result);
                if (cwtv > 0x0889u && cwtv <= 0x08FFu)
                {
                    result.format = module_format::MPTM;
                    result.formatName = "OpenMPT";
                }
            }
        }
        if ((isIT && cwtv >= 0x0100u && cwtv < 0x0214u) || (result.format == module_format::S3M && ((cwtv >= 0x3100u && cwtv < 0x3214u) || (cwtv >= 0x1300u && cwtv < 0x1320u))))
        {
            // old trackers had no filters, so any Zxx they saved means nothing
            for (auto& macro : result.parameteredMacros)
                macro.clear();
            for (auto& macro : result.fixedMacros)
                macro.clear();
        }
        if ((flags & FLAG_MODPLUG_MODE) != 0u)
        {
            // some of old ModPlug Tracker's behaviour
            if (result.openmptVersion == 0u)
                for (auto& ins : result.instruments)
                {
                    // the pitch envelope was a tick shorter
                    for (auto& point : ins.pitchEnvelope.points)
                        if (point.tick > 0u)
                            --point.tick;
                    ins.panSwing = static_cast<std::uint8_t>((ins.panSwing + 3u) / 4u);
                }
            if (result.openmptVersion < 0x01180000u)
            {
                result.playBehaviour.reset(behaviour::ITOffset);
                result.playBehaviour.reset(behaviour::FT2ST3OffsetOutOfRange);
            }
            if (result.openmptVersion < 0x01230000u)
                result.playBehaviour.reset(behaviour::FT2Periods);
            if (result.openmptVersion < 0x01260000u)
                result.playBehaviour.reset(behaviour::ITInstrWithNoteOff);
        }
        apply_openmpt_upgrades(result);

        // the sample data
        std::size_t offset = version >= 5u ? 12u + compressedSize : musicEnd;
        std::vector<std::pair<std::size_t, std::size_t>> chunks(result.samples.size(), { 0u, 0u });
        for (std::size_t index = 0u; index < result.samples.size(); ++index)
        {
            auto& s = result.samples[index];
            auto const& info = stored[index];
            auto const compression = info.flags & COMPRESSION_MASK;
            if (compression == 0u && info.compressedSize == 0)
            {
                sample_encoding encoding;
                encoding.width = (info.flags & SAMPLE_16BIT) != 0u ? sample_encoding::bits::Sixteen : sample_encoding::bits::Eight;
                encoding.stereo = (info.flags & SAMPLE_STEREO) != 0u;
                auto const loop = s.loop;
                auto const loopStart = s.loopStart;
                auto const loopEnd = s.loopEnd;
                offset += decode_pcm(aFile.bytes(offset, std::numeric_limits<std::size_t>::max()), s.length, encoding, s);
                s.loop = loop;
                s.loopStart = loopStart;
                s.loopEnd = loopEnd;
            }
            else if (info.compressedSize > 0)
            {
                chunks[index] = { offset, static_cast<std::size_t>(info.compressedSize) };
                offset += static_cast<std::size_t>(info.compressedSize);
            }
        }
        for (std::size_t index = 0u; index < result.samples.size(); ++index)
        {
            auto& s = result.samples[index];
            auto const& info = stored[index];
            if (info.compressedSize < 0 && static_cast<std::int64_t>(index) + info.compressedSize >= 0)
            {
                // the same waveform as an earlier sample
                auto const& original = result.samples[static_cast<std::size_t>(static_cast<std::int64_t>(index) + info.compressedSize)];
                s.data = original.data;
                s.stereo = original.stereo;
                s.length = original.length;
                continue;
            }
            if (info.length == 0u || chunks[index].second == 0u)
                continue;
            auto const compression = info.flags & COMPRESSION_MASK;
            std::uint32_t const sampleChannels = (info.flags & SAMPLE_STEREO) != 0u ? 2u : 1u;
            bool const sixteenBit = (info.flags & SAMPLE_16BIT) != 0u;
            if (compression == COMPRESSION_DELTA || compression == COMPRESSION_DELTA_PREDICTION)
            {
                // at best two bits a sample point
                std::uint64_t const maximum = static_cast<std::uint64_t>(chunks[index].second) * (4u / sampleChannels);
                auto const length = static_cast<std::uint32_t>(std::min<std::uint64_t>(info.length, maximum));
                bool const prediction = compression == COMPRESSION_DELTA_PREDICTION;
                reader const chunk{ aFile.bytes(chunks[index].first, chunks[index].second) };
                s.data.resize(static_cast<std::size_t>(length) * sampleChannels);
                if (sixteenBit)
                {
                    auto const decoded = unpack_delta<std::int16_t, std::uint16_t, 15, 8u>(chunk, 0u, length, sampleChannels, prediction);
                    for (std::size_t point = 0u; point < decoded.size(); ++point)
                        s.data[point] = static_cast<float>(decoded[point]) / 32768.0f;
                }
                else
                {
                    auto const decoded = unpack_delta<std::int8_t, std::uint8_t, 7, 4u>(chunk, 0u, length, sampleChannels, prediction);
                    for (std::size_t point = 0u; point < decoded.size(); ++point)
                        s.data[point] = static_cast<float>(decoded[point]) / 128.0f;
                }
                s.length = length;
                s.stereo = sampleChannels == 2u;
            }
            else if (compression == COMPRESSION_OGG || compression == COMPRESSION_SHARED_OGG)
            {
                // Ogg Vorbis: the stream's headers can be another sample's (the encoder delay is their size)
                std::size_t const sharedHeaderSize = info.encoderDelay;
                std::int64_t const sharedIndex = static_cast<std::int64_t>(index) + info.sharedHeader;
                bool const sharedHeader = info.sharedHeader != 0 && sharedIndex >= 0 && static_cast<std::size_t>(sharedIndex) < result.samples.size() &&
                    sharedHeaderSize > 0u && chunks[static_cast<std::size_t>(sharedIndex)].second >= sharedHeaderSize;
                auto data = reinterpret_cast<unsigned char const*>(aFile.bytes(chunks[index].first, chunks[index].second).data());
                int dataLeft = static_cast<int>(aFile.bytes(chunks[index].first, chunks[index].second).size());
                int consumed = 0;
                int error = 0;
                stb_vorbis* vorbis = nullptr;
                if (sharedHeader)
                {
                    auto const headerData = aFile.bytes(chunks[static_cast<std::size_t>(sharedIndex)].first, sharedHeaderSize);
                    vorbis = stb_vorbis_open_pushdata(reinterpret_cast<unsigned char const*>(headerData.data()), static_cast<int>(headerData.size()), &consumed, &error, nullptr);
                }
                else
                {
                    vorbis = stb_vorbis_open_pushdata(data, dataLeft, &consumed, &error, nullptr);
                    data += consumed;
                    dataLeft -= consumed;
                }
                s.data.assign(static_cast<std::size_t>(info.length) * sampleChannels, 0.0f);
                s.length = info.length;
                s.stereo = sampleChannels == 2u;
                if (vorbis != nullptr)
                {
                    std::uint32_t frame = 0u;
                    while ((error == VORBIS__no_error || (error == VORBIS_need_more_data && dataLeft > 0)) && frame < info.length)
                    {
                        int decodedChannels = 0;
                        int decoded = 0;
                        float** output = nullptr;
                        consumed = stb_vorbis_decode_frame_pushdata(vorbis, data, dataLeft, &decodedChannels, &output, &decoded);
                        data += consumed;
                        dataLeft -= consumed;
                        decoded = std::min<int>(decoded, static_cast<int>(info.length - frame));
                        if (decoded > 0 && decodedChannels == static_cast<int>(sampleChannels))
                        {
                            // as OpenMPT keeps them: 16 or 8 bits
                            float const scale = sixteenBit ? 32768.0f : 128.0f;
                            for (int point = 0; point < decoded; ++point)
                                for (std::uint32_t channel = 0u; channel < sampleChannels; ++channel)
                                {
                                    auto const value = std::clamp(std::round(output[channel][point] * scale), -scale, scale - 1.0f);
                                    s.data[(frame + static_cast<std::uint32_t>(point)) * sampleChannels + channel] = value / scale;
                                }
                        }
                        if (decoded > 0)
                            frame += static_cast<std::uint32_t>(decoded);
                        error = stb_vorbis_get_error(vorbis);
                        if (consumed == 0 && decoded <= 0)
                            break;
                    }
                    stb_vorbis_close(vorbis);
                }
                else
                {
                    s.data.clear();
                    s.length = 0u;
                }
            }
            else
            {
                // MP3 and OPL samples aren't supported: they play as silence
                s.data.clear();
                s.length = 0u;
            }
        }
        return result;
    }
}
