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

#include <neogfx/audio/i_audio.hpp>
#include <neogfx/audio/i_audio_instrument_atlas.hpp>
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

    audio_instrument::time_point audio_instrument::play_note(note aNote, std::chrono::duration<double> const& aDuration, float aAmplitude)
    {
        return play_note(iInputCursor, aNote, aDuration);
    }

    audio_instrument::time_point audio_instrument::play_note(std::chrono::duration<double> const& aWhen, note aNote, std::chrono::duration<double> const& aDuration, float aAmplitude)
    {
        return play_note(static_cast<time_point>(aWhen.count() * sample_rate()), aNote, aDuration, aAmplitude);
    }
        
    audio_instrument::time_point audio_instrument::play_note(time_point aWhen, note aNote, std::chrono::duration<double> const& aDuration, float aAmplitude)
    {
        auto noteLength = service<i_audio>().instrument_atlas().instrument(iInstrument, sample_rate(), aNote).length();

        iComposition.emplace_back(aNote, noteLength, aAmplitude, aWhen, static_cast<time_interval>(aDuration.count() * sample_rate()));
        iInputCursor = aWhen + iComposition.back().duration;
        return iInputCursor;
    }

    audio_instrument::time_point audio_instrument::rest(std::chrono::duration<double> const& aDuration)
    {
        iComposition.emplace_back(std::nullopt, std::nullopt, std::nullopt, iInputCursor, static_cast<time_interval>(aDuration.count() * sample_rate()));
        iInputCursor += iComposition.back().duration;
        return iInputCursor;
    }

    audio_frame_count audio_instrument::length() const
    {
        if (!iComposition.empty())
            return iComposition.back().start + iComposition.back().duration;
        else
            return 0ULL;
    }

    void audio_instrument::generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames)
    {
        generate_from(aChannel, iOutputCursor, aFrameCount, aOutputFrames);
    }

    void audio_instrument::generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames)
    {
        for (auto next = iComposition.begin(); next != iComposition.end(); ++next)
        {
            if (aFrameFrom < next->start || aFrameFrom > next->start + next->noteLength.value())
                continue;
            auto pos = aFrameFrom - next->start;
            auto count = std::min(next->noteLength.value() - pos, aFrameCount);
            thread_local std::vector<float> buffer;
            buffer.resize(count);
            if (next->note)
                service<i_audio>().instrument_atlas().instrument(iInstrument, sample_rate(), next->note.value()).generate_from(
                    aChannel, pos, count, buffer.data());
            auto output = aOutputFrames;
            for (auto const& sample : buffer)
                for (int channel = 0; channel < channel_count(aChannel); ++channel)
                    (*output++) += (sample * next->amplitude.value() * apply_envelope(pos + &sample - &buffer[0], next->duration));
        }
        iOutputCursor += aFrameCount;
    }
}