// i_audio_oscillator.hpp
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

#include <neogfx/audio/audio_primitives.hpp>

#pragma once

namespace neogfx
{
    enum class oscillator_function : std::uint32_t
    {
        Custom      = 0,
        Sine        = 1,
        Square      = 2,
        Triangle    = 3,
        Sawtooth    = 4
    };

    class i_audio_oscillator : public i_reference_counted
    {
    public:
        typedef i_audio_oscillator abstract_type;
    public:
        virtual ~i_audio_oscillator() = default;
    public:
        virtual audio_sample_rate sample_rate() const = 0;
        virtual void set_sample_rate(audio_sample_rate aSampleRate) = 0;
        virtual float frequency() const = 0;
        virtual void set_frequency(float aFrequency) = 0;
        virtual float amplitude() const = 0;
        virtual void set_amplitude(float aAmplitude) = 0;
        virtual oscillator_function function() const = 0;
        virtual void set_function(oscillator_function aFunction) = 0;
        virtual void set_function(std::function<float(float)> const& aFunction) = 0; ///< Note: not plugin-safe.
    public:
        virtual void generate(audio_sample_count aSampleCount, float* aOutputSamples) = 0;
        virtual void generate_from(audio_sample_index aSampleFrom, audio_sample_count aSampleCount, float* aOutputSamples) = 0;
    };
}