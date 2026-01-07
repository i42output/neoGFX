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

#pragma once

#include <neogfx/neogfx.hpp>

#include <neogfx/audio/audio_bitstream.hpp>
#include <neogfx/audio/i_audio_oscillator.hpp>

namespace neogfx
{
    class audio_oscillator : public audio_bitstream<i_audio_oscillator>
    {
    public:
        audio_oscillator(audio_sample_rate aSampleRate, float aFrequency, float aAmplitude = 1.0f, oscillator_function aFunction = oscillator_function::Sine);
        audio_oscillator(audio_sample_rate aSampleRate, float aFrequency, float aAmplitude, std::function<float(float)> const& aFunction);
    public:
        float frequency() const final;
        void set_frequency(float aFrequency) final;
        oscillator_function function() const final;
        void set_function(oscillator_function aFunction) final;
        void set_function(std::function<float(float)> const& aFunction) final; ///< Note: not plugin-safe.
    public:
        audio_frame_count length() const final;
        void generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames) final;
        void generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames) final;
    private:
        float iFrequency;
        oscillator_function iFunction;
        std::function<float(float)> iCustomFunction;
        audio_sample_index iCursor = 0ULL;
    };
}