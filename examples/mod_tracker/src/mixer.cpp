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
Portions of this file (the per-tick channel processing and the mixer's loop handling (OpenMPT's
soundlib/Sndmix.cpp, Fastmix.cpp and Snd_flt.cpp)) are derived from OpenMPT (https://openmpt.org/).

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

// Each tick, after the effects: the channels' volumes, pans and pitches as the envelopes, vibratos and
// the rest make them, and then the mix itself.

#include <mod_tracker/replayer.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

#include "tables.hpp"

namespace mod_tracker
{
    using namespace detail;

    namespace
    {
        constexpr std::int8_t DBM_SINE[32] =
        {
            33, 52, 69, 84, 96, 107, 116, 122, 125, 127, 125, 122, 116, 107, 96, 84,
            69, 52, 33, 13, -8, -31, -54, -79, -104, -128, -104, -79, -54, -31, -8, 13
        };

        constexpr std::uint32_t MAXIMUM_RAMP_FRAMES = 2048u;
    }

    void replayer::render(float* aOutput, std::uint64_t aFrames, std::uint64_t aMuted)
    {
        auto& s = iState;
        while (aFrames != 0u && !s.ended)
        {
            if (s.tickFramesLeft == 0u)
            {
                advance_tick();
                continue;
            }
            auto const frames = std::min(aFrames, s.tickFramesLeft);
            for (std::uint32_t index = 0u; index < s.channels.size(); ++index)
            {
                auto& ch = s.channels[index];
                std::uint32_t const owner = index < iModule.channels ? index : (ch.masterChannel != 0u ? ch.masterChannel - 1u : iModule.channels);
                bool const muted = owner < 64u && (aMuted & (std::uint64_t{ 1u } << owner)) != 0u;
                mix_channel(ch, aOutput, frames, aOutput != nullptr && !muted);
                if (ch.peak != 0.0f)
                {
                    if (owner < iPeaks.size())
                        iPeaks[owner] = std::max(iPeaks[owner], ch.peak);
                    ch.peak = 0.0f;
                }
            }
            if (aOutput != nullptr)
            {
                for (std::uint64_t sample = 0u; sample < frames * 2u; ++sample)
                    aOutput[sample] = std::clamp(aOutput[sample], -1.0f, 1.0f);
                aOutput += frames * 2u;
            }
            s.tickFramesLeft -= frames;
            iFrames += frames;
            aFrames -= frames;
        }
    }

    // ---------------------------------------------------------------------------------------------------
    // per-tick channel processing

    std::int32_t replayer::vibrato_delta(std::uint32_t aType, std::uint32_t aPosition)
    {
        if (behaves(behaviour::ITVibratoTremoloPanbrello))
        {
            aPosition &= 0xFFu;
            switch (aType & 0x03u)
            {
            case 0u:
            default:
                return IT_SINE[aPosition];
            case 1u:
                return 64 - static_cast<std::int32_t>(aPosition + 1u) / 2;
            case 2u:
                return aPosition < 128u ? 64 : 0;
            case 3u:
                return (random_byte() & 0x7F) - 0x40;
            }
        }
        if (is_dbm())
            return DBM_SINE[(aPosition / 2u) & 0x1Fu];
        aPosition &= 0x3Fu;
        switch (aType & 0x03u)
        {
        case 0u:
        default:
            return MOD_SINE[aPosition];
        case 1u:
            return (aPosition < 32u ? 0 : 255) - static_cast<std::int32_t>(aPosition) * 4;
        case 2u:
            return aPosition < 32u ? 127 : -127;
        case 3u:
            return MOD_RANDOM[aPosition];
        }
    }

    void replayer::process_tremolo(channel_state& aChannel, std::int32_t& aVolume)
    {
        if ((aChannel.flags & CHN_TREMOLO) == 0u)
            return;
        auto const& s = iState;
        // ProTracker doesn't apply tremolo on the first tick
        if (iModule.protrackerMode && s.firstTick)
            return;
        if (aVolume > 0 || behaves(behaviour::ITVibratoTremoloPanbrello))
        {
            std::uint32_t const attenuation = (is_xm() || is_mod() || behaves(behaviour::ITVibratoTremoloPanbrello)) ? 5u : 6u;
            auto delta = vibrato_delta(aChannel.tremoloType, aChannel.tremoloPosition);
            if ((aChannel.tremoloType & 0x03u) == 1u && behaves(behaviour::FT2MODTremoloRampWaveform))
            {
                // FastTracker 2's ramp down tremolo depends on the vibrato position (a copy-paste bug)
                std::uint8_t ramp = static_cast<std::uint8_t>((aChannel.tremoloPosition * 4u) & 0x7Fu);
                std::uint32_t vibratoPosition = aChannel.vibratoPosition;
                if (!s.firstTick && (aChannel.flags & CHN_VIBRATO) != 0u)
                    vibratoPosition += aChannel.vibratoSpeed;
                if ((vibratoPosition & 0x3Fu) >= 32u)
                    ramp ^= 0x7Fu;
                delta = (aChannel.tremoloPosition & 0x3Fu) >= 32u ? -static_cast<std::int32_t>(ramp) : ramp;
            }
            aVolume += (delta * aChannel.tremoloDepth) / (1 << attenuation);
        }
        if (!s.firstTick || (is_it() && !iModule.itOldEffects))
        {
            if (behaves(behaviour::ITVibratoTremoloPanbrello))
                aChannel.tremoloPosition = static_cast<std::uint8_t>(aChannel.tremoloPosition + 4u * aChannel.tremoloSpeed);
            else
                aChannel.tremoloPosition = static_cast<std::uint8_t>(aChannel.tremoloPosition + aChannel.tremoloSpeed);
        }
    }

    void replayer::process_tremor(channel_state& aChannel, std::int32_t& aVolume)
    {
        auto const& s = iState;
        if (behaves(behaviour::FT2Tremor))
        {
            // FastTracker 2's tremor
            if ((aChannel.tremorCount & 0x80u) != 0u)
            {
                if (!s.firstTick && aChannel.activeCommand == effect::Tremor)
                {
                    aChannel.tremorCount &= ~0x20u;
                    if (aChannel.tremorCount == 0x80u)
                        aChannel.tremorCount = static_cast<std::uint8_t>((aChannel.tremorParameter >> 4u) | 0xC0u);
                    else if (aChannel.tremorCount == 0xC0u)
                        aChannel.tremorCount = static_cast<std::uint8_t>((aChannel.tremorParameter & 0x0Fu) | 0x80u);
                    else
                        --aChannel.tremorCount;
                    aChannel.flags |= CHN_FASTVOLRAMP;
                }
                if ((aChannel.tremorCount & 0xE0u) == 0x80u)
                    aVolume = 0;
            }
        }
        else if (aChannel.activeCommand == effect::Tremor)
        {
            if (behaves(behaviour::ITTremor))
            {
                if ((aChannel.tremorCount & 0x80u) != 0u && aChannel.length != 0u)
                {
                    if (aChannel.tremorCount == 0x80u)
                        aChannel.tremorCount = static_cast<std::uint8_t>((aChannel.tremorParameter >> 4u) | 0xC0u);
                    else if (aChannel.tremorCount == 0xC0u)
                        aChannel.tremorCount = static_cast<std::uint8_t>((aChannel.tremorParameter & 0x0Fu) | 0x80u);
                    else
                        --aChannel.tremorCount;
                }
                if ((aChannel.tremorCount & 0xC0u) == 0x80u)
                    aVolume = 0;
            }
            else
            {
                std::uint8_t onTime = static_cast<std::uint8_t>(aChannel.tremorParameter >> 4u);
                std::uint8_t cycle = static_cast<std::uint8_t>(onTime + (aChannel.tremorParameter & 0x0Fu));
                if (!is_it() || iModule.itOldEffects)
                {
                    cycle = static_cast<std::uint8_t>(cycle + 2u);
                    ++onTime;
                }
                std::uint8_t count = aChannel.tremorCount;
                if (!is_xm())
                {
                    if (count >= cycle)
                        count = 0u;
                    if (count >= onTime)
                        aVolume = 0;
                    aChannel.tremorCount = static_cast<std::uint8_t>(count + 1u);
                }
                else
                {
                    if (s.firstTick)
                    {
                        if (count > 0u)
                            --count;
                    }
                    else
                        aChannel.tremorCount = static_cast<std::uint8_t>(count + 1u);
                    if (cycle != 0u && count % cycle >= onTime)
                        aVolume = 0;
                }
            }
            aChannel.flags |= CHN_FASTVOLRAMP;
        }
    }

    bool replayer::envelope_processed(channel_state const& aChannel, envelope const& aEnvelope, envelope_state const& aState) const
    {
        if (aChannel.ins == nullptr)
            return false;
        // S77 and the like pause the envelope rather than disabling it
        bool const playIfPaused = behaves(behaviour::ITEnvelopePositionHandling) || behaves(behaviour::FT2PanSustainRelease);
        return (aState.enabled || (aEnvelope.enabled && playIfPaused)) && !aEnvelope.points.empty();
    }

    void replayer::process_volume_envelope(channel_state& aChannel, std::int32_t& aVolume) const
    {
        auto const& envelope = aChannel.ins->volumeEnvelope;
        if (!envelope_processed(aChannel, envelope, aChannel.volumeEnvelope))
            return;
        bool const itPositions = behaves(behaviour::ITEnvelopePositionHandling);
        if (itPositions && aChannel.volumeEnvelope.position == 0u)
            return;
        auto const position = static_cast<std::int32_t>(aChannel.volumeEnvelope.position) - (itPositions ? 1 : 0);
        auto const value = envelope.value_at(position, 256);
        aVolume = (aVolume * std::clamp(value, 0, 512)) / 256;
    }

    void replayer::process_panning_envelope(channel_state& aChannel) const
    {
        auto const& envelope = aChannel.ins->panningEnvelope;
        if (!envelope_processed(aChannel, envelope, aChannel.panningEnvelope))
            return;
        bool const itPositions = behaves(behaviour::ITEnvelopePositionHandling);
        if (itPositions && aChannel.panningEnvelope.position == 0u)
            return;
        auto const position = static_cast<std::int32_t>(aChannel.panningEnvelope.position) - (itPositions ? 1 : 0);
        auto const value = envelope.value_at(position, 64) - 32;
        std::int32_t pan = aChannel.realPan;
        if (pan >= 128)
            pan += (value * (256 - pan)) / 32;
        else
            pan += (value * pan) / 32;
        aChannel.realPan = std::clamp(pan, 0, 256);
    }

    std::int32_t replayer::process_pitch_filter_envelope(channel_state& aChannel, std::int32_t& aPeriod) const
    {
        if (aChannel.ins == nullptr)
            return -1;
        auto const& envelope = aChannel.ins->pitchEnvelope;
        if (!envelope_processed(aChannel, envelope, aChannel.pitchEnvelope))
            return -1;
        bool const itPositions = behaves(behaviour::ITEnvelopePositionHandling);
        if (itPositions && aChannel.pitchEnvelope.position == 0u)
        {
            // Impulse Tracker still applies a stopped filter envelope as if it were at its midpoint
            if (behaves(behaviour::ITStoppedFilterEnvAtStart) && aChannel.pitchEnvelope.filter)
                return setup_channel_filter(aChannel, (aChannel.flags & CHN_FILTER) == 0u, 0);
            return -1;
        }
        auto const position = static_cast<std::int32_t>(aChannel.pitchEnvelope.position) - (itPositions ? 1 : 0);
        auto const value = envelope.value_at(position, 512) - 256;
        if (aChannel.pitchEnvelope.filter)
            return setup_channel_filter(aChannel, (aChannel.flags & CHN_FILTER) == 0u, value);
        auto const& t = tables();
        bool const frequencies = periods_are_frequencies();
        if (value < 0)
        {
            auto const index = static_cast<std::uint32_t>(std::min(-value, 255));
            aPeriod = muldiv(aPeriod, frequencies ? t.linearSlideDown[index] : t.linearSlideUp[index], 65536);
        }
        else
        {
            auto const index = static_cast<std::uint32_t>(std::min(value, 255));
            aPeriod = muldiv(aPeriod, frequencies ? t.linearSlideUp[index] : t.linearSlideDown[index], 65536);
        }
        return -1;
    }

    void replayer::increment_envelope_position(channel_state& aChannel, envelope const& aEnvelope, envelope_state& aState, bool aVolume, bool aPanning) const
    {
        if (aChannel.ins == nullptr || !aState.enabled || aEnvelope.points.empty())
            return;
        bool const itPositions = behaves(behaviour::ITEnvelopePositionHandling);
        std::uint32_t position = aState.position + (itPositions ? 0u : 1u);
        bool endReached = false;
        auto const tick_of = [&](std::uint32_t aPoint) -> std::uint32_t
            {
                return aEnvelope.points[std::min<std::size_t>(aPoint, aEnvelope.points.size() - 1u)].tick;
            };
        if (!itPositions)
        {
            // FastTracker 2's envelopes
            if (aEnvelope.loop)
            {
                std::uint32_t end = tick_of(aEnvelope.loopEnd);
                if (!is_xm())
                    ++end;
                bool const escapeLoop = aEnvelope.loopEnd == aEnvelope.sustainEnd && aEnvelope.sustain &&
                    (aChannel.flags & CHN_KEYOFF) != 0u && behaves(behaviour::FT2EnvelopeEscape);
                if (position == end && !escapeLoop)
                    position = tick_of(aEnvelope.loopStart);
            }
            if (aEnvelope.sustain && (aChannel.flags & CHN_KEYOFF) == 0u)
            {
                if (position == tick_of(aEnvelope.sustainEnd) + 1u)
                {
                    position = tick_of(aEnvelope.sustainStart);
                    // FastTracker 2: a panning envelope reaching its sustain point before key off stays there
                    if (behaves(behaviour::FT2PanSustainRelease) && aPanning && (aChannel.flags & CHN_KEYOFF) == 0u)
                        aState.enabled = false;
                }
            }
            else if (position > aEnvelope.points.back().tick)
            {
                position = aEnvelope.points.back().tick;
                endReached = true;
            }
        }
        else
        {
            // Impulse Tracker's envelopes
            std::uint32_t start = 0u;
            std::uint32_t end = 0u;
            if (aEnvelope.sustain && (aChannel.oldFlags & CHN_KEYOFF) == 0u)
            {
                start = tick_of(aEnvelope.sustainStart);
                end = tick_of(aEnvelope.sustainEnd) + 1u;
            }
            else if (aEnvelope.loop)
            {
                start = tick_of(aEnvelope.loopStart);
                end = tick_of(aEnvelope.loopEnd) + 1u;
            }
            else
            {
                start = end = aEnvelope.points.back().tick;
                if (position > end)
                    endReached = true;
            }
            if (position >= end)
                position = start;
        }
        if (aVolume && endReached)
        {
            if (is_it() || (aChannel.flags & CHN_KEYOFF) != 0u)
                aChannel.flags |= CHN_NOTEFADE;
            // a silent end stops the voice
            if (aEnvelope.points.back().value == 0u && (aChannel.masterChannel > 0u || is_it()))
            {
                aChannel.flags |= CHN_NOTEFADE;
                aChannel.fadeOutVolume = 0;
                aChannel.realVolume = 0;
                aChannel.calculatedVolume = 0;
            }
        }
        aState.position = position + (itPositions ? 1u : 0u);
    }

    void replayer::increment_envelope_positions(channel_state& aChannel) const
    {
        auto const& ins = *aChannel.ins;
        increment_envelope_position(aChannel, ins.volumeEnvelope, aChannel.volumeEnvelope, true, false);
        increment_envelope_position(aChannel, ins.panningEnvelope, aChannel.panningEnvelope, false, true);
        increment_envelope_position(aChannel, ins.pitchEnvelope, aChannel.pitchEnvelope, false, false);
    }

    void replayer::process_instrument_fade(channel_state& aChannel, std::int32_t& aVolume) const
    {
        if ((aChannel.flags & CHN_NOTEFADE) == 0u || aChannel.ins == nullptr)
            return;
        auto const fadeOut = aChannel.ins->fadeOut;
        if (fadeOut != 0u)
        {
            aChannel.fadeOutVolume -= static_cast<std::int32_t>(fadeOut * 2u);
            if (aChannel.fadeOutVolume <= 0)
                aChannel.fadeOutVolume = 0;
            aVolume = static_cast<std::int32_t>((static_cast<std::int64_t>(aVolume) * aChannel.fadeOutVolume) / 65536);
        }
        else if (aChannel.fadeOutVolume == 0)
            aVolume = 0;
    }

    void replayer::process_panbrello(channel_state& aChannel)
    {
        std::int32_t delta = aChannel.panbrelloOffset;
        if (aChannel.row.command == effect::Panbrello)
        {
            std::uint32_t position = 0u;
            if (behaves(behaviour::ITVibratoTremoloPanbrello))
                position = aChannel.panbrelloPosition;
            else
                position = (aChannel.panbrelloPosition + 0x10u) >> 2u;
            delta = vibrato_delta(aChannel.panbrelloType, position);
            // Impulse Tracker's random panbrello is sample-and-hold
            if (behaves(behaviour::ITSampleAndHoldPanbrello) && aChannel.panbrelloType == 3u)
            {
                if (aChannel.panbrelloPosition == 0u || aChannel.panbrelloPosition >= aChannel.panbrelloSpeed)
                {
                    aChannel.panbrelloPosition = 0u;
                    aChannel.panbrelloRandomMemory = static_cast<std::int8_t>(delta);
                }
                ++aChannel.panbrelloPosition;
                delta = aChannel.panbrelloRandomMemory;
            }
            else
                aChannel.panbrelloPosition = static_cast<std::uint8_t>(aChannel.panbrelloPosition + aChannel.panbrelloSpeed);
            if (behaves(behaviour::ITPanbrelloHold))
                aChannel.panbrelloOffset = static_cast<std::int8_t>(delta);
        }
        if (delta != 0)
        {
            delta = ((delta * static_cast<std::int32_t>(aChannel.panbrelloDepth)) + 2) / 8;
            aChannel.realPan = std::clamp(aChannel.realPan + delta, 0, 256);
        }
    }

    void replayer::process_arpeggio(channel_state& aChannel, std::int32_t& aPeriod)
    {
        auto const& s = iState;
        if (aChannel.activeCommand != effect::Arpeggio)
            return;
        if (behaves(behaviour::ITArpeggio))
        {
            // a row delay starts the arpeggio again
            auto const tick = s.tick % (s.speed + s.frameDelay);
            if (aChannel.arpeggio != 0u)
            {
                std::uint32_t ratio = 65536u;
                auto const& t = tables();
                switch (tick % 3u)
                {
                case 1u: ratio = t.linearSlideUp[(aChannel.arpeggio >> 4u) * 16u]; break;
                case 2u: ratio = t.linearSlideUp[(aChannel.arpeggio & 0x0Fu) * 16u]; break;
                default: break;
                }
                if (periods_are_frequencies())
                    aPeriod = muldivr(aPeriod, ratio, 65536);
                else
                    aPeriod = muldivr(aPeriod, 65536, ratio);
            }
        }
        else if (behaves(behaviour::FT2Arpeggio))
        {
            if (!s.firstTick)
            {
                // FastTracker 2's arpeggio table only has 16 entries; beyond that it reads the vibrato table
                std::int32_t position = static_cast<std::int32_t>(s.speed) - static_cast<std::int32_t>(s.tick % s.speed);
                if (position > 16)
                    position = 2;
                else if (position == 16)
                    position = 0;
                else
                    position %= 3;
                std::uint32_t note = 0u;
                switch (position)
                {
                case 1: note = aChannel.arpeggio >> 4u; break;
                case 2: note = aChannel.arpeggio & 0x0Fu; break;
                default: break;
                }
                if (position != 0)
                {
                    note += note_from_period(aPeriod, aChannel.finetune, aChannel.c5speed);
                    aPeriod = period_from_note(note, aChannel.finetune, aChannel.c5speed);
                    // FastTracker 2 has a different note limit for arpeggio
                    if (note >= 108u + NOTE_MIN)
                        aPeriod = std::max(aPeriod, period_from_note(108u + NOTE_MIN, 0, aChannel.c5speed));
                }
            }
        }
        else
        {
            std::uint32_t tick = s.tick;
            std::uint8_t note = !is_mod() ? aChannel.note : static_cast<std::uint8_t>(note_from_period(aPeriod, aChannel.finetune, aChannel.c5speed));
            if (is_dbm())
                tick += 2u;
            switch (tick % 3u)
            {
            case 1u: note = static_cast<std::uint8_t>(note + (aChannel.arpeggio >> 4u)); break;
            case 2u: note = static_cast<std::uint8_t>(note + (aChannel.arpeggio & 0x0Fu)); break;
            default: break;
            }
            if (note != aChannel.note || is_dbm() || behaves(behaviour::ST3PortaAfterArpeggio))
            {
                if (iModule.protrackerMode)
                {
                    // ProTracker's arpeggio wraps around
                    if (note == NOTE_MIDDLE_C + 24u)
                    {
                        aPeriod = 65536;
                        return;
                    }
                    else if (note > NOTE_MIDDLE_C + 24u)
                        note = static_cast<std::uint8_t>(note - 37u);
                }
                aPeriod = period_from_note(note, aChannel.finetune, aChannel.c5speed);
                if (is_dbm())
                    aChannel.period = aPeriod;
                else if (behaves(behaviour::ST3PortaAfterArpeggio))
                    aChannel.arpeggioLastNote = note;
            }
        }
    }

    void replayer::process_vibrato(channel_state& aChannel, std::int32_t& aPeriod)
    {
        if ((aChannel.flags & CHN_VIBRATO) == 0u)
            return;
        auto const& s = iState;
        bool const advance = !s.firstTick || (is_it() && !iModule.itOldEffects);
        if (advance && behaves(behaviour::ITVibratoTremoloPanbrello))
            aChannel.vibratoPosition = static_cast<std::uint8_t>(aChannel.vibratoPosition + 4u * aChannel.vibratoSpeed);
        auto delta = vibrato_delta(aChannel.vibratoType, aChannel.vibratoPosition);
        // ProTracker doesn't apply vibrato on the first tick
        if ((iModule.protrackerMode || is_dbm()) && s.firstTick)
            return;
        if ((is_xm() || is_mod()) && (aChannel.vibratoType & 0x03u) == 1u)
            delta = -delta;     // FastTracker 2's ramp down table is upside down
        std::uint32_t depth = 0u;
        if (behaves(behaviour::ITVibratoTremoloPanbrello))
        {
            // vibrato goes backwards with old effects
            if (iModule.itOldEffects)
                depth = 5u;
            else
            {
                depth = 6u;
                delta = -delta;
            }
        }
        else
        {
            if (iModule.s3mOldVibrato)
                depth = 5u;
            else if (is_dbm())
                depth = 7u;
            else if (is_it() && !iModule.itOldEffects)
                depth = 7u;
            else
                depth = 6u;
            if (behaves(behaviour::ST3VibratoMemory) && aChannel.row.command == effect::FineVibrato)
                depth += 2u;
        }
        delta = (-delta * static_cast<std::int32_t>(aChannel.vibratoDepth)) / (1 << depth);
        do_frequency_slide(aChannel, aPeriod, delta);
        if (advance && !behaves(behaviour::ITVibratoTremoloPanbrello))
            aChannel.vibratoPosition = static_cast<std::uint8_t>(aChannel.vibratoPosition + aChannel.vibratoSpeed);
    }

    void replayer::process_auto_vibrato(channel_state& aChannel, std::int32_t& aPeriod, std::int32_t& aPeriodFraction)
    {
        if (aChannel.smp == nullptr || aChannel.smp->vibratoDepth == 0u)
            return;
        auto const& smp = *aChannel.smp;
        auto const& t = tables();
        bool const frequencies = periods_are_frequencies();
        auto const up_table = [&](std::uint32_t aIndex) -> std::int64_t { aIndex = std::min(aIndex, 255u); return frequencies ? t.linearSlideUp[aIndex] : t.linearSlideDown[aIndex]; };
        auto const down_table = [&](std::uint32_t aIndex) -> std::int64_t { aIndex = std::min(aIndex, 255u); return frequencies ? t.linearSlideDown[aIndex] : t.linearSlideUp[aIndex]; };
        auto const fine_up_table = [&](std::uint32_t aIndex) -> std::int64_t { return frequencies ? FINE_LINEAR_SLIDE_UP[aIndex] : FINE_LINEAR_SLIDE_DOWN[aIndex]; };
        auto const fine_down_table = [&](std::uint32_t aIndex) -> std::int64_t { return frequencies ? FINE_LINEAR_SLIDE_DOWN[aIndex] : FINE_LINEAR_SLIDE_UP[aIndex]; };
        if (behaves(behaviour::ITVibratoTremoloPanbrello))
        {
            // Impulse Tracker's auto-vibrato
            if (smp.vibratoRate == 0u)
                return;
            auto const position = aChannel.autoVibratoPosition & 0xFFu;
            std::int32_t depth = aChannel.autoVibratoDepth + smp.vibratoSweep;
            depth = std::min(depth, static_cast<std::int32_t>(smp.vibratoDepth) * 256);
            aChannel.autoVibratoDepth = depth;
            depth /= 256;
            aChannel.autoVibratoPosition += smp.vibratoRate;
            std::int32_t delta = 0;
            switch (smp.vibratoType)
            {
            case vibrato_type::Random:
                delta = (random_byte() & 0x7F) - 0x40;
                break;
            case vibrato_type::RampDown:
                delta = 64 - static_cast<std::int32_t>(position + 1u) / 2;
                break;
            case vibrato_type::RampUp:
                delta = static_cast<std::int32_t>(position + 1u) / 2 - 64;
                break;
            case vibrato_type::Square:
                delta = position < 128u ? 64 : 0;
                break;
            case vibrato_type::Sine:
            default:
                delta = IT_SINE[position];
                break;
            }
            delta = (delta * depth) / 64;
            auto const l = static_cast<std::uint32_t>(std::abs(delta));
            std::int64_t period = std::min<std::int64_t>(aPeriod, std::numeric_limits<std::int32_t>::max() / 256) * 256;
            std::int64_t change = 0;
            if (delta < 0)
            {
                change = period * down_table(l / 4u) / 0x10000 - period;
                if ((l & 0x03u) != 0u)
                    change += period * fine_down_table(l & 0x03u) / 0x10000 - period;
            }
            else
            {
                change = period * up_table(l / 4u) / 0x10000 - period;
                if ((l & 0x03u) != 0u)
                    change += period * fine_up_table(l & 0x03u) / 0x10000 - period;
            }
            if (period + change <= std::numeric_limits<std::int32_t>::max())
            {
                aPeriod = static_cast<std::int32_t>((period + change) / 256);
                aPeriodFraction = static_cast<std::int32_t>(change & 0xFF);
            }
            else
            {
                aPeriod = std::numeric_limits<std::int32_t>::max() / 256;
                aPeriodFraction = 0;
            }
            return;
        }
        // OpenMPT's auto-vibrato
        std::int32_t depth = aChannel.autoVibratoDepth;
        std::int32_t const fullDepth = static_cast<std::int32_t>(smp.vibratoDepth) * 256;
        if (smp.vibratoSweep == 0u && !is_it())
            depth = fullDepth;
        else if (is_it())
        {
            depth += smp.vibratoSweep * 2;
            depth = std::min(depth, fullDepth);
            aChannel.autoVibratoDepth = depth;
        }
        else
        {
            if ((aChannel.flags & CHN_KEYOFF) == 0u && depth <= fullDepth)
            {
                depth += fullDepth / smp.vibratoSweep;
                aChannel.autoVibratoDepth = depth;
            }
            // FastTracker 2: key off before the sweep has finished resets the depth
            if (depth > fullDepth)
                depth = fullDepth;
            else if ((aChannel.flags & CHN_KEYOFF) != 0u && behaves(behaviour::FT2AutoVibratoAbortSweep))
                depth = fullDepth / smp.vibratoSweep;
        }
        aChannel.autoVibratoPosition += smp.vibratoRate;
        std::int32_t delta = 0;
        switch (smp.vibratoType)
        {
        case vibrato_type::Random:
            delta = MOD_RANDOM[aChannel.autoVibratoPosition & 0x3Fu];
            ++aChannel.autoVibratoPosition;
            break;
        case vibrato_type::RampDown:
            delta = static_cast<std::int32_t>((0x40u - (aChannel.autoVibratoPosition / 2u)) & 0x7Fu) - 0x40;
            break;
        case vibrato_type::RampUp:
            delta = static_cast<std::int32_t>((0x40u + (aChannel.autoVibratoPosition / 2u)) & 0x7Fu) - 0x40;
            break;
        case vibrato_type::Square:
            delta = (aChannel.autoVibratoPosition & 128u) != 0u ? 64 : -64;
            break;
        case vibrato_type::Sine:
        default:
            delta = -IT_SINE[aChannel.autoVibratoPosition & 0xFFu];
            break;
        }
        std::int32_t n = (delta * depth) / 256;
        if (!is_xm())
        {
            std::int64_t df1 = 0;
            std::int64_t df2 = 0;
            if (n < 0)
            {
                n = -n;
                auto const n1 = static_cast<std::uint32_t>(n) / 256u;
                df1 = down_table(n1);
                df2 = down_table(n1 + 1u);
            }
            else
            {
                auto const n1 = static_cast<std::uint32_t>(n) / 256u;
                df1 = up_table(n1);
                df2 = up_table(n1 + 1u);
            }
            n /= 4;
            auto const period = static_cast<std::int64_t>(aPeriod) * (df1 + ((df2 - df1) * (n & 0x3F) / 64)) / 256;
            aPeriodFraction = static_cast<std::int32_t>(period & 0xFF);
            aPeriod = static_cast<std::int32_t>(std::min<std::int64_t>(period / 256, std::numeric_limits<std::int32_t>::max()));
        }
        else
            aPeriod += n / 64;
    }

    std::int32_t replayer::setup_channel_filter(channel_state& aChannel, bool aReset, std::int32_t aEnvelopeModifier) const
    {
        std::int32_t const cutoff = std::clamp(static_cast<std::int32_t>(aChannel.cutoff) + aChannel.cutoffSwing, 0, 127);
        std::int32_t const resonance = std::clamp(static_cast<std::int32_t>(aChannel.resonance & 0x7Fu) + aChannel.resonanceSwing, 0, 127);
        if (!behaves(behaviour::MPTOldSwingBehaviour))
        {
            aChannel.cutoff = static_cast<std::uint8_t>(cutoff);
            aChannel.cutoffSwing = 0;
            aChannel.resonance = static_cast<std::uint8_t>(resonance);
            aChannel.resonanceSwing = 0;
        }
        std::int32_t const computedCutoff = cutoff * (aEnvelopeModifier + 256) / 256;
        if (behaves(behaviour::ITFilterBehaviour) && resonance == 0 && computedCutoff >= 254)
        {
            if (aChannel.triggerNote)
                aChannel.flags &= ~CHN_FILTER;
            return -1;
        }
        aChannel.flags |= CHN_FILTER;
        float const sampleRate = static_cast<float>(iOptions.sampleRate);
        float const dmpfac = std::pow(10.0f, static_cast<float>(-resonance) * ((24.0f / 128.0f) / 20.0f));
        float frequency = 110.0f * std::pow(2.0f, 0.25f + static_cast<float>(cutoff * (aEnvelopeModifier + 256)) / (iModule.extendedFilterRange ? 20.0f * 512.0f : 24.0f * 512.0f));
        frequency = std::clamp(frequency, 120.0f, 20000.0f);
        frequency = std::min(frequency, sampleRate * 0.5f);
        float const fc = frequency * 2.0f * 3.14159265358979f;
        float d = 0.0f;
        float e = 0.0f;
        if (behaves(behaviour::ITFilterBehaviour) && !iModule.extendedFilterRange)
        {
            float const r = sampleRate / fc;
            d = dmpfac * r + dmpfac - 1.0f;
            e = r * r;
        }
        else
        {
            float const r = fc / sampleRate;
            d = (1.0f - 2.0f * dmpfac) * r;
            d = std::min(d, 2.0f);
            d = (2.0f * dmpfac - d) / r;
            e = 1.0f / (r * r);
        }
        float const fg = 1.0f / (1.0f + d + e);
        float const fb0 = (d + e + e) / (1.0f + d + e);
        float const fb1 = -e / (1.0f + d + e);
        if (aChannel.filterMode == 1u)
        {
            aChannel.filterA0 = 1.0f - fg;
            aChannel.filterHighPass = 1.0f;
        }
        else
        {
            aChannel.filterA0 = fg;
            aChannel.filterHighPass = 0.0f;
        }
        aChannel.filterB0 = fb0;
        aChannel.filterB1 = fb1;
        if (aReset)
            aChannel.filterHistory = {};
        return computedCutoff;
    }

    std::int32_t replayer::handle_note_change_filter(channel_state& aChannel) const
    {
        if (!aChannel.triggerNote)
            return -1;
        bool useFilter = true;
        if (aChannel.ins != nullptr)
        {
            if (aChannel.ins->resonance)
                aChannel.resonance = *aChannel.ins->resonance;
            if (aChannel.ins->cutoff)
                aChannel.cutoff = *aChannel.ins->cutoff;
            if (useFilter && aChannel.ins->filterMode)
                aChannel.filterMode = *aChannel.ins->filterMode;
        }
        else
        {
            aChannel.volumeSwing = aChannel.panSwing = 0;
            aChannel.cutoffSwing = aChannel.resonanceSwing = 0;
        }
        std::int32_t cutoff = -1;
        if ((aChannel.cutoff < 0x7Fu || behaves(behaviour::ITFilterBehaviour)) && useFilter)
        {
            cutoff = setup_channel_filter(aChannel, true);
            if (cutoff >= 0)
                cutoff = aChannel.cutoff / 2;
        }
        return cutoff;
    }

    void replayer::process_ramping(channel_state& aChannel) const
    {
        aChannel.leftRamp = aChannel.rightRamp = 0.0f;
        aChannel.rampFrames = 0u;
        if (aChannel.leftGain != aChannel.newLeftGain || aChannel.rightGain != aChannel.newRightGain)
        {
            bool const rampUp = aChannel.newLeftGain > aChannel.leftGain || aChannel.newRightGain > aChannel.rightGain;
            std::uint32_t globalLength = rampUp ? iRampUpFrames : iRampDownFrames;
            // FastTracker 2 ramps over 5 milliseconds
            if (behaves(behaviour::FT2VolumeRamping) && is_xm())
                globalLength = static_cast<std::uint32_t>(muldivr(5, iOptions.sampleRate, 1000));
            std::uint32_t length = globalLength;
            // an instrument can have a ramp up of its own
            std::uint32_t instrumentLength = 0u;
            if (aChannel.ins != nullptr && rampUp)
            {
                instrumentLength = aChannel.ins->volumeRampUp;
                length = instrumentLength != 0u ? static_cast<std::uint32_t>(static_cast<std::uint64_t>(iOptions.sampleRate) * instrumentLength / 100000u) : globalLength;
            }
            // between two audible volumes the change is spread over the whole tick
            if (instrumentLength == 0u && (aChannel.leftGain != 0.0f || aChannel.rightGain != 0.0f) && (aChannel.newLeftGain != 0.0f || aChannel.newRightGain != 0.0f) &&
                (aChannel.flags & CHN_FASTVOLRAMP) == 0u)
                length = static_cast<std::uint32_t>(std::clamp<std::uint64_t>(iState.tickFramesLeft, globalLength, std::max(globalLength, MAXIMUM_RAMP_FRAMES)));
            length = std::max(length, 1u);
            aChannel.leftRamp = (aChannel.newLeftGain - aChannel.leftGain) / static_cast<float>(length);
            aChannel.rightRamp = (aChannel.newRightGain - aChannel.rightGain) / static_cast<float>(length);
            aChannel.rampFrames = length;
        }
        aChannel.flags &= ~CHN_FASTVOLRAMP;
    }

    void replayer::read_note()
    {
        auto& s = iState;
        for (std::uint32_t index = 0u; index < s.channels.size(); ++index)
        {
            auto& ch = s.channels[index];
            bool const patternChannel = index < iModule.channels;
            if ((ch.flags & CHN_MUTE) != 0u || (!patternChannel && ch.length == 0u))
            {
                if (patternChannel && ((ch.row.command == effect::Midi && s.firstTick) || ch.row.command == effect::SmoothMidi))
                {
                    auto const parameter = ch.row.parameter;
                    process_midi_macro(index, ch.row.command == effect::SmoothMidi,
                        parameter < 0x80u ? iModule.parameteredMacros[ch.activeMacro & 0x0Fu] : iModule.fixedMacros[parameter & 0x7Fu], parameter);
                }
                ch.currentSample = false;
                continue;
            }
            ch.increment = 0.0;
            ch.realVolume = 0;
            ch.calculatedVolume = 0;
            ch.rampFrames = 0u;
            instrument const* const ins = ch.ins;
            std::int32_t period = 0;
            bool const samplePlaying = ch.period != 0 && ch.length != 0u;
            if (samplePlaying)
            {
                std::int32_t volume = ch.volume;
                std::int32_t instrumentVolume = ch.instrumentVolume;
                // volume swing
                if (behaves(behaviour::ITSwingBehaviour))
                    instrumentVolume = std::clamp(instrumentVolume + ch.volumeSwing, 0, 64);
                else if (behaves(behaviour::MPTOldSwingBehaviour))
                    volume = std::clamp(volume + ch.volumeSwing, 0, 256);
                else
                {
                    ch.volume = std::clamp(ch.volume + ch.volumeSwing, 0, 256);
                    volume = ch.volume;
                    ch.volumeSwing = 0;
                }
                // panning swing
                if (behaves(behaviour::ITSwingBehaviour) || behaves(behaviour::MPTOldSwingBehaviour))
                    ch.realPan = std::clamp(ch.pan + ch.panSwing, 0, 256);
                else
                {
                    ch.pan = std::clamp(ch.pan + ch.panSwing, 0, 256);
                    ch.panSwing = 0;
                    ch.realPan = ch.pan;
                }
                process_tremolo(ch, volume);
                process_tremor(ch, volume);
                volume = std::clamp(volume, 0, 256) << 6;
                if (ins != nullptr)
                {
                    if (behaves(behaviour::ITEnvelopePositionHandling))
                        increment_envelope_positions(ch);
                    process_volume_envelope(ch, volume);
                    process_instrument_fade(ch, volume);
                    process_panning_envelope(ch);
                    if (!behaves(behaviour::ITPitchPanSeparation) && ch.note != NOTE_NONE && ins->pitchPanSeparation != 0)
                        process_pitch_pan_separation(ch.realPan, ch.note, *ins);
                }
                else if ((ch.flags & CHN_NOTEFADE) != 0u)
                {
                    // no envelope: key off is a note cut
                    ch.fadeOutVolume = 0;
                    volume = 0;
                }
                if (volume != 0)
                    ch.realVolume = muldiv(static_cast<std::int64_t>(volume) * s.globalVolume, static_cast<std::int64_t>(ch.channelVolume) * instrumentVolume, 1 << 20);
                ch.calculatedVolume = volume;
                // the period limits
                if (ch.period < iModule.minimumPeriod && !is_s3m() && !periods_are_frequencies())
                    ch.period = iModule.minimumPeriod;
                else if (ch.period >= iModule.maximumPeriod && behaves(behaviour::ApplyUpperPeriodLimit) && !periods_are_frequencies())
                    ch.period = iModule.maximumPeriod;
                period = ch.period;
                // glissando: semitones
                if ((ch.flags & (CHN_GLISSANDO | CHN_PORTAMENTO)) == (CHN_GLISSANDO | CHN_PORTAMENTO) &&
                    (!iModule.protrackerMode || (ch.row.is_tone_portamento() && !s.firstTick)))
                {
                    if (period != ch.cachedPeriod)
                    {
                        ch.cachedPeriod = period;
                        ch.glissandoPeriod = period_from_note(note_from_period(period, ch.finetune, ch.c5speed), ch.finetune, ch.c5speed);
                    }
                    period = ch.glissandoPeriod;
                }
                process_arpeggio(ch, period);
                // the Amiga's limits
                if ((iModule.amigaLimits || iModule.protrackerMode) && period != std::numeric_limits<std::int32_t>::max())
                {
                    std::int32_t low = 113 * 4;
                    std::int32_t high = 856 * 4;
                    if (!is_s3m())
                    {
                        auto const offset = xm_to_mod_finetune(ch.finetune) * 12u;
                        low = std::max<std::int32_t>(PROTRACKER_TUNED_PERIODS[offset + 11u] / 2, 113 * 4);
                        high = PROTRACKER_TUNED_PERIODS[offset] * 2;
                    }
                    period = std::clamp(period, low, high);
                    ch.period = std::clamp(ch.period, low, high);
                }
                process_panbrello(ch);
            }
            // nothing moves a surround channel's pan
            if ((ch.flags & CHN_SURROUND) != 0u && behaves(behaviour::ITNoSurroundPan))
                ch.realPan = 128;
            handle_note_change_filter(ch);
            if (patternChannel && ((ch.row.command == effect::Midi && s.firstTick) || ch.row.command == effect::SmoothMidi))
            {
                auto const parameter = ch.row.parameter;
                process_midi_macro(index, ch.row.command == effect::SmoothMidi,
                    parameter < 0x80u ? iModule.parameteredMacros[ch.activeMacro & 0x0Fu] : iModule.fixedMacros[parameter & 0x7Fu], parameter);
            }
            auto& c = s.channels[index];
            if (samplePlaying)
                process_pitch_filter_envelope(c, period);
            if (c.row.volumeCommand == volume_command::VibratoDepth &&
                (c.row.command == effect::Vibrato || c.row.command == effect::VibratoVolumeSlide || c.row.command == effect::FineVibrato))
            {
                if (is_xm())
                {
                    // FastTracker 2 advances vibrato twice (but applies it once) with vibrato in both columns
                    if (!s.firstTick)
                        c.vibratoPosition = static_cast<std::uint8_t>(c.vibratoPosition + c.vibratoSpeed);
                }
                else if (is_it())
                {
                    // Impulse Tracker applies it twice
                    vibrato(c, c.row.volume);
                    process_vibrato(c, period);
                }
            }
            process_vibrato(c, period);
            if (samplePlaying)
            {
                std::int32_t periodFraction = 0;
                process_auto_vibrato(c, period, periodFraction);
                if (period <= iModule.minimumPeriod)
                {
                    if (behaves(behaviour::ST3LimitPeriod))
                        c.length = 0u;
                    period = iModule.minimumPeriod;
                }
                // the increment in 32.32 fixed point, as OpenMPT has it
                auto const frequency = std::min<std::uint64_t>(frequency_from_period(period, c.c5speed, periodFraction), 0x7FFFFFFFu);
                auto const increment = (frequency << 32u) / (static_cast<std::uint64_t>(iOptions.sampleRate) << FREQUENCY_FRACTION_BITS);
                c.increment = std::max<double>(static_cast<double>(increment), 1.0) / 4294967296.0;
            }
            if (ins != nullptr && !behaves(behaviour::ITEnvelopePositionHandling))
                increment_envelope_positions(c);
            // a faded out note stops (unless FastTracker 2 can pick it up again with a portamento)
            if ((c.flags & CHN_NOTEFADE) != 0u && c.fadeOutVolume == 0 && c.leftGain == 0.0f && c.rightGain == 0.0f && !behaves(behaviour::FT2ProcessSilentChannels))
                c.length = 0u;
            c.newLeftGain = c.newRightGain = 0.0f;
            c.currentSample = c.smp != nullptr && c.smp->has_data() && c.length != 0u && c.sample_playing();
            if (c.currentSample)
            {
                std::int32_t const pan = std::clamp(c.realPan, 0, 256);
                float const volume = static_cast<float>(c.realVolume) / 16384.0f * iMasterGain;
                float left = 0.0f;
                float right = 0.0f;
                switch (iModule.mixLevels)
                {
                case mix_levels::v1_17RC3:
                    // soft panning
                    if (pan < 128)
                    {
                        left = volume * 0.5f;
                        right = volume * static_cast<float>(pan) / 256.0f;
                    }
                    else
                    {
                        left = volume * static_cast<float>(256 - pan) / 256.0f;
                        right = volume * 0.5f;
                    }
                    break;
                case mix_levels::CompatibleFT2:
                    {
                        // FastTracker 2's square root pan law; its pan can never be fully right
                        auto const p = std::min(pan, 255);
                        auto const& t = tables();
                        left = volume * (p > 0 ? static_cast<float>(t.xmPanning[256 - p]) : 65536.0f) / 65536.0f;
                        right = volume * static_cast<float>(t.xmPanning[p]) / 65536.0f;
                    }
                    break;
                default:
                    left = volume * static_cast<float>(256 - pan) / 256.0f;
                    right = volume * static_cast<float>(pan) / 256.0f;
                    break;
                }
                if ((c.flags & CHN_SURROUND) != 0u)
                    right = -right;
                // stereo separation
                float const middle = (left + right) * 0.5f;
                float const side = (left - right) * 0.5f * iSeparation;
                c.newLeftGain = middle + side;
                c.newRightGain = middle - side;
                if ((c.flags & CHN_PINGPONGFLAG) != 0u)
                    c.increment = -c.increment;
                process_ramping(c);
                // a background voice at no volume is finished with
                if (!patternChannel && !(c.volume != 0 && c.channelVolume != 0 && c.instrumentVolume != 0))
                    c.length = 0u;
            }
            else
            {
                c.leftGain = c.rightGain = 0.0f;
                c.length = 0u;
            }
            c.oldFlags = c.flags;
            c.triggerNote = false;
        }
        // finished background voices at the end are let go
        while (s.channels.size() > iModule.channels && s.channels.back().length == 0u)
        {
            auto const last = static_cast<std::uint32_t>(s.channels.size() - 1u);
            if (s.lastMovedChannel == last)
                break;
            s.channels.pop_back();
        }
    }

    // ---------------------------------------------------------------------------------------------------
    // mixing

    void replayer::stop_channel(channel_state& aChannel) const
    {
        aChannel.currentSample = false;
        aChannel.length = 0u;
        aChannel.position = 0.0;
        aChannel.rampFrames = 0u;
        aChannel.flags &= ~CHN_PINGPONGFLAG;
    }

    // At a loop boundary, or past the end of the sample: where the sample carries on from (and in which
    // direction), or false if it has finished.
    bool replayer::fix_position(channel_state& aChannel) const
    {
        bool const looped = (aChannel.flags & CHN_LOOP) != 0u;
        double const loopStart = looped ? aChannel.loopStart : 0.0;
        double const length = aChannel.length;
        if (aChannel.increment == 0.0 || aChannel.length == 0u)
            return false;
        if (aChannel.position < loopStart)
        {
            if (aChannel.increment < 0.0)
            {
                // the start of a loop, going backwards
                aChannel.position = loopStart + loopStart - aChannel.position;
                if (aChannel.position < loopStart || aChannel.position >= std::floor((loopStart + length) / 2.0))
                    aChannel.position = loopStart;
                if ((aChannel.flags & CHN_PINGPONGLOOP) != 0u)
                {
                    aChannel.flags &= ~CHN_PINGPONGFLAG;
                    aChannel.increment = -aChannel.increment;
                }
                else
                    aChannel.position = length - 1.0;
                if (!looped || aChannel.position >= length)
                {
                    aChannel.position = length;
                    return false;
                }
            }
            else if (aChannel.position < 0.0)
                aChannel.position = 0.0;
        }
        else if (aChannel.position >= length)
        {
            if (!looped)
                return false;
            if ((aChannel.flags & CHN_PINGPONGLOOP) != 0u)
            {
                if (aChannel.increment > 0.0)
                    aChannel.increment = -aChannel.increment;
                aChannel.flags |= CHN_PINGPONGFLAG;
                double const difference = behaves(behaviour::ITPingPongMode) ? 1.0 : 0.0;
                double const overshoot = aChannel.position - length;
                double const loopLength = static_cast<double>(aChannel.loopEnd) - aChannel.loopStart - difference;
                if (!behaves(behaviour::ImprecisePingPongLoops))
                {
                    if (std::floor(overshoot) < loopLength)
                        aChannel.position = length - difference - overshoot;
                    else
                        aChannel.position = aChannel.loopStart;
                }
                else
                {
                    // ModPlug's reflection, which is a frame out
                    double const whole = std::floor(aChannel.position);
                    double const fraction = aChannel.position - whole;
                    if (fraction == 0.0)
                        aChannel.position = length - (whole - length) - 1.0;
                    else
                        aChannel.position = length - (whole - length) + (1.0 - fraction);
                    // (only the whole part is reset)
                    if (std::floor(aChannel.position) <= aChannel.loopStart || std::floor(aChannel.position) >= length)
                        aChannel.position = length - std::min(length, difference + 1.0) + (aChannel.position - std::floor(aChannel.position));
                }
            }
            else
            {
                if (aChannel.increment < 0.0)
                    aChannel.increment = -aChannel.increment;
                aChannel.position += loopStart - length;
            }
        }
        if (aChannel.position < loopStart)
        {
            if (aChannel.position < 0.0 || aChannel.increment < 0.0)
                return false;
        }
        else
        {
            if (aChannel.position > length)
                return false;
            if (aChannel.position >= length && aChannel.increment > 0.0)
                return false;
        }
        return true;
    }

    std::uint64_t replayer::frames_to_boundary(channel_state const& aChannel) const
    {
        constexpr double MAXIMUM = 1e12;
        if (aChannel.increment > 0.0)
        {
            double const distance = static_cast<double>(aChannel.length) - aChannel.position;
            if (distance <= 0.0)
                return 1u;
            auto frames = std::ceil(distance / aChannel.increment);
            if (frames > MAXIMUM)
                return static_cast<std::uint64_t>(MAXIMUM);
            auto result = std::max<std::uint64_t>(static_cast<std::uint64_t>(frames), 1u);
            // the first frame at or past the end is the boundary
            while (result > 1u && aChannel.position + aChannel.increment * static_cast<double>(result - 1u) >= aChannel.length)
                --result;
            while (aChannel.position + aChannel.increment * static_cast<double>(result) < aChannel.length)
                ++result;
            return result;
        }
        double const loopStart = (aChannel.flags & CHN_LOOP) != 0u ? aChannel.loopStart : 0.0;
        double const distance = aChannel.position - loopStart;
        if (distance < 0.0)
            return 1u;
        auto frames = std::floor(distance / -aChannel.increment) + 1.0;
        if (frames > MAXIMUM)
            return static_cast<std::uint64_t>(MAXIMUM);
        auto result = std::max<std::uint64_t>(static_cast<std::uint64_t>(frames), 1u);
        while (result > 1u && aChannel.position + aChannel.increment * static_cast<double>(result - 1u) < loopStart)
            --result;
        while (aChannel.position + aChannel.increment * static_cast<double>(result) >= loopStart)
            ++result;
        return result;
    }

    void replayer::mix_channel(channel_state& aChannel, float* aOutput, std::uint64_t aFrames, bool aAudible)
    {
        if (!aChannel.currentSample)
            return;
        while (aFrames != 0u)
        {
            if (!fix_position(aChannel))
            {
                stop_channel(aChannel);
                return;
            }
            std::uint64_t frames = aFrames;
            if (aChannel.rampFrames != 0u)
                frames = std::min<std::uint64_t>(frames, aChannel.rampFrames);
            frames = std::min(frames, frames_to_boundary(aChannel));
            auto const& smp = *aChannel.smp;
            double const start = aChannel.position;
            double const increment = aChannel.increment;
            bool const silent = aChannel.rampFrames == 0u && aChannel.leftGain == 0.0f && aChannel.rightGain == 0.0f;
            bool const filtered = (aChannel.flags & CHN_FILTER) != 0u;
            // a resonant filter's history is kept up to date even when the channel isn't heard (muted, or the
            // song being advanced without mixing), so that it sounds the same when it is
            if (!silent && (aAudible || filtered))
            {
                float const* const data = smp.data.data();
                std::uint32_t const channels = smp.stereo ? 2u : 1u;
                std::uint32_t const dataFrames = static_cast<std::uint32_t>(smp.data.size() / channels);
                bool const looped = (aChannel.flags & CHN_LOOP) != 0u;
                bool const pingPong = (aChannel.flags & CHN_PINGPONGLOOP) != 0u;
                std::int64_t const pingPongDifference = behaves(behaviour::ITPingPongMode) ? 1 : 0;
                float left = aChannel.leftGain;
                float right = aChannel.rightGain;
                float const leftRamp = aChannel.rampFrames != 0u ? aChannel.leftRamp : 0.0f;
                float const rightRamp = aChannel.rampFrames != 0u ? aChannel.rightRamp : 0.0f;
                float peak = aChannel.peak;
                float const level = static_cast<float>(aChannel.realVolume) / 16384.0f;
                for (std::uint64_t frame = 0u; frame < frames; ++frame)
                {
                    double const position = start + increment * static_cast<double>(frame);
                    auto index = static_cast<std::int64_t>(std::floor(position));
                    float const fraction = static_cast<float>(position - static_cast<double>(index));
                    index = std::clamp<std::int64_t>(index, 0, static_cast<std::int64_t>(dataFrames) - 1);
                    // the frame after: at the end of a loop, the loop's start, or the loop played back the
                    // other way; at the end of the sample, the last frame held
                    std::int64_t next = index + 1;
                    if (next >= aChannel.length && looped && aChannel.loopEnd > aChannel.loopStart && aChannel.loopEnd == aChannel.length)
                    {
                        if (!pingPong)
                            next = aChannel.loopStart + (next - aChannel.length);
                        else
                            next = std::max<std::int64_t>(static_cast<std::int64_t>(aChannel.loopEnd) - 1 - pingPongDifference, aChannel.loopStart);
                    }
                    next = std::min<std::int64_t>(next, static_cast<std::int64_t>(dataFrames) - 1);
                    float values[2];
                    for (std::uint32_t c = 0u; c < channels; ++c)
                    {
                        float const s0 = data[index * channels + c];
                        float const s1 = data[next * channels + c];
                        values[c] = s0 + (s1 - s0) * fraction;
                    }
                    if (filtered)
                    {
                        for (std::uint32_t c = 0u; c < channels; ++c)
                        {
                            auto& history = aChannel.filterHistory[c];
                            float const value = values[c] * aChannel.filterA0 + std::clamp(history[0], -2.0f, 2.0f) * aChannel.filterB0 +
                                std::clamp(history[1], -2.0f, 2.0f) * aChannel.filterB1;
                            history[1] = history[0];
                            history[0] = value - values[c] * aChannel.filterHighPass;
                            values[c] = value;
                        }
                    }
                    if (!aAudible)
                        continue;
                    left += leftRamp;
                    right += rightRamp;
                    float const l = values[0];
                    float const r = channels == 2u ? values[1] : values[0];
                    aOutput[frame * 2u] += l * left;
                    aOutput[frame * 2u + 1u] += r * right;
                    peak = std::max(peak, std::max(std::abs(l), std::abs(r)) * level);
                }
                aChannel.peak = peak;
            }
            aChannel.position = start + increment * static_cast<double>(frames);
            if (aChannel.rampFrames != 0u)
            {
                if (aChannel.rampFrames <= frames)
                {
                    aChannel.rampFrames = 0u;
                    aChannel.leftGain = aChannel.newLeftGain;
                    aChannel.rightGain = aChannel.newRightGain;
                    aChannel.leftRamp = aChannel.rightRamp = 0.0f;
                    if ((aChannel.flags & CHN_NOTEFADE) != 0u && aChannel.fadeOutVolume == 0)
                    {
                        aChannel.length = 0u;
                        aChannel.currentSample = false;
                        return;
                    }
                }
                else
                {
                    aChannel.leftGain += aChannel.leftRamp * static_cast<float>(frames);
                    aChannel.rightGain += aChannel.rightRamp * static_cast<float>(frames);
                    aChannel.rampFrames -= static_cast<std::uint32_t>(frames);
                }
            }
            if (aOutput != nullptr)
                aOutput += frames * 2u;
            aFrames -= frames;
            // ProTracker: a sample swap happens when the old sample's loop (or the sample) ends
            bool const looped = (aChannel.flags & CHN_LOOP) != 0u;
            bool const pastLoopEnd = looped && aChannel.position >= aChannel.loopEnd;
            bool const pastSampleEnd = !looped && aChannel.length != 0u && aChannel.position >= aChannel.length && aChannel.masterChannel == 0u;
            bool const doSampleSwap = behaves(behaviour::MODSampleSwap) && aChannel.swapSample != 0u && aChannel.swapSample <= sample_count() &&
                aChannel.smp != &iModule.samples[aChannel.swapSample - 1u];
            if ((pastLoopEnd || pastSampleEnd) && doSampleSwap)
            {
                auto const& swap = iModule.samples[aChannel.swapSample - 1u];
                aChannel.smp = &swap;
                aChannel.flags = (aChannel.flags & CHN_CHANNELFLAGS) | sample_flags(swap);
                if (swap.loop != loop_type::None)
                    aChannel.length = swap.loopEnd;
                else if (!behaves(behaviour::MODOneShotLoops))
                    aChannel.length = swap.length;
                else
                    aChannel.length = 0u;
                aChannel.loopStart = swap.loopStart;
                aChannel.loopEnd = swap.loopEnd;
                aChannel.position = aChannel.loopStart;
                aChannel.swapSample = 0u;
                if (!swap.has_data())
                {
                    aChannel.currentSample = false;
                    return;
                }
            }
            else if (pastLoopEnd && !doSampleSwap && behaves(behaviour::MODOneShotLoops) && aChannel.loopStart == 0u)
            {
                // ProTracker's one shot loops: the whole sample once, then the loop
                aChannel.position = 0.0;
                aChannel.loopEnd = aChannel.length = aChannel.smp->loopEnd;
            }
        }
    }
}
