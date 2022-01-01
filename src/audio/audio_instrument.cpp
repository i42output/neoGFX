// audio_instrument.cpp
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
#include <neogfx/audio/audio_instrument.hpp>

namespace neogfx
{
    audio_instrument::audio_instrument(audio_sample_rate aSampleRate, neogfx::instrument aInstrument, float aAmplitude) :
        audio_bitstream{ aSampleRate, aAmplitude }, iInstrument{ aInstrument }
    {
    }

    audio_instrument::audio_instrument(i_audio_device const& aDevice, neogfx::instrument aInstrument, float aAmplitude) :
        audio_instrument{ aDevice.data_format().sampleRate, aInstrument, aAmplitude }
    {
    }

    audio_instrument::time_point audio_instrument::play_note(note aNote, std::chrono::duration<double> const& aDuration)
    {
        // todo
        return 0ULL;
    }

    audio_instrument::time_point audio_instrument::play_note(time_point aWhen, note aNote, std::chrono::duration<double> const& aDuration)
    {
        // todo
        return 0ULL;
    }

    audio_instrument::time_point audio_instrument::play_silence(std::chrono::duration<double> const& aDuration)
    {
        // todo
        return 0ULL;
    }

    audio_instrument::time_point audio_instrument::play_silence(time_point aWhen, std::chrono::duration<double> const& aDuration)
    {
        // todo
        return 0ULL;
    }

    audio_frame_count audio_instrument::length() const
    {
        // todo
        return 0ULL;
    }

    void audio_instrument::generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames)
    {
        // todo
    }

    void audio_instrument::generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames)
    {
        // todo
    }
}