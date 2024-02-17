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

#include <neogfx/audio/i_audio_oscillator.hpp>

#pragma once

namespace neogfx
{
    class audio_oscillator : public reference_counted<i_audio_oscillator>
    {
    public:
        audio_oscillator(audio_sample_rate aSampleRate, float aFrequency, float aAmplitude = 1.0f, oscillator_function aFunction = oscillator_function::Sine);
        audio_oscillator(audio_sample_rate aSampleRate, float aFrequency, float aAmplitude, std::function<float(float)> const& aFunction);
    public:
        audio_sample_rate sample_rate() const final;
        void set_sample_rate(audio_sample_rate aSampleRate) final;
        float frequency() const final;
        void set_frequency(float aFrequency) final;
        float amplitude() const final;
        void set_amplitude(float aAmplitude) final;
        oscillator_function function() const final;
        void set_function(oscillator_function aFunction) final;
        void set_function(std::function<float(float)> const& aFunction) final; ///< Note: not plugin-safe.
    public:
        void generate(audio_sample_count aSampleCount, float* aOutputSamples) final;
        void generate_from(audio_sample_index aSampleFrom, audio_sample_count aSampleCount, float* aOutputSamples) final;
    private:
        audio_sample_rate iSampleRate;
        float iFrequency;
        float iAmplitude;
        oscillator_function iFunction;
        std::function<float(float)> iCustomFunction;
        audio_sample_index iCursor = 0ULL;
    };
}