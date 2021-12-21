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
#include <neogfx/audio/audio_bitstream.hpp>

#pragma once

namespace neogfx
{
    class audio_waveform : public audio_bitstream<i_audio_waveform>
    {
    public:
        audio_waveform(audio_sample_rate aSampleRate, float aAmplitude = 1.0f);
        audio_waveform(i_audio_device const& aDevice, float aAmplitude = 1.0f);
    public:
        ~audio_waveform();
    public:
        void set_sample_rate(audio_sample_rate aSampleRate) override;
    public:
        i_audio_oscillator& create_oscillator(float aFrequency, float aAmplitude = 1.0f, oscillator_function aFunction = oscillator_function::Sine) final;
        i_audio_oscillator& create_oscillator(float aFrequency, float aAmplitude, std::function<float(float)> const& aFunction) final; ///< Note: not plugin-safe.
        i_audio_oscillator& add_oscillator(i_audio_oscillator& aOscillator) final;
        i_audio_oscillator& add_oscillator(i_ref_ptr<i_audio_oscillator> const& aOscillator) final;
        void remove_oscillator(i_audio_oscillator const& aOscillator) final;
    public:
        audio_frame_count length() const final;
        void generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames) final;
        void generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames) final;
    private:
        std::vector<ref_ptr<i_audio_oscillator>> iOscillators;
    };
}