// audio_waveform.hpp
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
#include <neogfx/audio/i_audio_waveform.hpp>

#pragma once

namespace neogfx
{
    class audio_waveform : public reference_counted<i_audio_waveform>
    {
    public:
        audio_waveform(audio_sample_rate aSampleRate, float aAmplitude = 1.0f);
        audio_waveform(i_audio_device const& aDevice, float aAmplitude = 1.0f);
    public:
        ~audio_waveform();
    public:
        audio_sample_rate sample_rate() const final;
        void set_sample_rate(audio_sample_rate aSampleRate) final;
        i_audio_oscillator& create_oscillator(float aFrequency, float aAmplitude = 1.0f, oscillator_function aFunction = oscillator_function::Sine) final;
        i_audio_oscillator& create_oscillator(float aFrequency, float aAmplitude, std::function<float(float)> const& aFunction) final; ///< Note: not plugin-safe.
        i_audio_oscillator& add_oscillator(i_audio_oscillator& aOscillator) final;
        i_audio_oscillator& add_oscillator(i_ref_ptr<i_audio_oscillator> const& aOscillator) final;
        void remove_oscillator(i_audio_oscillator const& aOscillator) final;
    public:
        float amplitude() const final;
        void set_amplitude(float aAmplitude) final;
        bool has_envelope() const final;
        adsr_envelope const& envelope() final;
        void clear_envelope() final;
        void set_envelope(adsr_envelope const& aEnvelope) final;
    public:
        void generate(audio_sample_count aSampleCount, float* aOutputSamples) final;
    private:
        audio_sample_rate iSampleRate;
        std::vector<ref_ptr<i_audio_oscillator>> iOscillators;
        float iAmplitude;
        std::optional<adsr_envelope> iEnvelope;
    };
}