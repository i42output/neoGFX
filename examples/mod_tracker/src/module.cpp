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
Portions of this file (the default play behaviours, envelope evaluation, effect conversion and IT
sample decompression) are derived from OpenMPT (https://openmpt.org/).

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

#include <mod_tracker/module.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>

#include "loader.hpp"

namespace mod_tracker
{
    namespace
    {
        char hex_digit(std::uint32_t aValue)
        {
            return "0123456789ABCDEF"[aValue & 0x0Fu];
        }

        std::string hex(std::uint32_t aValue)
        {
            return std::string{ hex_digit(aValue >> 4u), hex_digit(aValue) };
        }

        std::string decimal(std::uint32_t aValue)
        {
            return std::string{ static_cast<char>('0' + (aValue / 10u) % 10u), static_cast<char>('0' + aValue % 10u) };
        }

        // the octave a tracker calls the note that plays a sample at its middle C frequency
        std::uint32_t octave_offset(module_format aFormat)
        {
            switch (aFormat)
            {
            case module_format::MOD:
                return 3u;
            case module_format::S3M:
            case module_format::XM:
            case module_format::DBM:
                return 1u;
            default:
                return 0u;
            }
        }

        bool mod_family(module_format aFormat)
        {
            return aFormat == module_format::MOD || aFormat == module_format::XM || aFormat == module_format::DBM;
        }

        char mod_letter(effect aEffect)
        {
            switch (aEffect)
            {
            case effect::Arpeggio: return '0';
            case effect::PortamentoUp: return '1';
            case effect::PortamentoDown: return '2';
            case effect::TonePortamento: return '3';
            case effect::Vibrato: return '4';
            case effect::TonePortamentoVolumeSlide: return '5';
            case effect::VibratoVolumeSlide: return '6';
            case effect::Tremolo: return '7';
            case effect::Panning: return '8';
            case effect::Offset: return '9';
            case effect::VolumeSlide: return 'A';
            case effect::PositionJump: return 'B';
            case effect::Volume: return 'C';
            case effect::PatternBreak: return 'D';
            case effect::ExtendedMod: return 'E';
            case effect::Speed: return 'F';
            case effect::Tempo: return 'F';
            case effect::GlobalVolume: return 'G';
            case effect::GlobalVolumeSlide: return 'H';
            case effect::KeyOff: return 'K';
            case effect::SetEnvelopePosition: return 'L';
            case effect::PanningSlide: return 'P';
            case effect::Retrigger: return 'R';
            case effect::ExtendedS3M: return 'S';
            case effect::Tremor: return 'T';
            case effect::ExtraFinePortamento: return 'X';
            case effect::Panbrello: return 'Y';
            case effect::Midi: return 'Z';
            case effect::ChannelVolume: return 'M';
            case effect::ChannelVolumeSlide: return 'N';
            default: return '?';
            }
        }

        char s3m_letter(effect aEffect)
        {
            switch (aEffect)
            {
            case effect::Speed: return 'A';
            case effect::PositionJump: return 'B';
            case effect::PatternBreak: return 'C';
            case effect::VolumeSlide: return 'D';
            case effect::PortamentoDown: return 'E';
            case effect::PortamentoUp: return 'F';
            case effect::TonePortamento: return 'G';
            case effect::Vibrato: return 'H';
            case effect::Tremor: return 'I';
            case effect::Arpeggio: return 'J';
            case effect::VibratoVolumeSlide: return 'K';
            case effect::TonePortamentoVolumeSlide: return 'L';
            case effect::ChannelVolume: return 'M';
            case effect::ChannelVolumeSlide: return 'N';
            case effect::Offset: return 'O';
            case effect::PanningSlide: return 'P';
            case effect::Retrigger: return 'Q';
            case effect::Tremolo: return 'R';
            case effect::ExtendedS3M: return 'S';
            case effect::Tempo: return 'T';
            case effect::FineVibrato: return 'U';
            case effect::GlobalVolume: return 'V';
            case effect::GlobalVolumeSlide: return 'W';
            case effect::Panning: return 'X';
            case effect::Panbrello: return 'Y';
            case effect::Midi: return 'Z';
            case effect::SmoothMidi: return '\\';
            case effect::DelayCut: return ':';
            case effect::ExtendedParameter: return '#';
            default: return '?';
            }
        }
    }

    void sample::sanitise()
    {
        if (stereo)
            length = std::min<std::uint32_t>(length, static_cast<std::uint32_t>(data.size() / 2u));
        else
            length = std::min<std::uint32_t>(length, static_cast<std::uint32_t>(data.size()));
        sustainEnd = std::min(sustainEnd, length);
        loopEnd = std::min(loopEnd, length);
        if (sustainStart >= sustainEnd)
        {
            sustainStart = sustainEnd = 0u;
            sustain = loop_type::None;
        }
        if (loopStart >= loopEnd)
        {
            loopStart = loopEnd = 0u;
            loop = loop_type::None;
        }
    }

    std::int32_t envelope::value_at(std::int32_t aPosition, std::int32_t aRangeOut, std::int32_t aRangeIn) const
    {
        if (points.empty())
            return 0;
        constexpr std::int64_t PRECISION = 1 << 16;
        auto const last = static_cast<std::uint32_t>(points.size() - 1u);
        auto point = last;
        for (std::uint32_t index = 0u; index < last; ++index)
            if (aPosition <= points[index].tick)
            {
                point = index;
                break;
            }
        std::int32_t const x2 = points[point].tick;
        std::int64_t value = 0;
        if (aPosition >= x2)
            value = points[point].value * PRECISION / aRangeIn;
        else
        {
            std::int32_t x1 = 0;
            if (point != 0u)
            {
                value = points[point - 1u].value * PRECISION / aRangeIn;
                x1 = points[point - 1u].tick;
            }
            if (x2 > x1 && aPosition > x1)
                value += (aPosition - x1) * (points[point].value * PRECISION / aRangeIn - value) / (x2 - x1);
        }
        value = std::clamp<std::int64_t>(value, 0, PRECISION);
        return static_cast<std::int32_t>((value * aRangeOut + PRECISION / 2) / PRECISION);
    }

    void envelope::sanitise(std::uint8_t aMaximumValue)
    {
        for (auto& point : points)
            point.value = std::min(point.value, aMaximumValue);
        if (points.empty())
        {
            enabled = loop = sustain = false;
            return;
        }
        auto const last = static_cast<std::uint8_t>(points.size() - 1u);
        loopStart = std::min(loopStart, last);
        loopEnd = std::clamp(loopEnd, loopStart, last);
        sustainStart = std::min(sustainStart, last);
        sustainEnd = std::clamp(sustainEnd, sustainStart, last);
    }

    void module_data::reset_macros()
    {
        // Impulse Tracker's defaults: SF0 sets the filter cutoff, Z80 .. Z8F set the resonance
        for (auto& macro : parameteredMacros)
            macro.clear();
        for (auto& macro : fixedMacros)
            macro.clear();
        parameteredMacros[0] = "F0F000z";
        for (std::uint32_t index = 0u; index < 16u; ++index)
            fixedMacros[index] = "F0F001" + hex(index * 8u);
    }

    behaviours default_behaviours(module_format aFormat)
    {
        behaviours result;
        auto set = [&](std::initializer_list<behaviour> aBehaviours)
            {
                for (auto b : aBehaviours)
                    result.set(b);
            };
        auto const itBehaviours = [&]()
            {
                set({ behaviour::CompatiblePlay, behaviour::PeriodsAreHertz, behaviour::TempoClamp, behaviour::PerChannelGlobalVolSlide, behaviour::PanOverride,
                    behaviour::ITInstrWithoutNote, behaviour::ITVolColFinePortamento, behaviour::ITArpeggio, behaviour::ITOutOfRangeDelay,
                    behaviour::ITPortaMemoryShare, behaviour::ITPatternLoopTargetReset, behaviour::ITFT2PatternLoop, behaviour::ITPingPongNoReset,
                    behaviour::ITEnvelopeReset, behaviour::ITClearOldNoteAfterCut, behaviour::ITVibratoTremoloPanbrello, behaviour::ITTremor,
                    behaviour::ITRetrigger, behaviour::ITMultiSampleBehaviour, behaviour::ITPortaTargetReached, behaviour::ITPatternLoopBreak,
                    behaviour::ITOffset, behaviour::ITSwingBehaviour, behaviour::ITNNAReset, behaviour::ITSCxStopsSample,
                    behaviour::ITEnvelopePositionHandling, behaviour::ITPortamentoInstrument, behaviour::ITPingPongMode,
                    behaviour::ITRealNoteMapping, behaviour::ITHighOffsetNoRetrig, behaviour::ITFilterBehaviour, behaviour::ITNoSurroundPan,
                    behaviour::ITShortSampleRetrig, behaviour::ITPortaNoNote, behaviour::ITFT2DontResetNoteOffOnPorta, behaviour::ITVolColMemory,
                    behaviour::ITPortamentoSwapResetsPos, behaviour::ITEmptyNoteMapSlot, behaviour::ITFirstTickHandling,
                    behaviour::ITSampleAndHoldPanbrello, behaviour::ITClearPortaTarget, behaviour::ITPanbrelloHold, behaviour::ITPanningReset,
                    behaviour::ITPatternLoopWithJumps, behaviour::ITInstrWithNoteOff, behaviour::ITMultiSampleInstrumentNumber,
                    behaviour::RowDelayWithNoteDelay, behaviour::ITInstrWithNoteOffOldEffects, behaviour::ITDoNotOverrideChannelPan,
                    behaviour::ITDCTBehaviour, behaviour::ITPitchPanSeparation, behaviour::ITResetFilterOnPortaSmpChange,
                    behaviour::ITInitialNoteMemory, behaviour::ITNoSustainOnPortamento, behaviour::ITEmptyNoteMapSlotIgnoreCell,
                    behaviour::ITOffsetWithInstrNumber, behaviour::ITDoublePortamentoSlides, behaviour::ITCarryAfterNoteOff,
                    behaviour::ITNoteCutWithPorta, behaviour::ITVolColNoSlidePropagation, behaviour::ITStoppedFilterEnvAtStart,
                    behaviour::ITCompatGxxCarryPortaWithIns });
            };
        switch (aFormat)
        {
        case module_format::IT:
            itBehaviours();
            break;
        case module_format::MPTM:
            set({ behaviour::PeriodsAreHertz, behaviour::PerChannelGlobalVolSlide, behaviour::PanOverride, behaviour::ITArpeggio,
                behaviour::ITPortaMemoryShare, behaviour::ITPatternLoopTargetReset, behaviour::ITFT2PatternLoop, behaviour::ITPingPongNoReset,
                behaviour::ITClearOldNoteAfterCut, behaviour::ITVibratoTremoloPanbrello, behaviour::ITMultiSampleBehaviour,
                behaviour::ITPortaTargetReached, behaviour::ITPatternLoopBreak, behaviour::ITSwingBehaviour, behaviour::ITSCxStopsSample,
                behaviour::ITEnvelopePositionHandling, behaviour::ITPingPongMode, behaviour::ITRealNoteMapping, behaviour::ITPortaNoNote,
                behaviour::ITVolColMemory, behaviour::ITFirstTickHandling, behaviour::ITClearPortaTarget, behaviour::ITSampleAndHoldPanbrello,
                behaviour::ITPanbrelloHold, behaviour::ITPanningReset, behaviour::ITInstrWithNoteOff, behaviour::ITDoNotOverrideChannelPan,
                behaviour::ITDCTBehaviour, behaviour::ITPitchPanSeparation });
            break;
        case module_format::XM:
            set({ behaviour::CompatiblePlay, behaviour::TempoClamp, behaviour::PerChannelGlobalVolSlide, behaviour::PanOverride, behaviour::ITFT2PatternLoop,
                behaviour::ITFT2DontResetNoteOffOnPorta, behaviour::FT2Arpeggio, behaviour::FT2Retrigger, behaviour::FT2VolColVibrato,
                behaviour::FT2PortaNoNote, behaviour::FT2KeyOff, behaviour::FT2PanSlide, behaviour::FT2ST3OffsetOutOfRange,
                behaviour::FT2RestrictXCommand, behaviour::FT2RetrigWithNoteDelay, behaviour::FT2SetPanEnvPos, behaviour::FT2PortaIgnoreInstr,
                behaviour::FT2VolColMemory, behaviour::FT2LoopE60Restart, behaviour::FT2ProcessSilentChannels,
                behaviour::FT2ReloadSampleSettings, behaviour::FT2PortaDelay, behaviour::FT2Transpose, behaviour::FT2PatternLoopWithJumps,
                behaviour::FT2PortaTargetNoReset, behaviour::FT2EnvelopeEscape, behaviour::FT2Tremor, behaviour::FT2OutOfRangeDelay,
                behaviour::FT2Periods, behaviour::FT2PanWithDelayedNoteOff, behaviour::FT2VolColDelay, behaviour::FT2FinetunePrecision,
                behaviour::FT2NoteOffFlags, behaviour::RowDelayWithNoteDelay, behaviour::FT2MODTremoloRampWaveform,
                behaviour::FT2PortaUpDownMemory, behaviour::FT2PanSustainRelease, behaviour::FT2NoteDelayWithoutInstr,
                behaviour::FT2PortaResetDirection, behaviour::FT2AutoVibratoAbortSweep, behaviour::FT2OffsetMemoryRequiresNote });
            break;
        case module_format::S3M:
            set({ behaviour::CompatiblePlay, behaviour::TempoClamp, behaviour::PanOverride, behaviour::ITPanbrelloHold, behaviour::FT2ST3OffsetOutOfRange,
                behaviour::ST3NoMutedChannels, behaviour::ST3PortaSampleChange, behaviour::ST3EffectMemory, behaviour::ST3VibratoMemory,
                behaviour::ST3PortaAfterArpeggio, behaviour::RowDelayWithNoteDelay, behaviour::ST3OffsetWithoutInstrument,
                behaviour::ST3RetrigAfterNoteCut, behaviour::ApplyUpperPeriodLimit });
            break;
        case module_format::MOD:
            set({ behaviour::RowDelayWithNoteDelay });
            break;
        case module_format::DBM:
            // DigiBooster modules are played as Impulse Tracker modules, with a few differences
            itBehaviours();
            result.reset(behaviour::ITInitialNoteMemory);
            result.set(behaviour::SlidesAtSpeed1);
            result.reset(behaviour::ITVibratoTremoloPanbrello);
            result.reset(behaviour::ITArpeggio);
            result.reset(behaviour::ITInstrWithNoteOff);
            result.reset(behaviour::ITInstrWithNoteOffOldEffects);
            break;
        }
        return result;
    }

    module_data load_module(std::span<std::byte const> aData)
    {
        detail::reader const file{ aData };
        module_data result;
        if (detail::is_mo3(file))
            result = detail::load_mo3(file);
        else if (detail::is_it(file))
            result = detail::load_it(file);
        else if (detail::is_xm(file))
            result = detail::load_xm(file);
        else if (detail::is_s3m(file))
            result = detail::load_s3m(file);
        else if (detail::is_dbm(file))
            result = detail::load_dbm(file);
        else
            result = detail::load_mod(file);
        if (result.channels == 0u || result.channels > MAX_CHANNELS)
            throw module_load_error{ "unsupported number of channels" };
        result.channelSettings.resize(result.channels);
        for (auto& p : result.patterns)
            p.cells.resize(static_cast<std::size_t>(p.rows) * result.channels);
        for (auto& s : result.samples)
            s.sanitise();
        for (auto& i : result.instruments)
        {
            i.volumeEnvelope.sanitise();
            i.panningEnvelope.sanitise();
            i.pitchEnvelope.sanitise();
        }
        // a song needs at least one playable position
        bool playable = false;
        for (auto order : result.orders)
            if (result.valid_pattern(order))
                playable = true;
        if (!playable)
            throw module_load_error{ "the song is empty" };
        return result;
    }

    module_data load_module(std::string const& aPath)
    {
        std::ifstream file{ aPath, std::ios::binary };
        if (!file)
            throw module_load_error{ "cannot open '" + aPath + "'" };
        std::vector<char> const contents{ std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
        return load_module(std::span<std::byte const>{ reinterpret_cast<std::byte const*>(contents.data()), contents.size() });
    }

    std::vector<std::string> const& module_file_patterns()
    {
        static std::vector<std::string> const sPatterns = { "*.mod", "*.s3m", "*.xm", "*.it", "*.mptm", "*.dbm", "*.mo3" };
        return sPatterns;
    }

    std::string note_text(module_data const& aModule, cell const& aCell)
    {
        static constexpr char const* sNames[12] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" };
        switch (aCell.note)
        {
        case NOTE_NONE:
            return "...";
        case NOTE_OFF:
            return "===";
        case NOTE_CUT:
            return "^^^";
        case NOTE_FADE:
            return "~~~";
        default:
            break;
        }
        if (!is_note(aCell.note))
            return "???";
        auto const note = static_cast<std::uint32_t>(aCell.note - NOTE_MIN);
        auto const offset = octave_offset(aModule.format);
        if (note / 12u < offset)
            return "???";
        auto const octave = note / 12u - offset;
        return std::string{ sNames[note % 12u] } + static_cast<char>(octave < 10u ? '0' + octave : 'A' + octave - 10u);
    }

    std::string instrument_text(cell const& aCell)
    {
        if (aCell.instrument == 0u)
            return "..";
        return aCell.instrument < 100u ? decimal(aCell.instrument) : hex(aCell.instrument);
    }

    bool has_volume_column(module_data const& aModule)
    {
        return aModule.format != module_format::MOD;
    }

    std::string volume_text(module_data const&, cell const& aCell)
    {
        char letter = '.';
        switch (aCell.volumeCommand)
        {
        case volume_command::None: return "...";
        case volume_command::Volume: letter = 'v'; break;
        case volume_command::Panning: letter = 'p'; break;
        case volume_command::VolumeSlideUp: letter = 'c'; break;
        case volume_command::VolumeSlideDown: letter = 'd'; break;
        case volume_command::FineVolumeUp: letter = 'a'; break;
        case volume_command::FineVolumeDown: letter = 'b'; break;
        case volume_command::VibratoSpeed: letter = 'u'; break;
        case volume_command::VibratoDepth: letter = 'h'; break;
        case volume_command::PanSlideLeft: letter = 'l'; break;
        case volume_command::PanSlideRight: letter = 'r'; break;
        case volume_command::TonePortamento: letter = 'g'; break;
        case volume_command::PortamentoUp: letter = 'f'; break;
        case volume_command::PortamentoDown: letter = 'e'; break;
        case volume_command::Offset: letter = 'o'; break;
        }
        return std::string{ letter } + decimal(aCell.volume);
    }

    std::string effect_text(module_data const& aModule, cell const& aCell)
    {
        if (aCell.command == effect::None)
            return "...";
        auto parameter = static_cast<std::uint32_t>(aCell.parameter);
        // pattern breaks are kept as the row number but every format except IT writes them in decimal digits
        if (aCell.command == effect::PatternBreak && !aModule.is_it())
            parameter = ((parameter / 10u) << 4u) | (parameter % 10u);
        char letter = mod_family(aModule.format) ? mod_letter(aCell.command) : s3m_letter(aCell.command);
        if (aCell.command == effect::Dummy)
            letter = '.';
        // MOD's Exy shows the sub-command as part of the parameter, as do the others
        return std::string{ letter } + hex(parameter);
    }

    namespace detail
    {
        std::string reader::text(std::size_t aOffset, std::size_t aLength) const
        {
            std::string result;
            for (std::size_t index = 0u; index < aLength && has(aOffset + index, 1u); ++index)
            {
                auto const ch = static_cast<char>(u8(aOffset + index));
                if (ch == '\0')
                    break;
                result.push_back(ch >= ' ' && ch <= '~' ? ch : ' ');
            }
            while (!result.empty() && result.back() == ' ')
                result.pop_back();
            return result;
        }

        std::size_t decode_pcm(std::span<std::byte const> aData, std::uint32_t aFrames, sample_encoding const& aEncoding, sample& aSample)
        {
            auto const channels = aEncoding.stereo ? 2u : 1u;
            auto const bytesPerSample = aEncoding.bytes_per_sample();
            auto const wanted = static_cast<std::size_t>(aFrames) * channels * bytesPerSample;
            auto const available = std::min(wanted, aData.size());
            // a truncated sample keeps the frames it has
            auto const frames = static_cast<std::uint32_t>(available / (channels * bytesPerSample));
            aSample.stereo = aEncoding.stereo;
            aSample.length = frames;
            aSample.data.assign(static_cast<std::size_t>(frames) * channels, 0.0f);
            auto const read = [&](std::size_t aIndex) -> std::int64_t
                {
                    auto const offset = aIndex * bytesPerSample;
                    std::uint32_t value = 0u;
                    for (std::size_t b = 0u; b < bytesPerSample; ++b)
                    {
                        auto const byte = static_cast<std::uint32_t>(aData[offset + (aEncoding.bigEndian ? b : bytesPerSample - 1u - b)]);
                        value = (value << 8u) | byte;
                    }
                    switch (aEncoding.width)
                    {
                    case sample_encoding::bits::Eight:
                        return aEncoding.isSigned ? static_cast<std::int8_t>(value) : static_cast<std::int64_t>(value) - 0x80;
                    case sample_encoding::bits::Sixteen:
                        return aEncoding.isSigned ? static_cast<std::int16_t>(value) : static_cast<std::int64_t>(value) - 0x8000;
                    default:
                        return aEncoding.isSigned ? static_cast<std::int32_t>(value) : static_cast<std::int64_t>(value) - 0x80000000ll;
                    }
                };
            double const scale = aEncoding.width == sample_encoding::bits::Eight ? 1.0 / 128.0 :
                aEncoding.width == sample_encoding::bits::Sixteen ? 1.0 / 32768.0 : 1.0 / 2147483648.0;
            for (std::uint32_t channel = 0u; channel < channels; ++channel)
            {
                std::int64_t accumulator = 0;
                for (std::uint32_t frame = 0u; frame < frames; ++frame)
                {
                    auto const index = aEncoding.stereoInterleaved ?
                        static_cast<std::size_t>(frame) * channels + channel : static_cast<std::size_t>(channel) * frames + frame;
                    auto value = read(index);
                    if (aEncoding.delta)
                    {
                        // deltas wrap around at the sample width
                        accumulator += value;
                        switch (aEncoding.width)
                        {
                        case sample_encoding::bits::Eight:
                            accumulator = static_cast<std::int8_t>(accumulator);
                            break;
                        case sample_encoding::bits::Sixteen:
                            accumulator = static_cast<std::int16_t>(accumulator);
                            break;
                        default:
                            accumulator = static_cast<std::int32_t>(accumulator);
                            break;
                        }
                        value = accumulator;
                    }
                    aSample.data[static_cast<std::size_t>(frame) * channels + channel] = static_cast<float>(static_cast<double>(value) * scale);
                }
            }
            // a stereo sample stored one channel after the other takes up the whole of its space regardless
            return std::min(wanted, aData.size());
        }

        namespace
        {
            class bit_reader
            {
            public:
                bit_reader(std::span<std::byte const> aData) :
                    iData{ aData }
                {
                }
            public:
                bool read(std::uint32_t aBits, std::uint32_t& aValue)
                {
                    while (iAvailable < aBits)
                    {
                        if (iPosition >= iData.size())
                            return false;
                        iBuffer |= static_cast<std::uint32_t>(iData[iPosition++]) << iAvailable;
                        iAvailable += 8u;
                    }
                    aValue = iBuffer & ((1u << aBits) - 1u);
                    iBuffer >>= aBits;
                    iAvailable -= aBits;
                    return true;
                }
            private:
                std::span<std::byte const> iData;
                std::size_t iPosition = 0u;
                std::uint32_t iBuffer = 0u;
                std::uint32_t iAvailable = 0u;
            };
        }

        std::size_t decode_it_compressed(std::span<std::byte const> aData, std::uint32_t aFrames, bool a16Bit, bool aStereo, bool aIT215, sample& aSample)
        {
            auto const channels = aStereo ? 2u : 1u;
            // every sample point takes at least a bit, so a corrupt length can't ask for more than there is
            aFrames = static_cast<std::uint32_t>(std::min<std::uint64_t>(aFrames, (static_cast<std::uint64_t>(aData.size()) * 8u) / channels));
            aSample.stereo = aStereo;
            aSample.length = aFrames;
            aSample.data.assign(static_cast<std::size_t>(aFrames) * channels, 0.0f);
            // compression parameters for 8-bit and 16-bit data
            std::int32_t const defaultWidth = a16Bit ? 17 : 9;
            std::uint32_t const fetchA = a16Bit ? 4u : 3u;
            std::int32_t const lowerB = a16Bit ? -8 : -4;
            std::int32_t const upperB = a16Bit ? 7 : 3;
            std::uint32_t const blockFrames = a16Bit ? 0x4000u : 0x8000u;
            float const scale = a16Bit ? 1.0f / 32768.0f : 1.0f / 128.0f;
            std::size_t offset = 0u;
            for (std::uint32_t channel = 0u; channel < channels; ++channel)
            {
                std::uint32_t written = 0u;
                while (written < aFrames && offset + 2u <= aData.size())
                {
                    auto const blockSize = static_cast<std::size_t>(static_cast<std::uint32_t>(aData[offset]) | (static_cast<std::uint32_t>(aData[offset + 1u]) << 8u));
                    offset += 2u;
                    if (blockSize == 0u)
                        continue;
                    bit_reader bits{ aData.subspan(offset, std::min(blockSize, aData.size() - offset)) };
                    offset = std::min(offset + blockSize, aData.size());
                    auto remaining = std::min(aFrames - written, blockFrames);
                    std::int32_t width = defaultWidth;
                    std::uint32_t mem1 = 0u;
                    std::uint32_t mem2 = 0u;
                    auto const write = [&](std::int32_t aValue, std::int32_t aTopBit)
                        {
                            if ((aValue & aTopBit) != 0)
                                aValue -= aTopBit << 1;
                            mem1 += static_cast<std::uint32_t>(aValue);
                            mem2 += mem1;
                            auto const raw = aIT215 ? mem2 : mem1;
                            auto const value = a16Bit ? static_cast<float>(static_cast<std::int16_t>(raw)) : static_cast<float>(static_cast<std::int8_t>(raw));
                            aSample.data[static_cast<std::size_t>(written) * channels + channel] = value * scale;
                            ++written;
                            --remaining;
                        };
                    auto const change_width = [&](std::int32_t aWidth)
                        {
                            ++aWidth;
                            if (aWidth >= width)
                                ++aWidth;
                            width = aWidth;
                        };
                    while (remaining > 0u)
                    {
                        if (width > defaultWidth || width < 1)
                            break;
                        std::uint32_t raw = 0u;
                        if (!bits.read(static_cast<std::uint32_t>(width), raw))
                            break;
                        auto const v = static_cast<std::int32_t>(raw);
                        std::int32_t const topBit = 1 << (width - 1);
                        if (width <= 6)
                        {
                            if (v == topBit)
                            {
                                std::uint32_t newWidth = 0u;
                                if (!bits.read(fetchA, newWidth))
                                    break;
                                change_width(static_cast<std::int32_t>(newWidth));
                            }
                            else
                                write(v, topBit);
                        }
                        else if (width < defaultWidth)
                        {
                            if (v >= topBit + lowerB && v <= topBit + upperB)
                                change_width(v - (topBit + lowerB));
                            else
                                write(v, topBit);
                        }
                        else
                        {
                            if ((v & topBit) != 0)
                                width = (v & ~topBit) + 1;
                            else
                                write(v & ~topBit, 0);
                        }
                    }
                }
            }
            return offset;
        }

        std::pair<effect, std::uint8_t> convert_mod_effect(std::uint8_t aCommand, std::uint8_t aParameter)
        {
            static constexpr effect sEffects[] =
            {
                effect::Arpeggio, effect::PortamentoUp, effect::PortamentoDown, effect::TonePortamento,
                effect::Vibrato, effect::TonePortamentoVolumeSlide, effect::VibratoVolumeSlide, effect::Tremolo,
                effect::Panning, effect::Offset, effect::VolumeSlide, effect::PositionJump,
                effect::Volume, effect::PatternBreak, effect::ExtendedMod, effect::Tempo,
                effect::GlobalVolume, effect::GlobalVolumeSlide, effect::None, effect::None,             // G H I J
                effect::KeyOff, effect::SetEnvelopePosition, effect::None, effect::None,                 // K L M N
                effect::None, effect::PanningSlide, effect::None, effect::Retrigger,                     // O P Q R
                effect::None, effect::Tremor, effect::None, effect::None,                                // S T U V
                effect::Dummy, effect::ExtraFinePortamento, effect::Panbrello, effect::Midi,             // W X Y Z
                effect::SmoothMidi, effect::SmoothMidi, effect::ExtendedParameter
            };
            if (aCommand == 0x00u && aParameter == 0x00u)
                return { effect::None, aParameter };
            if (aCommand == 0x0Fu && aParameter < 0x20u)
                return { effect::Speed, aParameter };
            if (aCommand >= std::size(sEffects))
                return { effect::None, aParameter };
            auto const result = sEffects[aCommand];
            if (result == effect::PatternBreak)
                return { result, static_cast<std::uint8_t>((aParameter >> 4u) * 10u + (aParameter & 0x0Fu)) };
            return { result, aParameter };
        }

        std::pair<effect, std::uint8_t> convert_s3m_effect(std::uint8_t aCommand, std::uint8_t aParameter, bool aFromIT)
        {
            switch (aCommand | 0x40u)
            {
            case '@': return { aParameter != 0u ? effect::Dummy : effect::None, aParameter };
            case 'A': return { effect::Speed, aParameter };
            case 'B': return { effect::PositionJump, aParameter };
            case 'C': return { effect::PatternBreak, aFromIT ? aParameter : static_cast<std::uint8_t>((aParameter >> 4u) * 10u + (aParameter & 0x0Fu)) };
            case 'D': return { effect::VolumeSlide, aParameter };
            case 'E': return { effect::PortamentoDown, aParameter };
            case 'F': return { effect::PortamentoUp, aParameter };
            case 'G': return { effect::TonePortamento, aParameter };
            case 'H': return { effect::Vibrato, aParameter };
            case 'I': return { effect::Tremor, aParameter };
            case 'J': return { effect::Arpeggio, aParameter };
            case 'K': return { effect::VibratoVolumeSlide, aParameter };
            case 'L': return { effect::TonePortamentoVolumeSlide, aParameter };
            case 'M': return { effect::ChannelVolume, aParameter };
            case 'N': return { effect::ChannelVolumeSlide, aParameter };
            case 'O': return { effect::Offset, aParameter };
            case 'P': return { effect::PanningSlide, aParameter };
            case 'Q': return { effect::Retrigger, aParameter };
            case 'R': return { effect::Tremolo, aParameter };
            case 'S': return { effect::ExtendedS3M, aParameter };
            case 'T': return { effect::Tempo, aParameter };
            case 'U': return { effect::FineVibrato, aParameter };
            case 'V': return { effect::GlobalVolume, aParameter };
            case 'W': return { effect::GlobalVolumeSlide, aParameter };
            case 'X': return { effect::Panning, aParameter };
            case 'Y': return { effect::Panbrello, aParameter };
            case 'Z': return { effect::Midi, aParameter };
            case '\\': return { aFromIT ? effect::SmoothMidi : effect::Midi, aParameter };
            case ']': return { aFromIT ? effect::DelayCut : effect::None, aParameter };
            case '[': return { aFromIT ? effect::ExtendedParameter : effect::None, aParameter };
            default: return { effect::None, aParameter };
            }
        }

        namespace
        {
            constexpr std::uint8_t IT_PORTA_VOLUME_COLUMN[16] =
            {
                0x00, 0x01, 0x04, 0x08, 0x10, 0x20, 0x40, 0x60, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
            };

            std::pair<volume_command, std::uint8_t> to_volume_command(effect aEffect, std::uint8_t aParameter, bool aForce)
            {
                switch (aEffect)
                {
                case effect::Volume:
                    return { volume_command::Volume, std::min<std::uint8_t>(aParameter, 64u) };
                case effect::PortamentoUp:
                    if (!aForce && ((aParameter & 3u) != 0u || aParameter >= 0xE0u))
                        break;
                    return { volume_command::PortamentoUp, static_cast<std::uint8_t>(aParameter / 4u) };
                case effect::PortamentoDown:
                    if (!aForce && ((aParameter & 3u) != 0u || aParameter >= 0xE0u))
                        break;
                    return { volume_command::PortamentoDown, static_cast<std::uint8_t>(aParameter / 4u) };
                case effect::TonePortamento:
                    if (aParameter >= 0xF0u)
                        return { volume_command::TonePortamento, 9u };
                    for (std::uint8_t n = 0u; n < 10u; ++n)
                        if (aForce ? aParameter <= IT_PORTA_VOLUME_COLUMN[n] : aParameter == IT_PORTA_VOLUME_COLUMN[n])
                            return { volume_command::TonePortamento, n };
                    break;
                case effect::Vibrato:
                    if (aForce)
                        aParameter = std::min<std::uint8_t>(aParameter & 0x0Fu, 9u);
                    else if ((aParameter & 0x0Fu) > 9u || (aParameter & 0xF0u) != 0u)
                        break;
                    return { volume_command::VibratoDepth, static_cast<std::uint8_t>(aParameter & 0x0Fu) };
                case effect::FineVibrato:
                    if (aForce)
                        aParameter = 0u;
                    else if (aParameter != 0u)
                        break;
                    return { volume_command::VibratoDepth, aParameter };
                case effect::Panning:
                    return { volume_command::Panning, static_cast<std::uint8_t>(aParameter == 255u ? 64u : aParameter / 4u) };
                case effect::VolumeSlide:
                    if (aParameter == 0u)
                        break;
                    if ((aParameter & 0x0Fu) == 0u)
                        return { volume_command::VolumeSlideUp, static_cast<std::uint8_t>(aParameter >> 4u) };
                    if ((aParameter & 0xF0u) == 0u)
                        return { volume_command::VolumeSlideDown, aParameter };
                    if ((aParameter & 0x0Fu) == 0x0Fu)
                        return { volume_command::FineVolumeUp, static_cast<std::uint8_t>(aParameter >> 4u) };
                    if ((aParameter & 0xF0u) == 0xF0u)
                        return { volume_command::FineVolumeDown, static_cast<std::uint8_t>(aParameter & 0x0Fu) };
                    break;
                case effect::ExtendedS3M:
                    if ((aParameter & 0xF0u) == 0x80u)
                        return { volume_command::Panning, static_cast<std::uint8_t>(((aParameter & 0x0Fu) << 2u) + 2u) };
                    break;
                case effect::ExtendedMod:
                    switch (aParameter & 0xF0u)
                    {
                    case 0x80u:
                        return { volume_command::Panning, static_cast<std::uint8_t>(((aParameter & 0x0Fu) << 2u) + 2u) };
                    case 0xA0u:
                        return { volume_command::FineVolumeUp, static_cast<std::uint8_t>(aParameter & 0x0Fu) };
                    case 0xB0u:
                        return { volume_command::FineVolumeDown, static_cast<std::uint8_t>(aParameter & 0x0Fu) };
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                return { volume_command::None, 0u };
            }

            std::size_t effect_weight(effect aEffect)
            {
                // lowest to highest
                static constexpr effect sWeights[] =
                {
                    effect::None, effect::Dummy, effect::ExtendedParameter, effect::SetEnvelopePosition, effect::KeyOff, effect::Tremolo,
                    effect::FineVibrato, effect::Vibrato, effect::ExtraFinePortamento, effect::Panbrello, effect::ExtendedS3M,
                    effect::ExtendedMod, effect::DelayCut, effect::Midi, effect::SmoothMidi, effect::PanningSlide, effect::Panning,
                    effect::PortamentoUp, effect::PortamentoDown, effect::VolumeSlide, effect::VibratoVolumeSlide, effect::Volume,
                    effect::Offset, effect::Tremor, effect::Retrigger, effect::Arpeggio, effect::TonePortamento,
                    effect::TonePortamentoVolumeSlide, effect::ChannelVolumeSlide, effect::ChannelVolume, effect::GlobalVolumeSlide,
                    effect::GlobalVolume, effect::Tempo, effect::Speed, effect::PositionJump, effect::PatternBreak
                };
                for (std::size_t index = 0u; index < std::size(sWeights); ++index)
                    if (sWeights[index] == aEffect)
                        return index;
                return 0u;
            }
        }

        std::pair<effect, std::uint8_t> fill_two_effects(cell& aCell, effect aEffect1, std::uint8_t aParameter1, effect aEffect2, std::uint8_t aParameter2)
        {
            if (aEffect1 == aEffect2)
            {
                switch (aEffect1)
                {
                case effect::Arpeggio:
                case effect::Panning:
                case effect::Offset:
                case effect::PositionJump:
                case effect::Volume:
                case effect::PatternBreak:
                case effect::Speed:
                case effect::Tempo:
                case effect::ChannelVolume:
                case effect::GlobalVolume:
                case effect::KeyOff:
                case effect::SetEnvelopePosition:
                case effect::Midi:
                case effect::SmoothMidi:
                case effect::DelayCut:
                case effect::Dummy:
                    aEffect2 = effect::None;
                    break;
                default:
                    break;
                }
            }
            for (std::uint8_t n = 0u; n < 4u; ++n)
            {
                auto const volume = to_volume_command(aEffect1, aParameter1, n > 1u);
                if (aEffect1 == effect::None || volume.first != volume_command::None)
                {
                    aCell.volumeCommand = volume.first;
                    aCell.volume = volume.second;
                    aCell.command = aEffect2;
                    aCell.parameter = aParameter2;
                    return { effect::None, 0u };
                }
                std::swap(aEffect1, aEffect2);
                std::swap(aParameter1, aParameter2);
            }
            // only one effect can be kept
            if (effect_weight(aEffect1) > effect_weight(aEffect2))
            {
                std::swap(aEffect1, aEffect2);
                std::swap(aParameter1, aParameter2);
            }
            if (aEffect2 == effect::Offset)
            {
                aCell.volumeCommand = volume_command::Offset;
                aCell.volume = static_cast<std::uint8_t>(aParameter2 != 0u ? std::max(aParameter2 * 9 / 255, 1) : 0);
                aCell.command = aEffect1;
                aCell.parameter = aParameter1;
                return { effect::None, 0u };
            }
            aCell.volumeCommand = volume_command::None;
            aCell.volume = 0u;
            aCell.command = aEffect2;
            aCell.parameter = aParameter2;
            return { aEffect1, aParameter1 };
        }

        void setup_amiga_panning(module_data& aModule)
        {
            // the Amiga's left, right, right, left, not hard (as OpenMPT plays them): a little of each side in the other
            aModule.channelSettings.resize(aModule.channels);
            for (std::uint32_t index = 0u; index < aModule.channels; ++index)
            {
                bool const left = (index % 4u == 0u || index % 4u == 3u);
                aModule.channelSettings[index].pan = left ? 0x40u : 0xC0u;
                aModule.channelSettings[index].volume = 64u;
            }
        }
    }
}
