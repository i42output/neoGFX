// audio_playback.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>

#include <neolib/ecs/chrono.hpp>

#include <neogfx/audio/i_audio_bitstream.hpp>
#include <neogfx/audio/i_audio_device.hpp>
#include <neogfx/game/i_sequencer.hpp>

namespace neogfx
{
    // A sequencer clip payload that plays an audio bitstream while the playhead is inside the clip:
    //
    //     sequencer.emplace_clip<audio_playback>(track, time_interval{ 0.2 }, device, *sound);
    //
    // The sequencer calls advance() once per update with the offset into the clip, so playback starts
    // when the playhead first arrives and is then left alone; a discontinuity - a rewind, a seek, or a
    // frame that stepped further than the tolerance - restarts it at the offset seeked to. The device
    // and the bitstream must outlive the clip.
    //
    // The sequencer does not report a playhead that has left the clip, so a playback started here runs
    // to the end of the bitstream even if the playhead seeks away; call stop() via the clip's payload
    // if that matters.
    class audio_playback
    {
    public:
        // a frame that advances by more than this is treated as a seek rather than as playback
        static constexpr game::sequencer_duration DEFAULT_TOLERANCE = neolib::ecs::chrono::to_flicks(0.25).count();
    public:
        audio_playback(i_audio_device& aDevice, i_audio_bitstream& aBitstream, game::sequencer_duration aTolerance = DEFAULT_TOLERANCE) :
            iDevice{ aDevice }, iBitstream{ aBitstream }, iTolerance{ aTolerance }
        {
        }
        ~audio_playback()
        {
            stop();
        }
        audio_playback(audio_playback const&) = delete;
        audio_playback& operator=(audio_playback const&) = delete;
    public:
        // the bitstream's own length, as a clip duration; a clip given no duration of its own gets this one
        neolib::ecs::time_interval duration() const
        {
            return duration_of(iBitstream);
        }
        static neolib::ecs::time_interval duration_of(i_audio_bitstream const& aBitstream)
        {
            return neolib::ecs::time_interval{
                static_cast<double>(aBitstream.length()) / static_cast<double>(aBitstream.sample_rate()) };
        }
    public:
        void advance(game::sequencer_offset aPosition)
        {
            auto const continuous = (iPlayback != 0ULL && aPosition >= iPosition && aPosition - iPosition <= iTolerance);
            iPosition = aPosition;
            if (continuous)
                return;
            auto const from = static_cast<audio_frame_index>(
                neolib::ecs::chrono::to_seconds(neolib::ecs::chrono::flicks{ aPosition }) * static_cast<double>(iBitstream.sample_rate()));
            if (iBitstream.length() != 0ULL && from >= iBitstream.length())
            {
                // the playhead is at or past the end of the audio, so there is nothing left to start; anything
                // still playing is left to finish on its own rather than being cut off by the clip ending
                iPlayback = 0ULL;
                return;
            }
            stop();
            iPlayback = iDevice.play(iBitstream, from, i_audio_device::duration_of(iBitstream, from));
        }
        void stop()
        {
            if (iPlayback != 0ULL)
            {
                iDevice.stop(iPlayback);
                iPlayback = 0ULL;
            }
        }
    private:
        i_audio_device& iDevice;
        i_audio_bitstream& iBitstream;
        game::sequencer_duration iTolerance;
        game::sequencer_offset iPosition = 0;
        audio_playback_id iPlayback = 0ULL;
    };
}
