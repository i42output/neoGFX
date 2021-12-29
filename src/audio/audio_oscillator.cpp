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
        iSampleRate{ aSampleRate }, iFrequency{ aFrequency }, iAmplitude{ aAmplitude }, iFunction{ aFunction }
    {
    }

    audio_oscillator::audio_oscillator(audio_sample_rate aSampleRate, float aFrequency, float aAmplitude, std::function<float(float)> const& aFunction) :
        iSampleRate{ aSampleRate }, iFrequency{ aFrequency }, iAmplitude{ aAmplitude }, iFunction{ oscillator_function::Custom }, iCustomFunction{ aFunction }
    {
    }

    audio_sample_rate audio_oscillator::sample_rate() const
    {
        return iSampleRate;
    }

    void audio_oscillator::set_sample_rate(audio_sample_rate aSampleRate)
    {
        iSampleRate = aSampleRate;
    }

    float audio_oscillator::frequency() const
    {
        return iFrequency;
    }

    void audio_oscillator::set_frequency(float aFrequency)
    {
        iFrequency = aFrequency;
    }

    float audio_oscillator::amplitude() const
    {
        return iAmplitude;
    }

    void audio_oscillator::set_amplitude(float aAmplitude)
    {
        iAmplitude = aAmplitude;
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
    }

    void audio_oscillator::set_function(std::function<float(float)> const& aFunction)
    {
        iFunction = oscillator_function::Custom;
        iCustomFunction = aFunction;
    }

    void audio_oscillator::generate(audio_sample_count aSampleCount, float* aOutputSamples)
    {
        switch (function())
        {
        case oscillator_function::Custom:
            // todo
            break;
        case oscillator_function::Sine:
            for (auto cursor = iCursor; cursor < iCursor + aSampleCount; ++cursor)
            {
                auto x = static_cast<float>(cursor) / sample_rate() * math::two_pi<float>() * frequency();
                *(aOutputSamples++) = std::sin(x) * amplitude();
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
        iCursor += aSampleCount;
    }
}