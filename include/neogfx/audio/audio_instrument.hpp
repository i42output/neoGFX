// audio_instrument.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>
#include <neogfx/audio/i_audio_device.hpp>
#include <neogfx/audio/i_audio_instrument.hpp>
#include <neogfx/audio/audio_bitstream.hpp>

#pragma once

namespace neogfx
{
    class audio_instrument : public audio_bitstream<i_audio_instrument>
    {
    public:
        audio_instrument(audio_sample_rate aSampleRate, neogfx::instrument aInstrument, float aAmplitude = 1.0f);
        audio_instrument(i_audio_device const& aDevice, neogfx::instrument aInstrument, float aAmplitude = 1.0f);
    public:
        time_point play_note(note aNote, std::chrono::duration<double> const& aDuration, float aAmplitude = 1.0f) final;
        time_point play_note(time_point aWhen, note aNote, std::chrono::duration<double> const& aDuration, float aAmplitude = 1.0f) final;
        time_point play_silence(std::chrono::duration<double> const& aDuration) final;
        time_point play_silence(time_point aWhen, std::chrono::duration<double> const& aDuration) final;
    public:
        audio_frame_count length() const final;
        void generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames) final;
        void generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames) final;
    private:
        neogfx::instrument iInstrument;
        time_point iInputCursor = 0ULL;
        time_point iOutputCursor = 0ULL;
        struct part
        {
            std::optional<note> note;
            std::optional<float> amplitude;
            time_interval start;
            time_interval duration;
        };
        std::vector<part> iComposition;
    };
}