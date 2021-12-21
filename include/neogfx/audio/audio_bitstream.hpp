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
#include <neogfx/audio/i_audio_bitstream.hpp>

#pragma once

namespace neogfx
{
    template <typename Interface>
    class audio_bitstream : public reference_counted<Interface>
    {
    public:
        audio_bitstream(audio_sample_rate aSampleRate, float aAmplitude = 1.0f);
        audio_bitstream(i_audio_device const& aDevice, float aAmplitude = 1.0f);
    public:
        ~audio_bitstream();
    public:
        audio_sample_rate sample_rate() const final;
        void set_sample_rate(audio_sample_rate aSampleRate) override;
    public:
        float amplitude() const final;
        void set_amplitude(float aAmplitude) final;
        bool has_envelope() const final;
        adsr_envelope const& envelope() final;
        void clear_envelope() final;
        void set_envelope(adsr_envelope const& aEnvelope) final;
    private:
        audio_sample_rate iSampleRate;
        float iAmplitude;
        std::optional<adsr_envelope> iEnvelope;
    };
}