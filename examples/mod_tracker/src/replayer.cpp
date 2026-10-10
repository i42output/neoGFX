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

#include <mod_tracker/replayer.hpp>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <unordered_set>
#include <utility>

namespace mod_tracker
{
    namespace
    {
        // the PAL Amiga's Paula clock divided by two: a period of P plays CLOCK / P sample bytes per second
        constexpr double PAULA_CLOCK = 3546894.6;
        constexpr std::int32_t MIN_PERIOD = 113;
        constexpr std::int32_t MAX_PERIOD = 856;
        // multi-channel modules (from PC trackers) commonly go an octave either side of ProTracker's range
        constexpr std::int32_t MIN_PERIOD_EXTENDED = 28;
        constexpr std::int32_t MAX_PERIOD_EXTENDED = 3424;
        // time constant of the volume smoothing that keeps volume changes from clicking
        constexpr double GAIN_SMOOTHING_S = 0.002;
        // playback restarted this close to where the song already is carries on rather than seeking
        constexpr double CONTINUITY_TOLERANCE_S = 0.1;

        constexpr std::uint8_t SINE_TABLE[32] =
        {
              0,  24,  49,  74,  97, 120, 141, 161, 180, 197, 212, 224, 235, 244, 250, 253,
            255, 253, 250, 244, 235, 224, 212, 197, 180, 161, 141, 120,  97,  74,  49,  24
        };

        // a vibrato/tremolo waveform at aPosition (0 .. 63), -255 .. 255
        std::int32_t waveform(std::uint8_t aWaveform, std::uint8_t aPosition)
        {
            std::int32_t magnitude = 0;
            switch (aWaveform & 3u)
            {
            case 1: // ramp down
                magnitude = (aPosition & 31u) * 8;
                if (aPosition >= 32u)
                    magnitude = 255 - magnitude;
                break;
            case 2: // square
                magnitude = 255;
                break;
            default: // sine (and "random", which is played as a sine)
                magnitude = SINE_TABLE[aPosition & 31u];
                break;
            }
            return aPosition >= 32u ? -magnitude : magnitude;
        }
    }

    replayer::replayer(module_data const& aModule, render_options const& aOptions) :
        iModule{ aModule },
        iOptions{ aOptions },
        iMinPeriod{ aModule.channels == 4u ? MIN_PERIOD : MIN_PERIOD_EXTENDED },
        iMaxPeriod{ aModule.channels == 4u ? MAX_PERIOD : MAX_PERIOD_EXTENDED },
        iMasterGain{ static_cast<float>(2.0 / std::max<std::uint32_t>(aModule.channels, 2u)) },
        iSmoothing{ static_cast<float>(1.0 - std::exp(-1.0 / (GAIN_SMOOTHING_S * aOptions.sampleRate))) },
        iPeaks(aModule.channels, 0.0f)
    {
        iState.channels.resize(aModule.channels);
        // Amiga channel layout: left, right, right, left, ...
        for (std::uint32_t index = 0u; index < iState.channels.size(); ++index)
        {
            bool const left = (index % 4u == 0u || index % 4u == 3u);
            iState.channels[index].pan = 0.5 + (left ? -0.5 : 0.5) * std::clamp(aOptions.stereoSeparation, 0.0, 1.0);
        }
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
    }

    void replayer::restore_timing(replayer_state const& aState)
    {
        iState.orderPosition = aState.orderPosition;
        iState.row = aState.row;
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

    void replayer::render(float* aOutput, std::uint64_t aFrames, std::uint32_t aMuted)
    {
        while (aFrames != 0u && !iState.ended)
        {
            if (iState.tickFramesLeft == 0u)
            {
                advance_tick();
                continue;
            }
            auto const frames = std::min(aFrames, iState.tickFramesLeft);
            for (std::uint32_t index = 0u; index < iState.channels.size(); ++index)
                mix_channel(index, aOutput, frames, (aMuted & (1u << index)) != 0u);
            if (aOutput != nullptr)
            {
                for (std::uint64_t sample = 0u; sample < frames * 2u; ++sample)
                    aOutput[sample] = std::clamp(aOutput[sample], -1.0f, 1.0f);
                aOutput += frames * 2u;
            }
            iState.tickFramesLeft -= frames;
            iFrames += frames;
            aFrames -= frames;
        }
    }

    float replayer::take_peak(std::uint32_t aChannel)
    {
        return std::exchange(iPeaks[aChannel], 0.0f);
    }

    void replayer::advance_tick()
    {
        if (iState.rowPending)
        {
            if (iRowHandler && !iRowHandler(*this))
            {
                iState.ended = true;
                return;
            }
            iState.rowPending = false;
            iState.patternLoopJump = false;
            start_row();
            iState.tick = 0u;
            iState.ticksInRow = static_cast<std::uint32_t>(iState.speed) * (1u + iState.patternDelay);
            update_output(0u, false);
        }
        else if (++iState.tick < iState.ticksInRow)
        {
            run_tick(iState.tick % iState.speed);
            update_output(iState.tick % iState.speed, true);
        }
        else
        {
            // the next row starts (or the song ends) without a tick going by
            if (next_row())
                iState.rowPending = true;
            else
                iState.ended = true;
            return;
        }
        // a tick lasts 2.5 / tempo seconds; the fractional frame carries over so that rows don't drift
        iState.tickFraction += static_cast<double>(iOptions.sampleRate) * 2.5 / iState.tempo;
        iState.tickFramesLeft = static_cast<std::uint64_t>(iState.tickFraction);
        iState.tickFraction -= static_cast<double>(iState.tickFramesLeft);
    }

    // tick 0: read the row and apply its notes and the effects that act once per row
    void replayer::start_row()
    {
        auto const pattern = iModule.orders[iState.orderPosition];
        iState.positionJump.reset();
        iState.patternBreak.reset();
        iState.patternLoopTarget.reset();
        iState.patternDelay = 0u;

        for (std::uint32_t index = 0u; index < iState.channels.size(); ++index)
        {
            auto& ch = iState.channels[index];
            auto const& c = iModule.at(pattern, iState.row, index);
            ch.current = c;
            auto const x = static_cast<std::uint8_t>(c.parameter >> 4u);
            auto const y = static_cast<std::uint8_t>(c.parameter & 0x0Fu);
            bool const extended = (c.effect == 0xEu);
            bool const noteDelay = extended && x == 0xDu && y != 0u;
            bool const tonePorta = (c.effect == 0x3u || c.effect == 0x5u);

            if (c.sample != 0u && c.sample <= iModule.samples.size())
            {
                ch.selected = &iModule.samples[c.sample - 1u];
                ch.volume = ch.selected->volume;
                ch.finetune = ch.selected->finetune;
            }
            if (extended && x == 0x5u)
                ch.finetune = y >= 8u ? static_cast<std::int32_t>(y) - 16 : static_cast<std::int32_t>(y);

            bool triggered = false;
            if (c.period != 0u)
            {
                auto const period = target_period(ch, c);
                if (tonePorta)
                    ch.portaTarget = period;
                else if (!noteDelay)
                {
                    trigger(ch, period);
                    triggered = true;
                }
            }

            switch (c.effect)
            {
            case 0x3:
                if (c.parameter != 0u)
                    ch.portaSpeed = c.parameter;
                break;
            case 0x4:
                if (x != 0u)
                    ch.vibratoSpeed = x;
                if (y != 0u)
                    ch.vibratoDepth = y;
                break;
            case 0x7:
                if (x != 0u)
                    ch.tremoloSpeed = x;
                if (y != 0u)
                    ch.tremoloDepth = y;
                break;
            case 0x8:
                // not a ProTracker effect, but PC trackers wrote it into multi-channel modules
                if (iModule.channels != 4u)
                    ch.pan = c.parameter / 255.0;
                break;
            case 0x9:
                if (c.parameter != 0u)
                    ch.sampleOffset = static_cast<std::uint32_t>(c.parameter) * 256u;
                if (triggered && ch.instrument != nullptr)
                {
                    if (ch.sampleOffset < ch.instrument->data.size())
                        ch.position = ch.sampleOffset;
                    else
                        ch.playing = false;
                }
                break;
            case 0xB:
                iState.positionJump = c.parameter;
                break;
            case 0xC:
                ch.volume = std::min<std::int32_t>(c.parameter, 64);
                break;
            case 0xD:
                {
                    // the parameter is decimal; Soundtracker's pattern break had no parameter at all
                    auto const row = static_cast<std::uint32_t>(x) * 10u + y;
                    iState.patternBreak = row < ROWS_PER_PATTERN && !iModule.signature.empty() ? row : 0u;
                }
                break;
            case 0xE:
                switch (x)
                {
                case 0x1:
                    ch.period = std::max(ch.period - y, iMinPeriod);
                    break;
                case 0x2:
                    ch.period = std::min(ch.period + y, iMaxPeriod);
                    break;
                case 0x4:
                    ch.vibratoWaveform = y;
                    break;
                case 0x6:
                    if (y == 0u)
                        ch.loopRow = iState.row;
                    else if (ch.loopCount == 0u)
                    {
                        ch.loopCount = y;
                        iState.patternLoopTarget = ch.loopRow;
                    }
                    else if (--ch.loopCount != 0u)
                        iState.patternLoopTarget = ch.loopRow;
                    break;
                case 0x7:
                    ch.tremoloWaveform = y;
                    break;
                case 0xA:
                    ch.volume = std::min(ch.volume + y, 64);
                    break;
                case 0xB:
                    ch.volume = std::max(ch.volume - y, 0);
                    break;
                case 0xC:
                    if (y == 0u)
                        ch.volume = 0;
                    break;
                case 0xE:
                    // as in ProTracker, when more than one channel asks, the rightmost wins
                    iState.patternDelay = y;
                    break;
                default:
                    break;
                }
                break;
            case 0xF:
                if (c.parameter == 0u)
                    iState.songEnd = true;
                else if (c.parameter < 0x20u)
                    iState.speed = c.parameter;
                else
                    iState.tempo = c.parameter;
                break;
            default:
                break;
            }
        }
    }

    // ticks 1 .. speed - 1: the effects that act continuously
    void replayer::run_tick(std::uint32_t aTick)
    {
        for (auto& ch : iState.channels)
        {
            auto const& c = ch.current;
            auto const x = static_cast<std::uint8_t>(c.parameter >> 4u);
            auto const y = static_cast<std::uint8_t>(c.parameter & 0x0Fu);
            switch (c.effect)
            {
            case 0x1:
                ch.period = std::max<std::int32_t>(ch.period - c.parameter, iMinPeriod);
                break;
            case 0x2:
                ch.period = std::min<std::int32_t>(ch.period + c.parameter, iMaxPeriod);
                break;
            case 0x3:
                tone_portamento(ch);
                break;
            case 0x4:
                ch.vibratoPosition = static_cast<std::uint8_t>((ch.vibratoPosition + ch.vibratoSpeed) & 63u);
                break;
            case 0x5:
                tone_portamento(ch);
                volume_slide(ch, x, y);
                break;
            case 0x6:
                ch.vibratoPosition = static_cast<std::uint8_t>((ch.vibratoPosition + ch.vibratoSpeed) & 63u);
                volume_slide(ch, x, y);
                break;
            case 0x7:
                ch.tremoloPosition = static_cast<std::uint8_t>((ch.tremoloPosition + ch.tremoloSpeed) & 63u);
                break;
            case 0xA:
                volume_slide(ch, x, y);
                break;
            case 0xE:
                if (x == 0x9u && y != 0u && aTick % y == 0u)
                    retrigger(ch);
                else if (x == 0xCu && aTick == y)
                    ch.volume = 0;
                else if (x == 0xDu && aTick == y && c.period != 0u)
                    trigger(ch, target_period(ch, c));
                break;
            default:
                break;
            }
        }
    }

    void replayer::update_output(std::uint32_t aTick, bool aContinuous)
    {
        for (auto& ch : iState.channels)
        {
            auto const& c = ch.current;
            ch.outputPeriod = ch.period;
            ch.outputVolume = ch.volume;
            if (c.effect == 0x0u && c.parameter != 0u && ch.period != 0)
            {
                auto const step = aTick % 3u;
                if (step != 0u)
                {
                    auto const semitones = (step == 1u ? c.parameter >> 4u : c.parameter & 0x0Fu);
                    auto const note = period_note(static_cast<std::uint32_t>(ch.period), ch.finetune) + semitones;
                    ch.outputPeriod = note_period(std::min(note, PERIOD_TABLE_NOTES - 1u), ch.finetune);
                }
            }
            if ((c.effect == 0x4u || c.effect == 0x6u) && aContinuous)
                ch.outputPeriod += waveform(ch.vibratoWaveform, ch.vibratoPosition) * ch.vibratoDepth / 128;
            if (c.effect == 0x7u && aContinuous)
                ch.outputVolume = std::clamp(ch.volume + waveform(ch.tremoloWaveform, ch.tremoloPosition) * ch.tremoloDepth / 64, 0, 64);
            ch.outputPeriod = std::clamp(ch.outputPeriod, iMinPeriod / 2, iMaxPeriod * 2);
        }
    }

    bool replayer::next_row()
    {
        if (iState.songEnd)
            return false;
        if (iState.patternLoopTarget)
        {
            iState.row = *iState.patternLoopTarget;
            iState.patternLoopJump = true;
            return true;
        }
        if (iState.positionJump || iState.patternBreak)
        {
            iState.orderPosition = iState.positionJump ? *iState.positionJump : iState.orderPosition + 1u;
            iState.row = iState.patternBreak ? *iState.patternBreak : 0u;
            for (auto& ch : iState.channels)
                ch.loopRow = 0u, ch.loopCount = 0u;
        }
        else if (++iState.row == ROWS_PER_PATTERN)
        {
            ++iState.orderPosition;
            iState.row = 0u;
            for (auto& ch : iState.channels)
                ch.loopRow = 0u, ch.loopCount = 0u;
        }
        // off the end of the song: ProTracker starts again from the top (Noisetracker from the restart position)
        if (iState.orderPosition >= iModule.orders.size())
        {
            iState.orderPosition = iModule.restart < iModule.orders.size() ? iModule.restart : 0u;
            iState.row = 0u;
        }
        return true;
    }

    void replayer::mix_channel(std::uint32_t aIndex, float* aOutput, std::uint64_t aFrames, bool aMuted)
    {
        auto& ch = iState.channels[aIndex];
        auto const volume = static_cast<float>(ch.outputVolume) / 64.0f;
        auto const targetLeft = volume * iMasterGain * static_cast<float>(1.0 - ch.pan);
        auto const targetRight = volume * iMasterGain * static_cast<float>(ch.pan);
        if (!ch.playing || ch.instrument == nullptr || ch.outputPeriod <= 0)
        {
            // nothing playing: the gains still settle so that the next note doesn't start from a stale level
            ch.gainLeft = targetLeft;
            ch.gainRight = targetRight;
            return;
        }
        auto const& data = ch.instrument->data;
        bool const looped = ch.instrument->looped();
        auto const loopStart = static_cast<double>(ch.instrument->loopStart);
        auto const loopEnd = static_cast<double>(ch.instrument->loop_end());
        auto const end = looped ? loopEnd : static_cast<double>(data.size());
        auto const step = PAULA_CLOCK / ch.outputPeriod / iOptions.sampleRate;
        if (aOutput == nullptr || aMuted)
        {
            // not heard: the sample position moves on all the same, so that the channel is where it should be
            // when it is unmuted, or when a seek has finished advancing to its frame
            ch.gainLeft = targetLeft;
            ch.gainRight = targetRight;
            ch.position += step * static_cast<double>(aFrames);
            if (ch.position >= end)
            {
                if (looped)
                    ch.position = loopStart + std::fmod(ch.position - loopStart, loopEnd - loopStart);
                else
                    ch.playing = false;
            }
            return;
        }
        auto& peak = iPeaks[aIndex];
        for (std::uint64_t frame = 0u; frame < aFrames; ++frame)
        {
            auto const index = static_cast<std::size_t>(ch.position);
            auto const fraction = static_cast<float>(ch.position - static_cast<double>(index));
            auto next = index + 1u;
            if (static_cast<double>(next) >= end)
                next = looped ? static_cast<std::size_t>(loopStart) : index;
            auto const s0 = static_cast<float>(data[index]) / 128.0f;
            auto const s1 = static_cast<float>(data[next]) / 128.0f;
            auto const value = s0 + (s1 - s0) * fraction;
            ch.gainLeft += (targetLeft - ch.gainLeft) * iSmoothing;
            ch.gainRight += (targetRight - ch.gainRight) * iSmoothing;
            aOutput[frame * 2u] += value * ch.gainLeft;
            aOutput[frame * 2u + 1u] += value * ch.gainRight;
            peak = std::max(peak, std::abs(value) * volume);
            ch.position += step;
            if (ch.position >= end)
            {
                if (!looped)
                {
                    // settled, as for a channel with nothing playing, however the frames are divided into calls
                    ch.playing = false;
                    ch.gainLeft = targetLeft;
                    ch.gainRight = targetRight;
                    break;
                }
                ch.position = loopStart + std::fmod(ch.position - loopStart, loopEnd - loopStart);
            }
        }
    }

    std::int32_t replayer::target_period(channel_state const& aChannel, cell const& aCell) const
    {
        // the cell holds the finetune 0 period; a finetuned sample plays the same note slightly sharp or flat
        if (aCell.note != NO_NOTE)
            return note_period(aCell.note, aChannel.finetune);
        return aCell.period;
    }

    void replayer::trigger(channel_state& aChannel, std::int32_t aPeriod)
    {
        aChannel.instrument = aChannel.selected;
        aChannel.period = aPeriod;
        aChannel.position = 0.0;
        aChannel.playing = (aChannel.instrument != nullptr && !aChannel.instrument->data.empty());
        if ((aChannel.vibratoWaveform & 4u) == 0u)
            aChannel.vibratoPosition = 0u;
        if ((aChannel.tremoloWaveform & 4u) == 0u)
            aChannel.tremoloPosition = 0u;
    }

    void replayer::retrigger(channel_state& aChannel)
    {
        aChannel.position = 0.0;
        aChannel.playing = (aChannel.instrument != nullptr && !aChannel.instrument->data.empty());
    }

    void replayer::tone_portamento(channel_state& aChannel)
    {
        if (aChannel.portaTarget == 0 || aChannel.period == 0)
            return;
        if (aChannel.period < aChannel.portaTarget)
            aChannel.period = std::min<std::int32_t>(aChannel.period + aChannel.portaSpeed, aChannel.portaTarget);
        else if (aChannel.period > aChannel.portaTarget)
            aChannel.period = std::max<std::int32_t>(aChannel.period - aChannel.portaSpeed, aChannel.portaTarget);
    }

    void replayer::volume_slide(channel_state& aChannel, std::uint8_t aUp, std::uint8_t aDown)
    {
        if (aUp != 0u)
            aChannel.volume = std::min(aChannel.volume + aUp, 64);
        else
            aChannel.volume = std::max(aChannel.volume - aDown, 0);
    }

    std::size_t timeline::row_at(std::uint64_t aFrame) const
    {
        auto const next = std::upper_bound(rows.begin(), rows.end(), aFrame,
            [](std::uint64_t aLhs, row_info const& aRhs) { return aLhs < aRhs.frame; });
        return next == rows.begin() ? 0u : static_cast<std::size_t>(std::distance(rows.begin(), next) - 1);
    }

    std::optional<std::uint64_t> timeline::song_frame(std::uint64_t aPosition, bool aLooping) const
    {
        if (aPosition < frameCount)
            return aPosition;
        if (!aLooping || loop_length() == 0u)
            return {};
        return rows[*loopRow].frame + (aPosition - frameCount) % loop_length();
    }

    std::uint64_t timeline::pass_end(std::uint64_t aPosition) const
    {
        if (aPosition < frameCount || loop_length() == 0u)
            return frameCount;
        return frameCount + ((aPosition - frameCount) / loop_length() + 1u) * loop_length();
    }

    timeline scan(module_data const& aModule, render_options const& aOptions, std::stop_token aStop, std::atomic<float>& aProgress)
    {
        aProgress.store(0.0f);
        timeline result;
        result.sampleRate = aOptions.sampleRate;
        result.channels = aModule.channels;
        auto const maximumFrames = static_cast<std::uint64_t>(aOptions.maximumDuration_s * aOptions.sampleRate);

        replayer engine{ aModule, aOptions };
        // order position << 8 | row, for every row played; a row played again (other than by a pattern loop)
        // means the song has looped
        std::unordered_set<std::uint64_t> visited;
        engine.set_row_handler([&](replayer& aEngine)
            {
                if (aStop.stop_requested())
                    throw scan_cancelled{};
                auto const& state = aEngine.state();
                // the previous row's effects have all been applied by now, so its speed and tempo are known
                if (!result.rows.empty())
                {
                    result.rows.back().speed = state.speed;
                    result.rows.back().tempo = state.tempo;
                }
                auto const visit = (static_cast<std::uint64_t>(state.orderPosition) << 8u) | state.row;
                if (state.patternLoopJump)
                {
                    // a pattern loop replays rows of this pattern; they don't count as the song looping
                    std::erase_if(visited, [&](std::uint64_t aVisit)
                        {
                            return (aVisit >> 8u) == state.orderPosition && (aVisit & 0xFFu) >= state.row;
                        });
                    visited.insert(visit);
                }
                else if (!visited.insert(visit).second)
                {
                    auto const existing = std::find_if(result.rows.begin(), result.rows.end(), [&](row_info const& aRow)
                        {
                            return aRow.orderPosition == state.orderPosition && aRow.row == state.row;
                        });
                    if (existing != result.rows.end())
                        result.loopRow = static_cast<std::size_t>(std::distance(result.rows.begin(), existing));
                    return false;
                }
                if (aEngine.frames() >= maximumFrames)
                    return false;
                result.rows.push_back(row_info{ state.orderPosition, aModule.orders[state.orderPosition], state.row, aEngine.frames(), state.speed, state.tempo });
                result.snapshots.push_back(state);
                aProgress.store(std::max(aProgress.load(), static_cast<float>(state.orderPosition) / static_cast<float>(aModule.orders.size())));
                return true;
            });
        while (!engine.ended())
            engine.render(nullptr, aOptions.sampleRate);
        if (!result.rows.empty() && !result.loopRow)
        {
            // a song that stops (F00) never reaches another row to complete the last one's details
            result.rows.back().speed = engine.state().speed;
            result.rows.back().tempo = engine.state().tempo;
        }
        result.frameCount = engine.frames();
        aProgress.store(1.0f);
        return result;
    }

    stream::stream(module_data const& aModule, timeline const& aTimeline, render_options const& aOptions) :
        iTimeline{ aTimeline },
        iReplayer{ aModule, aOptions },
        iContinuityTolerance{ static_cast<std::uint64_t>(CONTINUITY_TOLERANCE_S * aOptions.sampleRate) }
    {
        // a new engine is exactly where the scan started, so playback from the top needs no seek
        iReplayer.set_row_handler([this](replayer&) { return row_starting(); });
    }

    void stream::render(std::uint64_t aPosition, float* aOutput, std::uint64_t aFrames)
    {
        auto const seekRequest = iSeekRequest.load(std::memory_order_acquire);
        if (seekRequest != iSeenSeekRequest)
        {
            iSeenSeekRequest = seekRequest;
            seek(iSeekTarget.load(std::memory_order_relaxed));
        }
        else if (aPosition != iNextCursor)
        {
            // playback was restarted; close to where the song already is means carry on, so that a pause and
            // resume (or the sequencer restarting playback after a stall) is seamless
            auto const distance = aPosition > iNextPosition ? aPosition - iNextPosition : iNextPosition - aPosition;
            if (distance > iContinuityTolerance)
                seek(aPosition);
        }
        iNextCursor = aPosition + aFrames;
        iNextPosition += aFrames;
        iReplayer.render(aOutput, aFrames, iMuted.load(std::memory_order_relaxed));
        for (std::uint32_t channel = 0u; channel < iTimeline.channels && channel < MAX_CHANNELS; ++channel)
        {
            auto const peak = iReplayer.take_peak(channel);
            if (peak > iPeaks[channel].load(std::memory_order_relaxed))
                iPeaks[channel].store(peak, std::memory_order_relaxed);
        }
    }

    bool stream::looping() const
    {
        return iLooping.load(std::memory_order_relaxed);
    }

    void stream::set_looping(bool aLooping)
    {
        iLooping.store(aLooping, std::memory_order_relaxed);
    }

    bool stream::muted(std::uint32_t aChannel) const
    {
        return (iMuted.load(std::memory_order_relaxed) & (1u << aChannel)) != 0u;
    }

    void stream::set_muted(std::uint32_t aChannel, bool aMuted)
    {
        if (aMuted)
            iMuted.fetch_or(1u << aChannel, std::memory_order_relaxed);
        else
            iMuted.fetch_and(~(1u << aChannel), std::memory_order_relaxed);
    }

    void stream::force_seek(std::uint64_t aPosition)
    {
        iSeekTarget.store(aPosition, std::memory_order_relaxed);
        iSeekRequest.fetch_add(1u, std::memory_order_release);
    }

    float stream::take_peak(std::uint32_t aChannel)
    {
        return iPeaks[aChannel].exchange(0.0f, std::memory_order_relaxed);
    }

    void stream::seek(std::uint64_t aPosition)
    {
        iNextPosition = aPosition;
        auto const frame = iTimeline.song_frame(aPosition, looping());
        if (!frame || iTimeline.rows.empty())
        {
            iReplayer.end();
            return;
        }
        auto const row = iTimeline.row_at(*frame);
        iReplayer.restore(iTimeline.snapshots[row]);
        iNextRow = row;
        // from the start of the row to the exact frame, without mixing
        iReplayer.render(nullptr, *frame - iTimeline.rows[row].frame);
    }

    bool stream::row_starting()
    {
        if (iNextRow >= iTimeline.rows.size())
        {
            // the end of the song: carry on into the loop (as the song itself would), taking the timing it had
            // the first time through so that every pass lasts exactly as long as the scan says
            if (!looping() || !iTimeline.loopRow)
                return false;
            iNextRow = *iTimeline.loopRow;
            iReplayer.restore_timing(iTimeline.snapshots[iNextRow]);
        }
        ++iNextRow;
        return true;
    }
}
