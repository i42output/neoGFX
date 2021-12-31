// audio_bitstream.cpp
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
#include <neogfx/audio/audio_bitstream.hpp>
#include <neogfx/audio/i_audio_waveform.hpp>

namespace neogfx
{
    template <typename Interface>
    audio_bitstream<Interface>::audio_bitstream(audio_sample_rate aSampleRate, float aAmplitude) :
        iSampleRate{ aSampleRate }, iAmplitude{ aAmplitude }
    {
    }

    template <typename Interface>
    audio_bitstream<Interface>::audio_bitstream(i_audio_device const& aDevice, float aAmplitude) :
        audio_bitstream{ aDevice.data_format().sampleRate, aAmplitude }
    {
    }
    
    template <typename Interface>
    audio_bitstream<Interface>::~audio_bitstream()
    {
    }

    template <typename Interface>
    audio_sample_rate audio_bitstream<Interface>::sample_rate() const
    {
        return iSampleRate;
    }

    template <typename Interface>
    void audio_bitstream<Interface>::set_sample_rate(audio_sample_rate aSampleRate)
    {
        iSampleRate = aSampleRate;
    }

    
    template <typename Interface>
    float audio_bitstream<Interface>::amplitude() const
    {
        return iAmplitude;
    }

    template <typename Interface>
    void audio_bitstream<Interface>::set_amplitude(float aAmplitude)
    {
        iAmplitude = aAmplitude;
    }

    template <typename Interface>
    bool audio_bitstream<Interface>::has_envelope() const
    {
        return iEnvelope != std::nullopt;
    }

    template <typename Interface>
    adsr_envelope const& audio_bitstream<Interface>::envelope()
    {
        return iEnvelope.value();
    }

    template <typename Interface>
    void audio_bitstream<Interface>::clear_envelope()
    {
        iEnvelope = std::nullopt;
    }

    template <typename Interface>
    void audio_bitstream<Interface>::set_envelope(adsr_envelope const& aEnvelope)
    {
        iEnvelope = aEnvelope;
    }

    template class audio_bitstream<i_audio_waveform>;
}