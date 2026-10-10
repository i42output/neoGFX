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
Portions of this file (the IT and MPTM loader, OpenMPT's extensions and module upgrades (OpenMPT's
soundlib/Load_it.cpp, InstrumentExtensions.cpp and UpgradeModule.cpp)) are derived from OpenMPT
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

// Impulse Tracker modules, and OpenMPT's MPTM format, which is IT with extensions: up to 64 channels,
// patterns of up to 200 rows, instruments with volume, panning and pitch (or filter) envelopes, new note
// actions, resonant filters and compressed samples.
//
// (Open)ModPlug Tracker saved a great many IT files, and played them in its own way, which it records in
// the file (or which is implied by the version that saved it); the behaviours are adjusted to match.

#include <mod_tracker/module.hpp>

#include <algorithm>
#include <cstring>
#include <string_view>

#include "loader.hpp"

namespace mod_tracker::detail
{
    namespace
    {
        constexpr std::size_t HEADER_SIZE = 192u;
        constexpr std::size_t SAMPLE_HEADER_SIZE = 80u;
        constexpr std::size_t INSTRUMENT_SIZE = 554u;
        constexpr std::size_t MACRO_LENGTH = 32u;

        // OpenMPT's versions: each part is written (and compared) in hexadecimal, 1.17.02.46 being 0x01170246
        constexpr std::uint32_t version(std::uint32_t aMajor, std::uint32_t aMinor, std::uint32_t aRevision, std::uint32_t aBuild)
        {
            return (aMajor << 24u) | (aMinor << 16u) | (aRevision << 8u) | aBuild;
        }

        // Schism Tracker's versions are dates, as days
        constexpr std::int32_t schism_date(std::int32_t aYear, std::int32_t aMonth, std::int32_t aDay)
        {
            std::int32_t const mm = (aMonth + 9) % 12;
            std::int32_t const yy = aYear - mm / 10;
            return yy * 365 + yy / 4 - yy / 100 + yy / 400 + (mm * 306 + 5) / 10 + (aDay - 1);
        }
        constexpr std::int32_t SCHISM_EPOCH = schism_date(2009, 10, 31);

        // the order of OpenMPT's behaviour flags, as saved in a file's "MSF." chunk
        constexpr behaviour OPENMPT_BEHAVIOURS[] =
        {
            behaviour::CompatiblePlay, behaviour::MPTOldSwingBehaviour, behaviour::Count, behaviour::Count, behaviour::FT2VolumeRamping,
            behaviour::MODVBlankTiming, behaviour::SlidesAtSpeed1, behaviour::PeriodsAreHertz, behaviour::TempoClamp,
            behaviour::PerChannelGlobalVolSlide, behaviour::PanOverride, behaviour::ITInstrWithoutNote, behaviour::ITVolColFinePortamento,
            behaviour::ITArpeggio, behaviour::ITOutOfRangeDelay, behaviour::ITPortaMemoryShare, behaviour::ITPatternLoopTargetReset,
            behaviour::ITFT2PatternLoop, behaviour::ITPingPongNoReset, behaviour::ITEnvelopeReset, behaviour::ITClearOldNoteAfterCut,
            behaviour::ITVibratoTremoloPanbrello, behaviour::ITTremor, behaviour::ITRetrigger, behaviour::ITMultiSampleBehaviour,
            behaviour::ITPortaTargetReached, behaviour::ITPatternLoopBreak, behaviour::ITOffset, behaviour::ITSwingBehaviour,
            behaviour::ITNNAReset, behaviour::ITSCxStopsSample, behaviour::ITEnvelopePositionHandling, behaviour::ITPortamentoInstrument,
            behaviour::ITPingPongMode, behaviour::ITRealNoteMapping, behaviour::ITHighOffsetNoRetrig, behaviour::ITFilterBehaviour,
            behaviour::ITNoSurroundPan, behaviour::ITShortSampleRetrig, behaviour::ITPortaNoNote, behaviour::ITFT2DontResetNoteOffOnPorta,
            behaviour::ITVolColMemory, behaviour::ITPortamentoSwapResetsPos, behaviour::ITEmptyNoteMapSlot, behaviour::ITFirstTickHandling,
            behaviour::ITSampleAndHoldPanbrello, behaviour::ITClearPortaTarget, behaviour::ITPanbrelloHold, behaviour::ITPanningReset,
            behaviour::ITPatternLoopWithJumpsOld, behaviour::ITInstrWithNoteOff, behaviour::FT2Arpeggio, behaviour::FT2Retrigger,
            behaviour::FT2VolColVibrato, behaviour::FT2PortaNoNote, behaviour::FT2KeyOff, behaviour::FT2PanSlide,
            behaviour::FT2ST3OffsetOutOfRange, behaviour::FT2RestrictXCommand, behaviour::FT2RetrigWithNoteDelay,
            behaviour::FT2SetPanEnvPos, behaviour::FT2PortaIgnoreInstr, behaviour::FT2VolColMemory, behaviour::FT2LoopE60Restart,
            behaviour::FT2ProcessSilentChannels, behaviour::FT2ReloadSampleSettings, behaviour::FT2PortaDelay, behaviour::FT2Transpose,
            behaviour::FT2PatternLoopWithJumps, behaviour::FT2PortaTargetNoReset, behaviour::FT2EnvelopeEscape, behaviour::FT2Tremor,
            behaviour::FT2OutOfRangeDelay, behaviour::FT2Periods, behaviour::FT2PanWithDelayedNoteOff, behaviour::FT2VolColDelay,
            behaviour::FT2FinetunePrecision, behaviour::ST3NoMutedChannels, behaviour::ST3EffectMemory, behaviour::ST3PortaSampleChange,
            behaviour::ST3VibratoMemory, behaviour::ST3LimitPeriod, behaviour::ST3PortaAfterArpeggio, behaviour::MODOneShotLoops,
            behaviour::MODIgnorePanning, behaviour::MODSampleSwap, behaviour::FT2NoteOffFlags, behaviour::ITMultiSampleInstrumentNumber,
            behaviour::RowDelayWithNoteDelay, behaviour::FT2MODTremoloRampWaveform, behaviour::FT2PortaUpDownMemory,
            behaviour::MODOutOfRangeNoteDelay, behaviour::MODTempoOnSecondTick, behaviour::FT2PanSustainRelease, behaviour::Count,
            behaviour::Count, behaviour::ST3OffsetWithoutInstrument, behaviour::Count, behaviour::FT2NoteDelayWithoutInstr,
            behaviour::Count, behaviour::ITInstrWithNoteOffOldEffects, behaviour::Count, behaviour::ITDoNotOverrideChannelPan,
            behaviour::ITPatternLoopWithJumps, behaviour::ITDCTBehaviour, behaviour::Count, behaviour::ST3RetrigAfterNoteCut,
            behaviour::ST3SampleSwap, behaviour::Count, behaviour::Count, behaviour::Count, behaviour::Count,
            behaviour::FT2PortaResetDirection, behaviour::ApplyUpperPeriodLimit, behaviour::Count, behaviour::ITPitchPanSeparation,
            behaviour::ImprecisePingPongLoops, behaviour::Count, behaviour::Count, behaviour::ITResetFilterOnPortaSmpChange,
            behaviour::ITInitialNoteMemory, behaviour::Count, behaviour::ITNoSustainOnPortamento, behaviour::ITEmptyNoteMapSlotIgnoreCell,
            behaviour::ITOffsetWithInstrNumber, behaviour::Count, behaviour::Count, behaviour::ITDoublePortamentoSlides,
            behaviour::S3MIgnoreCombinedFineSlides, behaviour::FT2AutoVibratoAbortSweep, behaviour::Count, behaviour::Count,
            behaviour::ITCarryAfterNoteOff, behaviour::FT2OffsetMemoryRequiresNote, behaviour::ITNoteCutWithPorta,
            behaviour::ITVolColNoSlidePropagation, behaviour::ITStoppedFilterEnvAtStart, behaviour::ITCompatGxxCarryPortaWithIns
        };

        void read_envelope(reader const& aFile, std::size_t aOffset, std::int32_t aValueOffset, envelope& aEnvelope)
        {
            auto const flags = aFile.u8(aOffset);
            auto const count = std::min<std::uint8_t>(aFile.u8(aOffset + 1u), 25u);
            aEnvelope.enabled = (flags & 0x01u) != 0u;
            aEnvelope.loop = (flags & 0x02u) != 0u;
            aEnvelope.sustain = (flags & 0x04u) != 0u;
            aEnvelope.carry = (flags & 0x08u) != 0u;
            aEnvelope.filter = (flags & 0x80u) != 0u;
            aEnvelope.loopStart = std::min<std::uint8_t>(aFile.u8(aOffset + 2u), 25u);
            aEnvelope.loopEnd = std::clamp<std::uint8_t>(aFile.u8(aOffset + 3u), aEnvelope.loopStart, 25u);
            aEnvelope.sustainStart = std::min<std::uint8_t>(aFile.u8(aOffset + 4u), 25u);
            aEnvelope.sustainEnd = std::clamp<std::uint8_t>(aFile.u8(aOffset + 5u), aEnvelope.sustainStart, 25u);
            aEnvelope.points.resize(count);
            for (std::uint32_t index = 0u; index < count; ++index)
            {
                auto& point = aEnvelope.points[index];
                point.value = static_cast<std::uint8_t>(std::clamp<std::int32_t>(aFile.i8(aOffset + 6u + index * 3u) + aValueOffset, 0, 64));
                point.tick = aFile.u16le(aOffset + 7u + index * 3u);
                if (index > 0u && point.tick < aEnvelope.points[index - 1u].tick && (point.tick & 0xFF00u) == 0u)
                {
                    point.tick = static_cast<std::uint16_t>(point.tick | (aEnvelope.points[index - 1u].tick & 0xFF00u));
                    if (point.tick < aEnvelope.points[index - 1u].tick)
                        point.tick = static_cast<std::uint16_t>(point.tick + 0x100u);
                }
            }
        }

        void read_instrument(reader const& aFile, std::size_t aOffset, std::uint16_t aCompatibleVersion, instrument& aInstrument)
        {
            if (!aFile.magic(aOffset, "IMPI"))
                return;
            if (aCompatibleVersion < 0x0200u)
            {
                // Impulse Tracker 1.x instruments: a volume envelope only
                aInstrument.name = aFile.text(aOffset + 32u, 26u);
                aInstrument.fadeOut = static_cast<std::uint32_t>(aFile.u16le(aOffset + 24u)) << 6u;
                aInstrument.nna = static_cast<new_note_action>(std::min<std::uint8_t>(aFile.u8(aOffset + 26u), 3u));
                aInstrument.dct = static_cast<duplicate_check>(std::min<std::uint8_t>(aFile.u8(aOffset + 27u), 3u));
                for (std::uint32_t note = 0u; note < 120u; ++note)
                {
                    auto const mapped = aFile.u8(aOffset + 64u + note * 2u);
                    aInstrument.keyboard[note] = aFile.u8(aOffset + 65u + note * 2u);
                    aInstrument.noteMap[note] = static_cast<std::uint8_t>(mapped < 120u ? mapped + NOTE_MIN : note + NOTE_MIN);
                }
                auto const flags = aFile.u8(aOffset + 17u);
                auto& env = aInstrument.volumeEnvelope;
                env.enabled = (flags & 0x01u) != 0u;
                env.loop = (flags & 0x02u) != 0u;
                env.sustain = (flags & 0x04u) != 0u;
                env.loopStart = aFile.u8(aOffset + 18u);
                env.loopEnd = aFile.u8(aOffset + 19u);
                env.sustainStart = aFile.u8(aOffset + 20u);
                env.sustainEnd = aFile.u8(aOffset + 21u);
                for (std::uint32_t index = 0u; index < 25u; ++index)
                {
                    auto const tick = aFile.u8(aOffset + 504u + index * 2u);
                    if (tick == 0xFFu)
                        break;
                    env.points.push_back(envelope_point{ tick, std::min<std::uint8_t>(aFile.u8(aOffset + 505u + index * 2u), 64u) });
                }
                if (std::max(env.loopStart, env.loopEnd) >= env.points.size())
                    env.loop = false;
                if (std::max(env.sustainStart, env.sustainEnd) >= env.points.size())
                    env.sustain = false;
                return;
            }
            aInstrument.name = aFile.text(aOffset + 32u, 26u);
            aInstrument.nna = static_cast<new_note_action>(std::min<std::uint8_t>(aFile.u8(aOffset + 17u), 3u));
            aInstrument.dct = static_cast<duplicate_check>(std::min<std::uint8_t>(aFile.u8(aOffset + 18u), 3u));
            aInstrument.dca = static_cast<duplicate_action>(std::min<std::uint8_t>(aFile.u8(aOffset + 19u), 2u));
            aInstrument.fadeOut = static_cast<std::uint32_t>(aFile.u16le(aOffset + 20u)) << 5u;
            aInstrument.pitchPanSeparation = aFile.i8(aOffset + 22u);
            aInstrument.pitchPanCenter = aFile.u8(aOffset + 23u);
            aInstrument.globalVolume = static_cast<std::uint16_t>(std::min<std::uint32_t>(aFile.u8(aOffset + 24u) / 2u, 64u));
            auto const pan = aFile.u8(aOffset + 25u);
            if ((pan & 0x80u) == 0u)
            {
                auto const position = static_cast<std::uint16_t>((pan & 0x7Fu) * 4u);
                aInstrument.pan = position > 256u ? 128u : position;
            }
            aInstrument.volumeSwing = std::min<std::uint8_t>(aFile.u8(aOffset + 26u), 100u);
            aInstrument.panSwing = std::min<std::uint8_t>(aFile.u8(aOffset + 27u), 64u);
            auto const cutoff = aFile.u8(aOffset + 58u);
            auto const resonance = aFile.u8(aOffset + 59u);
            if ((cutoff & 0x80u) != 0u)
                aInstrument.cutoff = static_cast<std::uint8_t>(cutoff & 0x7Fu);
            if ((resonance & 0x80u) != 0u)
                aInstrument.resonance = static_cast<std::uint8_t>(resonance & 0x7Fu);
            for (std::uint32_t note = 0u; note < 120u; ++note)
            {
                auto const mapped = aFile.u8(aOffset + 64u + note * 2u);
                aInstrument.keyboard[note] = aFile.u8(aOffset + 65u + note * 2u);
                aInstrument.noteMap[note] = static_cast<std::uint8_t>(mapped < 120u ? mapped + NOTE_MIN : note + NOTE_MIN);
            }
            read_envelope(aFile, aOffset + 304u, 0, aInstrument.volumeEnvelope);
            read_envelope(aFile, aOffset + 386u, 32, aInstrument.panningEnvelope);
            read_envelope(aFile, aOffset + 468u, 32, aInstrument.pitchEnvelope);
            // OpenMPT's extended instrument: a high byte for each sample number follows
            if ((aFile.magic(aOffset + 550u, "MPTX") || aFile.magic(aOffset + 550u, "XTPM")) && aFile.has(aOffset + INSTRUMENT_SIZE, 120u))
                for (std::uint32_t note = 0u; note < 120u; ++note)
                    aInstrument.keyboard[note] = static_cast<std::uint16_t>(aInstrument.keyboard[note] | (aFile.u8(aOffset + INSTRUMENT_SIZE + note) << 8u));
        }

        std::string read_macro(reader const& aFile, std::size_t aOffset)
        {
            std::string result;
            for (std::size_t index = 0u; index < MACRO_LENGTH; ++index)
            {
                auto const ch = static_cast<char>(aFile.u8(aOffset + index));
                if (ch == '\0')
                    break;
                // only the characters that mean something in a macro
                if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'z'))
                    result.push_back(ch);
            }
            return result;
        }
    }

    // OpenMPT's song extensions: a series of fields, each a four-character code, a size and its data
    void read_song_extensions(reader const& aFile, std::size_t aOffset, module_data& aModule)
    {
        cursor file{ aFile, aOffset };
        // the extensions say how the song is played, so start from nothing
        aModule.playBehaviour.reset();
        while (file.can_read(7u))
        {
            auto const code = file.u32le();
            auto const size = file.u16le();
            if (code == 0x04383232u || (code & 0x80808080u) != 0u || (code & 0x60606060u) == 0u || !file.can_read(size))
                break;
            auto const start = file.offset();
            auto const field = [&](std::uint32_t aDefault) -> std::uint32_t
                {
                    std::uint32_t value = 0u;
                    for (std::uint32_t index = 0u; index < std::min<std::uint32_t>(size, 4u); ++index)
                        value |= static_cast<std::uint32_t>(aFile.u8(start + index)) << (index * 8u);
                    return size == 0u ? aDefault : value;
                };
            auto const is = [&](char const* aCode)
                {
                    // the codes are written as big-endian character constants
                    std::uint32_t const be = (static_cast<std::uint32_t>(static_cast<std::uint8_t>(aCode[0])) << 24u) |
                        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(aCode[1])) << 16u) |
                        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(aCode[2])) << 8u) |
                        static_cast<std::uint32_t>(static_cast<std::uint8_t>(aCode[3]));
                    return code == be;
                };
            if (is("DT.."))
                aModule.tempo = static_cast<double>(field(125u)) + (aModule.tempo - static_cast<std::uint32_t>(aModule.tempo));
            else if (is("RPB."))
                aModule.rowsPerBeat = std::max(field(4u), 1u);
            else if (is("TM.."))
                aModule.tempoMode = field(0u) <= 2u ? static_cast<tempo_mode>(field(0u)) : tempo_mode::Classic;
            else if (is("SPA."))
                aModule.samplePreAmp = std::min(field(48u), 2000u);
            else if (is("DGV."))
                aModule.globalVolume = std::min(field(256u), 256u);
            else if (is("PMM."))
            {
                auto const levels = field(0u);
                aModule.mixLevels = levels <= static_cast<std::uint32_t>(mix_levels::CompatibleFT2) ? static_cast<mix_levels>(levels) : mix_levels::Original;
            }
            else if (is("RP.."))
                aModule.restart = static_cast<std::uint16_t>(field(0u));
            else if (is("LSWV"))
            {
                if (field(0u) != 0u)
                    aModule.openmptVersion = field(0u);
            }
            else if (is("MSF."))
            {
                aModule.playBehaviour.reset();
                for (std::uint32_t index = 0u; index < size; ++index)
                {
                    auto const bits = aFile.u8(start + index);
                    for (std::uint32_t bit = 0u; bit < 8u; ++bit)
                    {
                        auto const flag = index * 8u + bit;
                        if ((bits & (1u << bit)) != 0u && flag < std::size(OPENMPT_BEHAVIOURS) && OPENMPT_BEHAVIOURS[flag] != behaviour::Count)
                            aModule.playBehaviour.set(OPENMPT_BEHAVIOURS[flag]);
                    }
                }
            }
            else if ((code & 0xFFFFFFFFu) == 0x52465444u) // "DTFR": fractional tempo
                aModule.tempo = static_cast<std::uint32_t>(aModule.tempo) + field(0u) / 10000.0;
            file.seek(start + size);
        }
    }

    // OpenMPT's extended instrument properties: for each property, a four-character code and a size, then that
    // property for every instrument; returns where the song extensions start
    std::size_t read_instrument_extensions(reader const& aFile, std::size_t aOffset, std::vector<instrument>& aInstruments)
    {
        if (!aFile.magic(aOffset, "XTPM"))
            return aOffset;
        cursor file{ aFile, aOffset + 4u };
        while (file.can_read(6u))
        {
            auto const code = file.u32le();
            if (code == 0x4D505453u || code == 0x04383232u || (code & 0x80808080u) != 0u || (code & 0x60606060u) == 0u)
            {
                file.seek(file.offset() - 4u);
                break;
            }
            auto const size = file.u16le();
            // the codes are written as big-endian character constants, so read back to front
            char const name[5] = { static_cast<char>(code >> 24u), static_cast<char>(code >> 16u), static_cast<char>(code >> 8u), static_cast<char>(code), 0 };
            std::string_view const field{ name, 4u };
            for (auto& ins : aInstruments)
            {
                auto const start = file.offset();
                if (!aFile.has(start, size))
                    break;
                auto const integer = [&]() -> std::uint32_t
                    {
                        std::uint32_t value = 0u;
                        for (std::uint32_t index = 0u; index < std::min<std::uint32_t>(size, 4u); ++index)
                            value |= static_cast<std::uint32_t>(aFile.u8(start + index)) << (index * 8u);
                        return value;
                    };
                auto const ticks = [&](envelope& aEnvelope)
                    {
                        for (std::size_t point = 0u; point < aEnvelope.points.size() && point < size / 2u; ++point)
                            aEnvelope.points[point].tick = aFile.u16le(start + point * 2u);
                    };
                auto const values = [&](envelope& aEnvelope)
                    {
                        for (std::size_t point = 0u; point < aEnvelope.points.size() && point < size; ++point)
                            aEnvelope.points[point].value = static_cast<std::uint8_t>(std::min<std::uint32_t>(aFile.u8(start + point), 64u));
                    };
                auto const resize = [&](envelope& aEnvelope)
                    {
                        aEnvelope.points.resize(std::min<std::uint32_t>(integer(), 240u));
                    };
                if (field == "FO..")
                    ins.fadeOut = integer();
                else if (field == "GV..")
                    ins.globalVolume = static_cast<std::uint16_t>(std::min<std::uint32_t>(integer(), 64u));
                else if (field == "P...")
                {
                    if (ins.pan)
                        ins.pan = static_cast<std::uint16_t>(std::min<std::uint32_t>(integer(), 256u));
                }
                else if (field == "VS..")
                    ins.volumeSwing = static_cast<std::uint8_t>(std::min<std::uint32_t>(integer(), 100u));
                else if (field == "PS..")
                    ins.panSwing = static_cast<std::uint8_t>(std::min<std::uint32_t>(integer(), 64u));
                else if (field == "NNA.")
                    ins.nna = static_cast<new_note_action>(std::min<std::uint32_t>(integer(), 3u));
                else if (field == "DCT.")
                    ins.dct = static_cast<duplicate_check>(std::min<std::uint32_t>(integer(), 3u));
                else if (field == "DNA.")
                    ins.dca = static_cast<duplicate_action>(std::min<std::uint32_t>(integer(), 2u));
                else if (field == "PPS.")
                    ins.pitchPanSeparation = static_cast<std::int8_t>(integer());
                else if (field == "PPC.")
                    ins.pitchPanCenter = static_cast<std::uint8_t>(integer());
                else if (field == "VR..")
                    ins.volumeRampUp = static_cast<std::uint16_t>(integer());
                else if (field == "FM..")
                {
                    auto const mode = integer();
                    if (mode <= 1u)
                        ins.filterMode = static_cast<std::uint8_t>(mode);
                }
                else if (field == "VE..")
                    resize(ins.volumeEnvelope);
                else if (field == "PE..")
                    resize(ins.panningEnvelope);
                else if (field == "PiE.")
                    resize(ins.pitchEnvelope);
                else if (field == "VP[.")
                    ticks(ins.volumeEnvelope);
                else if (field == "PP[.")
                    ticks(ins.panningEnvelope);
                else if (field == "PiP[")
                    ticks(ins.pitchEnvelope);
                else if (field == "VE[.")
                    values(ins.volumeEnvelope);
                else if (field == "PE[.")
                    values(ins.panningEnvelope);
                else if (field == "PiE[")
                    values(ins.pitchEnvelope);
                else if (field == "VLS.")
                    ins.volumeEnvelope.loopStart = static_cast<std::uint8_t>(integer());
                else if (field == "VLE.")
                    ins.volumeEnvelope.loopEnd = static_cast<std::uint8_t>(integer());
                else if (field == "VSB.")
                    ins.volumeEnvelope.sustainStart = static_cast<std::uint8_t>(integer());
                else if (field == "VSE.")
                    ins.volumeEnvelope.sustainEnd = static_cast<std::uint8_t>(integer());
                else if (field == "PLS.")
                    ins.panningEnvelope.loopStart = static_cast<std::uint8_t>(integer());
                else if (field == "PLE.")
                    ins.panningEnvelope.loopEnd = static_cast<std::uint8_t>(integer());
                else if (field == "PSB.")
                    ins.panningEnvelope.sustainStart = static_cast<std::uint8_t>(integer());
                else if (field == "PSE.")
                    ins.panningEnvelope.sustainEnd = static_cast<std::uint8_t>(integer());
                else if (field == "PiLS")
                    ins.pitchEnvelope.loopStart = static_cast<std::uint8_t>(integer());
                else if (field == "PiLE")
                    ins.pitchEnvelope.loopEnd = static_cast<std::uint8_t>(integer());
                else if (field == "PiSB")
                    ins.pitchEnvelope.sustainStart = static_cast<std::uint8_t>(integer());
                else if (field == "PiSE")
                    ins.pitchEnvelope.sustainEnd = static_cast<std::uint8_t>(integer());
                else if (field == "NM[.")
                {
                    for (std::size_t note = 0u; note < ins.noteMap.size() && note < size; ++note)
                        ins.noteMap[note] = aFile.u8(start + note);
                }
                file.seek(start + size);
            }
        }
        return file.offset();
    }

    void apply_openmpt_upgrades(module_data& aModule)
    {
        auto const v = aModule.openmptVersion;
        if (v == 0u)
            return;
        auto& b = aModule.playBehaviour;
        if (v < version(0x1, 0x17, 0x2, 0x46) && v != version(0x1, 0x17, 0x0, 0x0))
            b.reset(behaviour::CompatiblePlay);
        bool const compatibleIT = b[behaviour::CompatiblePlay] && aModule.is_it();
        bool const compatibleXM = b[behaviour::CompatiblePlay] && aModule.format == module_format::XM;
        if (v < version(0x1, 0x20, 0x0, 0x0))
        {
            for (auto& ins : aModule.instruments)
            {
                ins.volumeSwing = static_cast<std::uint8_t>(std::min<std::uint32_t>(ins.volumeSwing * 100u / 64u, 100u));
                if (!compatibleIT || v < version(0x1, 0x18, 0x0, 0x0))
                    ins.pitchPanSeparation = static_cast<std::int8_t>((ins.pitchPanSeparation + (ins.pitchPanSeparation >= 0 ? 1 : -1)) / 2);
            }
            if (aModule.is_it() && (v < version(0x1, 0x17, 0x3, 0x2) || !compatibleIT))
                for (auto& s : aModule.samples)
                    if (s.vibratoSweep == 0u && (s.vibratoDepth | s.vibratoRate) != 0u)
                        s.vibratoSweep = 255u;
            if (v < version(0x1, 0x17, 0x2, 0x50))
                for (auto const& ins : aModule.instruments)
                    if ((ins.volumeSwing | ins.panSwing) != 0u)
                    {
                        b.set(behaviour::MPTOldSwingBehaviour);
                        break;
                    }
        }
        if (v < version(0x1, 0x22, 0x3, 0x12) && v != version(0x1, 0x22, 0x0, 0x0) && aModule.is_it() &&
            (b[behaviour::CompatiblePlay] || b[behaviour::MPTOldSwingBehaviour]))
            for (auto& ins : aModule.instruments)
                if (ins.panSwing != 0u && ins.panningEnvelope.enabled)
                    ins.panSwing = 0u;
        if (v < version(0x1, 0x26, 0x0, 0x0))
            for (auto& ins : aModule.instruments)
            {
                ins.pitchPanSeparation = static_cast<std::int8_t>((ins.pitchPanSeparation + (ins.pitchPanSeparation >= 0 ? 1 : -1)) / 2);
                if (!compatibleIT || v < version(0x1, 0x18, 0x0, 0x0))
                    ins.panSwing = static_cast<std::uint8_t>((ins.panSwing + 3u) / 4u);
            }
        if (v < version(0x1, 0x30, 0x0, 0x54))
            for (auto const& s : aModule.samples)
                if (s.has_data() && (s.loop == loop_type::PingPong || s.sustain == loop_type::PingPong))
                {
                    b.set(behaviour::ImprecisePingPongLoops);
                    break;
                }
        struct versioned
        {
            behaviour flag;
            std::uint32_t since;
        };
        if (compatibleIT && v < version(0x1, 0x26, 0x0, 0x0))
        {
            static constexpr versioned sFlags[] =
            {
                { behaviour::TempoClamp, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::PerChannelGlobalVolSlide, version(0x1, 0x17, 0x3, 0x2) },
                { behaviour::PanOverride, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::ITInstrWithoutNote, version(0x1, 0x17, 0x2, 0x46) },
                { behaviour::ITVolColFinePortamento, version(0x1, 0x17, 0x2, 0x49) }, { behaviour::ITArpeggio, version(0x1, 0x17, 0x2, 0x49) },
                { behaviour::ITOutOfRangeDelay, version(0x1, 0x17, 0x2, 0x49) }, { behaviour::ITPortaMemoryShare, version(0x1, 0x17, 0x2, 0x49) },
                { behaviour::ITPatternLoopTargetReset, version(0x1, 0x17, 0x2, 0x49) }, { behaviour::ITFT2PatternLoop, version(0x1, 0x17, 0x2, 0x49) },
                { behaviour::ITPingPongNoReset, version(0x1, 0x17, 0x2, 0x51) }, { behaviour::ITEnvelopeReset, version(0x1, 0x17, 0x2, 0x51) },
                { behaviour::ITClearOldNoteAfterCut, version(0x1, 0x17, 0x2, 0x52) }, { behaviour::ITVibratoTremoloPanbrello, version(0x1, 0x17, 0x3, 0x2) },
                { behaviour::ITTremor, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::ITRetrigger, version(0x1, 0x17, 0x3, 0x2) },
                { behaviour::ITMultiSampleBehaviour, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::ITPortaTargetReached, version(0x1, 0x17, 0x3, 0x2) },
                { behaviour::ITPatternLoopBreak, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::ITOffset, version(0x1, 0x17, 0x3, 0x2) },
                { behaviour::ITSwingBehaviour, version(0x1, 0x18, 0x0, 0x0) }, { behaviour::ITNNAReset, version(0x1, 0x18, 0x0, 0x0) },
                { behaviour::ITSCxStopsSample, version(0x1, 0x18, 0x0, 0x1) }, { behaviour::ITEnvelopePositionHandling, version(0x1, 0x18, 0x1, 0x0) },
                { behaviour::ITPortamentoInstrument, version(0x1, 0x19, 0x0, 0x1) }, { behaviour::ITPingPongMode, version(0x1, 0x19, 0x0, 0x21) },
                { behaviour::ITRealNoteMapping, version(0x1, 0x19, 0x0, 0x30) }, { behaviour::ITHighOffsetNoRetrig, version(0x1, 0x20, 0x0, 0x14) },
                { behaviour::ITFilterBehaviour, version(0x1, 0x20, 0x0, 0x35) }, { behaviour::ITNoSurroundPan, version(0x1, 0x20, 0x0, 0x53) },
                { behaviour::ITShortSampleRetrig, version(0x1, 0x20, 0x0, 0x54) }, { behaviour::ITPortaNoNote, version(0x1, 0x20, 0x0, 0x56) },
                { behaviour::RowDelayWithNoteDelay, version(0x1, 0x20, 0x0, 0x76) }, { behaviour::ITFT2DontResetNoteOffOnPorta, version(0x1, 0x20, 0x2, 0x6) },
                { behaviour::ITVolColMemory, version(0x1, 0x21, 0x1, 0x16) }, { behaviour::ITPortamentoSwapResetsPos, version(0x1, 0x21, 0x1, 0x25) },
                { behaviour::ITEmptyNoteMapSlot, version(0x1, 0x21, 0x1, 0x25) }, { behaviour::ITFirstTickHandling, version(0x1, 0x22, 0x7, 0x9) },
                { behaviour::ITSampleAndHoldPanbrello, version(0x1, 0x22, 0x7, 0x19) }, { behaviour::ITClearPortaTarget, version(0x1, 0x23, 0x4, 0x3) },
                { behaviour::ITPanbrelloHold, version(0x1, 0x24, 0x1, 0x6) }, { behaviour::ITPanningReset, version(0x1, 0x24, 0x1, 0x6) },
                { behaviour::ITPatternLoopWithJumpsOld, version(0x1, 0x25, 0x0, 0x19) }
            };
            for (auto const& flag : sFlags)
                b.set(flag.flag, v >= flag.since || v == (flag.since & 0xFFFF0000u));
        }
        else if (compatibleXM && v < version(0x1, 0x26, 0x0, 0x0))
        {
            static constexpr versioned sFlags[] =
            {
                { behaviour::TempoClamp, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::PerChannelGlobalVolSlide, version(0x1, 0x17, 0x3, 0x2) },
                { behaviour::PanOverride, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::ITFT2PatternLoop, version(0x1, 0x17, 0x3, 0x2) },
                { behaviour::FT2Arpeggio, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::FT2Retrigger, version(0x1, 0x17, 0x3, 0x2) },
                { behaviour::FT2VolColVibrato, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::FT2PortaNoNote, version(0x1, 0x17, 0x3, 0x2) },
                { behaviour::FT2KeyOff, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::FT2PanSlide, version(0x1, 0x17, 0x3, 0x2) },
                { behaviour::FT2ST3OffsetOutOfRange, version(0x1, 0x17, 0x3, 0x2) }, { behaviour::FT2RestrictXCommand, version(0x1, 0x18, 0x0, 0x0) },
                { behaviour::FT2RetrigWithNoteDelay, version(0x1, 0x18, 0x0, 0x0) }, { behaviour::FT2SetPanEnvPos, version(0x1, 0x18, 0x0, 0x0) },
                { behaviour::FT2PortaIgnoreInstr, version(0x1, 0x18, 0x0, 0x1) }, { behaviour::FT2VolColMemory, version(0x1, 0x18, 0x1, 0x0) },
                { behaviour::FT2LoopE60Restart, version(0x1, 0x18, 0x2, 0x1) }, { behaviour::FT2ProcessSilentChannels, version(0x1, 0x18, 0x2, 0x1) },
                { behaviour::FT2ReloadSampleSettings, version(0x1, 0x20, 0x0, 0x36) }, { behaviour::FT2PortaDelay, version(0x1, 0x20, 0x0, 0x40) },
                { behaviour::FT2Transpose, version(0x1, 0x20, 0x0, 0x62) }, { behaviour::FT2PatternLoopWithJumps, version(0x1, 0x20, 0x0, 0x69) },
                { behaviour::FT2PortaTargetNoReset, version(0x1, 0x20, 0x0, 0x69) }, { behaviour::FT2EnvelopeEscape, version(0x1, 0x20, 0x0, 0x77) },
                { behaviour::FT2Tremor, version(0x1, 0x20, 0x1, 0x11) }, { behaviour::FT2OutOfRangeDelay, version(0x1, 0x20, 0x2, 0x2) },
                { behaviour::FT2Periods, version(0x1, 0x22, 0x3, 0x1) }, { behaviour::FT2PanWithDelayedNoteOff, version(0x1, 0x22, 0x3, 0x2) },
                { behaviour::FT2VolColDelay, version(0x1, 0x22, 0x7, 0x19) }, { behaviour::FT2FinetunePrecision, version(0x1, 0x22, 0x7, 0x19) }
            };
            for (auto const& flag : sFlags)
                b.set(flag.flag, v >= flag.since);
        }
        if (aModule.is_it())
        {
            static constexpr versioned sFlags[] =
            {
                { behaviour::ITInstrWithNoteOff, version(0x1, 0x26, 0x0, 0x1) }, { behaviour::ITMultiSampleInstrumentNumber, version(0x1, 0x27, 0x0, 0x27) },
                { behaviour::ITInstrWithNoteOffOldEffects, version(0x1, 0x28, 0x2, 0x6) }, { behaviour::ITDoNotOverrideChannelPan, version(0x1, 0x29, 0x0, 0x22) },
                { behaviour::ITPatternLoopWithJumps, version(0x1, 0x29, 0x0, 0x32) }, { behaviour::ITDCTBehaviour, version(0x1, 0x29, 0x0, 0x57) },
                { behaviour::ITPitchPanSeparation, version(0x1, 0x30, 0x0, 0x53) }, { behaviour::ITResetFilterOnPortaSmpChange, version(0x1, 0x30, 0x8, 0x2) },
                { behaviour::ITInitialNoteMemory, version(0x1, 0x31, 0x0, 0x25) }, { behaviour::ITNoSustainOnPortamento, version(0x1, 0x32, 0x0, 0x13) },
                { behaviour::ITEmptyNoteMapSlotIgnoreCell, version(0x1, 0x32, 0x0, 0x13) }, { behaviour::ITOffsetWithInstrNumber, version(0x1, 0x32, 0x0, 0x15) },
                { behaviour::ITDoublePortamentoSlides, version(0x1, 0x32, 0x0, 0x27) }, { behaviour::ITCarryAfterNoteOff, version(0x1, 0x32, 0x0, 0x40) },
                { behaviour::ITNoteCutWithPorta, version(0x1, 0x32, 0x1, 0x2) }, { behaviour::ITVolColNoSlidePropagation, version(0x1, 0x32, 0x2, 0x3) },
                { behaviour::ITStoppedFilterEnvAtStart, version(0x1, 0x32, 0x3, 0x4) }, { behaviour::ITCompatGxxCarryPortaWithIns, version(0x1, 0x32, 0x10, 0x2) }
            };
            for (auto const& flag : sFlags)
            {
                auto const masked = flag.since & 0xFFFF0000u;
                if (v < masked || (v > masked && v < flag.since))
                    b.reset(flag.flag);
            }
        }
        else if (aModule.format == module_format::XM)
        {
            static constexpr versioned sFlags[] =
            {
                { behaviour::FT2NoteOffFlags, version(0x1, 0x27, 0x0, 0x27) }, { behaviour::RowDelayWithNoteDelay, version(0x1, 0x27, 0x0, 0x37) },
                { behaviour::FT2MODTremoloRampWaveform, version(0x1, 0x27, 0x0, 0x37) }, { behaviour::FT2PortaUpDownMemory, version(0x1, 0x27, 0x0, 0x37) },
                { behaviour::FT2PanSustainRelease, version(0x1, 0x28, 0x0, 0x9) }, { behaviour::FT2NoteDelayWithoutInstr, version(0x1, 0x28, 0x0, 0x44) },
                { behaviour::ITFT2DontResetNoteOffOnPorta, version(0x1, 0x29, 0x0, 0x34) }, { behaviour::FT2PortaResetDirection, version(0x1, 0x30, 0x0, 0x40) },
                { behaviour::FT2AutoVibratoAbortSweep, version(0x1, 0x32, 0x0, 0x29) }, { behaviour::FT2OffsetMemoryRequiresNote, version(0x1, 0x32, 0x0, 0x43) }
            };
            for (auto const& flag : sFlags)
                if (v < flag.since)
                    b.reset(flag.flag);
            if (v < version(0x1, 0x19, 0x0, 0x0))
                b.set(behaviour::FT2NoteDelayWithoutInstr);
        }
        else if (aModule.format == module_format::S3M)
        {
            static constexpr versioned sFlags[] =
            {
                { behaviour::ST3NoMutedChannels, version(0x1, 0x18, 0x0, 0x0) }, { behaviour::ST3EffectMemory, version(0x1, 0x20, 0x0, 0x0) },
                { behaviour::RowDelayWithNoteDelay, version(0x1, 0x20, 0x0, 0x0) }, { behaviour::ST3PortaSampleChange, version(0x1, 0x22, 0x0, 0x0) },
                { behaviour::ST3VibratoMemory, version(0x1, 0x26, 0x0, 0x0) }, { behaviour::ITPanbrelloHold, version(0x1, 0x26, 0x0, 0x0) },
                { behaviour::ST3PortaAfterArpeggio, version(0x1, 0x27, 0x0, 0x0) }, { behaviour::ST3OffsetWithoutInstrument, version(0x1, 0x28, 0x0, 0x0) },
                { behaviour::ST3RetrigAfterNoteCut, version(0x1, 0x29, 0x0, 0x0) }, { behaviour::FT2ST3OffsetOutOfRange, version(0x1, 0x29, 0x0, 0x0) },
                { behaviour::ApplyUpperPeriodLimit, version(0x1, 0x30, 0x0, 0x45) }
            };
            for (auto const& flag : sFlags)
                if (v < flag.since)
                    b.reset(flag.flag);
        }
        if (v < version(0x1, 0x17, 0x0, 0x0))
            b.set(behaviour::TempoClamp);
        else if (v <= version(0x1, 0x20, 0x1, 0x3) && v != version(0x1, 0x20, 0x0, 0x0))
            b.set(behaviour::SlidesAtSpeed1);
        if (aModule.linearSlides)
        {
            if (v < version(0x1, 0x24, 0x0, 0x0))
                b.reset(behaviour::PeriodsAreHertz);
            else if (v < version(0x1, 0x26, 0x0, 0x0) && aModule.is_it())
                b.set(behaviour::PeriodsAreHertz);
        }
        else if (v < version(0x1, 0x30, 0x0, 0x36) && v != version(0x1, 0x30, 0x0, 0x0))
            b.reset(behaviour::PeriodsAreHertz);
    }

    bool is_it(reader const& aFile)
    {
        return aFile.has(0u, HEADER_SIZE) && (aFile.magic(0u, "IMPM") || aFile.magic(0u, "tpm.")) &&
            aFile.u8(48u) <= 128u && aFile.u8(49u) <= 128u;
    }

    module_data load_it(reader const& aFile)
    {
        module_data result;
        result.format = module_format::IT;
        result.title = aFile.text(4u, 26u);
        auto const orderCount = aFile.u16le(32u);
        auto const instrumentCount = aFile.u16le(34u);
        auto const sampleCount = aFile.u16le(36u);
        auto const patternCount = aFile.u16le(38u);
        auto const cwtv = aFile.u16le(40u);
        auto const cmwt = aFile.u16le(42u);
        auto const flags = aFile.u16le(44u);
        auto const special = aFile.u16le(46u);
        auto const headerGlobalVolume = aFile.u8(48u);
        auto const mixingVolume = aFile.u8(49u);
        auto const headerSpeed = aFile.u8(50u);
        auto const headerTempo = aFile.u8(51u);
        auto const reserved = aFile.u32le(60u);
        bool const reservedIsOMPT = aFile.magic(60u, "OMPT");

        // an MPTM file carries extensions at an offset given by the file's last four bytes
        if (aFile.magic(0u, "tpm."))
            result.format = module_format::MPTM;
        else if (cwtv > 0x888u && cwtv <= 0xFFFu && aFile.size() >= 4u)
        {
            auto const start = aFile.u32le(aFile.size() - 4u);
            if (start >= 0x100u && start < aFile.size() && aFile.magic(start, "228"))
                result.format = module_format::MPTM;
        }
        result.formatName = result.format == module_format::MPTM ? "OpenMPT" : "Impulse Tracker";
        result.playBehaviour = default_behaviours(module_format::IT);

        // which version of (Open)ModPlug Tracker saved it, if any
        bool modplugMade = false;
        if (result.format == module_format::IT)
        {
            if ((cwtv & 0xF000u) == 0x5000u)
            {
                result.openmptVersion = static_cast<std::uint32_t>(cwtv & 0x0FFFu) << 16u;
                if (reservedIsOMPT)
                    modplugMade = true;
                else if (result.openmptVersion >= version(0x1, 0x29, 0x0, 0x0))
                    result.openmptVersion |= reserved & 0xFFFFu;
            }
            else if (cmwt == 0x888u || cwtv == 0x888u)
            {
                modplugMade = true;
                result.openmptVersion = version(0x1, 0x17, 0x0, 0x0);
            }
            else if (cwtv == 0x0214u && cmwt == 0x0202u && reserved == 0u)
            {
                modplugMade = true;
                result.openmptVersion = version(0x1, 0x9, 0x0, 0x0);
            }
            else if (cwtv == 0x0300u && cmwt == 0x0300u && reserved == 0u && orderCount == 256u && aFile.u8(52u) == 128u && aFile.u8(53u) == 0u)
            {
                modplugMade = true;
                result.openmptVersion = version(0x1, 0x17, 0x2, 0x20);
            }
        }

        result.linearSlides = (flags & 0x0008u) != 0u;
        result.itOldEffects = (flags & 0x0010u) != 0u;
        result.itCompatibleGxx = (flags & 0x0020u) != 0u;
        result.extendedFilterRange = (flags & 0x1000u) != 0u;
        result.globalVolume = std::min<std::uint32_t>(headerGlobalVolume * 2u, 256u);
        if (headerSpeed != 0u)
            result.speed = headerSpeed;
        result.tempo = std::max<std::uint32_t>(31u, headerTempo);
        result.samplePreAmp = std::min<std::uint32_t>(mixingVolume, 128u);
        result.minimumPeriod = 0;

        // orders and parapointers
        std::size_t offset = HEADER_SIZE;
        for (std::uint32_t index = 0u; index < orderCount; ++index)
        {
            auto const order = aFile.u8(offset + index);
            result.orders.push_back(order == 0xFFu ? module_data::ORDER_END : order == 0xFEu ? module_data::ORDER_SKIP : order);
        }
        bool const lastOrderIsEnd = !result.orders.empty() && result.orders.back() == module_data::ORDER_END;
        while (!result.orders.empty() && result.orders.back() == module_data::ORDER_END)
            result.orders.pop_back();
        offset += orderCount;
        auto const pointers = [&](std::uint32_t aCount)
            {
                std::vector<std::uint32_t> result(aCount);
                for (auto& pointer : result)
                {
                    pointer = aFile.u32le(offset);
                    offset += 4u;
                }
                return result;
            };
        auto const instrumentPointers = pointers(instrumentCount);
        auto const samplePointers = pointers(sampleCount);
        auto const patternPointers = pointers(patternCount);
        std::uint32_t firstPointer = 0xFFFFFFFFu;
        for (auto const* list : { &instrumentPointers, &samplePointers, &patternPointers })
            for (auto pointer : *list)
                if (pointer != 0u)
                    firstPointer = std::min(firstPointer, pointer);
        if ((special & 0x01u) != 0u)
            firstPointer = std::min(firstPointer, aFile.u32le(56u));

        // edit history, then the MIDI configuration
        if ((special & 0x02u) != 0u && aFile.has(offset, 2u))
        {
            auto const entries = aFile.u16le(offset);
            if (aFile.has(offset + 2u, entries * 8u) && offset + 2u + entries * 8u <= firstPointer)
                offset += 2u + entries * 8u;
        }
        result.reset_macros();
        if (((flags & 0x0080u) != 0u || (special & 0x08u) != 0u) && aFile.has(offset, MACRO_LENGTH * (9u + 16u + 128u)))
        {
            offset += MACRO_LENGTH * 9u;
            for (auto& macro : result.parameteredMacros)
            {
                macro = read_macro(aFile, offset);
                offset += MACRO_LENGTH;
            }
            for (auto& macro : result.fixedMacros)
            {
                macro = read_macro(aFile, offset);
                offset += MACRO_LENGTH;
            }
        }
        bool modplugExtensions = false;
        if (aFile.magic(offset, "PNAM"))
        {
            offset += 8u + aFile.u32le(offset + 4u);
            modplugExtensions = true;
        }
        if (aFile.magic(offset, "CNAM"))
        {
            offset += 8u + aFile.u32le(offset + 4u);
            modplugExtensions = true;
        }
        if (offset + 4u <= firstPointer && aFile.has(offset, 4u) && (aFile.magic(offset, "FX00") || aFile.magic(offset, "F255") || aFile.magic(offset, "CHFX")))
            modplugExtensions = true;
        if (result.format == module_format::IT && cwtv == 0x0217u && cmwt == 0x0200u && reserved == 0u)
        {
            bool unusedPanning = false;
            for (std::uint32_t index = 0u; index < 64u; ++index)
                if (aFile.u8(64u + index) == 0xFFu)
                    unusedPanning = true;
            result.openmptVersion = (modplugExtensions || lastOrderIsEnd || unusedPanning) ? version(0x1, 0x16, 0x0, 0x0) : version(0x1, 0x17, 0x0, 0x0);
            modplugMade = true;
        }

        // instruments
        if ((flags & 0x0004u) != 0u)
        {
            result.instruments.resize(instrumentCount);
            for (std::uint32_t index = 0u; index < instrumentCount; ++index)
                if (instrumentPointers[index] != 0u && aFile.has(instrumentPointers[index], cmwt < 0x200u ? 554u : INSTRUMENT_SIZE))
                    read_instrument(aFile, instrumentPointers[index], cmwt, result.instruments[index]);
            if (result.instruments.empty())
                result.instruments.resize(1u);
        }

        // samples
        bool const muteBuggySamples = !modplugMade && cwtv >= 0x0100u && cwtv <= 0x0217u && (cwtv < 0x0207u || reserved != 0u);
        std::size_t extensions = sampleCount != 0u ? samplePointers.back() + SAMPLE_HEADER_SIZE : 0u;
        result.samples.resize(sampleCount);
        for (std::uint32_t index = 0u; index < sampleCount; ++index)
        {
            auto const header = samplePointers[index];
            if (header == 0u || !aFile.has(header, SAMPLE_HEADER_SIZE))
                continue;
            auto& s = result.samples[index];
            auto const globalVolume = aFile.u8(header + 17u);
            auto const sampleFlags = aFile.u8(header + 18u);
            auto const volume = aFile.u8(header + 19u);
            s.name = aFile.text(header + 20u, 26u);
            auto const cvt = aFile.u8(header + 46u);
            auto const pan = aFile.u8(header + 47u);
            auto length = aFile.u32le(header + 48u);
            s.loopStart = aFile.u32le(header + 52u);
            s.loopEnd = aFile.u32le(header + 56u);
            auto const c5speed = aFile.u32le(header + 60u);
            s.sustainStart = aFile.u32le(header + 64u);
            s.sustainEnd = aFile.u32le(header + 68u);
            auto const dataPointer = aFile.u32le(header + 72u);
            s.vibratoRate = aFile.u8(header + 76u);
            s.vibratoDepth = static_cast<std::uint8_t>(aFile.u8(header + 77u) & 0x7Fu);
            s.vibratoSweep = aFile.u8(header + 78u);
            static constexpr vibrato_type sVibratoTypes[8] =
            {
                vibrato_type::Sine, vibrato_type::RampDown, vibrato_type::Square, vibrato_type::Random,
                vibrato_type::RampUp, vibrato_type::Sine, vibrato_type::Sine, vibrato_type::Sine
            };
            s.vibratoType = sVibratoTypes[aFile.u8(header + 79u) & 7u];
            s.volume = static_cast<std::uint16_t>(std::min<std::uint32_t>(volume * 4u, 256u));
            s.globalVolume = std::min<std::uint8_t>(globalVolume, 64u);
            if ((pan & 0x80u) != 0u)
                s.pan = static_cast<std::uint16_t>(std::min<std::uint32_t>((pan & 0x7Fu) * 4u, 256u));
            if ((sampleFlags & 0x10u) != 0u)
                s.loop = (sampleFlags & 0x40u) != 0u ? loop_type::PingPong : loop_type::Forward;
            if ((sampleFlags & 0x20u) != 0u)
                s.sustain = (sampleFlags & 0x80u) != 0u ? loop_type::PingPong : loop_type::Forward;
            s.c5speed = c5speed == 0u ? 8363u : std::max<std::uint32_t>(c5speed, 256u);
            if (muteBuggySamples && (sampleFlags & 0x01u) == 0u)
                length = 0u;
            if (length == 0u || dataPointer == 0u || cvt == 0x40u || cvt == 0x80u)
            {
                s.length = 0u;
                continue;
            }
            bool const is16Bit = (sampleFlags & 0x02u) != 0u;
            bool const stereo = (sampleFlags & 0x04u) != 0u && cwtv >= 0x214u;
            std::size_t consumed = 0u;
            if ((sampleFlags & 0x08u) != 0u)
                consumed = decode_it_compressed(aFile.bytes(dataPointer, aFile.size()), length, is16Bit, stereo, (cvt & 0x04u) != 0u, s);
            else if (!is16Bit && cvt == 0xFFu)
                s.length = 0u; // ModPlug ADPCM: not supported
            else
            {
                sample_encoding encoding;
                encoding.width = is16Bit ? sample_encoding::bits::Sixteen : sample_encoding::bits::Eight;
                encoding.isSigned = (cvt & 0x01u) != 0u;
                encoding.bigEndian = (cvt & 0x02u) != 0u;
                encoding.delta = (cvt & 0x04u) != 0u;
                encoding.stereo = stereo;
                consumed = decode_pcm(aFile.bytes(dataPointer, static_cast<std::size_t>(length) * encoding.bytes_per_sample() * (stereo ? 2u : 1u)),
                    length, encoding, s);
            }
            extensions = std::max<std::size_t>(extensions, dataPointer + consumed);
        }

        // patterns; the channels used are the highest that has anything in it
        std::uint32_t channels = 1u;
        for (auto pointer : patternPointers)
        {
            if (pointer == 0u || !aFile.has(pointer, 8u))
                continue;
            auto const length = aFile.u16le(pointer);
            std::array<std::uint8_t, 128u> masks = {};
            cursor data{ aFile, pointer + 8u };
            std::size_t const end = pointer + 8u + length;
            while (data.offset() < end && data.can_read(1u))
            {
                auto const b = data.u8();
                if (b == 0u)
                    continue;
                auto const channel = static_cast<std::uint32_t>(((b & 0x7Fu) - 1u) & 0x7Fu);
                if ((b & 0x80u) != 0u)
                    masks[channel] = data.u8();
                if ((masks[channel] & 0x0Fu) != 0u)
                {
                    if (channel < MAX_CHANNELS)
                        channels = std::max(channels, channel + 1u);
                    static constexpr std::uint8_t sSkip[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 2, 3, 3, 4, 3, 4, 4, 5 };
                    data.skip(sSkip[masks[channel] & 0x0Fu]);
                }
            }
            extensions = std::max<std::size_t>(extensions, end);
        }
        result.channels = channels;
        result.channelSettings.resize(channels);
        for (std::uint32_t index = 0u; index < channels; ++index)
        {
            auto const pan = aFile.u8(64u + index);
            if (pan == 0xFFu)
                continue;
            auto& settings = result.channelSettings[index];
            settings.volume = std::min<std::uint8_t>(aFile.u8(128u + index), 64u);
            settings.muted = (pan & 0x80u) != 0u;
            auto const position = static_cast<std::uint8_t>(pan & 0x7Fu);
            if (position <= 64u)
                settings.pan = static_cast<std::uint16_t>(position * 4u);
            if (position == 100u)
                settings.surround = true;
        }
        result.patterns.resize(patternCount);
        for (std::uint32_t index = 0u; index < patternCount; ++index)
        {
            auto& p = result.patterns[index];
            auto const pointer = patternPointers[index];
            if (pointer == 0u || !aFile.has(pointer, 8u))
            {
                p.rows = 64u;
                p.cells.assign(64u * channels, cell{});
                continue;
            }
            auto const length = aFile.u16le(pointer);
            p.rows = std::clamp<std::uint32_t>(aFile.u16le(pointer + 2u), 1u, 1024u);
            p.cells.assign(static_cast<std::size_t>(p.rows) * channels, cell{});
            std::array<std::uint8_t, 128u> masks = {};
            std::array<cell, 128u> last = {};
            cursor data{ aFile, pointer + 8u };
            std::size_t const end = pointer + 8u + length;
            std::uint32_t row = 0u;
            cell dummy;
            while (row < p.rows && data.offset() < end && data.can_read(1u))
            {
                auto const b = data.u8();
                if (b == 0u)
                {
                    ++row;
                    continue;
                }
                auto const channel = static_cast<std::uint32_t>(((b & 0x7Fu) - 1u) & 0x7Fu);
                if ((b & 0x80u) != 0u)
                    masks[channel] = data.u8();
                auto const mask = masks[channel];
                cell& c = channel < channels ? p.cells[row * channels + channel] : dummy;
                if ((mask & 0x10u) != 0u)
                    c.note = last[channel].note;
                if ((mask & 0x20u) != 0u)
                    c.instrument = last[channel].instrument;
                if ((mask & 0x40u) != 0u)
                {
                    c.volumeCommand = last[channel].volumeCommand;
                    c.volume = last[channel].volume;
                }
                if ((mask & 0x80u) != 0u)
                {
                    c.command = last[channel].command;
                    c.parameter = last[channel].parameter;
                }
                if ((mask & 0x01u) != 0u)
                {
                    auto note = data.u8();
                    if (note < 0x80u)
                        note = static_cast<std::uint8_t>(note + NOTE_MIN);
                    else if (note == 0xFFu)
                        note = NOTE_OFF;
                    else if (note == 0xFEu)
                        note = NOTE_CUT;
                    else if (note == 0xFDu && result.format != module_format::MPTM)
                        note = NOTE_NONE;
                    else
                        note = NOTE_FADE;
                    if (is_note(note) || note == NOTE_NONE || note >= NOTE_FADE)
                        c.note = note;
                    else
                        c.note = NOTE_NONE;
                    last[channel].note = c.note;
                }
                if ((mask & 0x02u) != 0u)
                {
                    c.instrument = data.u8();
                    last[channel].instrument = c.instrument;
                }
                if ((mask & 0x04u) != 0u)
                {
                    auto const volume = data.u8();
                    if (volume <= 64u) { c.volumeCommand = volume_command::Volume; c.volume = volume; }
                    else if (volume >= 128u && volume <= 192u) { c.volumeCommand = volume_command::Panning; c.volume = static_cast<std::uint8_t>(volume - 128u); }
                    else if (volume < 75u) { c.volumeCommand = volume_command::FineVolumeUp; c.volume = static_cast<std::uint8_t>(volume - 65u); }
                    else if (volume < 85u) { c.volumeCommand = volume_command::FineVolumeDown; c.volume = static_cast<std::uint8_t>(volume - 75u); }
                    else if (volume < 95u) { c.volumeCommand = volume_command::VolumeSlideUp; c.volume = static_cast<std::uint8_t>(volume - 85u); }
                    else if (volume < 105u) { c.volumeCommand = volume_command::VolumeSlideDown; c.volume = static_cast<std::uint8_t>(volume - 95u); }
                    else if (volume < 115u) { c.volumeCommand = volume_command::PortamentoDown; c.volume = static_cast<std::uint8_t>(volume - 105u); }
                    else if (volume < 125u) { c.volumeCommand = volume_command::PortamentoUp; c.volume = static_cast<std::uint8_t>(volume - 115u); }
                    else if (volume >= 193u && volume <= 202u) { c.volumeCommand = volume_command::TonePortamento; c.volume = static_cast<std::uint8_t>(volume - 193u); }
                    else if (volume >= 203u && volume <= 212u)
                    {
                        c.volumeCommand = volume_command::VibratoDepth;
                        c.volume = static_cast<std::uint8_t>(volume - 203u);
                        // old ModPlug versions saved this as the vibrato speed
                        if (c.volume != 0u && result.openmptVersion != 0u && result.openmptVersion <= version(0x1, 0x17, 0x2, 0x54))
                            c.volumeCommand = volume_command::VibratoSpeed;
                    }
                    else if (volume >= 223u && volume <= 232u) { c.volumeCommand = volume_command::Offset; c.volume = static_cast<std::uint8_t>(volume - 223u); }
                    last[channel].volumeCommand = c.volumeCommand;
                    last[channel].volume = c.volume;
                }
                if ((mask & 0x08u) != 0u)
                {
                    auto const command = data.u8();
                    auto const parameter = data.u8();
                    std::tie(c.command, c.parameter) = convert_s3m_effect(command, parameter, true);
                    if (c.command == effect::ExtendedS3M && (c.parameter & 0xF0u) == 0xA0u && cwtv < 0x0200u)
                        c.command = effect::Dummy;
                    else if (c.command == effect::GlobalVolume && c.parameter > 0x80u && cwtv >= 0x1000u && cwtv <= 0x1050u)
                        c.parameter = 0x80u;
                    last[channel].command = c.command;
                    last[channel].parameter = c.parameter;
                }
            }
        }

        // OpenMPT's extensions follow the sample and pattern data
        auto const songExtensions = read_instrument_extensions(aFile, extensions, result.instruments);
        if (modplugMade)
            result.mixLevels = mix_levels::Original;
        if (aFile.magic(songExtensions, "STPM"))
            modplugMade = true;
        if (modplugMade)
            result.playBehaviour.reset();
        if (aFile.magic(songExtensions, "STPM"))
            read_song_extensions(aFile, songExtensions + 4u, result);
        if (result.format == module_format::MPTM && result.openmptVersion == 0u)
            result.openmptVersion = version(0x1, 0x17, 0x2, 0x0);
        if (result.format == module_format::MPTM && !aFile.magic(songExtensions, "STPM"))
            result.playBehaviour = default_behaviours(module_format::MPTM);
        // the quirks of the other trackers that save ITs
        if (result.openmptVersion == 0u)
        {
            switch (cwtv >> 12u)
            {
            case 0x0u:
                if (cwtv == 0x0214u && cmwt == 0x0214u && aFile.magic(60u, "CHBI"))
                {
                    result.tracker = "ChibiTracker";
                    result.playBehaviour.reset(behaviour::ITShortSampleRetrig);
                    result.samplePreAmp /= 2u;
                }
                else if (cmwt < 0x0300u)
                    result.tracker = "Impulse Tracker";
                break;
            case 0x1u:
                {
                    // Schism Tracker: its version is the date it was built, and it has fixed its quirks over time
                    result.tracker = "Schism Tracker";
                    std::int32_t const date = SCHISM_EPOCH + (cwtv == 0x1FFFu ? static_cast<std::int32_t>(reserved) : static_cast<std::int32_t>(cwtv) - 0x1050);
                    struct quirk
                    {
                        std::int32_t fixed;
                        behaviour quirk;
                    };
                    static constexpr quirk QUIRKS[] =
                    {
                        { schism_date(2015, 1, 29), behaviour::PeriodsAreHertz },
                        { schism_date(2016, 5, 13), behaviour::ITShortSampleRetrig },
                        { schism_date(2021, 5, 2), behaviour::ITDoNotOverrideChannelPan },
                        { schism_date(2021, 5, 2), behaviour::ITPanningReset },
                        { schism_date(2021, 11, 1), behaviour::ITPitchPanSeparation },
                        { schism_date(2022, 4, 30), behaviour::ITEmptyNoteMapSlot },
                        { schism_date(2022, 4, 30), behaviour::ITPortamentoSwapResetsPos },
                        { schism_date(2022, 4, 30), behaviour::ITMultiSampleInstrumentNumber },
                        { schism_date(2023, 3, 9), behaviour::ITInitialNoteMemory },
                        { schism_date(2023, 10, 17), behaviour::ITDCTBehaviour },
                        { schism_date(2023, 10, 19), behaviour::ITSampleAndHoldPanbrello },
                        { schism_date(2023, 10, 19), behaviour::ITPortaNoNote },
                        { schism_date(2023, 10, 22), behaviour::ITFirstTickHandling },
                        { schism_date(2023, 10, 22), behaviour::ITMultiSampleInstrumentNumber },
                        { schism_date(2024, 3, 9), behaviour::ITPanbrelloHold },
                        { schism_date(2024, 5, 12), behaviour::ITNoSustainOnPortamento },
                        { schism_date(2024, 5, 12), behaviour::ITEmptyNoteMapSlotIgnoreCell },
                        { schism_date(2024, 5, 27), behaviour::ITOffsetWithInstrNumber },
                        { schism_date(2024, 10, 13), behaviour::ITDoublePortamentoSlides },
                        { schism_date(2025, 1, 8), behaviour::ITCarryAfterNoteOff },
                        { schism_date(2026, 7, 13), behaviour::ITCompatGxxCarryPortaWithIns }
                    };
                    for (auto const& q : QUIRKS)
                        if (date < q.fixed)
                            result.playBehaviour.reset(q.quirk);
                    // Hertz in Amiga mode came later still
                    if (date < schism_date(2021, 5, 2) && !result.linearSlides)
                        result.playBehaviour.reset(behaviour::PeriodsAreHertz);
                    if (date < schism_date(2021, 11, 1))
                        result.playBehaviour.set(behaviour::ImprecisePingPongLoops);
                }
                break;
            default:
                break;
            }
        }
        else
            result.tracker = "OpenMPT";
        apply_openmpt_upgrades(result);
        // early Impulse Tracker had no filters, so any Zxx it saved means nothing
        if ((cwtv < 0x0214u && cmwt < 0x0214u) || (result.openmptVersion != 0u && result.openmptVersion <= version(0x1, 0x0, 0x0, 0xA6)))
        {
            for (auto& macro : result.parameteredMacros)
                macro.clear();
            for (auto& macro : result.fixedMacros)
                macro.clear();
        }
        if (result.openmptVersion != 0u)
            result.formatName = result.format == module_format::MPTM ? "OpenMPT" : "Impulse Tracker (ModPlug)";
        return result;
    }
}
