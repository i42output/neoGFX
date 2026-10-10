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
Portions of this file (the S3M loader (OpenMPT's soundlib/Load_s3m.cpp)) are derived from OpenMPT
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

// Scream Tracker 3 modules: up to 32 channels, 99 samples, 64-row patterns, a volume column that only
// sets the volume, and effects named by letter.

#include <mod_tracker/module.hpp>

#include <algorithm>

#include "loader.hpp"

namespace mod_tracker::detail
{
    namespace
    {
        constexpr std::size_t HEADER_SIZE = 96u;

        enum tracker : std::uint16_t
        {
            TRACKER_MASK = 0xF000u,
            SCREAM_TRACKER = 0x1000u,
            IMAGO_ORPHEUS = 0x2000u,
            IMPULSE_TRACKER = 0x3000u,
            SCHISM_TRACKER = 0x4000u,
            OPENMPT = 0x5000u,
            BEROTRACKER = 0x6000u,
            ST3_00 = 0x1300u,
            ST3_01 = 0x1301u,
            ST3_20 = 0x1320u
        };
    }

    bool is_s3m(reader const& aFile)
    {
        if (!aFile.has(0u, HEADER_SIZE) || !aFile.magic(44u, "SCRM") || aFile.u8(29u) != 0x10u)
            return false;
        auto const version = aFile.u16le(42u);
        return version == 1u || version == 2u;
    }

    module_data load_s3m(reader const& aFile)
    {
        module_data result;
        result.format = module_format::S3M;
        result.formatName = "Scream Tracker 3";
        result.title = aFile.text(0u, 28u);
        auto const orderCount = aFile.u16le(32u);
        auto const sampleCount = aFile.u16le(34u);
        auto const patternCount = aFile.u16le(36u);
        auto const flags = aFile.u16le(38u);
        auto const cwtv = aFile.u16le(40u);
        auto const formatVersion = aFile.u16le(42u);
        auto const headerGlobalVolume = aFile.u8(48u);
        auto const headerSpeed = aFile.u8(49u);
        auto const headerTempo = aFile.u8(50u);
        auto const masterVolume = aFile.u8(51u);
        auto const ultraClicks = aFile.u8(52u);
        bool const usePanningTable = aFile.u8(53u) == 0xFCu;
        auto const special = aFile.u16le(62u);

        // the channels in use are the last one that isn't disabled and all before it
        std::uint32_t channels = 4u;
        std::array<std::uint8_t, 32u> channelTypes = {};
        for (std::uint32_t index = 0u; index < 32u; ++index)
        {
            channelTypes[index] = aFile.u8(64u + index);
            if (channelTypes[index] != 0xFFu)
                channels = index + 1u;
        }
        result.channels = channels;

        result.playBehaviour = default_behaviours(module_format::S3M);
        result.minimumPeriod = 64;
        result.maximumPeriod = 32767;

        // which tracker saved it decides some of how it plays
        bool isST3 = false;
        bool nonCompatibleTracker = false;
        bool keepMacros = false;
        switch (cwtv & TRACKER_MASK)
        {
        case SCREAM_TRACKER:
            if (cwtv == ST3_20 && special == 0u && (orderCount & 1u) == 0u && ultraClicks == 0u && (flags & ~0x50u) == 0u && usePanningTable)
            {
                // ModPlug Tracker / OpenMPT, Schism Tracker
                nonCompatibleTracker = true;
                keepMacros = true;
                result.playBehaviour.set(behaviour::ST3LimitPeriod);
            }
            else if (cwtv == ST3_20 && special == 0u && ultraClicks == 0u && (flags == 0u || flags == 8u) && !usePanningTable)
                ; // PlayerPRO, Velvet Studio, early Impulse Tracker
            else
                isST3 = true;
            break;
        case IMAGO_ORPHEUS:
            nonCompatibleTracker = true;
            break;
        case IMPULSE_TRACKER:
            nonCompatibleTracker = true;
            result.playBehaviour.set(behaviour::PeriodsAreHertz);
            result.playBehaviour.set(behaviour::ITRetrigger);
            result.playBehaviour.set(behaviour::ITShortSampleRetrig);
            result.playBehaviour.set(behaviour::ST3SampleSwap);
            result.playBehaviour.set(behaviour::ITPortaNoNote);
            result.playBehaviour.set(behaviour::ITPortamentoSwapResetsPos);
            result.minimumPeriod = 1;
            break;
        case SCHISM_TRACKER:
            nonCompatibleTracker = true;
            result.minimumPeriod = 1;
            break;
        case BEROTRACKER:
            result.playBehaviour.set(behaviour::ST3LimitPeriod);
            break;
        default:
            break;
        }
        if (nonCompatibleTracker)
        {
            result.playBehaviour.reset(behaviour::ST3NoMutedChannels);
            result.playBehaviour.reset(behaviour::ST3EffectMemory);
            result.playBehaviour.reset(behaviour::ST3PortaSampleChange);
            result.playBehaviour.reset(behaviour::ST3VibratoMemory);
            result.playBehaviour.reset(behaviour::ST3PortaAfterArpeggio);
            result.playBehaviour.reset(behaviour::ST3OffsetWithoutInstrument);
            result.playBehaviour.reset(behaviour::ApplyUpperPeriodLimit);
        }

        result.amigaLimits = (flags & 0x10u) != 0u;
        result.s3mOldVibrato = (flags & 0x01u) != 0u;
        result.fastVolumeSlides = cwtv == ST3_00 || (flags & 0x40u) != 0u;
        result.speed = (headerSpeed == 0u || (headerSpeed == 255u && isST3)) ? 6u : headerSpeed;
        result.tempo = headerTempo < 33u ? (isST3 ? 125.0 : 32.0) : static_cast<double>(headerTempo);
        result.globalVolume = std::min<std::uint32_t>(headerGlobalVolume, 64u) * 4u;
        if (result.globalVolume == 0u && cwtv < ST3_20)
            result.globalVolume = 256u;
        if (formatVersion == 1u && masterVolume < 8u)
            result.samplePreAmp = std::min((masterVolume + 1u) * 0x10u, 0x7Fu);
        else if (masterVolume == 2u || masterVolume == (2u | 0x10u))
            result.samplePreAmp = 0x20u;
        else if ((masterVolume & 0x7Fu) == 0u)
            result.samplePreAmp = 48u;
        else
            result.samplePreAmp = std::max<std::uint32_t>(masterVolume & 0x7Fu, 0x10u);
        bool const isStereo = (masterVolume & 0x80u) != 0u || (cwtv & TRACKER_MASK) == OPENMPT;
        if (!isStereo)
            result.samplePreAmp = (result.samplePreAmp * 8u + 5u) / 11u;

        // channel panning: the left/right flag of the channel type, then the optional panning table
        result.channelSettings.resize(channels);
        std::size_t offset = HEADER_SIZE;
        for (std::uint32_t index = 0u; index < channels; ++index)
        {
            auto const type = static_cast<std::uint8_t>(channelTypes[index] & 0x7Fu);
            if (channelTypes[index] != 0xFFu && isStereo)
                result.channelSettings[index].pan = (type & 8u) != 0u ? 0xCCu : 0x33u;
            if (type >= 16u && type <= 29u)
                result.channelSettings[index].pan = 128u;   // AdLib channel
        }

        // orders, then the sample and pattern parapointers
        for (std::uint32_t index = 0u; index < orderCount; ++index)
        {
            auto const order = aFile.u8(offset + index);
            if (order == 0xFFu)
                result.orders.push_back(module_data::ORDER_END);
            else if (order == 0xFEu)
                result.orders.push_back(module_data::ORDER_SKIP);
            else
                result.orders.push_back(order);
        }
        // trailing end-of-song markers say nothing
        while (!result.orders.empty() && result.orders.back() == module_data::ORDER_END)
            result.orders.pop_back();
        offset += orderCount;
        std::vector<std::uint32_t> samplePointers(sampleCount);
        for (auto& pointer : samplePointers)
        {
            pointer = static_cast<std::uint32_t>(aFile.u16le(offset)) * 16u;
            offset += 2u;
        }
        std::vector<std::uint32_t> patternPointers(patternCount);
        for (auto& pointer : patternPointers)
        {
            pointer = static_cast<std::uint32_t>(aFile.u16le(offset)) * 16u;
            offset += 2u;
        }
        if (usePanningTable)
        {
            for (std::uint32_t index = 0u; index < channels; ++index)
            {
                auto const pan = aFile.has(offset + index, 1u) ? aFile.u8(offset + index) : 0u;
                bool const adlib = (channelTypes[index] & 0x7Fu) >= 16u && (channelTypes[index] & 0x7Fu) <= 29u;
                if ((pan & 0x20u) != 0u && (!isST3 || !adlib))
                    result.channelSettings[index].pan = static_cast<std::uint16_t>(((pan & 0x0Fu) * 256u + 8u) / 15u);
            }
        }

        // samples
        bool const signedSamples = (formatVersion == 1u);
        std::uint16_t gusAddresses = 0u;
        bool anySamples = false;
        result.samples.resize(sampleCount);
        for (std::uint32_t index = 0u; index < sampleCount; ++index)
        {
            auto const header = samplePointers[index];
            if (header == 0u || !aFile.has(header, 80u))
                continue;
            auto& s = result.samples[index];
            auto const type = aFile.u8(header);
            s.name = aFile.text(header + 48u, 28u);
            auto const dataPointer = (static_cast<std::uint32_t>(aFile.u8(header + 13u)) << 20u) |
                (static_cast<std::uint32_t>(aFile.u8(header + 15u)) << 12u) | (static_cast<std::uint32_t>(aFile.u8(header + 14u)) << 4u);
            auto const length = aFile.u32le(header + 16u);
            auto const loopStart = aFile.u32le(header + 20u);
            auto const loopEnd = aFile.u32le(header + 24u);
            auto const volume = aFile.u8(header + 28u);
            auto const pack = aFile.u8(header + 30u);
            auto const sampleFlags = aFile.u8(header + 31u);
            auto c5speed = aFile.u32le(header + 32u);
            auto const gusAddress = aFile.u16le(header + 40u);
            s.volume = static_cast<std::uint16_t>(std::min<std::uint8_t>(volume, 64u) * 4u);
            if (isST3)
                c5speed = std::min<std::uint32_t>(c5speed, 0xFFFFu);
            s.c5speed = c5speed == 0u ? 8363u : std::max<std::uint32_t>(c5speed, 1024u);
            if (type != 1u && type != 0u)
                continue; // AdLib instruments aren't played
            if (type == 1u)
            {
                s.length = length;
                s.loopStart = length != 0u ? std::min(loopStart, length - 1u) : 0u;
                s.loopEnd = std::min(loopEnd, length);
                s.loop = (sampleFlags & 0x01u) != 0u ? loop_type::Forward : loop_type::None;
                if (s.loopEnd < 2u || s.loopStart >= s.loopEnd)
                {
                    s.loopStart = s.loopEnd = 0u;
                    s.loop = loop_type::None;
                }
            }
            gusAddresses |= gusAddress;
            if (type == 1u && length != 0u && pack == 0u)
            {
                anySamples = true;
                sample_encoding encoding;
                encoding.width = (sampleFlags & 0x04u) != 0u ? sample_encoding::bits::Sixteen : sample_encoding::bits::Eight;
                encoding.stereo = (sampleFlags & 0x02u) != 0u;
                encoding.isSigned = signedSamples;
                decode_pcm(aFile.bytes(dataPointer, static_cast<std::size_t>(length) * encoding.bytes_per_sample() * (encoding.stereo ? 2u : 1u)), length, encoding, s);
            }
        }
        if (isST3 && anySamples)
        {
            if (gusAddresses == 0u && cwtv != ST3_00)
                isST3 = false;
            else
            {
                // saved with the Gravis Ultrasound driver loaded, or with the SoundBlaster driver
                bool const useGUS = gusAddresses > 1u;
                result.playBehaviour.set(behaviour::ST3PortaSampleChange, useGUS);
                result.playBehaviour.set(behaviour::ST3SampleSwap, !useGUS);
                result.playBehaviour.set(behaviour::ITShortSampleRetrig, !useGUS);
                if (useGUS)
                    result.samplePreAmp = 48u;
            }
        }
        if (isST3)
            result.playBehaviour.set(behaviour::S3MIgnoreCombinedFineSlides);

        // patterns: 64 rows, packed
        result.patterns.resize(std::min<std::uint32_t>(patternCount, 255u));
        for (std::uint32_t index = 0u; index < result.patterns.size(); ++index)
        {
            auto& p = result.patterns[index];
            p.rows = 64u;
            p.cells.assign(64u * channels, cell{});
            if (patternPointers[index] == 0u || !aFile.has(patternPointers[index], 2u))
                continue;
            cursor data{ aFile, patternPointers[index] + 2u };
            std::uint32_t row = 0u;
            cell dummy;
            while (row < 64u && data.can_read(1u))
            {
                auto const info = data.u8();
                if (info == 0u)
                {
                    ++row;
                    continue;
                }
                auto const channel = static_cast<std::uint32_t>(info & 0x1Fu);
                cell& c = channel < channels ? p.cells[row * channels + channel] : dummy;
                if ((info & 0x20u) != 0u)
                {
                    auto const note = data.u8();
                    auto const instrument = data.u8();
                    if (note < 0xF0u)
                        c.note = static_cast<std::uint8_t>(std::clamp<std::uint32_t>((note & 0x0Fu) + 12u * (note >> 4u) + 12u + NOTE_MIN, NOTE_MIN, NOTE_MAX));
                    else if (note == 0xFEu)
                        c.note = NOTE_CUT;
                    c.instrument = instrument;
                }
                if ((info & 0x40u) != 0u)
                {
                    auto const volume = data.u8();
                    if (volume >= 128u && volume <= 192u)
                    {
                        c.volumeCommand = volume_command::Panning;
                        c.volume = static_cast<std::uint8_t>(volume - 128u);
                    }
                    else
                    {
                        c.volumeCommand = volume_command::Volume;
                        c.volume = std::min<std::uint8_t>(volume, 64u);
                    }
                }
                if ((info & 0x80u) != 0u)
                {
                    auto const command = data.u8();
                    auto const parameter = data.u8();
                    std::tie(c.command, c.parameter) = convert_s3m_effect(command, parameter, false);
                    // the old SoundBlaster stereo control command
                    if (c.command == effect::ExtendedS3M && (c.parameter & 0xF0u) == 0xA0u && cwtv < ST3_20)
                    {
                        auto const type = static_cast<std::uint8_t>(channelTypes[std::min(channel, 31u)] & 0x7Fu);
                        if (type >= 0x10u)
                            c.command = effect::Dummy;
                        else if (c.parameter == 0xA0u || c.parameter == 0xA2u)
                            c.parameter = (type & 8u) != 0u ? 0x8Cu : 0x83u;
                        else if (c.parameter == 0xA1u || c.parameter == 0xA3u)
                            c.parameter = (type & 8u) != 0u ? 0x83u : 0x8Cu;
                        else if (c.parameter <= 0xA7u)
                            c.parameter = 0x88u;
                        else
                            c.command = effect::Dummy;
                    }
                    else if (c.command == effect::Offset && c.parameter == 0u && isST3 && cwtv <= ST3_01)
                        c.command = effect::Dummy;
                }
            }
        }
        // Scream Tracker ignored Zxx, so only files from trackers that had filters keep the filter macros
        if ((cwtv & TRACKER_MASK) > SCREAM_TRACKER && ((cwtv & TRACKER_MASK) != IMPULSE_TRACKER || cwtv >= 0x3214u))
            keepMacros = true;
        result.reset_macros();
        if (!keepMacros)
        {
            for (auto& macro : result.parameteredMacros)
                macro.clear();
            for (auto& macro : result.fixedMacros)
                macro.clear();
        }
        return result;
    }
}
