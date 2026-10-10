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
    private:
        std::shared_ptr<module_data const> iModule;
        std::shared_ptr<timeline const> iTimeline;
        mod_tracker::stream iStream;
        std::vector<float> iMix;
        ng::audio_frame_index iCursor = 0u;
    };
}
