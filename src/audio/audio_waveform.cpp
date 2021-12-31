// audio_waveform.cpp
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
#include <bit>
#include <neogfx/audio/audio_waveform.hpp>
#include <neogfx/audio/audio_oscillator.hpp>

namespace neogfx
{
    audio_waveform::audio_waveform(audio_sample_rate aSampleRate, float aAmplitude) :
        audio_bitstream{ aSampleRate, aAmplitude }
    {
    }

    audio_waveform::audio_waveform(i_audio_device const& aDevice, float aAmplitude) :
        audio_waveform{ aDevice.data_format().sampleRate, aAmplitude }
    {
    }
    
    audio_waveform::~audio_waveform()
    {
    }

    void audio_waveform::set_sample_rate(audio_sample_rate aSampleRate)
    {
        audio_bitstream::set_sample_rate(aSampleRate);
        for (auto& oscillator : iOscillators)
            oscillator->set_sample_rate(sample_rate());
    }

    i_audio_oscillator& audio_waveform::create_oscillator(float aFrequency, float aAmplitude, oscillator_function aFunction)
    {
        return add_oscillator(make_ref<audio_oscillator>(sample_rate(), aFrequency, aAmplitude, aFunction));
    }

    i_audio_oscillator& audio_waveform::create_oscillator(float aFrequency, float aAmplitude, std::function<float(float)> const& aFunction)
    {
        return add_oscillator(make_ref<audio_oscillator>(sample_rate(), aFrequency, aAmplitude, aFunction));
    }
    
    i_audio_oscillator& audio_waveform::add_oscillator(i_audio_oscillator& aOscillator)
    {
        return add_oscillator(ref_ptr<i_audio_oscillator>{ ref_ptr<i_audio_oscillator>{}, &aOscillator});
    }

    i_audio_oscillator& audio_waveform::add_oscillator(i_ref_ptr<i_audio_oscillator> const& aOscillator)
    {
        iOscillators.push_back(aOscillator);
        return *iOscillators.back();
    }

    void audio_waveform::remove_oscillator(i_audio_oscillator const& aOscillator)
    {
        auto existing = std::find_if(iOscillators.begin(), iOscillators.end(), [&](auto const& o) { return o.ptr() == &aOscillator; });
        if (existing != iOscillators.end())
            iOscillators.erase(existing);
    }

    void audio_waveform::generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames)
    {
        std::fill(aOutputFrames, aOutputFrames + aFrameCount * channel_count(aChannel), 0.0f);

        for (auto const& o : iOscillators)
        {
            thread_local std::vector<float> componentResult;
            componentResult.resize(aFrameCount);

            o->generate(aFrameCount, componentResult.data());

            auto outputSample = aOutputFrames;
            for (auto sampleComponent : componentResult)
            {
                for (int channel = 0; channel < channel_count(aChannel); ++channel)
                    *(outputSample++) += (sampleComponent * amplitude());
            }
        }
    }
}