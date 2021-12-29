// i_audio_waveform.hpp
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
#include <neogfx/audio/i_audio_oscillator.hpp>

#pragma once

namespace neogfx
{
    class i_audio_waveform : public i_reference_counted
    {
    public:
        typedef i_audio_waveform abstract_type;
    public:
        virtual ~i_audio_waveform() = default;
    public:
        virtual audio_sample_rate sample_rate() const = 0;
        virtual void set_sample_rate(audio_sample_rate aSampleRate) = 0;
        virtual i_audio_oscillator& create_oscillator(float aFrequency, float aAmplitude = 1.0f, oscillator_function aFunction = oscillator_function::Sine) = 0;
        virtual i_audio_oscillator& create_oscillator(float aFrequency, float aAmplitude, std::function<float(float)> const& aFunction) = 0; ///< Note: not plugin-safe.
        virtual i_audio_oscillator& add_oscillator(i_audio_oscillator& aOscillator) = 0;
        virtual i_audio_oscillator& add_oscillator(i_ref_ptr<i_audio_oscillator> const& aOscillator) = 0;
        virtual void remove_oscillator(i_audio_oscillator const& aOscillator) = 0;
    public:
        virtual float amplitude() const = 0;
        virtual void set_amplitude(float aAmplitude) = 0;
        virtual bool has_envelope() const = 0;
        virtual adsr_envelope const& envelope() = 0;
        virtual void clear_envelope() = 0;
        virtual void set_envelope(adsr_envelope const& aEnvelope) = 0;
    public:
        virtual void generate(audio_sample_count aSampleCount, float* aOutputSamples) = 0;
    };
}