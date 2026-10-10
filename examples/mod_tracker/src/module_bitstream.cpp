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

#include <mod_tracker/mod_tracker.hpp>

#include <algorithm>

#include <mod_tracker/module_bitstream.hpp>

namespace mod_tracker
{
    module_bitstream::module_bitstream(std::shared_ptr<module_data const> aModule, std::shared_ptr<timeline const> aTimeline, render_options const& aOptions) :
        audio_bitstream{ aOptions.sampleRate },
        iModule{ std::move(aModule) },
        iTimeline{ std::move(aTimeline) },
        iStream{ *iModule, *iTimeline, aOptions },
        // the mix buffer is allocated up front so that the audio thread never allocates
        iMix(MIX_FRAMES * 2u)
    {
    }

    ng::audio_frame_count module_bitstream::length() const
    {
        return 0u;
    }

    void module_bitstream::generate(ng::audio_channel aChannel, ng::audio_frame_count aFrameCount, float aGain, float* aOutputFrames)
    {
        generate_from(aChannel, iCursor, aFrameCount, aGain, aOutputFrames);
        iCursor += aFrameCount;
    }

    void module_bitstream::generate_from(ng::audio_channel aChannel, ng::audio_frame_index aFrameFrom, ng::audio_frame_count aFrameCount, float aGain, float* aOutputFrames)
    {
        auto const outputChannels = ng::channel_count(aChannel);
        if (outputChannels == 0u)
            return;
        auto const gain = amplitude() * aGain;
        // the mix is stereo; it goes to the front left and right channels of the output if it has them, and
        // is folded down into every channel if it hasn't
        bool const stereo =
            (aChannel & ng::audio_channel::FrontLeft) != ng::audio_channel::None &&
            (aChannel & ng::audio_channel::FrontRight) != ng::audio_channel::None;
        auto const left = stereo ? ng::channel_index(aChannel, ng::audio_channel::FrontLeft) : 0u;
        auto const right = stereo ? ng::channel_index(aChannel, ng::audio_channel::FrontRight) : 0u;
        for (ng::audio_frame_count done = 0u; done < aFrameCount;)
        {
            auto const frames = std::min(aFrameCount - done, MIX_FRAMES);
            std::fill(iMix.begin(), std::next(iMix.begin(), static_cast<std::ptrdiff_t>(frames * 2u)), 0.0f);
            iStream.render(aFrameFrom + done, iMix.data(), frames);
            // the mix goes into the next block of the FIFO for the GUI if there is one free; if the GUI has
            // fallen behind, the GUI goes without
            auto const written = iCaptureWritten.load(std::memory_order_relaxed);
            auto* const capture = written - iCaptureRead.load(std::memory_order_acquire) < CAPTURE_BLOCKS ?
                &iCaptureBlocks[written & (CAPTURE_BLOCKS - 1u)] : nullptr;
            auto output = aOutputFrames + done * outputChannels;
            for (ng::audio_frame_count frame = 0u; frame < frames; ++frame, output += outputChannels)
            {
                auto const l = iMix[frame * 2u] * gain;
                auto const r = iMix[frame * 2u + 1u] * gain;
                if (capture != nullptr)
                {
                    capture->samples[frame * 2u] = l;
                    capture->samples[frame * 2u + 1u] = r;
                }
                if (stereo)
                {
                    output[left] += l;
                    output[right] += r;
                }
                else
                    for (std::uint64_t channel = 0u; channel < outputChannels; ++channel)
                        output[channel] += (l + r) * 0.5f;
            }
            if (capture != nullptr)
            {
                capture->position = aFrameFrom + done;
                capture->frames = frames;
                iCaptureWritten.store(written + 1u, std::memory_order_release);
            }
            done += frames;
        }
    }

    stream& module_bitstream::song_stream()
    {
        return iStream;
    }

    bool module_bitstream::capture(std::uint32_t aChannel, std::uint64_t aEnd, float* aSamples, std::uint64_t aFrames)
    {
        if (aChannel > 1u || aFrames > HISTORY_FRAMES)
            return false;
        // take what the audio thread has output since the last call from the FIFO into the history, and hand
        // the blocks back; a block that doesn't follow on from the one before (a seek, or blocks the audio
        // thread dropped) starts a new run
        auto const written = iCaptureWritten.load(std::memory_order_acquire);
        auto const read = iCaptureRead.load(std::memory_order_relaxed);
        for (auto next = read; next != written; ++next)
        {
            auto const& block = iCaptureBlocks[next & (CAPTURE_BLOCKS - 1u)];
            if (block.position != iHistoryEnd)
                iHistoryStart = block.position;
            for (std::uint64_t frame = 0u; frame < block.frames; ++frame)
            {
                auto const slot = ((block.position + frame) & (HISTORY_FRAMES - 1u)) * 2u;
                iHistory[slot] = block.samples[frame * 2u];
                iHistory[slot + 1u] = block.samples[frame * 2u + 1u];
            }
            iHistoryEnd = block.position + block.frames;
        }
        if (written != read)
            iCaptureRead.store(written, std::memory_order_release);
        if (iHistoryEnd == iHistoryStart)
            return false;
        // the playhead and the audio device's cursor are close but not the same: the device mixes a little
        // ahead of what is heard; the window ends at the playhead unless the playhead is somewhere the history
        // doesn't cover (just after a seek), when it ends at the latest output
        auto const oldest = iHistoryEnd > HISTORY_FRAMES - aFrames ? iHistoryEnd - (HISTORY_FRAMES - aFrames) : 0u;
        auto const end = aEnd <= iHistoryEnd && aEnd >= oldest ? aEnd : iHistoryEnd;
        for (std::uint64_t index = 0u; index < aFrames; ++index)
        {
            // frames before the window's run of output began (or before the song) are silence
            auto const frame = end - aFrames + index;
            aSamples[index] = end < aFrames - index || frame < iHistoryStart ? 0.0f :
                iHistory[(frame & (HISTORY_FRAMES - 1u)) * 2u + aChannel];
        }
        return true;
    }
}
