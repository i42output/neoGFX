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
Portions of this file (the row, note, instrument and effect processing (OpenMPT's
soundlib/Snd_fx.cpp)) are derived from OpenMPT (https://openmpt.org/).

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

// The tick engine: rows, notes, instruments and effects. The many ways in which the trackers differ are
// followed as OpenMPT documents them (as the "behaviours" of the module), so that a module plays as it did
// in the tracker it was made with.

#include <mod_tracker/replayer.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

#include "tables.hpp"

namespace mod_tracker
{
    namespace
    {
        // the sample an invalid sample number refers to: silent, but with the default settings
        sample const& empty_sample()
        {
            static sample const sEmpty = []()
                {
                    sample result;
                    result.volume = 256u;
                    result.globalVolume = 64u;
                    result.c5speed = 8363u;
                    return result;
                }();
            return sEmpty;
        }

        // ModPlug's attenuation by the number of channels (for its "original" mix levels)
        constexpr std::uint8_t PRE_AMP_ATTENUATION[16] =
        {
            0x60, 0x60, 0x60, 0x70, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC
        };

        constexpr std::uint32_t MAX_VOICES = 256u;
        constexpr std::uint32_t MAX_SAMPLE_LENGTH = 0x10000000u;
        constexpr std::uint8_t NOTE_MIN_SPECIAL = 251u;

        // where IT's volume column sample offset (and DigiBooster's combined offsets) start
        std::uint32_t cue_point(std::uint32_t aIndex)
        {
            return (aIndex + 1u) << 11u;
        }
    }

    using namespace detail;

    replayer::replayer(module_data const& aModule, render_options const& aOptions) :
        iModule{ aModule },
        iOptions{ aOptions },
        iRampUpFrames{ std::max<std::uint32_t>(1u, static_cast<std::uint32_t>(std::lround(aOptions.sampleRate * 363e-6))) },
        iRampDownFrames{ std::max<std::uint32_t>(1u, static_cast<std::uint32_t>(std::lround(aOptions.sampleRate * 952e-6))) },
        iPeaks(aModule.channels, 0.0f)
    {
        // The mix: OpenMPT's pre-amp and mix levels, so that modules are as loud relative to each other as
        // they are there.
        double level = 0.25;
        std::uint32_t const attenuation = PRE_AMP_ATTENUATION[std::clamp<std::uint32_t>(aModule.channels, 1u, 31u) / 2u];
        switch (aModule.mixLevels)
        {
        case mix_levels::Original:
        case mix_levels::v1_17RC1:
        case mix_levels::v1_17RC2:
            level = 16.0 / attenuation;
            break;
        case mix_levels::v1_17RC3:
            level = 0.5;
            break;
        case mix_levels::Compatible:
        case mix_levels::CompatibleFT2:
        default:
            level = 0.25;
            break;
        }
        iMasterGain = static_cast<float>(aModule.samplePreAmp / 128.0 * level * 4.0);
        iSeparation = static_cast<float>(std::clamp(aOptions.stereoSeparation, 0.0, 1.0));

        auto& s = iState;
        s.channels.reserve(MAX_VOICES);
        s.channels.resize(aModule.channels);
        for (std::uint32_t index = 0u; index < aModule.channels; ++index)
        {
            auto& ch = s.channels[index];
            ch.note = ch.newNote = behaves(behaviour::ITInitialNoteMemory) ? NOTE_MIN : NOTE_NONE;
            ch.fadeOutVolume = 0;
            ch.flags = CHN_KEYOFF | CHN_NOTEFADE;
            ch.instrumentVolume = 64;
            if (behaves(behaviour::ITRetrigger))
            {
                ch.retriggerParameter = 1u;
                ch.retriggerCount = 0u;
            }
            auto const& settings = aModule.channelSettings[index];
            ch.pan = settings.pan;
            ch.channelVolume = settings.volume;
            if (settings.surround)
                ch.flags |= CHN_SURROUND;
            if (settings.muted)
                ch.flags |= CHN_MUTE;
        }
        s.speed = std::max<std::uint32_t>(aModule.speed, 1u);
        s.tempo = aModule.tempo;
        s.globalVolume = static_cast<std::int32_t>(aModule.globalVolume);
        s.nextOrder = 0u;
        s.nextRow = 0u;
        if (!resolve_row())
            s.ended = true;
        s.rowPending = true;
    }

    void replayer::set_row_handler(row_handler aHandler)
    {
        iRowHandler = std::move(aHandler);
    }

    replayer_state const& replayer::state() const
    {
        return iState;
    }

    void replayer::restore(replayer_state const& aState)
    {
        iState = aState;
        // background voices are added during a tick while references to other channels are held
        iState.channels.reserve(MAX_VOICES);
    }

    void replayer::restore_timing(replayer_state const& aState)
    {
        iState.speed = aState.speed;
        iState.tempo = aState.tempo;
        iState.tickFraction = aState.tickFraction;
    }

    bool replayer::ended() const
    {
        return iState.ended;
    }

    void replayer::end()
    {
        iState.ended = true;
    }

    std::uint64_t replayer::frames() const
    {
        return iFrames;
    }

    float replayer::take_peak(std::uint32_t aChannel)
    {
        return aChannel < iPeaks.size() ? std::exchange(iPeaks[aChannel], 0.0f) : 0.0f;
    }

    std::int32_t replayer::random_byte()
    {
        iState.randomSeed = iState.randomSeed * 1103515245u + 12345u;
        return static_cast<std::int8_t>(iState.randomSeed >> 16u);
    }

    sample const* replayer::sample_slot(std::uint32_t aSample) const
    {
        return aSample >= 1u && aSample <= sample_count() ? &iModule.samples[aSample - 1u] : &empty_sample();
    }

    instrument const* replayer::instrument_slot(std::uint32_t aInstrument) const
    {
        return aInstrument >= 1u && aInstrument <= instrument_count() ? &iModule.instruments[aInstrument - 1u] : nullptr;
    }

    std::uint32_t replayer::sample_flags(sample const& aSample)
    {
        std::uint32_t result = 0u;
        if (aSample.loop != loop_type::None)
            result |= CHN_LOOP;
        if (aSample.loop == loop_type::PingPong)
            result |= CHN_PINGPONGLOOP;
        if (aSample.sustain != loop_type::None)
            result |= CHN_SUSTAINLOOP;
        if (aSample.sustain == loop_type::PingPong)
            result |= CHN_PINGPONGSUSTAIN;
        if (aSample.pan)
            result |= CHN_PANNING;
        if (aSample.stereo)
            result |= CHN_STEREO;
        return result;
    }

    void replayer::reset_envelopes(channel_state& aChannel) const
    {
        aChannel.volumeEnvelope.position = 0u;
        aChannel.panningEnvelope.position = 0u;
        aChannel.pitchEnvelope.position = 0u;
    }

    void replayer::update_instrument_volume(channel_state& aChannel, sample const* aSample, instrument const* aInstrument) const
    {
        aChannel.instrumentVolume = 64;
        if (aSample != nullptr)
            aChannel.instrumentVolume = aSample->globalVolume;
        if (aInstrument != nullptr)
            aChannel.instrumentVolume = aChannel.instrumentVolume * aInstrument->globalVolume / 64;
    }

    void replayer::set_instrument_pan(channel_state& aChannel, std::int32_t aPan) const
    {
        if (behaves(behaviour::ITDoNotOverrideChannelPan))
        {
            aChannel.restorePanOnNewNote = static_cast<std::uint16_t>(aChannel.pan + 1);
            if ((aChannel.flags & CHN_SURROUND) != 0u)
                aChannel.restorePanOnNewNote |= 0x8000u;
        }
        aChannel.pan = aPan;
    }

    void replayer::restore_pan_and_filter(channel_state& aChannel) const
    {
        if (aChannel.restorePanOnNewNote > 0u)
        {
            aChannel.pan = (aChannel.restorePanOnNewNote & 0x7FFF) - 1;
            if ((aChannel.restorePanOnNewNote & 0x8000u) != 0u)
                aChannel.flags |= CHN_SURROUND;
            aChannel.restorePanOnNewNote = 0u;
        }
        if (aChannel.restoreResonanceOnNewNote > 0u)
        {
            aChannel.resonance = static_cast<std::uint8_t>(aChannel.restoreResonanceOnNewNote - 1u);
            aChannel.restoreResonanceOnNewNote = 0u;
        }
        if (aChannel.restoreCutoffOnNewNote > 0u)
        {
            aChannel.cutoff = static_cast<std::uint8_t>(aChannel.restoreCutoffOnNewNote - 1u);
            aChannel.restoreCutoffOnNewNote = 0u;
        }
    }

    void replayer::process_pitch_pan_separation(std::int32_t& aPan, std::uint32_t aNote, instrument const& aInstrument) const
    {
        if (aInstrument.pitchPanSeparation == 0 || aNote == NOTE_NONE)
            return;
        auto const delta = (static_cast<std::int32_t>(aNote) - static_cast<std::int32_t>(aInstrument.pitchPanCenter) - NOTE_MIN) * aInstrument.pitchPanSeparation / 2;
        aPan = std::clamp(aPan + delta, 0, 256);
    }

    std::uint32_t replayer::sample_index(std::uint32_t aNote, std::uint32_t aInstrument) const
    {
        std::uint32_t result = 0u;
        if (instrument_count() != 0u)
        {
            if (is_note(static_cast<std::uint8_t>(aNote)) && aInstrument >= 1u && aInstrument <= instrument_count())
                result = iModule.instruments[aInstrument - 1u].keyboard[aNote - NOTE_MIN];
        }
        else
            result = aInstrument;
        return result <= sample_count() ? result : 0u;
    }

    // ---------------------------------------------------------------------------------------------------
    // rows and ticks

    std::uint32_t replayer::ticks_on_row() const
    {
        return (iState.speed + iState.frameDelay) * std::max<std::uint32_t>(iState.patternDelay, 1u);
    }

    std::uint64_t replayer::tick_duration()
    {
        // tempos are kept to four decimal places, and (as in the trackers) a tick is a whole number of frames
        auto const rawTempo = std::max<std::int64_t>(std::llround(iState.tempo * 10000.0), 1);
        switch (iModule.tempoMode)
        {
        case tempo_mode::Classic:
        default:
            return std::max<std::uint64_t>(static_cast<std::uint64_t>(iOptions.sampleRate) * 5u * 10000u / static_cast<std::uint64_t>(rawTempo * 2), 1u);
        case tempo_mode::Alternative:
            return std::max<std::uint64_t>(static_cast<std::uint64_t>(iOptions.sampleRate) * 10000u / static_cast<std::uint64_t>(rawTempo), 1u);
        case tempo_mode::Modern:
            {
                // the fractional frame carries over so that rows don't drift
                auto const frames = iOptions.sampleRate * 60.0 /
                    (std::max(iState.tempo, 1.0) * std::max<std::uint32_t>(iState.speed, 1u) * std::max<std::uint32_t>(iModule.rowsPerBeat, 1u));
                auto whole = static_cast<std::int64_t>(frames);
                iState.tickFraction += frames - static_cast<double>(whole);
                if (iState.tickFraction >= 1.0)
                {
                    ++whole;
                    iState.tickFraction -= 1.0;
                }
                return static_cast<std::uint64_t>(std::max<std::int64_t>(whole, 1));
            }
        }
    }

    // The song moves on to its next row: work out which order position, pattern and row that is, going
    // past markers and back to the restart position at the end of the order list.
    bool replayer::resolve_row()
    {
        auto& s = iState;
        s.ignoreRow = s.patternDelay > 1u && s.breakToRow && is_mod();
        bool const patternTransition = s.nextRow == 0u || s.breakToRow;
        if (patternTransition && is_s3m())
            for (std::uint32_t index = 0u; index < iModule.channels; ++index)
                s.channels[index].patternLoop = 0u;
        s.patternDelay = 0u;
        s.frameDelay = 0u;
        s.tick = 0u;
        s.row = s.nextRow;
        s.orderPosition = s.nextOrder;

        auto const& orders = iModule.orders;
        auto const songEnd = static_cast<std::uint32_t>(orders.size());
        auto const order_pattern = [&](std::uint32_t aOrder) -> std::uint32_t
            {
                return aOrder < songEnd ? orders[aOrder] : module_data::ORDER_END;
            };
        auto pattern = order_pattern(s.orderPosition);
        while (pattern >= iModule.patterns.size())
        {
            if (pattern == module_data::ORDER_END || s.orderPosition >= songEnd)
            {
                // the end of the song: back to the restart position, or the start of this sub-song
                std::uint32_t restart = iModule.restart;
                if (restart == 0u && s.orderPosition <= songEnd && s.orderPosition > 0u)
                    for (std::uint32_t order = s.orderPosition - 1u; order > 0u; --order)
                        if (orders[order] == module_data::ORDER_END)
                        {
                            restart = order + 1u;
                            break;
                        }
                s.orderPosition = restart;
                s.breakToRow = false;
                while (s.orderPosition < songEnd && orders[s.orderPosition] == module_data::ORDER_SKIP)
                    ++s.orderPosition;
                if (s.orderPosition >= songEnd || !iModule.valid_pattern(orders[s.orderPosition]))
                    return false;
            }
            else
                ++s.orderPosition;
            pattern = order_pattern(s.orderPosition);
        }
        s.pattern = pattern;
        s.nextOrder = s.orderPosition;
        if (s.row >= iModule.patterns[pattern].rows)
            s.row = 0u;
        return true;
    }

    // The resolved row starts: it is reported, its cells are read and the quirks of row transitions applied.
    bool replayer::start_row()
    {
        auto& s = iState;
        for (;;)
        {
            if (iRowHandler && !iRowHandler(*this))
            {
                s.ended = true;
                return false;
            }
            s.patternLoopJump = false;
            // where the song goes next, unless an effect says otherwise
            s.nextRow = s.row + 1u;
            if (s.nextRow >= iModule.patterns[s.pattern].rows)
            {
                s.nextOrder = s.orderPosition + 1u;
                s.nextRow = 0u;
                if (behaves(behaviour::FT2LoopE60Restart))
                {
                    s.nextRow = s.nextPatternStartRow;
                    s.nextPatternStartRow = 0u;
                }
            }
            for (std::uint32_t index = 0u; index < iModule.channels; ++index)
            {
                auto& ch = s.channels[index];
                auto const& next = iModule.at(s.pattern, s.row, index);
                // ScreamTracker: a portamento right after an arpeggio carries on from where the arpeggio left off
                if (behaves(behaviour::ST3PortaAfterArpeggio) && ch.activeCommand == effect::Arpeggio &&
                    (next.command == effect::PortamentoUp || next.command == effect::PortamentoDown))
                    ch.period = period_from_note(ch.arpeggioLastNote, ch.finetune, ch.c5speed);
                // ProTracker: a note delayed out of the row is heard on the next row if that has no note
                if (behaves(behaviour::MODOutOfRangeNoteDelay) && !is_note(next.note) && is_note(ch.row.note) &&
                    ch.row.command == effect::ExtendedMod && (ch.row.parameter & 0xF0u) == 0xD0u && (ch.row.parameter & 0x0Fu) >= s.speed)
                    ch.period = period_from_note(ch.row.note, ch.finetune, 0u);
                // ProTracker sets the tempo after the first tick, which at speed 1 is the next row
                if (behaves(behaviour::MODTempoOnSecondTick) && !behaves(behaviour::MODVBlankTiming) && s.speed == 1u && ch.row.command == effect::Tempo)
                    s.tempo = std::max<std::uint8_t>(1u, ch.row.parameter);
                ch.leftGain = ch.newLeftGain;
                ch.rightGain = ch.newRightGain;
                ch.rampFrames = 0u;
                ch.flags &= ~(CHN_VIBRATO | CHN_TREMOLO);
                if (!behaves(behaviour::ITVibratoTremoloPanbrello))
                    ch.panbrelloOffset = 0;
                ch.activeCommand = effect::None;
                ch.row = next;
            }
            if (!s.ignoreRow)
                return true;
            // ProTracker: a pattern break on a row with a row delay skips the row it breaks to
            s.tick = s.speed;
            if (!resolve_row())
            {
                s.ended = true;
                return false;
            }
        }
    }

    void replayer::advance_tick()
    {
        auto& s = iState;
        if (!s.rowPending && s.tick + 1u >= ticks_on_row())
        {
            if (!resolve_row())
            {
                s.ended = true;
                return;
            }
            s.rowPending = true;
        }
        if (s.rowPending)
        {
            if (!start_row())
                return;
            s.rowPending = false;
        }
        else
            ++s.tick;
        if (s.speed == 0u)
            s.speed = 1u;
        if (s.tick != 0u)
        {
            s.firstTick = false;
            // a row delay repeats the first tick's effects (except in FastTracker 2 and most of ProTracker)
            if (!is_xm() && (!is_mod() || iModule.protrackerMode) && s.tick < ticks_on_row() && s.tick % (s.speed + s.frameDelay) == 0u)
                s.firstTick = true;
        }
        else
        {
            s.firstTick = true;
            s.breakToRow = false;
        }
        process_effects();
        s.tickFramesLeft = tick_duration();
        read_note();
    }

    // ---------------------------------------------------------------------------------------------------
    // effects

    void replayer::process_effects()
    {
        auto& s = iState;
        s.breakRow.reset();
        s.patternLoopRow.reset();
        s.positionJump.reset();
        for (std::uint32_t index = 0u; index < iModule.channels; ++index)
        {
            auto& ch = s.channels[index];
            std::uint32_t const tickCount = s.tick % (s.speed + s.frameDelay);
            std::uint32_t instr = ch.row.instrument;
            auto volcmd = ch.row.volumeCommand;
            std::uint32_t vol = ch.row.volume;
            auto cmd = ch.row.command;
            std::uint32_t param = ch.row.parameter;
            bool porta = ch.row.is_tone_portamento();
            std::uint32_t startTick = 0u;
            ch.firstTick = s.firstTick;

            // Impulse Tracker ignores the whole cell (global effects too) when its note maps to no sample
            if (behaves(behaviour::ITEmptyNoteMapSlotIgnoreCell) && instr > 0u && instr <= instrument_count())
            {
                auto const note = ch.row.note != NOTE_NONE ? ch.row.note : ch.newNote;
                if (is_note(note) && iModule.instruments[instr - 1u].keyboard[note - NOTE_MIN] == 0u)
                {
                    ch.newNote = ch.lastNote = note;
                    ch.newInstrument = static_cast<std::uint8_t>(instr);
                    ch.row = cell{};
                    continue;
                }
            }

            // the effects that decide when the note starts: note delay, and the row delays
            if (cmd == effect::DelayCut)
            {
                startTick = (param & 0xF0u) >> 4u;
                note_cut(index, startTick + (param & 0x0Fu), behaves(behaviour::ITSCxStopsSample));
            }
            else if (cmd == effect::ExtendedMod || cmd == effect::ExtendedS3M)
            {
                if (param == 0u && (is_s3m() || is_it()))
                    param = ch.oldExtendedCommand;
                else
                    ch.oldExtendedCommand = static_cast<std::uint8_t>(param);
                if ((param & 0xF0u) == 0xD0u)
                {
                    startTick = param & 0x0Fu;
                    if (startTick == 0u)
                    {
                        if (is_it())
                            startTick = 1u;
                        else if (is_s3m())
                            continue;
                    }
                    else if (startTick >= s.speed + s.frameDelay && behaves(behaviour::ITOutOfRangeDelay))
                    {
                        if (instr != 0u)
                            ch.newInstrument = static_cast<std::uint8_t>(instr);
                        continue;
                    }
                }
                else if (s.firstTick && (param & 0xF0u) == 0xE0u)
                {
                    // only the first row delay on a row counts in Scream Tracker 3 and Impulse Tracker
                    if (!(is_s3m() || is_it()) || s.patternDelay == 0u)
                        if (!is_s3m() || (param & 0x0Fu) != 0u)
                            s.patternDelay = 1u + (param & 0x0Fu);
                }
            }
            if (startTick != 0u && ch.row.note == NOTE_OFF && ch.row.volumeCommand == volume_command::Panning && behaves(behaviour::FT2PanWithDelayedNoteOff))
                ch.row.volumeCommand = volume_command::None;

            bool triggerNote = (s.tick == startTick);
            if (behaves(behaviour::FT2OutOfRangeDelay) && startTick >= s.speed)
                triggerNote = false;
            else if (behaves(behaviour::RowDelayWithNoteDelay) && startTick > 0u && tickCount == startTick)
                triggerNote = true;
            if (behaves(behaviour::ITFirstTickHandling))
                ch.firstTick = (tickCount == startTick);
            ch.triggerNote = false;
            if (behaves(behaviour::FT2PortaDelay) && startTick != 0u)
                porta = false;

            if (iModule.protrackerMode && instr != 0u && s.tick == 0u)
            {
                ch.previousNoteOffset = 0u;
                // ProTracker loads the sample's settings on the first tick, even when the note is delayed
                if (!triggerNote && ch.sample_playing())
                {
                    ch.newInstrument = static_cast<std::uint8_t>(instr);
                    ch.swapSample = static_cast<std::uint16_t>(sample_index(ch.lastNote, instr));
                    if (instr <= sample_count())
                    {
                        ch.volume = iModule.samples[instr - 1u].volume;
                        ch.finetune = iModule.samples[instr - 1u].finetune;
                    }
                }
            }

            // notes, instruments and the volume column's volume and panning
            if (triggerNote)
            {
                std::uint32_t note = ch.row.note;
                if (instr != 0u)
                {
                    ch.newInstrument = static_cast<std::uint8_t>(instr);
                    ch.swapSample = static_cast<std::uint16_t>(sample_index(is_note(static_cast<std::uint8_t>(note)) ? note : ch.lastNote, instr));
                }
                if (is_note(static_cast<std::uint8_t>(note)) && behaves(behaviour::FT2Transpose))
                {
                    // notes beyond FastTracker 2's range are ignored
                    auto transpose = ch.transpose;
                    if (instr != 0u && !porta)
                    {
                        auto const smp = sample_index(note, instr);
                        if (smp > 0u)
                            transpose = iModule.samples[smp - 1u].relativeNote;
                    }
                    auto const computed = static_cast<std::int32_t>(note) + transpose;
                    if (computed < NOTE_MIN + 11 || computed > NOTE_MIN + 130)
                        note = NOTE_NONE;
                }
                else if (is_it() && instrument_count() != 0u && (is_note(static_cast<std::uint8_t>(note)) || note == NOTE_NONE))
                {
                    // invalid instrument numbers do nothing, but are remembered
                    auto const check = instr != 0u ? instr : ch.oldInstrument;
                    if (check != 0u && check > instrument_count())
                    {
                        note = NOTE_NONE;
                        instr = 0u;
                    }
                }
                if (cmd == effect::KeyOff && param == 0u && behaves(behaviour::FT2KeyOff))
                {
                    note = NOTE_NONE;
                    instr = 0u;
                }
                bool retriggerEnvelopes = (note == NOTE_NONE && instr != 0u);
                bool reloadSampleSettings = behaves(behaviour::FT2ReloadSampleSettings) && instr != 0u;
                bool keepInstrument = is_it() || behaves(behaviour::ST3SampleSwap);
                if (behaves(behaviour::MODSampleSwap))
                {
                    if (!ch.sample_playing() && instr <= sample_count() && instr != 0u && iModule.samples[instr - 1u].loop != loop_type::None)
                        keepInstrument = true;
                }
                if (is_xm())
                {
                    // XM: a note off and no envelope is a note cut
                    if (note == NOTE_OFF && ((instr == 0u && volcmd != volume_command::Volume && cmd != effect::Volume) || !behaves(behaviour::FT2KeyOff)) &&
                        (ch.ins == nullptr || !ch.ins->volumeEnvelope.enabled))
                    {
                        ch.flags |= CHN_FASTVOLRAMP;
                        ch.volume = 0;
                        note = NOTE_NONE;
                        instr = 0u;
                        retriggerEnvelopes = false;
                        if (s.firstTick && behaves(behaviour::FT2NoteOffFlags))
                            ch.flags |= CHN_NOTEFADE;
                    }
                    else if (behaves(behaviour::FT2RetrigWithNoteDelay) && !s.firstTick)
                    {
                        // FastTracker 2's note delay
                        retriggerEnvelopes = true;
                        porta = false;
                        if (note == NOTE_NONE)
                            note = static_cast<std::uint32_t>(static_cast<std::int32_t>(ch.note) - ch.transpose) & 0xFFu;
                        else if (note >= NOTE_MIN_SPECIAL)
                        {
                            note = NOTE_NONE;
                            keepInstrument = false;
                            reloadSampleSettings = true;
                        }
                        else if (instr != 0u || !behaves(behaviour::FT2NoteDelayWithoutInstr))
                        {
                            keepInstrument = true;
                            reloadSampleSettings = true;
                        }
                    }
                }
                if ((retriggerEnvelopes && !behaves(behaviour::FT2ReloadSampleSettings)) || reloadSampleSettings)
                {
                    sample const* oldSample = nullptr;
                    if (instrument_count() != 0u)
                        oldSample = ch.smp;
                    else if (instr <= sample_count())
                        oldSample = sample_slot(instr);
                    if (oldSample != nullptr)
                    {
                        if (!is_s3m() || oldSample->has_data())
                        {
                            ch.volume = oldSample->volume;
                            ch.flags |= CHN_FASTVOLRAMP;
                        }
                        if (reloadSampleSettings)
                            set_instrument_pan(ch, oldSample->pan.value_or(128u));
                    }
                }
                if (behaves(behaviour::FT2Tremor) && instr != 0u)
                    ch.tremorCount = 0x20u;
                if (instrument_count() != 0u && behaves(behaviour::ITInstrWithNoteOffOldEffects) && instr != 0u && !is_note(static_cast<std::uint8_t>(note)))
                {
                    if ((porta && iModule.itCompatibleGxx) || (!porta && iModule.itOldEffects))
                    {
                        reset_envelopes(ch);
                        ch.flags |= CHN_FASTVOLRAMP;
                        ch.fadeOutVolume = 65536;
                    }
                }
                if (retriggerEnvelopes)
                {
                    // an instrument without a note
                    if (behaves(behaviour::ITInstrWithoutNote))
                    {
                        bool const triggerAfterSampleEnd = behaves(behaviour::ITMultiSampleInstrumentNumber) && !ch.sample_playing();
                        if (instrument_count() != 0u)
                        {
                            if (instr <= instrument_count() && (ch.ins != instrument_slot(instr) || triggerAfterSampleEnd))
                                note = ch.note;
                        }
                        else if (ch.smp != sample_slot(instr) || triggerAfterSampleEnd)
                            note = ch.note;
                    }
                    if (instrument_count() != 0u && is_xm())
                    {
                        reset_envelopes(ch);
                        ch.flags |= CHN_FASTVOLRAMP;
                        ch.flags &= ~CHN_NOTEFADE;
                        ch.autoVibratoDepth = 0;
                        ch.autoVibratoPosition = 0u;
                        ch.fadeOutVolume = 65536;
                        if (behaves(behaviour::FT2NoteOffFlags))
                            ch.flags &= ~CHN_KEYOFF;
                    }
                    if (!keepInstrument)
                        instr = 0u;
                }
                // a note cut, off or fade ignores the instrument
                if (note >= NOTE_MIN_SPECIAL)
                {
                    if (behaves(behaviour::ITInstrWithNoteOff) && instr != 0u)
                    {
                        auto const smp = sample_index(ch.lastNote, instr);
                        if (smp > 0u)
                            ch.volume = iModule.samples[smp - 1u].volume;
                    }
                    if (!behaves(behaviour::ITInstrWithNoteOffOldEffects) || !iModule.itOldEffects)
                        instr = 0u;
                }
                auto const previousNewNote = ch.newNote;
                if (is_note(static_cast<std::uint8_t>(note)))
                {
                    ch.newNote = ch.lastNote = static_cast<std::uint8_t>(note);
                    if (!porta)
                        check_nna(index, instr, note, false);
                    restore_pan_and_filter(s.channels[index]);
                }
                auto& c = s.channels[index];
                if (instr != 0u)
                {
                    auto const* oldSample = c.smp;
                    instrument_change(index, instr, porta, true);
                    if (is_mod())
                    {
                        if (!porta || !behaves(behaviour::MODSampleSwap))
                            c.newInstrument = 0u;
                    }
                    else if (!behaves(behaviour::ITInstrWithNoteOff) || is_note(static_cast<std::uint8_t>(note)))
                        c.newInstrument = 0u;
                    if (behaves(behaviour::ITPortamentoSwapResetsPos))
                    {
                        if (is_note(static_cast<std::uint8_t>(note)) && oldSample != c.smp)
                            c.position = 0.0;
                    }
                    else if (is_it() && oldSample != c.smp && is_note(static_cast<std::uint8_t>(note)))
                        porta = false;
                    else if (behaves(behaviour::ST3SampleSwap) && oldSample != c.smp && (porta || !is_note(static_cast<std::uint8_t>(note))) && c.position > c.length)
                        c.length = 0u;
                    else if (behaves(behaviour::MODSampleSwap) && !c.sample_playing())
                        c.position = 0.0;
                }
                if (note != NOTE_NONE)
                {
                    bool const instrumentChange = instr == 0u && c.newInstrument != 0u && is_note(static_cast<std::uint8_t>(note));
                    if (instrumentChange)
                    {
                        if (behaves(behaviour::ITEmptyNoteMapSlotIgnoreCell) && is_note(previousNewNote))
                            c.newNote = previousNewNote;
                        instrument_change(index, c.newInstrument, porta, c.smp == nullptr && c.ins == nullptr, !is_xm());
                        c.newNote = static_cast<std::uint8_t>(note);
                        c.swapSample = 0u;
                        c.newInstrument = 0u;
                    }
                    note_change(index, note, porta, !is_xm());
                    if (porta && is_xm() && instr != 0u)
                    {
                        c.flags |= CHN_FASTVOLRAMP;
                        reset_envelopes(c);
                        c.autoVibratoDepth = 0;
                        c.autoVibratoPosition = 0u;
                    }
                }
                if (volcmd == volume_command::Volume)
                {
                    c.volume = static_cast<std::int32_t>(std::min(vol, 64u) << 2u);
                    c.flags |= CHN_FASTVOLRAMP;
                }
                else if (volcmd == volume_command::Panning)
                    panning(c, vol, pan_bits::Six);
            }

            auto& c = s.channels[index];
            // Scream Tracker 3 doesn't even process the effects of muted channels
            if (behaves(behaviour::ST3NoMutedChannels) && iModule.channelSettings[index].muted)
                continue;

            // the volume column's effects
            bool doVolumeColumn = s.tick >= startTick;
            if (behaves(behaviour::FT2VolColDelay) && startTick != 0u)
                doVolumeColumn = s.tick != 0u && (s.tick != startTick || (c.row.instrument == 0u && volcmd != volume_command::TonePortamento));
            if (behaves(behaviour::ITDoublePortamentoSlides) && c.firstTick)
            {
                bool const effectColumnTonePortamento = (cmd == effect::TonePortamento || cmd == effect::TonePortamentoVolumeSlide);
                if (effectColumnTonePortamento)
                    init_tone_portamento(c, static_cast<std::uint16_t>(cmd == effect::TonePortamentoVolumeSlide ? 0u : param));
                if (volcmd == volume_command::TonePortamento)
                    init_tone_portamento(c, volume_column_tone_portamento(c.row, startTick).first);
                if (vol != 0u && (volcmd == volume_command::PortamentoUp || volcmd == volume_command::PortamentoDown))
                {
                    c.oldPortaUp = c.oldPortaDown = static_cast<std::uint8_t>(vol << 2u);
                    if (!effectColumnTonePortamento && tone_portamento_shares_memory())
                        c.portamentoSlide = static_cast<std::uint16_t>(vol << 2u);
                }
                if (param != 0u && (cmd == effect::PortamentoUp || cmd == effect::PortamentoDown))
                {
                    c.oldPortaUp = c.oldPortaDown = static_cast<std::uint8_t>(param);
                    if (tone_portamento_shares_memory())
                        c.portamentoSlide = static_cast<std::uint16_t>(param);
                }
            }
            if (volcmd > volume_command::Panning && doVolumeColumn)
            {
                if (volcmd == volume_command::TonePortamento)
                {
                    auto const [speed, clearEffect] = volume_column_tone_portamento(c.row, startTick);
                    if (clearEffect)
                        cmd = effect::None;
                    tone_portamento(index, speed);
                }
                else
                {
                    if (behaves(behaviour::FT2VolColMemory) && vol == 0u)
                    {
                        switch (volcmd)
                        {
                        case volume_command::Volume:
                        case volume_command::Panning:
                        case volume_command::VibratoDepth:
                            break;
                        case volume_command::PanSlideLeft:
                            if (!s.firstTick)
                                c.pan = 0;
                            [[fallthrough]];
                        default:
                            volcmd = volume_command::None;
                            break;
                        }
                    }
                    else if (!behaves(behaviour::ITVolColMemory))
                    {
                        if (vol != 0u)
                            c.oldVolumeParameter = static_cast<std::uint8_t>(vol);
                        else
                            vol = c.oldVolumeParameter;
                    }
                    switch (volcmd)
                    {
                    case volume_command::VolumeSlideUp:
                    case volume_command::VolumeSlideDown:
                        if (vol == 0u && behaves(behaviour::ITVolColMemory))
                        {
                            vol = c.oldVolumeParameter;
                            if (vol == 0u)
                                break;
                        }
                        else
                            c.oldVolumeParameter = static_cast<std::uint8_t>(vol);
                        volume_slide(c, static_cast<std::uint8_t>(volcmd == volume_command::VolumeSlideUp ? (vol << 4u) : vol), behaves(behaviour::ITVolColNoSlidePropagation));
                        break;
                    case volume_command::FineVolumeUp:
                        if (s.tick == startTick || !behaves(behaviour::ITVolColMemory))
                            fine_volume_up(c, static_cast<std::uint8_t>(vol), behaves(behaviour::ITVolColMemory));
                        break;
                    case volume_command::FineVolumeDown:
                        if (s.tick == startTick || !behaves(behaviour::ITVolColMemory))
                            fine_volume_down(c, static_cast<std::uint8_t>(vol), behaves(behaviour::ITVolColMemory));
                        break;
                    case volume_command::VibratoSpeed:
                        if (behaves(behaviour::FT2VolColVibrato))
                            c.vibratoSpeed = static_cast<std::uint8_t>(vol & 0x0Fu);
                        else
                            vibrato(c, vol << 4u);
                        break;
                    case volume_command::VibratoDepth:
                        vibrato(c, vol);
                        break;
                    case volume_command::PanSlideLeft:
                        panning_slide(c, static_cast<std::uint8_t>(vol), !behaves(behaviour::FT2VolColMemory));
                        break;
                    case volume_command::PanSlideRight:
                        panning_slide(c, static_cast<std::uint8_t>(vol << 4u), !behaves(behaviour::FT2VolColMemory));
                        break;
                    case volume_command::PortamentoUp:
                        portamento_up(index, static_cast<std::uint8_t>(vol << 2u), behaves(behaviour::ITVolColFinePortamento));
                        break;
                    case volume_command::PortamentoDown:
                        portamento_down(index, static_cast<std::uint8_t>(vol << 2u), behaves(behaviour::ITVolColFinePortamento));
                        break;
                    case volume_command::Offset:
                        if (triggerNote && c.smp != nullptr && vol <= 9u)
                        {
                            std::uint32_t offset = 0u;
                            if (vol == 0u)
                                offset = c.oldOffset;
                            else
                                offset = c.oldOffset = cue_point(vol - 1u);
                            sample_offset(c, offset);
                        }
                        break;
                    default:
                        break;
                    }
                }
            }

            // the effect column
            switch (cmd)
            {
            case effect::None:
                break;
            case effect::Volume:
                if (s.firstTick)
                {
                    c.volume = param < 64u ? static_cast<std::int32_t>(param * 4u) : 256;
                    c.flags |= CHN_FASTVOLRAMP;
                }
                break;
            case effect::PortamentoUp:
                if (param != 0u || !is_mod())
                    portamento_up(index, static_cast<std::uint8_t>(param), false);
                break;
            case effect::PortamentoDown:
                if (param != 0u || !is_mod())
                    portamento_down(index, static_cast<std::uint8_t>(param), false);
                break;
            case effect::VolumeSlide:
                if (param != 0u || !is_mod())
                    volume_slide(c, static_cast<std::uint8_t>(param));
                break;
            case effect::TonePortamento:
                tone_portamento(index, static_cast<std::uint16_t>(param));
                break;
            case effect::TonePortamentoVolumeSlide:
                if (param != 0u || !is_mod())
                {
                    if (!c.firstTick || !behaves(behaviour::S3MIgnoreCombinedFineSlides))
                        volume_slide(c, static_cast<std::uint8_t>(param));
                }
                tone_portamento(index, 0u);
                break;
            case effect::Vibrato:
                vibrato(c, param);
                break;
            case effect::VibratoVolumeSlide:
                if (param != 0u || !is_mod())
                {
                    if (!c.firstTick || !behaves(behaviour::S3MIgnoreCombinedFineSlides))
                        volume_slide(c, static_cast<std::uint8_t>(param));
                }
                vibrato(c, 0u);
                break;
            case effect::Speed:
                if (s.firstTick)
                    set_speed(param);
                break;
            case effect::Tempo:
                if (behaves(behaviour::MODVBlankTiming))
                {
                    if (s.firstTick && param != 0u)
                        set_speed(param);
                }
                else
                {
                    if (is_s3m() || is_it())
                    {
                        if (param != 0u)
                            c.oldTempo = static_cast<std::uint8_t>(param);
                        else
                            param = c.oldTempo;
                    }
                    set_tempo(param);
                }
                break;
            case effect::Offset:
                if (triggerNote)
                {
                    // FastTracker 2: portamento and offset together ignore the offset
                    if (porta && (is_xm() || is_dbm()))
                        break;
                    process_sample_offset(index);
                }
                break;
            case effect::Arpeggio:
                if (s.tick != 0u)
                    break;
                if ((c.period == 0 || c.note == NOTE_NONE) && !behaves(behaviour::ITArpeggio) && is_it())
                    break;
                if (param == 0u && (is_xm() || is_mod()))
                    break;
                c.activeCommand = effect::Arpeggio;
                if (param != 0u)
                    c.arpeggio = static_cast<std::uint8_t>(param);
                break;
            case effect::Retrigger:
                if (is_xm())
                {
                    if ((param & 0xF0u) == 0u)
                        param |= c.retriggerParameter & 0xF0u;
                    if ((param & 0x0Fu) == 0u)
                        param |= c.retriggerParameter & 0x0Fu;
                    param |= 0x100u;
                }
                if (behaves(behaviour::ITRetrigger))
                {
                    if (param != 0u)
                        c.retriggerParameter = static_cast<std::uint8_t>(param & 0xFFu);
                    retrigger_note(index, c.retriggerParameter, volcmd == volume_command::Offset ? static_cast<std::int32_t>(vol) + 1 : 0);
                }
                else
                {
                    if (param != 0u)
                        c.retriggerParameter = static_cast<std::uint8_t>(param & 0xFFu);
                    else
                        param = c.retriggerParameter;
                    retrigger_note(index, param, volcmd == volume_command::Offset ? static_cast<std::int32_t>(vol) + 1 : 0);
                }
                break;
            case effect::Tremor:
                if (!s.firstTick)
                    break;
                if (behaves(behaviour::ITTremor))
                {
                    if (param != 0u && !iModule.itOldEffects)
                    {
                        // old effects make each of on and off a tick longer
                        if ((param & 0xF0u) != 0u)
                            param -= 0x10u;
                        if ((param & 0x0Fu) != 0u)
                            param -= 0x01u;
                        c.tremorParameter = static_cast<std::uint8_t>(param);
                    }
                    c.tremorCount |= 0x80u;
                }
                else if (behaves(behaviour::FT2Tremor))
                    c.tremorCount |= 0x80u;
                c.activeCommand = effect::Tremor;
                if (param != 0u)
                    c.tremorParameter = static_cast<std::uint8_t>(param);
                break;
            case effect::GlobalVolume:
                if (!s.firstTick)
                    break;
                if (!(is_it() || is_dbm()))
                    param *= 2u;
                if (param <= 128u)
                    s.globalVolume = static_cast<std::int32_t>(param * 2u);
                else if (!(is_it() || is_s3m()))
                    s.globalVolume = 256;
                break;
            case effect::GlobalVolumeSlide:
                global_volume_slide(static_cast<std::uint8_t>(param), behaves(behaviour::PerChannelGlobalVolSlide) ? index : 0u);
                break;
            case effect::Panning:
                if (s.firstTick)
                    panning(c, param, pan_bits::Eight);
                break;
            case effect::PanningSlide:
                panning_slide(c, static_cast<std::uint8_t>(param));
                break;
            case effect::Tremolo:
                tremolo(c, param);
                break;
            case effect::FineVibrato:
                fine_vibrato(c, param);
                break;
            case effect::ExtendedMod:
                extended_mod_commands(index, static_cast<std::uint8_t>(param));
                break;
            case effect::ExtendedS3M:
                extended_s3m_commands(index, static_cast<std::uint8_t>(param));
                break;
            case effect::KeyOff:
                if (behaves(behaviour::FT2KeyOff))
                {
                    if (s.tick == param)
                    {
                        // XM: key off and no envelope is a note cut
                        if (c.ins == nullptr || !c.ins->volumeEnvelope.enabled)
                        {
                            if (param == 0u && (c.row.instrument != 0u || c.row.volumeCommand != volume_command::None))
                                c.flags |= CHN_NOTEFADE;
                            else
                            {
                                c.flags |= CHN_FASTVOLRAMP;
                                c.volume = 0;
                            }
                        }
                        key_off(c);
                    }
                }
                else if (s.firstTick)
                    key_off(c);
                break;
            case effect::ExtraFinePortamento:
                switch (param & 0xF0u)
                {
                case 0x10u:
                    extra_fine_portamento_up(c, static_cast<std::uint8_t>(param & 0x0Fu));
                    break;
                case 0x20u:
                    extra_fine_portamento_down(c, static_cast<std::uint8_t>(param & 0x0Fu));
                    break;
                case 0x50u:
                case 0x60u:
                case 0x70u:
                case 0x90u:
                case 0xA0u:
                    if (!behaves(behaviour::FT2RestrictXCommand))
                        extended_s3m_commands(index, static_cast<std::uint8_t>(param));
                    break;
                default:
                    break;
                }
                break;
            case effect::ChannelVolume:
                if (!s.firstTick)
                    break;
                if (param <= 64u)
                {
                    c.channelVolume = static_cast<std::int32_t>(param);
                    c.flags |= CHN_FASTVOLRAMP;
                }
                break;
            case effect::ChannelVolumeSlide:
                channel_volume_slide(c, static_cast<std::uint8_t>(param));
                break;
            case effect::Panbrello:
                panbrello(c, param);
                break;
            case effect::SetEnvelopePosition:
                if (s.firstTick)
                {
                    c.volumeEnvelope.position = param;
                    // FastTracker 2 only sets the panning envelope's position if the volume envelope has a sustain point
                    if (!behaves(behaviour::FT2SetPanEnvPos) || (c.ins != nullptr && c.ins->volumeEnvelope.sustain))
                    {
                        c.panningEnvelope.position = param;
                        c.pitchEnvelope.position = param;
                    }
                }
                break;
            case effect::PositionJump:
                s.nextPatternStartRow = 0u;
                s.positionJump = param;
                // FastTracker 2 resets the pattern break's row when a position jump follows it
                if ((is_mod() || is_xm()) && s.breakRow)
                    s.breakRow = 0u;
                break;
            case effect::PatternBreak:
                if (auto const row = pattern_break(static_cast<std::uint8_t>(param)))
                    s.breakRow = *row;
                break;
            default:
                break;
            }

            if (behaves(behaviour::ST3EffectMemory) && cmd != effect::None && param != 0u)
                update_s3m_effect_memory(c, static_cast<std::uint8_t>(param));
            if (c.row.instrument != 0u)
                c.oldInstrument = c.row.instrument;
        }

        // where the song goes next
        if (s.firstTick && handle_next_row())
            s.breakToRow = true;
    }

    bool replayer::handle_next_row()
    {
        auto& s = iState;
        bool const doPatternLoop = s.patternLoopRow.has_value();
        bool const doBreakRow = s.breakRow.has_value();
        bool const doPositionJump = s.positionJump.has_value();
        bool breakToRow = false;
        s.patternLoopJump = false;
        // a pattern break or position jump only applies if there's no pattern loop, except in FastTracker 2
        // and (for position jumps) Impulse Tracker
        if ((doBreakRow || doPositionJump) &&
            (!doPatternLoop || behaves(behaviour::FT2PatternLoopWithJumps) ||
                (behaves(behaviour::ITPatternLoopWithJumps) && doPositionJump) || (behaves(behaviour::ITPatternLoopWithJumpsOld) && doPositionJump)))
        {
            std::uint32_t target = doPositionJump ? *s.positionJump : s.orderPosition + 1u;
            std::uint32_t const row = doBreakRow ? *s.breakRow : 0u;
            breakToRow = true;
            if (target >= iModule.orders.size())
                target = iModule.restart;
            if (target != s.orderPosition && !behaves(behaviour::ITPatternLoopBreak) && !behaves(behaviour::FT2PatternLoopWithJumps) && !is_mod())
                for (std::uint32_t index = 0u; index < iModule.channels; ++index)
                    s.channels[index].patternLoopCount = 0u;
            s.nextRow = row;
            s.nextOrder = target;
        }
        else if (doPatternLoop)
        {
            s.nextOrder = s.orderPosition;
            s.nextRow = *s.patternLoopRow;
            s.patternLoopJump = true;
            // FastTracker 2 skips the first row of the loop if there's a row delay (Impulse Tracker and Scream Tracker 3 don't)
            if (s.patternDelay != 0u && (iModule.format != module_format::IT || !behaves(behaviour::ITPatternLoopWithJumps)) && !is_s3m())
                ++s.nextRow;
            if (*s.patternLoopRow >= iModule.patterns[s.pattern].rows)
            {
                ++s.nextOrder;
                s.nextRow = 0u;
            }
        }
        return breakToRow;
    }

    // ---------------------------------------------------------------------------------------------------
    // notes and instruments

    void replayer::instrument_change(std::uint32_t aChannel, std::uint32_t aInstrument, bool aPorta, bool aUpdateVolume, bool aResetEnvelopes)
    {
        auto& s = iState;
        auto& ch = s.channels[aChannel];
        instrument const* ins = aInstrument <= instrument_count() ? instrument_slot(aInstrument) : nullptr;
        sample const* smp = sample_slot(aInstrument <= sample_count() ? aInstrument : 0u);
        auto const oldInstrumentVolume = ch.instrumentVolume;
        auto const note = ch.newNote;

        if (note == NOTE_NONE && behaves(behaviour::ITInstrWithoutNote))
            return;
        if (ins != nullptr && is_note(note))
        {
            // Impulse Tracker ignores empty note map slots
            if (ins->keyboard[note - NOTE_MIN] == 0u && behaves(behaviour::ITEmptyNoteMapSlot))
            {
                ch.ins = ins;
                return;
            }
            if (ins->noteMap[note - NOTE_MIN] > NOTE_MAX)
                return;
            auto const n = ins->keyboard[note - NOTE_MIN];
            smp = n != 0u ? sample_slot(n <= sample_count() ? n : 0u) : nullptr;
        }
        else if (instrument_count() != 0u)
        {
            if (note >= NOTE_MIN_SPECIAL)
                return;
            if (behaves(behaviour::ITEmptyNoteMapSlot))
            {
                ch.ins = nullptr;
                ch.swapSample = 0u;
                ch.newInstrument = 0u;
                return;
            }
            smp = nullptr;
        }

        bool returnAfterVolumeAdjust = false;
        bool instrumentChanged = (ins != ch.ins);
        bool const sampleChanged = (ch.smp != nullptr) && (smp != ch.smp);
        if (sampleChanged && aPorta)
        {
            // no sample change during portamento with compatible Gxx
            if (behaves(behaviour::ITPortamentoInstrument) && iModule.itCompatibleGxx && ch.increment != 0.0)
                smp = ch.smp;
            // FastTracker 2, ProTracker and Scream Tracker 3 keep the old sample playing with the new sample's settings
            if ((!instrumentChanged && is_xm() && ins != nullptr) || (is_mod() && ch.sample_playing()) ||
                (behaves(behaviour::ST3PortaSampleChange) && ch.sample_playing()))
                returnAfterVolumeAdjust = true;
            if (behaves(behaviour::ITResetFilterOnPortaSmpChange) && instrument_count() == 0u)
                ch.triggerNote = true;
        }
        // a lone instrument number only changes the sample properties, not the sample, in instrument mode
        if (instrument_count() != 0u && !instrumentChanged && sampleChanged && ch.currentSample &&
            behaves(behaviour::ITMultiSampleInstrumentNumber) && !is_note(ch.row.note))
            returnAfterVolumeAdjust = true;
        // envelope pickup after a note cut
        if (!ch.sample_playing() && is_it())
            instrumentChanged = true;
        // FastTracker 2: a new instrument with portamento keeps the old instrument but reloads its settings
        if ((instrumentChanged || sampleChanged) && aPorta && behaves(behaviour::FT2PortaIgnoreInstr) && (ch.ins != nullptr || ch.smp != nullptr))
        {
            ins = ch.ins;
            smp = ch.smp;
            instrumentChanged = false;
        }
        else
            ch.ins = ins;

        if (aUpdateVolume && (!(is_mod() || is_s3m()) || (smp != nullptr && smp->has_data())))
            ch.volume = smp != nullptr ? smp->volume : 0;

        if (returnAfterVolumeAdjust && sampleChanged && smp != nullptr)
        {
            if (behaves(behaviour::MODSampleSwap))
                ch.finetune = smp->finetune;
            if (is_s3m() && smp->has_data())
                ch.c5speed = smp->c5speed;
        }
        if (returnAfterVolumeAdjust)
            return;

        ch.swapSample = 0u;
        ch.newInstrument = 0u;
        if (ins != nullptr && ((!behaves(behaviour::ITNNAReset) && smp != nullptr) || instrumentChanged))
            ch.nna = ins->nna;
        update_instrument_volume(ch, smp, ins);
        // FastTracker 2 only resets panning on instrument numbers; Impulse Tracker only on notes
        if ((aUpdateVolume || !is_xm()) && !behaves(behaviour::ITPanningReset))
            apply_instrument_panning(ch, ins, smp);

        if (aResetEnvelopes)
        {
            bool reset = false;
            bool resetAlways = false;
            if (behaves(behaviour::ITEnvelopeReset))
            {
                bool const instrumentNumber = (aInstrument != 0u);
                reset = (ch.length == 0u || (instrumentNumber && aPorta && iModule.itCompatibleGxx) ||
                    (instrumentNumber && !aPorta && (ch.flags & (CHN_NOTEFADE | CHN_KEYOFF)) != 0u && iModule.itOldEffects));
                resetAlways = ch.fadeOutVolume == 0 || instrumentChanged ||
                    (behaves(behaviour::ITCarryAfterNoteOff) ? !is_note(ch.row.note) : (ch.flags & CHN_KEYOFF) != 0u);
            }
            else
            {
                reset = (!aPorta || !(is_it() || is_dbm()) || iModule.itCompatibleGxx || ch.length == 0u ||
                    ((ch.flags & CHN_NOTEFADE) != 0u && ch.fadeOutVolume == 0));
                resetAlways = !(is_it() || is_dbm()) || instrumentChanged || ins == nullptr || (ch.flags & (CHN_KEYOFF | CHN_NOTEFADE)) != 0u;
            }
            if (reset)
            {
                ch.flags |= CHN_FASTVOLRAMP;
                if (ins != nullptr)
                {
                    if (resetAlways)
                        reset_envelopes(ch);
                    else
                    {
                        // carried envelopes pick up where the last note moved to the background left off
                        bool const compatibleGxxCarryReset = behaves(behaviour::ITCompatGxxCarryPortaWithIns) && aPorta && iModule.itCompatibleGxx;
                        channel_state const* last = s.lastMovedChannel && *s.lastMovedChannel < s.channels.size() ? &s.channels[*s.lastMovedChannel] : nullptr;
                        auto const carry = [&](envelope const& aEnvelope, envelope_state& aState, envelope_state const* aLast)
                            {
                                if (!aEnvelope.carry)
                                    aState.position = 0u;
                                else if (compatibleGxxCarryReset)
                                    aState.position = aLast != nullptr ? aLast->position : 0u;
                            };
                        carry(ins->volumeEnvelope, ch.volumeEnvelope, last != nullptr ? &last->volumeEnvelope : nullptr);
                        carry(ins->panningEnvelope, ch.panningEnvelope, last != nullptr ? &last->panningEnvelope : nullptr);
                        carry(ins->pitchEnvelope, ch.pitchEnvelope, last != nullptr ? &last->pitchEnvelope : nullptr);
                    }
                }
                if (!behaves(behaviour::ITVibratoTremoloPanbrello))
                {
                    ch.autoVibratoDepth = 0;
                    ch.autoVibratoPosition = 0u;
                }
            }
            else if (ins != nullptr && !ins->volumeEnvelope.enabled)
            {
                if (behaves(behaviour::ITPortamentoInstrument))
                    ch.volumeEnvelope.position = 0u;
                else
                    reset_envelopes(ch);
            }
        }

        if (smp == nullptr)
        {
            ch.smp = nullptr;
            ch.instrumentVolume = 0;
            return;
        }
        bool const wasKeyOff = (ch.flags & CHN_KEYOFF) != 0u;
        if (aPorta && smp == ch.smp)
        {
            // portamento doesn't reset the ping-pong direction
            if (instrumentChanged && ins != nullptr && behaves(behaviour::ITNoSustainOnPortamento))
                ch.flags &= ~(CHN_KEYOFF | CHN_NOTEFADE);
            // a sample cut by SCx has to have its length and loops updated
            if ((is_s3m() || is_it()) && ch.length != 0u)
                return;
            if (!is_xm() || !behaves(behaviour::ITFT2DontResetNoteOffOnPorta) || ch.row.instrument != 0u)
                ch.flags &= ~(CHN_KEYOFF | CHN_NOTEFADE);
            ch.flags &= (CHN_CHANNELFLAGS | CHN_PINGPONGFLAG);
        }
        else
        {
            ch.flags &= ~(CHN_KEYOFF | CHN_NOTEFADE);
            if ((behaves(behaviour::ITPingPongNoReset) || !is_it()) && smp == ch.smp && !instrumentChanged)
                ch.flags &= (CHN_CHANNELFLAGS | CHN_PINGPONGFLAG);
            else
                ch.flags &= CHN_CHANNELFLAGS;
            if (ins != nullptr)
            {
                ch.volumeEnvelope.enabled = ins->volumeEnvelope.enabled;
                ch.panningEnvelope.enabled = ins->panningEnvelope.enabled;
                ch.pitchEnvelope.enabled = ins->pitchEnvelope.enabled;
                ch.pitchEnvelope.filter = ins->pitchEnvelope.filter;
                if (ins->pitchEnvelope.enabled && ins->pitchEnvelope.filter && !behaves(behaviour::ITFilterBehaviour) && ch.cutoff == 0u)
                    ch.cutoff = 0x7Fu;
                if (ins->cutoff)
                    ch.cutoff = *ins->cutoff;
                if (ins->resonance)
                    ch.resonance = *ins->resonance;
            }
        }
        if (aPorta && ch.length == 0u && (behaves(behaviour::FT2PortaNoNote) || behaves(behaviour::ITPortaNoNote)))
            ch.increment = 0.0;   // the note just stopped and must not start again
        // a note off with an instrument number and old effects retriggers the envelopes but keeps the sample
        if (ch.row.note == NOTE_OFF && behaves(behaviour::ITInstrWithNoteOffOldEffects) && iModule.itOldEffects && sampleChanged)
        {
            if (ch.smp != nullptr)
                ch.flags |= sample_flags(*ch.smp);
            ch.instrumentVolume = oldInstrumentVolume;
            ch.volume = smp->volume;
            if (smp->pan)
                set_instrument_pan(ch, *smp->pan);
            return;
        }
        ch.smp = smp;
        ch.length = smp->length;
        ch.loopStart = smp->loopStart;
        ch.loopEnd = smp->loopEnd;
        // ProTracker's "one shot" loops: a loop starting at 0 plays the whole sample once first
        if (behaves(behaviour::MODOneShotLoops) && ch.loopStart == 0u)
            ch.loopEnd = smp->length;
        ch.flags |= sample_flags(*smp);
        if (behaves(behaviour::ITVibratoTremoloPanbrello))
        {
            ch.autoVibratoDepth = 0;
            ch.autoVibratoPosition = 0u;
        }
        if (!aPorta || sampleChanged || !(is_mod() || is_xm()))
        {
            ch.c5speed = smp->c5speed;
            ch.finetune = smp->finetune;
        }
        ch.transpose = use_finetune_and_transpose() ? smp->relativeNote : 0;
        if (!behaves(behaviour::FT2PortaTargetNoReset) && !is_mod())
            ch.portamentoTarget = 0;
        if ((ch.flags & CHN_SUSTAINLOOP) != 0u && (!behaves(behaviour::ITNoSustainOnPortamento) || !aPorta || (ins != nullptr && !wasKeyOff)))
        {
            ch.loopStart = smp->sustainStart;
            ch.loopEnd = smp->sustainEnd;
            if ((ch.flags & CHN_PINGPONGSUSTAIN) != 0u)
                ch.flags |= CHN_PINGPONGLOOP;
            ch.flags |= CHN_LOOP;
        }
        if ((ch.flags & CHN_LOOP) != 0u && ch.loopEnd < ch.length)
            ch.length = ch.loopEnd;
        if (ch.position >= ch.length && is_it())
            ch.position = 0.0;
    }

    void replayer::note_change(std::uint32_t aChannel, std::uint32_t aNote, bool aPorta, bool aResetEnvelopes, bool aManual)
    {
        (void)aManual;
        if (aNote < NOTE_MIN)
            return;
        auto& ch = iState.channels[aChannel];
        auto const originalNote = aNote;
        sample const* smp = ch.smp;
        instrument const* ins = ch.ins;
        auto const realNote = aNote;
        std::uint32_t note = aNote;
        if (ins != nullptr && note - NOTE_MIN < ins->keyboard.size())
        {
            auto const n = ins->keyboard[note - NOTE_MIN];
            if (n > 0u)
                smp = sample_slot(n <= sample_count() ? n : 0u);
            else if (behaves(behaviour::ITEmptyNoteMapSlot))
                return;
            note = ins->noteMap[note - NOTE_MIN];
        }
        // note off, cut and fade
        if (note > NOTE_MAX)
        {
            if (note == NOTE_OFF || !is_it())
            {
                key_off(ch);
                if (!aPorta && behaves(behaviour::ITInstrWithNoteOffOldEffects) && iModule.itOldEffects && ch.row.instrument != 0u)
                    ch.flags &= ~(CHN_NOTEFADE | CHN_KEYOFF);
            }
            else if (instrument_count() != 0u)
                ch.flags |= CHN_NOTEFADE;
            if (note == NOTE_CUT)
            {
                ch.flags |= CHN_NOTEFADE | CHN_FASTVOLRAMP;
                if (!is_it() || (instrument_count() != 0u && !behaves(behaviour::ITInstrWithNoteOff)))
                    ch.volume = 0;
                if (behaves(behaviour::ITInstrWithNoteOff))
                    ch.increment = 0.0;
                ch.fadeOutVolume = 0;
            }
            if (behaves(behaviour::ITClearOldNoteAfterCut))
                ch.note = ch.newNote = NOTE_NONE;
            return;
        }
        if (!aPorta && is_xm() && smp != nullptr)
        {
            ch.transpose = smp->relativeNote;
            ch.finetune = smp->finetune;
        }
        if (!aPorta && smp != nullptr && behaves(behaviour::ITMultiSampleBehaviour))
            ch.c5speed = smp->c5speed;
        if (aPorta && !ch.sample_playing())
        {
            if (behaves(behaviour::FT2PortaNoNote))
            {
                // FastTracker 2 ignores a note with portamento if nothing was playing
                ch.period = 0;
                return;
            }
            else if (behaves(behaviour::ITPortaNoNote))
                aPorta = false;
        }
        if (use_finetune_and_transpose())
            note = static_cast<std::uint32_t>(std::clamp(static_cast<std::int32_t>(note) + ch.transpose, NOTE_MIN + 11, NOTE_MIN + 130));
        else
            note = std::clamp<std::uint32_t>(note, NOTE_MIN, NOTE_MAX);
        ch.note = static_cast<std::uint8_t>(behaves(behaviour::ITRealNoteMapping) ? std::clamp<std::uint32_t>(realNote, NOTE_MIN, NOTE_MAX) : note);
        if (!aPorta || is_s3m() || is_it())
        {
            ch.swapSample = 0u;
            ch.newInstrument = 0u;
        }
        auto const period = period_from_note(note, ch.finetune, ch.c5speed);
        ch.panbrelloOffset = 0;
        if (behaves(behaviour::ITPanningReset))
            apply_instrument_panning(ch, ins, smp);
        if (behaves(behaviour::ITPitchPanSeparation) && ins != nullptr && ins->pitchPanSeparation != 0)
        {
            if (ch.restorePanOnNewNote == 0u)
                ch.restorePanOnNewNote = static_cast<std::uint16_t>(ch.pan + 1);
            process_pitch_pan_separation(ch.pan, originalNote, *ins);
        }
        if (aResetEnvelopes && !aPorta)
        {
            ch.volumeSwing = 0;
            ch.panSwing = 0;
            ch.resonanceSwing = 0;
            ch.cutoffSwing = 0;
            if (ins != nullptr)
            {
                if (behaves(behaviour::ITNNAReset))
                    ch.nna = ins->nna;
                if (!ins->volumeEnvelope.carry)
                    ch.volumeEnvelope.position = 0u;
                if (!ins->panningEnvelope.carry)
                    ch.panningEnvelope.position = 0u;
                if (!ins->pitchEnvelope.carry)
                    ch.pitchEnvelope.position = 0u;
                if (ins->volumeSwing != 0u)
                {
                    auto const base = behaves(behaviour::ITSwingBehaviour) ? ch.instrumentVolume : (ch.volume + 1) / 2;
                    ch.volumeSwing = ((random_byte() * ins->volumeSwing) / 64 + 1) * base / 199;
                }
                if (ins->panSwing != 0u)
                {
                    ch.panSwing = (random_byte() * ins->panSwing * 4) / 128;
                    if (!behaves(behaviour::ITSwingBehaviour) && ch.restorePanOnNewNote == 0u)
                        ch.restorePanOnNewNote = static_cast<std::uint16_t>(ch.pan + 1);
                }
            }
        }
        if (smp == nullptr)
            return;
        if (period != 0)
        {
            if (!aPorta || ch.period == 0)
                ch.period = period;
            if (aPorta || !(behaves(behaviour::FT2PortaTargetNoReset) || behaves(behaviour::ITClearPortaTarget) || is_mod()))
            {
                ch.portamentoTarget = period;
                ch.portamentoTargetReached = false;
            }
            if (!aPorta || (ch.length == 0u && !is_s3m()))
            {
                ch.smp = smp;
                ch.length = smp->length;
                ch.loopEnd = smp->length;
                ch.loopStart = 0u;
                ch.position = 0.0;
                if ((iModule.protrackerMode || behaves(behaviour::ST3OffsetWithoutInstrument)) && ch.row.instrument == 0u)
                    ch.position = std::min<std::uint32_t>(ch.previousNoteOffset, ch.length != 0u ? ch.length - 1u : 0u);
                else
                    ch.previousNoteOffset = 0u;
                ch.flags = (ch.flags & CHN_CHANNELFLAGS) | sample_flags(*smp);
                ch.flags &= ~CHN_PORTAMENTO;
                if ((ch.flags & CHN_SUSTAINLOOP) != 0u)
                {
                    ch.loopStart = smp->sustainStart;
                    ch.loopEnd = smp->sustainEnd;
                    if ((ch.flags & CHN_PINGPONGSUSTAIN) != 0u)
                        ch.flags |= CHN_PINGPONGLOOP;
                    else
                        ch.flags &= ~CHN_PINGPONGLOOP;
                    ch.flags |= CHN_LOOP;
                    if (ch.length > ch.loopEnd)
                        ch.length = ch.loopEnd;
                }
                else if ((ch.flags & CHN_LOOP) != 0u)
                {
                    ch.loopStart = smp->loopStart;
                    ch.loopEnd = smp->loopEnd;
                    if (ch.length > ch.loopEnd)
                        ch.length = ch.loopEnd;
                }
                if (behaves(behaviour::MODOneShotLoops) && ch.loopStart == 0u)
                    ch.loopEnd = ch.length = smp->length;
                // the "retrigger" waveforms start again
                if (ch.vibratoType < 4u)
                {
                    if (!behaves(behaviour::ITVibratoTremoloPanbrello) && is_it() && !iModule.itOldEffects)
                        ch.vibratoPosition = 0x10u;
                    else if (!is_dbm())
                        ch.vibratoPosition = 0u;
                }
                if (!behaves(behaviour::ITVibratoTremoloPanbrello) && ch.tremoloType < 4u)
                    ch.tremoloPosition = 0u;
            }
            if (ch.position >= ch.length)
                ch.position = ch.loopStart;
        }
        else
            aPorta = false;

        if (!aPorta || !(is_it() || is_dbm()) || ((ch.flags & CHN_NOTEFADE) != 0u && ch.fadeOutVolume == 0) ||
            (iModule.itCompatibleGxx && ch.row.instrument != 0u))
        {
            if ((is_it() || is_dbm()) && (ch.flags & CHN_NOTEFADE) != 0u && ch.fadeOutVolume == 0)
            {
                reset_envelopes(ch);
                if (!behaves(behaviour::ITVibratoTremoloPanbrello))
                {
                    ch.autoVibratoDepth = 0;
                    ch.autoVibratoPosition = 0u;
                }
                ch.flags &= ~CHN_NOTEFADE;
                ch.fadeOutVolume = 65536;
            }
            if (!aPorta || !iModule.itCompatibleGxx || ch.row.instrument != 0u)
            {
                if (!is_xm() || ch.row.instrument != 0u)
                {
                    ch.flags &= ~CHN_NOTEFADE;
                    ch.fadeOutVolume = 65536;
                }
            }
        }
        // note off isn't reset by portamento unless compatible Gxx is on
        if (!(behaves(behaviour::ITFT2DontResetNoteOffOnPorta) && aPorta && (!iModule.itCompatibleGxx || ch.row.instrument == 0u)))
            ch.flags &= ~CHN_KEYOFF;
        if (!aPorta)
        {
            ch.triggerNote = true;
            ch.flags &= ~CHN_FILTER;
            ch.flags |= CHN_FASTVOLRAMP;
            if (!behaves(behaviour::ITRetrigger) && !behaves(behaviour::ITTremor) && !behaves(behaviour::FT2Retrigger) && !behaves(behaviour::FT2Tremor))
            {
                ch.retriggerCount = 0u;
                ch.tremorCount = 0u;
            }
            if (aResetEnvelopes)
            {
                ch.autoVibratoDepth = 0;
                ch.autoVibratoPosition = 0u;
            }
            // the new note ramps up from silence
            ch.leftGain = ch.rightGain = 0.0f;
            ch.rampFrames = 0u;
            ch.filterHistory = {};
        }
    }

    void replayer::apply_instrument_panning(channel_state& aChannel, instrument const* aInstrument, sample const* aSample)
    {
        std::optional<std::int32_t> pan;
        if (aInstrument != nullptr && aInstrument->pan)
            pan = *aInstrument->pan;
        if (aSample != nullptr && aSample->pan)
            pan = *aSample->pan;
        if (pan)
        {
            set_instrument_pan(aChannel, *pan);
            // sample and instrument panning override surround
            if (behaves(behaviour::PanOverride))
                aChannel.flags &= ~CHN_SURROUND;
        }
    }

    std::optional<std::uint32_t> replayer::background_channel(std::uint32_t aChannel)
    {
        auto& s = iState;
        for (std::uint32_t index = iModule.channels; index < s.channels.size(); ++index)
            if (s.channels[index].length == 0u)
                return index;
        if (s.channels.size() < MAX_VOICES)
        {
            s.channels.emplace_back();
            return static_cast<std::uint32_t>(s.channels.size() - 1u);
        }
        // all in use: the quietest goes
        std::int32_t volume = 0x800100;
        if (aChannel < s.channels.size())
        {
            auto const& source = s.channels[aChannel];
            if (source.fadeOutVolume == 0 && source.length != 0u)
                return {};
            volume = (source.realVolume << 9) | source.volume;
        }
        std::optional<std::uint32_t> result;
        std::uint32_t envelopePosition = 0u;
        for (std::uint32_t index = iModule.channels; index < s.channels.size(); ++index)
        {
            auto const& c = s.channels[index];
            if (c.length != 0u && c.fadeOutVolume == 0)
                return index;
            std::int32_t v = (c.realVolume << 9) | c.volume;
            if ((c.flags & CHN_LOOP) != 0u)
                v /= 2;
            if (v < volume || (v == volume && (c.volumeEnvelope.position > envelopePosition || !c.volumeEnvelope.enabled)))
            {
                envelopePosition = c.volumeEnvelope.position;
                volume = v;
                result = index;
            }
        }
        return result;
    }

    void replayer::check_nna(std::uint32_t aChannel, std::uint32_t aInstrument, std::uint32_t aNote, bool aForceCut)
    {
        auto& s = iState;
        if (!is_note(static_cast<std::uint8_t>(aNote)))
            return;
        // without new note actions, the old note is cut, but fades out quickly in the background so that it doesn't click
        if (!is_it() || instrument_count() == 0u || aForceCut)
        {
            {
                auto const& source = s.channels[aChannel];
                if ((source.flags & CHN_MUTE) != 0u)
                    return;
                if (source.length == 0u || (source.leftGain == 0.0f && source.rightGain == 0.0f && source.newLeftGain == 0.0f && source.newRightGain == 0.0f))
                    return;
            }
            auto const nnaChannel = background_channel(aChannel);
            if (s.lastMovedChannel == aChannel)
                s.lastMovedChannel = nnaChannel;
            if (!nnaChannel)
                return;
            auto& source = s.channels[aChannel];
            auto& voice = s.channels[*nnaChannel];
            voice = source;
            voice.flags &= ~(CHN_VIBRATO | CHN_TREMOLO | CHN_MUTE | CHN_PORTAMENTO);
            voice.panbrelloOffset = 0;
            voice.masterChannel = static_cast<std::uint8_t>(aChannel + 1u);
            voice.activeCommand = effect::None;
            voice.row = cell{};
            voice.fadeOutVolume = 0;
            voice.flags |= CHN_NOTEFADE | CHN_FASTVOLRAMP;
            source.length = 0u;
            source.position = 0.0;
            source.leftGain = source.rightGain = 0.0f;
            source.rampFrames = 0u;
            return;
        }
        if (aInstrument > instrument_count())
            aInstrument = 0u;
        auto& source = s.channels[aChannel];
        sample const* smp = source.smp;
        instrument const* ins = aInstrument > 0u ? instrument_slot(aInstrument) : source.ins;
        auto dnaNote = aNote;
        if (ins != nullptr)
        {
            auto const n = ins->keyboard[aNote - NOTE_MIN];
            if (!behaves(behaviour::ITDCTBehaviour) || !behaves(behaviour::ITRealNoteMapping))
                dnaNote = ins->noteMap[aNote - NOTE_MIN];
            if (n > 0u)
                smp = sample_slot(n <= sample_count() ? n : 0u);
            else if (behaves(behaviour::ITEmptyNoteMapSlot))
                return;
        }
        if ((source.flags & CHN_MUTE) != 0u)
            return;
        // duplicate note checks, on the channel and its background voices
        for (std::uint32_t index = aChannel; index < s.channels.size(); ++index)
        {
            if (index < iModule.channels && index != aChannel)
                continue;
            auto& c = s.channels[index];
            if ((c.masterChannel == aChannel + 1u || index == aChannel) && c.ins != nullptr)
            {
                bool applyDNA = false;
                switch (c.ins->dct)
                {
                case duplicate_check::None:
                    break;
                case duplicate_check::Note:
                    if (dnaNote != NOTE_NONE && c.note == dnaNote && ins == c.ins)
                        applyDNA = true;
                    break;
                case duplicate_check::Sample:
                    if (smp != nullptr && smp == c.smp && (ins == c.ins || !behaves(behaviour::ITDCTBehaviour)))
                        applyDNA = true;
                    break;
                case duplicate_check::Instrument:
                    if (ins == c.ins)
                        applyDNA = true;
                    break;
                }
                if (applyDNA)
                {
                    switch (c.ins->dca)
                    {
                    case duplicate_action::Cut:
                        key_off(c);
                        c.volume = 0;
                        break;
                    case duplicate_action::NoteOff:
                        key_off(c);
                        break;
                    case duplicate_action::NoteFade:
                        c.flags |= CHN_NOTEFADE;
                        break;
                    }
                    if (c.volume == 0)
                    {
                        c.fadeOutVolume = 0;
                        c.flags |= CHN_NOTEFADE | CHN_FASTVOLRAMP;
                    }
                }
            }
        }
        s.lastMovedChannel.reset();
        if (!s.channels[aChannel].sample_playing())
            return;
        auto const nnaChannel = background_channel(aChannel);
        if (s.channels[aChannel].ins == ins)
            s.lastMovedChannel = nnaChannel;
        if (!nnaChannel)
            return;
        auto& from = s.channels[aChannel];
        auto& voice = s.channels[*nnaChannel];
        voice = from;
        voice.flags &= ~(CHN_VIBRATO | CHN_TREMOLO | CHN_PORTAMENTO);
        voice.panbrelloOffset = 0;
        voice.masterChannel = static_cast<std::uint8_t>(aChannel < iModule.channels ? aChannel + 1u : 0u);
        voice.activeCommand = effect::None;
        switch (from.nna)
        {
        case new_note_action::NoteOff:
            key_off(voice);
            break;
        case new_note_action::Cut:
            voice.fadeOutVolume = 0;
            voice.flags |= CHN_NOTEFADE;
            break;
        case new_note_action::NoteFade:
            voice.flags |= CHN_NOTEFADE;
            break;
        case new_note_action::Continue:
            break;
        }
        if (voice.volume == 0)
        {
            voice.fadeOutVolume = 0;
            voice.flags |= CHN_NOTEFADE | CHN_FASTVOLRAMP;
        }
        from.length = 0u;
        from.position = 0.0;
    }

    void replayer::key_off(channel_state& aChannel)
    {
        bool const keyIsOn = (aChannel.flags & CHN_KEYOFF) == 0u;
        aChannel.flags |= CHN_KEYOFF;
        if (aChannel.ins != nullptr && !aChannel.volumeEnvelope.enabled)
            aChannel.flags |= CHN_NOTEFADE;
        if (aChannel.length == 0u)
            return;
        if ((aChannel.flags & CHN_SUSTAINLOOP) != 0u && aChannel.smp != nullptr && keyIsOn)
        {
            // out of the sustain loop and into the normal loop, if there is one
            auto const& smp = *aChannel.smp;
            if (smp.loop != loop_type::None)
            {
                if (smp.loop == loop_type::PingPong)
                    aChannel.flags |= CHN_PINGPONGLOOP;
                else
                    aChannel.flags &= ~(CHN_PINGPONGLOOP | CHN_PINGPONGFLAG);
                aChannel.flags |= CHN_LOOP;
                aChannel.length = smp.length;
                aChannel.loopStart = smp.loopStart;
                aChannel.loopEnd = smp.loopEnd;
                if (aChannel.length > aChannel.loopEnd)
                    aChannel.length = aChannel.loopEnd;
                if (aChannel.position > aChannel.length && aChannel.loopEnd > aChannel.loopStart)
                {
                    auto const whole = static_cast<std::uint32_t>(aChannel.position);
                    aChannel.position = aChannel.loopStart + (whole - aChannel.loopStart) % (aChannel.loopEnd - aChannel.loopStart);
                }
            }
            else
            {
                aChannel.flags &= ~(CHN_LOOP | CHN_PINGPONGLOOP | CHN_PINGPONGFLAG);
                aChannel.length = smp.length;
            }
        }
        if (aChannel.ins != nullptr)
        {
            auto const& ins = *aChannel.ins;
            if ((ins.volumeEnvelope.loop || is_xm()) && ins.fadeOut != 0u)
                aChannel.flags |= CHN_NOTEFADE;
        }
    }

    void replayer::note_cut(std::uint32_t aChannel, std::uint32_t aTick, bool aCutSample)
    {
        auto& s = iState;
        auto& ch = s.channels[aChannel];
        auto tickCount = s.tick;
        // a note cut next to a note and a tone portamento is ignored, unless the row is delayed
        if (behaves(behaviour::ITNoteCutWithPorta) && is_note(ch.row.note) && ch.row.is_tone_portamento())
        {
            auto const rowLength = s.speed + s.frameDelay;
            if (s.tick < rowLength)
                return;
            if (s.patternDelay != 0u && s.tick >= rowLength)
                tickCount %= rowLength;
        }
        if (tickCount == aTick)
        {
            if (aCutSample)
            {
                if (behaves(behaviour::ITNoteCutWithPorta))
                    ch.period = 0;
                ch.increment = 0.0;
                ch.fadeOutVolume = 0;
                ch.flags |= CHN_NOTEFADE;
            }
            else
                ch.volume = 0;
            ch.flags |= CHN_FASTVOLRAMP;
        }
    }

    // ---------------------------------------------------------------------------------------------------
    // effect helpers

    void replayer::portamento_up(std::uint32_t aChannel, std::uint8_t aParameter, bool aFineAsRegular)
    {
        auto& ch = iState.channels[aChannel];
        if (aParameter != 0u && !behaves(behaviour::ITDoublePortamentoSlides))
        {
            // FastTracker 2 has separate memories for each portamento
            if (!behaves(behaviour::FT2PortaUpDownMemory))
                ch.oldPortaDown = aParameter;
            ch.oldPortaUp = aParameter;
        }
        else
            aParameter = ch.oldPortaUp;
        bool const combined = !aFineAsRegular && !(is_mod() || is_xm());
        if (combined && aParameter >= 0xE0u)
        {
            if ((aParameter & 0x0Fu) != 0u)
            {
                if ((aParameter & 0xF0u) == 0xF0u)
                {
                    fine_portamento_up(ch, aParameter & 0x0Fu);
                    return;
                }
                if ((aParameter & 0xF0u) == 0xE0u && !is_dbm())
                {
                    extra_fine_portamento_up(ch, aParameter & 0x0Fu);
                    return;
                }
            }
            if (!is_dbm())
                return;
        }
        if (!ch.firstTick || (iState.speed == 1u && behaves(behaviour::SlidesAtSpeed1)))
            do_frequency_slide(ch, ch.period, aParameter * 4);
    }

    void replayer::portamento_down(std::uint32_t aChannel, std::uint8_t aParameter, bool aFineAsRegular)
    {
        auto& ch = iState.channels[aChannel];
        if (aParameter != 0u && !behaves(behaviour::ITDoublePortamentoSlides))
        {
            if (!behaves(behaviour::FT2PortaUpDownMemory))
                ch.oldPortaUp = aParameter;
            ch.oldPortaDown = aParameter;
        }
        else
            aParameter = ch.oldPortaDown;
        bool const combined = !aFineAsRegular && !(is_mod() || is_xm());
        if (combined && aParameter >= 0xE0u)
        {
            if ((aParameter & 0x0Fu) != 0u)
            {
                if ((aParameter & 0xF0u) == 0xF0u)
                {
                    fine_portamento_down(ch, aParameter & 0x0Fu);
                    return;
                }
                if ((aParameter & 0xF0u) == 0xE0u && !is_dbm())
                {
                    extra_fine_portamento_down(ch, aParameter & 0x0Fu);
                    return;
                }
            }
            if (!is_dbm())
                return;
        }
        if (!ch.firstTick || (iState.speed == 1u && behaves(behaviour::SlidesAtSpeed1)))
            do_frequency_slide(ch, ch.period, aParameter * -4);
    }

    void replayer::fine_portamento_up(channel_state& aChannel, std::uint8_t aParameter)
    {
        if (is_xm())
        {
            // FastTracker 2: E1x, E2x, X1x and X2x memories are separate
            if (aParameter != 0u)
                aChannel.oldFinePortaUpDown = static_cast<std::uint8_t>((aChannel.oldFinePortaUpDown & 0x0Fu) | (aParameter << 4u));
            else
                aParameter = static_cast<std::uint8_t>(aChannel.oldFinePortaUpDown >> 4u);
        }
        if (aChannel.firstTick && aChannel.period != 0 && aParameter != 0u)
            do_frequency_slide(aChannel, aChannel.period, aParameter * 4);
    }

    void replayer::fine_portamento_down(channel_state& aChannel, std::uint8_t aParameter)
    {
        if (is_xm())
        {
            if (aParameter != 0u)
                aChannel.oldFinePortaUpDown = static_cast<std::uint8_t>(aParameter | (aChannel.oldFinePortaUpDown & 0xF0u));
            else
                aParameter = static_cast<std::uint8_t>(aChannel.oldFinePortaUpDown & 0x0Fu);
        }
        if (aChannel.firstTick && aChannel.period != 0 && aParameter != 0u)
        {
            do_frequency_slide(aChannel, aChannel.period, aParameter * -4);
            if (aChannel.period > 0xFFFF && !behaves(behaviour::PeriodsAreHertz) && (!iModule.linearSlides || is_xm()))
                aChannel.period = 0xFFFF;
        }
    }

    void replayer::extra_fine_portamento_up(channel_state& aChannel, std::uint8_t aParameter)
    {
        if (is_xm())
        {
            if (aParameter != 0u)
                aChannel.oldExtraFinePortaUpDown = static_cast<std::uint8_t>((aChannel.oldExtraFinePortaUpDown & 0x0Fu) | (aParameter << 4u));
            else
                aParameter = static_cast<std::uint8_t>(aChannel.oldExtraFinePortaUpDown >> 4u);
        }
        if (aChannel.firstTick && aChannel.period != 0 && aParameter != 0u)
            do_frequency_slide(aChannel, aChannel.period, aParameter);
    }

    void replayer::extra_fine_portamento_down(channel_state& aChannel, std::uint8_t aParameter)
    {
        if (is_xm())
        {
            if (aParameter != 0u)
                aChannel.oldExtraFinePortaUpDown = static_cast<std::uint8_t>(aParameter | (aChannel.oldExtraFinePortaUpDown & 0xF0u));
            else
                aParameter = static_cast<std::uint8_t>(aChannel.oldExtraFinePortaUpDown & 0x0Fu);
        }
        if (aChannel.firstTick && aChannel.period != 0 && aParameter != 0u)
        {
            do_frequency_slide(aChannel, aChannel.period, -static_cast<std::int32_t>(aParameter));
            if (aChannel.period > 0xFFFF && !behaves(behaviour::PeriodsAreHertz) && (!iModule.linearSlides || is_xm()))
                aChannel.period = 0xFFFF;
        }
    }

    bool replayer::tone_portamento_shares_memory() const
    {
        return !iModule.itCompatibleGxx && behaves(behaviour::ITPortaMemoryShare);
    }

    void replayer::init_tone_portamento(channel_state& aChannel, std::uint16_t aParameter)
    {
        // Impulse Tracker shares the memory with portamento up and down
        if (tone_portamento_shares_memory())
        {
            if (aParameter == 0u)
                aParameter = aChannel.oldPortaUp;
            aChannel.oldPortaUp = aChannel.oldPortaDown = static_cast<std::uint8_t>(aParameter);
        }
        if (aParameter != 0u)
            aChannel.portamentoSlide = aParameter;
    }

    std::pair<std::uint16_t, bool> replayer::volume_column_tone_portamento(cell const& aCell, std::uint32_t aStartTick) const
    {
        if (is_it() || is_dbm())
            return { IT_PORTAMENTO_VOLUME_COLUMN[aCell.volume & 0x0Fu], false };
        bool clearEffect = false;
        std::uint16_t volume = aCell.volume;
        if (aCell.command == effect::TonePortamento && is_xm())
        {
            // FastTracker 2: a volume column portamento doubles and replaces the effect column's
            clearEffect = true;
            volume = static_cast<std::uint16_t>(volume * 2u);
        }
        if (behaves(behaviour::FT2PortaDelay) && aStartTick != 0u)
            return { 0u, clearEffect };
        return { static_cast<std::uint16_t>(volume * 16u), clearEffect };
    }

    void replayer::tone_portamento(std::uint32_t aChannel, std::uint16_t aParameter)
    {
        auto& s = iState;
        auto& ch = s.channels[aChannel];
        ch.flags |= CHN_PORTAMENTO;
        if (!behaves(behaviour::ITDoublePortamentoSlides))
            init_tone_portamento(ch, aParameter);
        std::int32_t delta = ch.portamentoSlide;
        bool const doPorta = !ch.firstTick || is_dbm() || (s.speed == 1u && behaves(behaviour::SlidesAtSpeed1));
        delta *= 4;
        if (ch.period != 0 && ch.portamentoTarget != 0 && doPorta)
        {
            auto const actualDelta = periods_are_frequencies() ? delta : -delta;
            if (behaves(behaviour::ITDoublePortamentoSlides) && delta == 0 && ch.row.command == effect::TonePortamentoVolumeSlide)
            {
                // Lxx with no tone portamento set up always goes down
                if (ch.period > 1 && iModule.linearSlides)
                    --ch.period;
                if (ch.period < ch.portamentoTarget)
                    ch.period = ch.portamentoTarget;
            }
            else if (ch.period < ch.portamentoTarget || ch.portamentoTargetReached)
            {
                do_frequency_slide(ch, ch.period, actualDelta, true);
                if (ch.period > ch.portamentoTarget)
                    ch.period = ch.portamentoTarget;
            }
            else if (ch.period > ch.portamentoTarget)
            {
                do_frequency_slide(ch, ch.period, -actualDelta, true);
                if (ch.period < ch.portamentoTarget)
                    ch.period = ch.portamentoTarget;
                if (ch.period == ch.portamentoTarget && behaves(behaviour::FT2PortaResetDirection))
                    ch.portamentoTargetReached = true;
            }
        }
        // portamento with no note: once the target is reached it's forgotten
        if (ch.period == ch.portamentoTarget && (behaves(behaviour::ITPortaTargetReached) || is_mod()))
            ch.portamentoTarget = 0;
    }

    void replayer::vibrato(channel_state& aChannel, std::uint32_t aParameter)
    {
        if ((aParameter & 0x0Fu) != 0u)
            aChannel.vibratoDepth = static_cast<std::uint8_t>((aParameter & 0x0Fu) * 4u);
        if ((aParameter & 0xF0u) != 0u)
            aChannel.vibratoSpeed = static_cast<std::uint8_t>((aParameter >> 4u) & 0x0Fu);
        aChannel.flags |= CHN_VIBRATO;
    }

    void replayer::fine_vibrato(channel_state& aChannel, std::uint32_t aParameter)
    {
        if ((aParameter & 0x0Fu) != 0u)
            aChannel.vibratoDepth = static_cast<std::uint8_t>(aParameter & 0x0Fu);
        if ((aParameter & 0xF0u) != 0u)
            aChannel.vibratoSpeed = static_cast<std::uint8_t>((aParameter >> 4u) & 0x0Fu);
        aChannel.flags |= CHN_VIBRATO;
        // Scream Tracker 3 doesn't tell vibrato and fine vibrato apart in its memory
        if (behaves(behaviour::ST3VibratoMemory) && (aParameter & 0x0Fu) != 0u)
            aChannel.vibratoDepth = static_cast<std::uint8_t>(aChannel.vibratoDepth * 4u);
    }

    void replayer::panbrello(channel_state& aChannel, std::uint32_t aParameter)
    {
        if ((aParameter & 0x0Fu) != 0u)
            aChannel.panbrelloDepth = static_cast<std::uint8_t>(aParameter & 0x0Fu);
        if ((aParameter & 0xF0u) != 0u)
            aChannel.panbrelloSpeed = static_cast<std::uint8_t>((aParameter >> 4u) & 0x0Fu);
    }

    void replayer::panning(channel_state& aChannel, std::uint32_t aParameter, pan_bits aBits)
    {
        if (behaves(behaviour::MODIgnorePanning))
            return;
        if (aBits == pan_bits::Eight || behaves(behaviour::PanOverride))
            aChannel.flags &= ~CHN_SURROUND;
        if (aBits == pan_bits::Four)
            aChannel.pan = static_cast<std::int32_t>((aParameter * 256u + 8u) / 15u);
        else if (aBits == pan_bits::Six)
            aChannel.pan = static_cast<std::int32_t>(std::min(aParameter, 64u) * 4u);
        else if (!is_s3m())
            aChannel.pan = static_cast<std::int32_t>(aParameter);
        else
        {
            // Scream Tracker 3: 7-bit panning, and A4 for surround
            if (aParameter <= 0x80u)
                aChannel.pan = static_cast<std::int32_t>(aParameter << 1u);
            else if (aParameter == 0xA4u)
            {
                aChannel.flags |= CHN_SURROUND;
                aChannel.pan = 0x80;
            }
        }
        aChannel.flags |= CHN_FASTVOLRAMP;
        aChannel.restorePanOnNewNote = 0u;
        if (behaves(behaviour::PanOverride))
        {
            aChannel.panSwing = 0;
            aChannel.panbrelloOffset = 0;
        }
    }

    void replayer::volume_slide(channel_state& aChannel, std::uint8_t aParameter, bool aVolumeColumn)
    {
        if (!aVolumeColumn)
        {
            if (aParameter != 0u)
                aChannel.oldVolumeSlide = aParameter;
            else
                aParameter = aChannel.oldVolumeSlide;
        }
        if (is_mod() || is_xm())
        {
            // MOD and XM: the up nibble takes precedence
            if ((aParameter & 0xF0u) != 0u)
                aParameter &= 0xF0u;
            else
                aParameter &= 0x0Fu;
        }
        std::int32_t newVolume = aChannel.volume;
        if (!(is_mod() || is_xm()))
        {
            if ((aParameter & 0x0Fu) == 0x0Fu)
            {
                if ((aParameter & 0xF0u) != 0u)
                {
                    fine_volume_up(aChannel, static_cast<std::uint8_t>(aParameter >> 4u), false);
                    return;
                }
                else if (aChannel.firstTick && !iModule.fastVolumeSlides)
                    newVolume -= 0x0F * 4;
            }
            else if ((aParameter & 0xF0u) == 0xF0u)
            {
                if ((aParameter & 0x0Fu) != 0u)
                {
                    fine_volume_down(aChannel, static_cast<std::uint8_t>(aParameter & 0x0Fu), false);
                    return;
                }
                else if (aChannel.firstTick && !iModule.fastVolumeSlides)
                    newVolume += 0x0F * 4;
            }
        }
        if (!aChannel.firstTick || iModule.fastVolumeSlides || (iState.speed == 1u && is_dbm()))
        {
            // Impulse Tracker ignores slides with both nibbles set
            if ((aParameter & 0x0Fu) != 0u)
            {
                if (!is_it() || (aParameter & 0xF0u) == 0u)
                    newVolume -= static_cast<std::int32_t>((aParameter & 0x0Fu) * 4u);
            }
            else
                newVolume += static_cast<std::int32_t>((aParameter & 0xF0u) >> 2u);
            if (is_mod())
                aChannel.flags |= CHN_FASTVOLRAMP;
        }
        aChannel.volume = std::clamp(newVolume, 0, 256);
    }

    void replayer::panning_slide(channel_state& aChannel, std::uint8_t aParameter, bool aMemory)
    {
        auto const& s = iState;
        if (aMemory)
        {
            if (aParameter != 0u)
                aChannel.oldPanSlide = aParameter;
            else
                aParameter = aChannel.oldPanSlide;
        }
        if (is_xm())
        {
            if ((aParameter & 0xF0u) != 0u)
                aParameter &= 0xF0u;
            else
                aParameter &= 0x0Fu;
        }
        std::int32_t slide = 0;
        if (!is_xm())
        {
            if ((aParameter & 0x0Fu) == 0x0Fu && (aParameter & 0xF0u) != 0u)
            {
                if (s.firstTick)
                    slide = -static_cast<std::int32_t>((aParameter & 0xF0u) / 4u);
            }
            else if ((aParameter & 0xF0u) == 0xF0u && (aParameter & 0x0Fu) != 0u)
            {
                if (s.firstTick)
                    slide = static_cast<std::int32_t>((aParameter & 0x0Fu) * 4u);
            }
            else if (!s.firstTick)
            {
                if ((aParameter & 0x0Fu) != 0u)
                {
                    if (!is_it() || (aParameter & 0xF0u) == 0u)
                        slide = static_cast<std::int32_t>((aParameter & 0x0Fu) * 4u);
                }
                else
                    slide = -static_cast<std::int32_t>((aParameter & 0xF0u) / 4u);
            }
        }
        else if (!s.firstTick)
        {
            if ((aParameter & 0xF0u) != 0u)
                slide = static_cast<std::int32_t>((aParameter & 0xF0u) / 4u);
            else
                slide = -static_cast<std::int32_t>((aParameter & 0x0Fu) * 4u);
            // FastTracker 2's panning slide is as fine as Impulse Tracker's fine slide
            if (behaves(behaviour::FT2PanSlide))
                slide /= 4;
        }
        if (slide != 0)
        {
            aChannel.pan = std::clamp(aChannel.pan + slide, 0, 256);
            aChannel.restorePanOnNewNote = 0u;
        }
    }

    void replayer::fine_volume_up(channel_state& aChannel, std::uint8_t aParameter, bool aVolumeColumn)
    {
        if (is_xm())
        {
            if (aParameter != 0u)
                aChannel.oldFineVolumeUpDown = static_cast<std::uint8_t>((aParameter << 4u) | (aChannel.oldFineVolumeUpDown & 0x0Fu));
            else
                aParameter = static_cast<std::uint8_t>(aChannel.oldFineVolumeUpDown >> 4u);
        }
        else if (aVolumeColumn)
        {
            if (aParameter != 0u)
                aChannel.oldVolumeParameter = aParameter;
            else
                aParameter = aChannel.oldVolumeParameter;
        }
        else
        {
            if (aParameter != 0u)
                aChannel.oldFineVolumeUpDown = aParameter;
            else
                aParameter = aChannel.oldFineVolumeUpDown;
        }
        if (aChannel.firstTick)
        {
            aChannel.volume = std::min(aChannel.volume + aParameter * 4, 256);
            if (is_mod())
                aChannel.flags |= CHN_FASTVOLRAMP;
        }
    }

    void replayer::fine_volume_down(channel_state& aChannel, std::uint8_t aParameter, bool aVolumeColumn)
    {
        if (is_xm())
        {
            if (aParameter != 0u)
                aChannel.oldFineVolumeUpDown = static_cast<std::uint8_t>(aParameter | (aChannel.oldFineVolumeUpDown & 0xF0u));
            else
                aParameter = static_cast<std::uint8_t>(aChannel.oldFineVolumeUpDown & 0x0Fu);
        }
        else if (aVolumeColumn)
        {
            if (aParameter != 0u)
                aChannel.oldVolumeParameter = aParameter;
            else
                aParameter = aChannel.oldVolumeParameter;
        }
        else
        {
            if (aParameter != 0u)
                aChannel.oldFineVolumeUpDown = aParameter;
            else
                aParameter = aChannel.oldFineVolumeUpDown;
        }
        if (aChannel.firstTick)
        {
            aChannel.volume = std::max(aChannel.volume - aParameter * 4, 0);
            if (is_mod())
                aChannel.flags |= CHN_FASTVOLRAMP;
        }
    }

    void replayer::tremolo(channel_state& aChannel, std::uint32_t aParameter)
    {
        if ((aParameter & 0x0Fu) != 0u)
            aChannel.tremoloDepth = static_cast<std::uint8_t>((aParameter & 0x0Fu) << 2u);
        if ((aParameter & 0xF0u) != 0u)
            aChannel.tremoloSpeed = static_cast<std::uint8_t>((aParameter >> 4u) & 0x0Fu);
        aChannel.flags |= CHN_TREMOLO;
    }

    void replayer::channel_volume_slide(channel_state& aChannel, std::uint8_t aParameter)
    {
        auto const& s = iState;
        std::int32_t slide = 0;
        if (aParameter != 0u)
            aChannel.oldChannelVolumeSlide = aParameter;
        else
            aParameter = aChannel.oldChannelVolumeSlide;
        if ((aParameter & 0x0Fu) == 0x0Fu && (aParameter & 0xF0u) != 0u)
        {
            if (s.firstTick)
                slide = aParameter >> 4u;
        }
        else if ((aParameter & 0xF0u) == 0xF0u && (aParameter & 0x0Fu) != 0u)
        {
            if (s.firstTick)
                slide = -static_cast<std::int32_t>(aParameter & 0x0Fu);
        }
        else if (!s.firstTick)
        {
            if ((aParameter & 0x0Fu) != 0u)
            {
                if (!(is_it() || is_dbm()) || (aParameter & 0xF0u) == 0u)
                    slide = -static_cast<std::int32_t>(aParameter & 0x0Fu);
            }
            else
                slide = static_cast<std::int32_t>((aParameter & 0xF0u) >> 4u);
        }
        if (slide != 0)
            aChannel.channelVolume = std::clamp(aChannel.channelVolume + slide, 0, 64);
    }

    void replayer::extended_mod_commands(std::uint32_t aChannel, std::uint8_t aParameter)
    {
        auto& s = iState;
        auto& ch = s.channels[aChannel];
        auto const command = static_cast<std::uint8_t>(aParameter & 0xF0u);
        auto const parameter = static_cast<std::uint8_t>(aParameter & 0x0Fu);
        switch (command)
        {
        case 0x10u:
            if (parameter != 0u || is_xm())
                fine_portamento_up(ch, parameter);
            break;
        case 0x20u:
            if (parameter != 0u || is_xm())
                fine_portamento_down(ch, parameter);
            break;
        case 0x30u:
            if (parameter != 0u)
                ch.flags |= CHN_GLISSANDO;
            else
                ch.flags &= ~CHN_GLISSANDO;
            break;
        case 0x40u:
            ch.vibratoType = static_cast<std::uint8_t>(parameter & 0x07u);
            break;
        case 0x50u:
            if (!s.firstTick)
                break;
            if (is_mod())
            {
                ch.finetune = mod_to_xm_finetune(parameter);
                if (ch.period != 0 && is_note(ch.row.note))
                    ch.period = period_from_note(ch.note, ch.finetune, ch.c5speed);
            }
            else if (is_note(ch.row.note))
            {
                ch.finetune = mod_to_xm_finetune(static_cast<std::uint32_t>(parameter - 8) & 0x0Fu);
                ch.finetune = mod_to_xm_finetune(static_cast<std::uint8_t>(parameter - 8u));
                if (ch.period != 0)
                    ch.period = period_from_note(ch.note, ch.finetune, ch.c5speed);
            }
            break;
        case 0x60u:
            if (s.firstTick)
                pattern_loop(aChannel, parameter);
            break;
        case 0x70u:
            ch.tremoloType = static_cast<std::uint8_t>(parameter & 0x07u);
            break;
        case 0x80u:
            if (s.firstTick)
                panning(ch, parameter, pan_bits::Four);
            break;
        case 0x90u:
            retrigger_note(aChannel, parameter, 0);
            break;
        case 0xA0u:
            if (parameter != 0u || is_xm())
                fine_volume_up(ch, parameter, false);
            break;
        case 0xB0u:
            if (parameter != 0u || is_xm())
                fine_volume_down(ch, parameter, false);
            break;
        case 0xC0u:
            note_cut(aChannel, parameter, false);
            break;
        case 0xF0u:
            if (!is_mod())
                ch.activeMacro = parameter;
            break;
        default:
            break;
        }
    }

    void replayer::extended_s3m_commands(std::uint32_t aChannel, std::uint8_t aParameter)
    {
        auto& s = iState;
        auto& ch = s.channels[aChannel];
        auto const command = static_cast<std::uint8_t>(aParameter & 0xF0u);
        auto parameter = static_cast<std::uint8_t>(aParameter & 0x0Fu);
        switch (command)
        {
        case 0x10u:
            if (parameter != 0u)
                ch.flags |= CHN_GLISSANDO;
            else
                ch.flags &= ~CHN_GLISSANDO;
            break;
        case 0x20u:
            if (!s.firstTick)
                break;
            ch.c5speed = S3M_FINETUNES[parameter];
            ch.finetune = mod_to_xm_finetune(parameter);
            if (ch.period != 0)
                ch.period = period_from_note(ch.note, ch.finetune, ch.c5speed);
            break;
        case 0x30u:
            if (is_s3m())
                ch.vibratoType = static_cast<std::uint8_t>(parameter & 0x03u);
            else if (behaves(behaviour::ITVibratoTremoloPanbrello))
                ch.vibratoType = parameter < 0x04u ? parameter : 0u;
            else
                ch.vibratoType = static_cast<std::uint8_t>(parameter & 0x07u);
            break;
        case 0x40u:
            if (is_s3m())
                ch.tremoloType = static_cast<std::uint8_t>(parameter & 0x03u);
            else if (behaves(behaviour::ITVibratoTremoloPanbrello))
                ch.tremoloType = parameter < 0x04u ? parameter : 0u;
            else
                ch.tremoloType = static_cast<std::uint8_t>(parameter & 0x07u);
            break;
        case 0x50u:
            if (behaves(behaviour::ITVibratoTremoloPanbrello))
            {
                ch.panbrelloType = parameter < 0x04u ? parameter : 0u;
                ch.panbrelloPosition = 0u;
            }
            else
                ch.panbrelloType = static_cast<std::uint8_t>(parameter & 0x07u);
            break;
        case 0x60u:
            // a delay of this many ticks; they add up
            if (s.firstTick && s.tick == 0u)
                s.frameDelay += parameter;
            break;
        case 0x70u:
            if (!s.firstTick)
                break;
            switch (parameter)
            {
            case 0u:
            case 1u:
            case 2u:
                // past note cut, off or fade, for the channel's background voices
                for (std::uint32_t index = iModule.channels; index < s.channels.size(); ++index)
                {
                    auto& voice = s.channels[index];
                    if (voice.masterChannel != aChannel + 1u)
                        continue;
                    if (parameter == 1u)
                        key_off(voice);
                    else if (parameter == 2u)
                        voice.flags |= CHN_NOTEFADE;
                    else
                    {
                        voice.flags |= CHN_NOTEFADE;
                        voice.fadeOutVolume = 0;
                    }
                }
                break;
            case 3u: ch.nna = new_note_action::Cut; break;
            case 4u: ch.nna = new_note_action::Continue; break;
            case 5u: ch.nna = new_note_action::NoteOff; break;
            case 6u: ch.nna = new_note_action::NoteFade; break;
            case 7u: ch.volumeEnvelope.enabled = false; break;
            case 8u: ch.volumeEnvelope.enabled = true; break;
            case 9u: ch.panningEnvelope.enabled = false; break;
            case 0xAu: ch.panningEnvelope.enabled = true; break;
            case 0xBu: ch.pitchEnvelope.enabled = false; break;
            case 0xCu: ch.pitchEnvelope.enabled = true; break;
            case 0xDu:
            case 0xEu:
                if (iModule.format == module_format::MPTM)
                {
                    ch.pitchEnvelope.enabled = true;
                    ch.pitchEnvelope.filter = parameter != 0xDu;
                }
                break;
            default:
                break;
            }
            break;
        case 0x80u:
            if (s.firstTick)
                panning(ch, parameter, pan_bits::Four);
            break;
        case 0x90u:
            if (s.firstTick)
                extended_channel_effect(ch, parameter);
            break;
        case 0xA0u:
            if (s.firstTick)
            {
                ch.oldHighOffset = parameter;
                if (!behaves(behaviour::ITHighOffsetNoRetrig) && is_note(ch.row.note))
                {
                    auto const position = static_cast<std::uint32_t>(parameter) << 16u;
                    if (position < ch.length)
                        ch.position = position;
                }
            }
            break;
        case 0xB0u:
            if (s.firstTick)
                pattern_loop(aChannel, parameter);
            break;
        case 0xC0u:
            if (parameter == 0u)
            {
                // SC0 is SC1 in Impulse Tracker, and does nothing in Scream Tracker 3
                if (is_it())
                    parameter = 1u;
                else if (is_s3m())
                    return;
            }
            note_cut(aChannel, parameter, behaves(behaviour::ITSCxStopsSample) || is_s3m());
            break;
        case 0xF0u:
            if (!is_s3m())
                ch.activeMacro = parameter;
            break;
        default:
            break;
        }
    }

    void replayer::extended_channel_effect(channel_state& aChannel, std::uint32_t aParameter)
    {
        switch (aParameter & 0x0Fu)
        {
        case 0x00u:
            aChannel.flags &= ~CHN_SURROUND;
            break;
        case 0x01u:
            aChannel.flags |= CHN_SURROUND;
            aChannel.pan = 128;
            break;
        case 0x0Eu:
            aChannel.flags &= ~CHN_PINGPONGFLAG;
            break;
        case 0x0Fu:
            // play backwards, from the end if the sample has only just started
            if (aChannel.position == 0.0 && aChannel.length != 0u && (is_note(aChannel.row.note) || (aChannel.flags & CHN_LOOP) == 0u))
                aChannel.position = static_cast<double>(aChannel.length) - 1e-9;
            aChannel.flags |= CHN_PINGPONGFLAG;
            break;
        default:
            break;
        }
    }

    void replayer::process_sample_offset(std::uint32_t aChannel)
    {
        auto& ch = iState.channels[aChannel];
        auto const& m = ch.row;
        bool const percentage = (m.volumeCommand == volume_command::Offset && m.volume == 0u);
        std::uint32_t offset = static_cast<std::uint32_t>(m.parameter) << 8u;
        // FastTracker 2: 9xx without a note doesn't update the memory
        if (offset != 0u && (!behaves(behaviour::FT2OffsetMemoryRequiresNote) || is_note(m.note)))
            ch.oldOffset = offset;
        else if (m.volumeCommand != volume_command::Offset)
            offset = ch.oldOffset;
        std::uint32_t const high = percentage ? 0u : static_cast<std::uint32_t>(ch.oldHighOffset) << 16u;
        if (m.volumeCommand == volume_command::Offset)
        {
            if (m.volume == 0u)
                offset = static_cast<std::uint32_t>(muldivr(ch.length, offset, 256u << 8u));
            else if (m.volume <= 9u && ch.smp != nullptr)
                offset += cue_point(m.volume - 1u);
            ch.oldOffset = offset;
        }
        sample_offset(ch, offset + high);
    }

    void replayer::sample_offset(channel_state& aChannel, std::uint32_t aOffset)
    {
        aOffset = std::min(aOffset, MAX_SAMPLE_LENGTH);
        if (behaves(behaviour::ST3OffsetWithoutInstrument))
            aChannel.previousNoteOffset = 0u;
        aChannel.previousNoteOffset += aOffset;
        // Scream Tracker 3 (with the GUS) wraps an offset past a loop's end around the loop
        if (aOffset >= aChannel.loopEnd && is_s3m() && (aChannel.flags & CHN_LOOP) != 0u && aChannel.loopEnd > 0u && aChannel.loopEnd > aChannel.loopStart)
            aOffset = (aOffset - aChannel.loopStart) % (aChannel.loopEnd - aChannel.loopStart) + aChannel.loopStart;
        auto const note = (behaves(behaviour::ITOffsetWithInstrNumber) && aChannel.row.instrument != 0u) ? aChannel.newNote : aChannel.row.note;
        if (!is_note(note))
            return;
        if (aChannel.ins != nullptr)
        {
            auto const smp = aChannel.ins->keyboard[note - NOTE_MIN];
            if (smp == 0u || smp > sample_count())
                return;
        }
        if (iModule.protrackerMode)
        {
            // ProTracker adds the offset twice
            aChannel.position = aChannel.previousNoteOffset;
            aChannel.previousNoteOffset += aOffset;
        }
        else
            aChannel.position = aOffset;
        if (aChannel.position >= aChannel.length || ((aChannel.flags & CHN_LOOP) != 0u && aChannel.position >= aChannel.loopEnd))
        {
            // an offset past the end of the sample
            if (behaves(behaviour::FT2ST3OffsetOutOfRange))
            {
                aChannel.flags |= CHN_FASTVOLRAMP;
                aChannel.period = 0;
            }
            else if (!(is_xm() || is_mod()))
            {
                if (behaves(behaviour::ITOffset))
                    aChannel.position = iModule.itOldEffects ? aChannel.length : 0.0;
                else
                {
                    aChannel.position = aChannel.loopStart;
                    if (iModule.itOldEffects && aChannel.length > 4u)
                        aChannel.position = aChannel.length - 2u;
                }
            }
            else if (is_mod() && (aChannel.flags & CHN_LOOP) != 0u)
                aChannel.position = aChannel.loopStart;
        }
    }

    void replayer::retrigger_note(std::uint32_t aChannel, std::uint32_t aParameter, std::int32_t aOffset)
    {
        auto& s = iState;
        auto* ch = &s.channels[aChannel];
        std::uint32_t retriggerSpeed = aParameter & 0x0Fu;
        std::uint32_t retriggerCount = ch->retriggerCount;
        bool doRetrigger = false;
        if (behaves(behaviour::ITRetrigger))
        {
            if (s.tick == 0u && ch->row.note != NOTE_NONE)
                ch->retriggerCount = static_cast<std::uint8_t>(aParameter & 0x0Fu);
            else if (ch->retriggerCount == 0u || --ch->retriggerCount == 0u)
            {
                ch->retriggerCount = static_cast<std::uint8_t>(aParameter & 0x0Fu);
                doRetrigger = true;
            }
        }
        else if (behaves(behaviour::FT2Retrigger) && (aParameter & 0x100u) != 0u)
        {
            // FastTracker 2's Rxy
            if (s.firstTick)
            {
                if (ch->row.instrument > 0u && (is_note(ch->row.note) || ch->row.note == NOTE_NONE))
                    retriggerCount = 1u;
                if (ch->row.volumeCommand == volume_command::Volume && ch->row.volume != 0u)
                {
                    ch->retriggerCount = static_cast<std::uint8_t>(retriggerCount);
                    return;
                }
            }
            if (retriggerCount >= retriggerSpeed)
            {
                if (!s.firstTick || !is_note(ch->row.note))
                {
                    doRetrigger = true;
                    retriggerCount = 0u;
                }
            }
        }
        else
        {
            if (is_s3m() || is_it())
            {
                if (retriggerSpeed == 0u)
                    retriggerSpeed = 1u;
                if (retriggerCount != 0u && (retriggerCount % retriggerSpeed) == 0u)
                    doRetrigger = true;
                ++retriggerCount;
            }
            else if (is_mod())
            {
                // ProTracker's E9x
                auto const tick = s.tick % s.speed;
                if (tick == 0u && is_note(ch->row.note))
                    return;
                if (retriggerSpeed != 0u && (tick % retriggerSpeed) == 0u)
                    doRetrigger = true;
            }
            else
            {
                auto realSpeed = retriggerSpeed;
                // FastTracker 2: a retrigger with a volume command makes the first interval a tick longer
                if ((aParameter & 0x100u) != 0u && ch->row.volumeCommand == volume_command::Volume && (ch->row.parameter & 0xF0u) != 0u)
                    ++realSpeed;
                if (!s.firstTick || (aParameter & 0x100u) != 0u)
                {
                    if (realSpeed == 0u)
                        realSpeed = 1u;
                    if ((aParameter & 0x100u) == 0u && s.speed != 0u && (s.tick % realSpeed) == 0u)
                        doRetrigger = true;
                    ++retriggerCount;
                }
                else if (is_xm())
                    retriggerCount = 0u;
                if (retriggerCount >= realSpeed)
                {
                    if (s.tick != 0u || ((aParameter & 0x100u) != 0u && ch->row.note == NOTE_NONE))
                        doRetrigger = true;
                }
                if (behaves(behaviour::FT2Retrigger) && aParameter == 0u)
                    doRetrigger = (s.tick == 0u);
            }
        }
        // a sample that has stopped isn't retriggered
        if (ch->length == 0u && behaves(behaviour::ITShortSampleRetrig))
            return;
        if (behaves(behaviour::ST3RetrigAfterNoteCut) && ch->fadeOutVolume == 0)
            return;
        if (doRetrigger)
        {
            auto const dv = (aParameter >> 4u) & 0x0Fu;
            auto volume = ch->volume;
            if (dv != 0u)
            {
                if (!behaves(behaviour::FT2Retrigger) || ch->row.volumeCommand != volume_command::Volume)
                {
                    if (RETRIGGER_MULTIPLY[dv] != 0)
                        volume = (volume * RETRIGGER_MULTIPLY[dv]) / 16;
                    else
                        volume += RETRIGGER_ADD[dv] * 4;
                }
                volume = std::clamp(volume, 0, 256);
                ch->flags |= CHN_FASTVOLRAMP;
            }
            std::uint32_t const note = ch->newNote;
            auto const oldPeriod = ch->period;
            if (is_note(static_cast<std::uint8_t>(note)) && ch->length != 0u && !is_s3m())
            {
                check_nna(aChannel, 0u, note, true);
                ch = &s.channels[aChannel];
            }
            bool resetEnvelopes = false;
            if (is_xm())
            {
                if (ch->row.instrument != 0u && aParameter < 0x100u)
                {
                    instrument_change(aChannel, ch->row.instrument, false, false);
                    resetEnvelopes = true;
                }
                if (aParameter < 0x100u)
                    resetEnvelopes = true;
            }
            // ProTracker: a retrigger with a lone instrument number changes the sample at once
            if (behaves(behaviour::MODSampleSwap) && ch->row.instrument != 0u)
            {
                auto const oldFinetune = ch->finetune;
                instrument_change(aChannel, ch->row.instrument, false, false);
                ch->finetune = oldFinetune;
            }
            bool const fading = (ch->flags & CHN_NOTEFADE) != 0u;
            auto const oldPreviousNoteOffset = ch->previousNoteOffset;
            if (is_s3m())
                ch->previousNoteOffset = 0u;
            bool const itS3MStyle = behaves(behaviour::ITRetrigger) || (is_s3m() && ch->length != 0u);
            note_change(aChannel, note, itS3MStyle, resetEnvelopes);
            if (ch->row.instrument == 0u)
                ch->previousNoteOffset = oldPreviousNoteOffset;
            if (fading && is_xm())
                ch->flags |= CHN_NOTEFADE;
            ch->volume = volume;
            if (instrument_count() != 0u)
                ch->row.note = static_cast<std::uint8_t>(note);
            if (is_it() && ch->row.note == NOTE_NONE && oldPeriod != 0)
                ch->period = oldPeriod;
            if (!(is_s3m() || is_it()))
                retriggerCount = 0u;
            if (itS3MStyle)
                ch->position = 0.0;
            --aOffset;
            if (ch->smp != nullptr && aOffset >= 0 && aOffset <= 9)
            {
                std::uint32_t offset = 0u;
                if (aOffset == 0)
                    offset = ch->oldOffset;
                else
                    offset = ch->oldOffset = cue_point(static_cast<std::uint32_t>(aOffset - 1));
                sample_offset(*ch, offset);
            }
        }
        if (behaves(behaviour::FT2Retrigger) && (aParameter & 0x100u) != 0u)
            ++retriggerCount;
        if (!behaves(behaviour::ITRetrigger))
            ch->retriggerCount = static_cast<std::uint8_t>(retriggerCount);
    }

    void replayer::do_frequency_slide(channel_state& aChannel, std::int32_t& aPeriod, std::int32_t aAmount, bool aTonePortamento) const
    {
        if (aPeriod == 0 || aAmount == 0)
            return;
        auto const& t = tables();
        bool const hertz = behaves(behaviour::PeriodsAreHertz);
        if (iModule.linearSlides && !(is_xm() || is_mod()))
        {
            // Impulse Tracker's linear slides: a fine table or a coarse one, never both
            auto const oldPeriod = aPeriod;
            auto absolute = static_cast<std::uint32_t>(std::abs(aAmount));
            auto const up_table = [&](std::uint32_t aIndex) { return hertz ? t.linearSlideUp[aIndex] : t.linearSlideDown[aIndex]; };
            auto const down_table = [&](std::uint32_t aIndex) { return hertz ? t.linearSlideDown[aIndex] : t.linearSlideUp[aIndex]; };
            auto const fine_up_table = [&](std::uint32_t aIndex) { return hertz ? FINE_LINEAR_SLIDE_UP[aIndex] : FINE_LINEAR_SLIDE_DOWN[aIndex]; };
            auto const fine_down_table = [&](std::uint32_t aIndex) { return hertz ? FINE_LINEAR_SLIDE_DOWN[aIndex] : FINE_LINEAR_SLIDE_UP[aIndex]; };
            if (absolute < 16u)
            {
                if (aAmount > 0)
                    aPeriod = muldivr(aPeriod, fine_up_table(absolute), 65536);
                else
                    aPeriod = muldivr(aPeriod, fine_down_table(absolute), 65536);
            }
            else
            {
                absolute /= 4u;
                while (absolute > 0u)
                {
                    auto const n = std::min<std::uint32_t>(absolute, 255u);
                    if (aAmount > 0)
                        aPeriod = muldivr(aPeriod, up_table(n), 65536);
                    else
                        aPeriod = muldivr(aPeriod, down_table(n), 65536);
                    absolute -= n;
                }
            }
            if (aPeriod == oldPeriod)
            {
                bool const increase = hertz == (aAmount > 0);
                if (increase && aPeriod < 0x7FFFFFFF)
                    ++aPeriod;
                else if (!increase && aPeriod > 1)
                    --aPeriod;
            }
        }
        else if (!iModule.linearSlides && hertz)
        {
            // Impulse Tracker's Amiga slides, done in Hertz
            constexpr std::int64_t AMIGA = 1712ll * 8363ll;
            if (aAmount < 0)
                aPeriod = static_cast<std::int32_t>(std::min<std::int64_t>(AMIGA * aPeriod / (static_cast<std::int64_t>(aPeriod) * -aAmount + AMIGA), 0x7FFFFFFF));
            else
            {
                auto const divisor = AMIGA - static_cast<std::int64_t>(aPeriod) * aAmount;
                if (divisor <= 0)
                {
                    if (aTonePortamento)
                        aPeriod = 0x7FFFFFFF;
                    else
                    {
                        aPeriod = 0;
                        aChannel.fadeOutVolume = 0;
                        aChannel.flags |= CHN_NOTEFADE | CHN_FASTVOLRAMP;
                    }
                    return;
                }
                aPeriod = static_cast<std::int32_t>(std::min<std::int64_t>(AMIGA * aPeriod / divisor, 0x7FFFFFFF));
            }
        }
        else
            aPeriod -= aAmount;
        if (aPeriod < 1)
        {
            aPeriod = 1;
            if (is_s3m() && !aTonePortamento)
            {
                aChannel.fadeOutVolume = 0;
                aChannel.flags |= CHN_NOTEFADE | CHN_FASTVOLRAMP;
            }
        }
    }

    void replayer::set_speed(std::uint32_t aParameter)
    {
        if (aParameter > 0u)
            iState.speed = aParameter;
    }

    void replayer::set_tempo(std::uint32_t aParameter)
    {
        auto& s = iState;
        std::uint32_t const minimumParameter = (is_xm() || is_mod()) ? 1u : 32u;
        // the limits of OpenMPT's (extended) format specifications
        double const minimumTempo = is_s3m() ? 33.0 : 32.0;
        double maximumTempo = behaves(behaviour::TempoClamp) ? 255.0 : (iModule.format == module_format::IT ? 512.0 : 1000.0);
        if (aParameter >= minimumParameter && s.firstTick == !behaves(behaviour::MODTempoOnSecondTick))
            s.tempo = std::min<double>(aParameter, maximumTempo);
        else if (aParameter < minimumParameter && !s.firstTick)
        {
            // a tempo slide
            auto const difference = static_cast<double>(aParameter & 0x0Fu);
            if ((aParameter & 0xF0u) == 0x10u)
                s.tempo += difference;
            else
                s.tempo -= difference;
            s.tempo = std::clamp(s.tempo, minimumTempo, maximumTempo);
        }
    }

    void replayer::pattern_loop(std::uint32_t aChannel, std::uint8_t aParameter)
    {
        auto& s = iState;
        if (behaves(behaviour::ST3NoMutedChannels) && (s.channels[aChannel].flags & CHN_MUTE) != 0u)
            return;
        // Scream Tracker 3 has a single pattern loop for the whole pattern
        auto& ch = s.channels[is_s3m() ? 0u : aChannel];
        if (aParameter == 0u)
        {
            ch.patternLoop = s.row;
            return;
        }
        if (ch.patternLoopCount != 0u)
        {
            --ch.patternLoopCount;
            if (ch.patternLoopCount == 0u)
            {
                // the next loop without its own start begins after this one
                if (behaves(behaviour::ITPatternLoopTargetReset) || is_s3m())
                    ch.patternLoop = s.row + 1u;
                return;
            }
        }
        else
        {
            if (!behaves(behaviour::ITFT2PatternLoop) && !(is_mod() || is_s3m()))
                for (std::uint32_t index = 0u; index < iModule.channels; ++index)
                    if (&s.channels[index] != &ch && s.channels[index].patternLoopCount != 0u)
                        return;
            ch.patternLoopCount = aParameter;
        }
        s.nextPatternStartRow = ch.patternLoop;
        // FastTracker 2: E6x overrides the target of a pattern break to its left
        if (s.breakRow && behaves(behaviour::FT2PatternLoopWithJumps))
            s.breakRow = ch.patternLoop;
        s.patternLoopRow = ch.patternLoop;
        // Impulse Tracker: SBx takes precedence over a position jump to its left
        if (behaves(behaviour::ITPatternLoopWithJumps))
            s.positionJump.reset();
    }

    void replayer::global_volume_slide(std::uint8_t aParameter, std::uint32_t aChannel)
    {
        auto& s = iState;
        auto& ch = s.channels[aChannel];
        if (aParameter != 0u)
            ch.oldGlobalVolumeSlide = aParameter;
        else
            aParameter = ch.oldGlobalVolumeSlide;
        if (is_xm())
        {
            if ((aParameter & 0xF0u) != 0u)
                aParameter &= 0xF0u;
            else
                aParameter &= 0x0Fu;
        }
        std::int32_t slide = 0;
        if ((aParameter & 0x0Fu) == 0x0Fu && (aParameter & 0xF0u) != 0u)
        {
            if (s.firstTick)
                slide = (aParameter >> 4u) * 2;
        }
        else if ((aParameter & 0xF0u) == 0xF0u && (aParameter & 0x0Fu) != 0u)
        {
            if (s.firstTick)
                slide = -static_cast<std::int32_t>((aParameter & 0x0Fu) * 2u);
        }
        else if (!s.firstTick)
        {
            if ((aParameter & 0xF0u) != 0u)
            {
                if (!(is_it() || is_dbm()) || (aParameter & 0x0Fu) == 0u)
                    slide = static_cast<std::int32_t>((aParameter & 0xF0u) >> 4u) * 2;
            }
            else
                slide = -static_cast<std::int32_t>((aParameter & 0x0Fu) * 2u);
        }
        if (slide != 0)
        {
            if (!(is_it() || is_dbm()))
                slide *= 2;
            s.globalVolume = std::clamp(s.globalVolume + slide, 0, 256);
        }
    }

    std::optional<std::uint32_t> replayer::pattern_break(std::uint8_t aParameter) const
    {
        // Scream Tracker 3 ignores invalid pattern breaks
        if (aParameter >= 64u && is_s3m())
            return {};
        const_cast<replayer_state&>(iState).nextPatternStartRow = 0u;
        return aParameter;
    }

    void replayer::update_s3m_effect_memory(channel_state& aChannel, std::uint8_t aParameter) const
    {
        // Scream Tracker 3 shares one memory among most of its effects
        aChannel.oldVolumeSlide = aParameter;
        aChannel.oldPortaUp = aParameter;
        aChannel.oldPortaDown = aParameter;
        aChannel.tremorParameter = aParameter;
        aChannel.arpeggio = aParameter;
        aChannel.retriggerParameter = aParameter;
        aChannel.tremoloDepth = static_cast<std::uint8_t>((aParameter & 0x0Fu) << 2u);
        aChannel.tremoloSpeed = static_cast<std::uint8_t>((aParameter >> 4u) & 0x0Fu);
        aChannel.oldExtendedCommand = aParameter;
    }

    void replayer::process_midi_macro(std::uint32_t aChannel, bool aSmooth, std::string const& aMacro, std::uint8_t aParameter)
    {
        // only the macros for the internal device (the resonant filter) are played: F0 F0 nn xx
        auto& ch = iState.channels[aChannel];
        // (on the audio thread: no allocation; a macro is at most 32 characters)
        struct macro_bytes
        {
            std::array<std::uint8_t, 64u> data = {};
            std::size_t count = 0u;
            void push_back(std::uint8_t aByte) { if (count < data.size()) data[count++] = aByte; }
            std::size_t size() const { return count; }
            std::uint8_t operator[](std::size_t aIndex) const { return data[aIndex]; }
        } bytes;
        bool firstNibble = true;
        std::uint8_t current = 0u;
        for (auto c : aMacro)
        {
            bool nibble = false;
            std::uint8_t value = 0u;
            if (c >= '0' && c <= '9')
            {
                nibble = true;
                value = static_cast<std::uint8_t>(c - '0');
            }
            else if (c >= 'A' && c <= 'F')
            {
                nibble = true;
                value = static_cast<std::uint8_t>(c - 'A' + 10);
            }
            else if (c == 'z')
                value = aParameter & 0x7Fu;
            else if (c == 'x')
                value = static_cast<std::uint8_t>(std::min(ch.pan / 2, 127));
            else if (c == 'y')
                value = static_cast<std::uint8_t>(std::min(ch.realPan / 2, 127));
            else if (c == 'n')
                value = is_note(ch.lastNote) ? static_cast<std::uint8_t>(ch.lastNote - NOTE_MIN) : 0u;
            else if (c == 'c' || c == 'v' || c == 'u' || c == 'a' || c == 'b' || c == 'p' || c == 'h' || c == 'm' || c == 'o' || c == 's')
                value = 0u;
            else
                continue;
            if (nibble)
            {
                if (firstNibble)
                    current = value;
                else
                {
                    current = static_cast<std::uint8_t>((current << 4u) | value);
                    bytes.push_back(current);
                }
                firstNibble = !firstNibble;
            }
            else
            {
                if (!firstNibble)
                {
                    bytes.push_back(current);
                    firstNibble = true;
                }
                bytes.push_back(value);
            }
        }
        if (!firstNibble)
            bytes.push_back(current);
        for (std::size_t index = 0u; index + 3u < bytes.size(); ++index)
        {
            if (bytes[index] != 0xF0u || bytes[index + 1u] != 0xF0u)
                continue;
            auto const code = bytes[index + 2u];
            auto const value = bytes[index + 3u];
            auto const smooth = [&](std::uint8_t aCurrent) -> std::uint8_t
                {
                    if (!aSmooth)
                        return value;
                    auto const ticksLeft = ticks_on_row() > iState.tick ? ticks_on_row() - iState.tick : 1u;
                    if (ticksLeft <= 1u)
                        return value;
                    auto const step = (static_cast<float>(value) - aCurrent) / static_cast<float>(ticksLeft);
                    return static_cast<std::uint8_t>(std::clamp(std::lround(aCurrent + step), 0l, 127l));
                };
            if (code == 0x00u && value < 0x80u)
            {
                ch.cutoff = smooth(ch.cutoff);
                ch.restoreCutoffOnNewNote = 0u;
                setup_channel_filter(ch, (ch.flags & CHN_FILTER) == 0u);
            }
            else if (code == 0x01u && value < 0x80u)
            {
                ch.resonance = smooth(ch.resonance);
                ch.restoreResonanceOnNewNote = 0u;
                setup_channel_filter(ch, (ch.flags & CHN_FILTER) == 0u);
            }
            else if (code == 0x02u && value < 0x20u)
            {
                ch.filterMode = static_cast<std::uint8_t>(value >> 4u);
                setup_channel_filter(ch, (ch.flags & CHN_FILTER) == 0u);
            }
            index += 3u;
        }
    }

    // ---------------------------------------------------------------------------------------------------
    // periods and frequencies

    bool replayer::use_finetune_and_transpose() const
    {
        return is_mod() || is_xm();
    }

    bool replayer::periods_are_frequencies() const
    {
        return behaves(behaviour::PeriodsAreHertz) && !use_finetune_and_transpose();
    }

    std::int32_t replayer::period_from_note(std::uint32_t aNote, std::int32_t aFinetune, std::uint32_t aC5Speed) const
    {
        if (aNote == NOTE_NONE || aNote >= NOTE_MIN_SPECIAL)
            return 0;
        auto note = aNote - NOTE_MIN;
        auto const& t = tables();
        if (!use_finetune_and_transpose())
        {
            if (aC5Speed == 0u)
                aC5Speed = 8363u;
            if (periods_are_frequencies())
            {
                auto const frequency = static_cast<std::uint64_t>(aC5Speed) * (static_cast<std::uint64_t>(t.linearSlideUp[(note % 12u) * 16u]) << (note / 12u)) / (65536ull << 5u);
                return static_cast<std::int32_t>(std::min<std::uint64_t>(frequency, 0x7FFFFFFFull));
            }
            if (iModule.linearSlides)
                return static_cast<std::int32_t>((static_cast<std::uint32_t>(S3M_PERIODS[note % 12u]) << 5u) >> (note / 12u));
            auto const divisor = static_cast<std::uint64_t>(aC5Speed) << (note / 12u);
            return static_cast<std::int32_t>(8363ull * (static_cast<std::uint64_t>(S3M_PERIODS[note % 12u]) << 5u) / std::max<std::uint64_t>(divisor, 1u));
        }
        if (is_xm() || iModule.linearSlides)
        {
            if (note < 12u)
                note = 12u;
            note -= 12u;
            if (behaves(behaviour::FT2FinetunePrecision))
                aFinetune &= ~7;
            if (iModule.linearSlides)
                return std::max(1, static_cast<std::int32_t>((120 - static_cast<std::int32_t>(note)) << 6) - aFinetune / 2);
            // FastTracker 2's Amiga periods, interpolated between finetunes
            std::int32_t finetune = aFinetune;
            auto const rnote = static_cast<std::int32_t>((note % 12u) << 3u);
            auto const roct = note / 12u;
            std::int32_t rfine = finetune / 16;
            auto i = std::clamp(rnote + rfine + 8, 0, 103);
            std::int32_t per1 = XM_PERIODS[i];
            if (finetune < 0)
            {
                --rfine;
                finetune = -finetune;
            }
            else
                ++rfine;
            i = std::clamp(rnote + rfine + 8, 0, 103);
            std::int32_t per2 = XM_PERIODS[i];
            rfine = finetune & 0x0F;
            per1 *= 16 - rfine;
            per2 *= rfine;
            return ((per1 + per2) << 1) >> roct;
        }
        // ProTracker's periods
        auto const finetune = xm_to_mod_finetune(aFinetune);
        if (finetune != 0u || note < 24u || note >= 24u + 84u)
            return static_cast<std::int32_t>((static_cast<std::uint32_t>(PROTRACKER_TUNED_PERIODS[finetune * 12u + note % 12u]) << 5u) >> (note / 12u));
        return static_cast<std::int32_t>(PROTRACKER_PERIODS[note - 24u]) << 2;
    }

    std::uint32_t replayer::note_from_period(std::int32_t aPeriod, std::int32_t aFinetune, std::uint32_t aC5Speed) const
    {
        if (aPeriod == 0)
            return 0u;
        if (behaves(behaviour::FT2Periods))
            aFinetune += 64;
        std::uint32_t minimum = NOTE_MIN;
        std::uint32_t count = NOTE_MAX - NOTE_MIN + 1u;
        bool const frequencies = periods_are_frequencies();
        while (count > 0u)
        {
            auto const step = count / 2u;
            auto const middle = minimum + step;
            auto const n = period_from_note(middle, aFinetune, aC5Speed);
            if ((n > aPeriod && !frequencies) || (n < aPeriod && frequencies) || n == 0)
            {
                minimum = middle + 1u;
                count -= step + 1u;
            }
            else
                count = step;
        }
        return minimum;
    }

    // The frequency a period plays at, in sixteenths of a Hertz (as OpenMPT works it out, in whole numbers)
    std::uint32_t replayer::frequency_from_period(std::int32_t aPeriod, std::uint32_t aC5Speed, std::int32_t aPeriodFraction) const
    {
        if (aPeriod <= 0)
            return 0u;
        std::uint32_t period = static_cast<std::uint32_t>(aPeriod);
        if (is_xm() || (iModule.linearSlides && use_finetune_and_transpose()))
        {
            if (behaves(behaviour::FT2Periods))
                period &= 0xFFFFu;
            if (iModule.linearSlides)
            {
                std::uint32_t octave = 0u;
                if (behaves(behaviour::FT2Periods))
                {
                    // FastTracker 2's octaves wrap around (unsigned arithmetic, as it does)
                    std::uint32_t const divisor = (9216u + 767u - period) / 768u;
                    octave = (14u - divisor) & 0x1Fu;
                }
                else
                {
                    if (period > 29u * 768u)
                        return 0u;
                    octave = period / 768u + 2u;
                }
                return (tables().xmLinear[period % 768u] << (FREQUENCY_FRACTION_BITS + 2u)) >> octave;
            }
            if (period == 0u)
                period = 1u;
            return static_cast<std::uint32_t>((static_cast<std::uint64_t>(8363u * 1712u) << FREQUENCY_FRACTION_BITS) / period);
        }
        if (use_finetune_and_transpose())
            return static_cast<std::uint32_t>((static_cast<std::uint64_t>(3546895u * 4u) << FREQUENCY_FRACTION_BITS) / period);
        period = std::min(period, 0xFFFFFFFFu >> 8u);
        auto const fraction = static_cast<std::uint64_t>(static_cast<std::uint32_t>(aPeriodFraction));
        if (periods_are_frequencies())
            return static_cast<std::uint32_t>(((static_cast<std::uint64_t>(period) << 8u) + fraction) >> (8u - FREQUENCY_FRACTION_BITS));
        auto const divisor = (static_cast<std::uint64_t>(period) << 8u) + fraction;
        if (divisor == 0u)
            return 0u;
        if (aC5Speed == 0u)
            aC5Speed = 8363u;
        std::uint64_t const numerator = (static_cast<std::uint64_t>(1712u) << 8u) << FREQUENCY_FRACTION_BITS;
        if (iModule.linearSlides)
            return static_cast<std::uint32_t>(std::min<std::uint64_t>(static_cast<std::uint64_t>(aC5Speed) * numerator / divisor, 0xFFFFFFFFu));
        return static_cast<std::uint32_t>(std::min<std::uint64_t>(8363u * numerator / divisor, 0xFFFFFFFFu));
    }
}
