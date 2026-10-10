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

#pragma once

#include <mod_tracker/mod_tracker.hpp>

#include <array>
#include <atomic>
#include <memory>
#include <vector>

#include <neogfx/audio/audio_bitstream.hpp>

#include <mod_tracker/module.hpp>
#include <mod_tracker/replayer.hpp>

namespace mod_tracker
{
    // A module as an audio bitstream: the audio device calls generate_from() on its own thread and the
    // song is mixed there and then, a buffer at a time. It has no length (it plays until stopped) and its
    // frame index is the playback position, so the sequencer's audio_playback clip positions it like any
    // other bitstream. Unlike most bitstreams it can only be playing once at a time: it has the state of
    // the song it is in the middle of.
    class module_bitstream : public ng::audio_bitstream<ng::i_audio_bitstream>
    {
    public:
        module_bitstream(std::shared_ptr<module_data const> aModule, std::shared_ptr<timeline const> aTimeline, render_options const& aOptions);
    public:
        ng::audio_frame_count length() const override;
        void generate(ng::audio_channel aChannel, ng::audio_frame_count aFrameCount, float aGain, float* aOutputFrames) override;
        void generate_from(ng::audio_channel aChannel, ng::audio_frame_index aFrameFrom, ng::audio_frame_count aFrameCount, float aGain, float* aOutputFrames) override;
    public:
        mod_tracker::stream& song_stream();
        // the last aFrames frames of the left (0) or right (1) channel of the output, ending at (or as close as
        // possible to) playback position aEnd; called from the GUI thread (only) while the audio thread plays
        bool capture(std::uint32_t aChannel, std::uint64_t aEnd, float* aSamples, std::uint64_t aFrames);
    private:
        // the song is mixed this many frames at a time
        static constexpr ng::audio_frame_count MIX_FRAMES = 1024u;
        // the output is passed to the GUI through a single producer, single consumer FIFO of this many blocks,
        // one block per mix; the GUI keeps this many frames of it (both powers of two)
        static constexpr std::uint64_t CAPTURE_BLOCKS = 32u;
        static constexpr std::uint64_t HISTORY_FRAMES = 16384u;
        struct capture_block
        {
            std::uint64_t position;
            std::uint64_t frames;
            std::array<float, MIX_FRAMES * 2u> samples;
        };
    private:
        std::shared_ptr<module_data const> iModule;
        std::shared_ptr<timeline const> iTimeline;
        mod_tracker::stream iStream;
        std::vector<float> iMix;
        ng::audio_frame_index iCursor = 0u;
        // the FIFO: the blocks are plain data; the audio thread fills the blocks between iCaptureWritten and
        // iCaptureRead + CAPTURE_BLOCKS and the GUI thread empties those between iCaptureRead and
        // iCaptureWritten, and each hands blocks over to the other with a release store of its own count
        // (counts of blocks, which only ever go up), so the two never touch the same block at the same time
        std::array<capture_block, CAPTURE_BLOCKS> iCaptureBlocks = {};
        std::atomic<std::uint64_t> iCaptureWritten = 0u;
        std::atomic<std::uint64_t> iCaptureRead = 0u;
        // the GUI thread's own: the output it has taken from the FIFO, a ring of stereo frames indexed by
        // playback position; the end of what it holds and the start of the run of consecutive positions it ends
        std::array<float, HISTORY_FRAMES * 2u> iHistory = {};
        std::uint64_t iHistoryStart = 0u;
        std::uint64_t iHistoryEnd = 0u;
    };
}
