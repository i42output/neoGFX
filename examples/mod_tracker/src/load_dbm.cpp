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
Portions of this file (the DigiBooster Pro loader (OpenMPT's soundlib/Load_dbm.cpp)) are derived
from OpenMPT (https://openmpt.org/).

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

// DigiBooster Pro modules (Amiga): IFF-style chunks, up to 128 channels, instruments that are each one
// sample with envelopes, and two effect columns per cell. The second effect goes in the volume column
// where it fits, as other players do. DigiBooster's echo DSP isn't played.

#include <mod_tracker/module.hpp>

#include <algorithm>
#include <map>
#include <set>

#include "loader.hpp"

namespace mod_tracker::detail
{
    namespace
    {
        constexpr std::size_t INSTRUMENT_SIZE = 50u;
        constexpr std::size_t ENVELOPE_SIZE = 136u;

        struct chunk
        {
            std::size_t offset = 0u;
            std::size_t length = 0u;
            bool found = false;
        };

        std::pair<effect, std::uint8_t> convert_effect(std::uint8_t aCommand, std::uint8_t aParameter)
        {
            static constexpr effect sEffects[] =
            {
                effect::Arpeggio, effect::PortamentoUp, effect::PortamentoDown, effect::TonePortamento,
                effect::Vibrato, effect::TonePortamentoVolumeSlide, effect::VibratoVolumeSlide, effect::Tremolo,
                effect::Panning, effect::Offset, effect::VolumeSlide, effect::PositionJump,
                effect::Volume, effect::PatternBreak, effect::ExtendedMod, effect::Tempo,
                effect::GlobalVolume, effect::GlobalVolumeSlide, effect::None, effect::None,
                effect::KeyOff, effect::SetEnvelopePosition, effect::None, effect::None,
                effect::None, effect::PanningSlide, effect::None, effect::None,
                effect::None, effect::None, effect::None,
                effect::Dummy,      // toggle the echo DSP
                effect::Dummy, effect::Dummy, effect::Dummy, effect::Dummy  // echo parameters
            };
            auto command = aCommand < std::size(sEffects) ? sEffects[aCommand] : effect::None;
            auto parameter = aParameter;
            switch (command)
            {
            case effect::Arpeggio:
                if (parameter == 0u)
                    command = effect::None;
                break;
            case effect::PatternBreak:
                parameter = static_cast<std::uint8_t>((parameter >> 4u) * 10u + (parameter & 0x0Fu));
                break;
            case effect::VolumeSlide:
            case effect::TonePortamentoVolumeSlide:
            case effect::VibratoVolumeSlide:
                // the up nibble takes precedence
                if ((parameter & 0xF0u) != 0x00u && (parameter & 0xF0u) != 0xF0u && (parameter & 0x0Fu) != 0x0Fu)
                    parameter &= 0xF0u;
                break;
            case effect::GlobalVolume:
                parameter = parameter <= 64u ? static_cast<std::uint8_t>(parameter * 2u) : 128u;
                break;
            case effect::ExtendedMod:
                switch (parameter & 0xF0u)
                {
                case 0x30u: // play backwards
                    command = effect::ExtendedS3M;
                    parameter = 0x9Fu;
                    break;
                case 0x40u: // turn the channel's sound off
                    command = effect::ExtendedS3M;
                    parameter = 0xC0u;
                    break;
                case 0x50u: // turn the channel off or on
                    if ((parameter & 0x0Fu) <= 0x01u)
                    {
                        command = effect::ChannelVolume;
                        parameter = (parameter == 0x50u) ? 0x00u : 0x40u;
                    }
                    break;
                case 0x70u: // coarse offset
                    command = effect::ExtendedS3M;
                    parameter = static_cast<std::uint8_t>(0xA0u | (parameter & 0x0Fu));
                    break;
                default:
                    break;
                }
                break;
            case effect::Tempo:
                if (parameter <= 0x1Fu)
                    command = effect::Speed;
                break;
            default:
                break;
            }
            return { command, parameter };
        }

        bool global_effect(effect aEffect, std::uint8_t aParameter)
        {
            switch (aEffect)
            {
            case effect::PositionJump:
            case effect::PatternBreak:
            case effect::Speed:
            case effect::Tempo:
            case effect::GlobalVolume:
            case effect::GlobalVolumeSlide:
                return true;
            case effect::ExtendedMod:
                return (aParameter & 0xF0u) == 0x00u || (aParameter & 0xF0u) == 0x60u || (aParameter & 0xF0u) == 0xE0u;
            case effect::ExtendedS3M:
                return (aParameter & 0xF0u) == 0x60u || (aParameter & 0xF0u) == 0x90u || (aParameter & 0xF0u) == 0xB0u || (aParameter & 0xF0u) == 0xE0u;
            default:
                return false;
            }
        }
    }

    bool is_dbm(reader const& aFile)
    {
        return aFile.has(0u, 8u) && aFile.magic(0u, "DBM0") && aFile.u8(4u) <= 3u;
    }

    module_data load_dbm(reader const& aFile)
    {
        module_data result;
        result.format = module_format::DBM;
        result.formatName = "DigiBooster Pro";
        auto const majorVersion = aFile.u8(4u);

        std::map<std::string, chunk> chunks;
        for (std::size_t offset = 8u; aFile.has(offset, 8u);)
        {
            std::string id;
            for (std::size_t index = 0u; index < 4u; ++index)
                id.push_back(static_cast<char>(aFile.u8(offset + index)));
            auto const length = static_cast<std::size_t>(aFile.u32be(offset + 4u));
            if (!chunks.contains(id))
                chunks[id] = chunk{ offset + 8u, std::min(length, aFile.size() - std::min(aFile.size(), offset + 8u)), true };
            offset += 8u + length;
        }
        auto const& info = chunks["INFO"];
        if (!info.found || info.length < 10u)
            throw module_load_error{ "DBM file has no INFO chunk" };
        auto const instrumentCount = aFile.u16be(info.offset);
        auto sampleCount = static_cast<std::uint32_t>(aFile.u16be(info.offset + 2u));
        auto const songCount = aFile.u16be(info.offset + 4u);
        auto const patternCount = aFile.u16be(info.offset + 6u);
        result.channels = std::clamp<std::uint32_t>(aFile.u16be(info.offset + 8u), 1u, MAX_CHANNELS);
        auto const fileChannels = static_cast<std::uint32_t>(aFile.u16be(info.offset + 8u));
        result.channelSettings.assign(result.channels, channel_settings{});
        result.playBehaviour = default_behaviours(module_format::DBM);
        result.itCompatibleGxx = true;
        result.itOldEffects = true;
        result.reset_macros();
        for (auto& macro : result.parameteredMacros)
            macro.clear();
        for (auto& macro : result.fixedMacros)
            macro.clear();

        if (auto const& name = chunks["NAME"]; name.found)
            result.title = aFile.text(name.offset, name.length);

        // the first song is the one played
        if (auto const& song = chunks["SONG"]; song.found && songCount > 0u && song.length >= 46u)
        {
            if (result.title.empty())
                result.title = aFile.text(song.offset, 44u);
            auto const orders = aFile.u16be(song.offset + 44u);
            for (std::uint32_t index = 0u; index < orders && aFile.has(song.offset + 46u + index * 2u, 2u); ++index)
                result.orders.push_back(aFile.u16be(song.offset + 46u + index * 2u));
        }

        // instruments, each playing one sample; a sample used by two instruments with different settings is
        // duplicated, as the settings belong to the sample
        std::map<std::uint32_t, std::uint32_t> copyOf;
        result.instruments.resize(instrumentCount);
        result.samples.resize(sampleCount);
        if (auto const& inst = chunks["INST"]; inst.found)
        {
            std::set<std::uint32_t> used;
            for (std::uint32_t index = 0u; index < instrumentCount; ++index)
            {
                auto const offset = inst.offset + index * INSTRUMENT_SIZE;
                if (!aFile.has(offset, INSTRUMENT_SIZE))
                    break;
                auto& ins = result.instruments[index];
                ins.name = aFile.text(offset, 30u);
                std::uint32_t mapped = aFile.u16be(offset + 30u);
                sample s;
                s.volume = static_cast<std::uint16_t>(std::min<std::uint16_t>(aFile.u16be(offset + 32u), 64u) * 4u);
                s.c5speed = static_cast<std::uint32_t>((static_cast<std::uint64_t>(aFile.u32be(offset + 34u)) * 8303u + 8363u / 2u) / 8363u);
                auto const loopStart = aFile.u32be(offset + 38u);
                auto const loopLength = aFile.u32be(offset + 42u);
                auto const pan = static_cast<std::int32_t>(aFile.i16be(offset + 46u));
                auto const flags = aFile.u16be(offset + 48u);
                if (loopLength != 0u && (flags & 0x03u) != 0u)
                {
                    s.loopStart = loopStart;
                    s.loopEnd = loopStart + loopLength;
                    s.loop = (flags & 0x02u) != 0u ? loop_type::PingPong : loop_type::Forward;
                }
                ins.fadeOut = 0u;
                ins.pan = static_cast<std::uint16_t>(std::clamp(pan + 128, 0, 256));
                if (mapped == 0u || mapped > sampleCount)
                    continue;
                if (used.contains(mapped))
                {
                    auto const& original = result.samples[mapped - 1u];
                    if (s.volume != original.volume || s.loop != original.loop || s.loopStart != original.loopStart ||
                        s.loopEnd != original.loopEnd || s.c5speed != original.c5speed)
                    {
                        result.samples.push_back(sample{});
                        copyOf[static_cast<std::uint32_t>(result.samples.size())] = mapped;
                        mapped = static_cast<std::uint32_t>(result.samples.size());
                    }
                }
                used.insert(mapped);
                auto& target = result.samples[mapped - 1u];
                s.name = ins.name;
                target = s;
                ins.keyboard.fill(static_cast<std::uint16_t>(mapped));
            }
        }
        auto const read_envelopes = [&](chunk const& aChunk, bool aPanning)
            {
                if (!aChunk.found || aChunk.length < 2u)
                    return;
                auto const count = aFile.u16be(aChunk.offset);
                for (std::uint32_t index = 0u; index < count; ++index)
                {
                    auto const offset = aChunk.offset + 2u + index * ENVELOPE_SIZE;
                    if (!aFile.has(offset, ENVELOPE_SIZE))
                        break;
                    auto const instrumentNumber = aFile.u16be(offset);
                    if (instrumentNumber == 0u || instrumentNumber > result.instruments.size())
                        continue;
                    auto& env = aPanning ? result.instruments[instrumentNumber - 1u].panningEnvelope : result.instruments[instrumentNumber - 1u].volumeEnvelope;
                    auto const flags = aFile.u8(offset + 2u);
                    auto const segments = aFile.u8(offset + 3u);
                    auto const sustain1 = aFile.u8(offset + 4u);
                    auto const loopBegin = aFile.u8(offset + 5u);
                    auto const loopEnd = aFile.u8(offset + 6u);
                    auto const sustain2 = aFile.u8(offset + 7u);
                    if (segments != 0u)
                    {
                        env.enabled = (flags & 0x01u) != 0u;
                        env.sustain = (flags & (0x02u | 0x08u)) != 0u;
                        env.loop = (flags & 0x04u) != 0u;
                    }
                    auto const points = static_cast<std::uint32_t>(std::min<std::uint8_t>(segments, 31u)) + 1u;
                    env.points.resize(points);
                    env.loopStart = loopBegin;
                    env.loopEnd = loopEnd;
                    if ((flags & 0x0Au) == 0x02u)
                        env.sustainStart = env.sustainEnd = sustain1;
                    else if ((flags & 0x0Au) == 0x08u)
                        env.sustainStart = env.sustainEnd = sustain2;
                    else
                        env.sustainStart = env.sustainEnd = std::min(sustain1, sustain2);
                    for (std::uint32_t point = 0u; point < points; ++point)
                    {
                        env.points[point].tick = aFile.u16be(offset + 8u + point * 4u);
                        auto value = aFile.u16be(offset + 10u + point * 4u);
                        // panning envelopes are -128 .. 128 in DigiBooster Pro 3
                        if (aPanning && majorVersion > 2u)
                            value = static_cast<std::uint16_t>((value + 128u) / 4u);
                        env.points[point].value = static_cast<std::uint8_t>(std::min<std::uint16_t>(value, 64u));
                    }
                }
            };
        read_envelopes(chunks["VENV"], false);
        read_envelopes(chunks["PENV"], true);
        // a note off cuts an instrument that has no volume envelope
        for (auto& ins : result.instruments)
            if (!ins.volumeEnvelope.enabled)
                ins.fadeOut = 32767u;

        // patterns
        result.patterns.resize(patternCount);
        if (auto const& patt = chunks["PATT"]; patt.found)
        {
            std::size_t offset = patt.offset;
            for (auto& p : result.patterns)
            {
                if (!aFile.has(offset, 6u))
                    break;
                p.rows = std::clamp<std::uint32_t>(aFile.u16be(offset), 1u, 1024u);
                auto const packedSize = aFile.u32be(offset + 2u);
                offset += 6u;
                p.cells.assign(static_cast<std::size_t>(p.rows) * result.channels, cell{});
                cursor data{ aFile, offset };
                std::size_t const end = offset + packedSize;
                offset = end;
                std::uint32_t row = 0u;
                std::vector<std::pair<effect, std::uint8_t>> lost;
                auto const place_lost = [&]()
                    {
                        for (auto const& command : lost)
                            for (std::uint32_t channel = 0u; channel < result.channels; ++channel)
                            {
                                auto& c = p.cells[row * result.channels + channel];
                                if (c.command == effect::None)
                                {
                                    c.command = command.first;
                                    c.parameter = command.second;
                                    break;
                                }
                            }
                        lost.clear();
                    };
                while (data.offset() < end && data.can_read(1u))
                {
                    auto const channel = data.u8();
                    if (channel == 0u)
                    {
                        place_lost();
                        if (++row >= p.rows)
                            break;
                        continue;
                    }
                    cell dummy;
                    cell& c = channel <= std::min(result.channels, fileChannels) ? p.cells[row * result.channels + channel - 1u] : dummy;
                    auto const b = data.u8();
                    if ((b & 0x01u) != 0u)
                    {
                        auto const note = data.u8();
                        if (note == 0x1Fu)
                            c.note = NOTE_OFF;
                        else if (note > 0u && note < 0xFEu)
                            c.note = static_cast<std::uint8_t>(std::min<std::uint32_t>((note >> 4u) * 12u + (note & 0x0Fu) + 13u, NOTE_MAX));
                    }
                    if ((b & 0x02u) != 0u)
                        c.instrument = data.u8();
                    if ((b & 0x3Cu) != 0u)
                    {
                        std::uint8_t c1 = 0u, p1 = 0u, c2 = 0u, p2 = 0u;
                        if ((b & 0x04u) != 0u) c2 = data.u8();
                        if ((b & 0x08u) != 0u) p2 = data.u8();
                        if ((b & 0x10u) != 0u) c1 = data.u8();
                        if ((b & 0x20u) != 0u) p1 = data.u8();
                        auto [e1, q1] = convert_effect(c1, p1);
                        auto [e2, q2] = convert_effect(c2, p2);
                        if (e2 == effect::Volume || (e2 == effect::None && e1 != effect::Volume))
                        {
                            std::swap(e1, e2);
                            std::swap(q1, q2);
                        }
                        else if (e1 == effect::TonePortamento && e2 == effect::Offset && q2 == 0u)
                            e2 = effect::None;
                        else if (e2 == effect::TonePortamento && e1 == effect::Offset && q1 == 0u)
                            e1 = effect::None;
                        auto const lostCommand = fill_two_effects(c, e1, q1, e2, q2);
                        if (global_effect(lostCommand.first, lostCommand.second))
                            lost.insert(lost.begin(), lostCommand);
                    }
                }
            }
        }

        // samples: signed big-endian PCM, 8, 16 or 32 bits
        if (auto const& smpl = chunks["SMPL"]; smpl.found)
        {
            std::size_t offset = smpl.offset;
            for (std::uint32_t index = 1u; index <= result.samples.size(); ++index)
            {
                if (auto const original = copyOf.find(index); original != copyOf.end())
                    continue;
                if (index > sampleCount || !aFile.has(offset, 8u))
                    break;
                auto const flags = aFile.u32be(offset);
                auto const length = aFile.u32be(offset + 4u);
                offset += 8u;
                if ((flags & 7u) == 0u)
                    continue;
                sample_encoding encoding;
                encoding.width = (flags & 4u) != 0u ? sample_encoding::bits::ThirtyTwo : (flags & 2u) != 0u ? sample_encoding::bits::Sixteen : sample_encoding::bits::Eight;
                encoding.bigEndian = true;
                auto& s = result.samples[index - 1u];
                auto const bytes = static_cast<std::size_t>(length) * encoding.bytes_per_sample();
                decode_pcm(aFile.bytes(offset, bytes), length, encoding, s);
                offset += bytes;
            }
            for (auto const& [copy, original] : copyOf)
            {
                auto& s = result.samples[copy - 1u];
                s.data = result.samples[original - 1u].data;
                s.length = result.samples[original - 1u].length;
                s.stereo = result.samples[original - 1u].stereo;
            }
        }
        return result;
    }
}
