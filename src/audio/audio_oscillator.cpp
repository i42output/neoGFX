// audio_oscillator.hpp
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

#include <neogfx/core/numerical.hpp>
#include <neogfx/audio/audio_oscillator.hpp>

namespace neogfx
{
    audio_oscillator::audio_oscillator(audio_sample_rate aSampleRate, float aFrequency, float aAmplitude, oscillator_function aFunction) :
        audio_bitstream{ aSampleRate, aAmplitude}, iFrequency{ aFrequency }, iFunction{ aFunction }
    {
    }

    audio_oscillator::audio_oscillator(audio_sample_rate aSampleRate, float aFrequency, float aAmplitude, std::function<float(float)> const& aFunction) :
        audio_bitstream{ aSampleRate, aAmplitude }, iFrequency{ aFrequency }, iFunction{ oscillator_function::Custom }, iCustomFunction{ aFunction }
    {
    }

    float audio_oscillator::frequency() const
    {
        return iFrequency;
    }

    void audio_oscillator::set_frequency(float aFrequency)
    {
        iFrequency = aFrequency;
        iCursor = 0ULL;
    }

    oscillator_function audio_oscillator::function() const
    {
        return iFunction;
    }

    void audio_oscillator::set_function(oscillator_function aFunction)
    {
        iFunction = aFunction;
        if (iFunction != oscillator_function::Custom)
            iCustomFunction = nullptr;
        iCursor = 0ULL;
    }

    void audio_oscillator::set_function(std::function<float(float)> const& aFunction)
    {
        iFunction = oscillator_function::Custom;
        iCustomFunction = aFunction;
        iCursor = 0ULL;
    }

    audio_frame_count audio_oscillator::length() const
    {
        return sample_rate();
    }

    void audio_oscillator::generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames)
    {
        generate_from(aChannel, iCursor, aFrameCount, aOutputFrames);
    }

    void audio_oscillator::generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames)
    {
        iCursor = static_cast<audio_sample_index>(aFrameFrom);

        // todo: multiple channels

        switch (function())
        {
        case oscillator_function::Custom:
            // todo
            break;
        case oscillator_function::Sine:
            for (auto cursor = iCursor; cursor < iCursor + static_cast<audio_sample_count>(aFrameCount); ++cursor)
            {
                auto x = static_cast<float>(cursor) / sample_rate() * math::two_pi<float>() * frequency();
                *(aOutputFrames++) = std::sin(x) * amplitude();
            }
            break;
        case oscillator_function::Square:
            // todo
            break;
        case oscillator_function::Triangle:
            // todo
            break;
        case oscillator_function::Sawtooth:
            // todo
            break;
        default:
            break;
        }

        iCursor += static_cast<audio_sample_count>(aFrameCount);
    }
}