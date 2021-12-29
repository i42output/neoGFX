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
#include <neogfx/audio/audio_waveform.hpp>
#include <neogfx/audio/audio_oscillator.hpp>

namespace neogfx
{
    audio_waveform::audio_waveform(audio_sample_rate aSampleRate, float aAmplitude) :
        iSampleRate{ aSampleRate }, iAmplitude{ aAmplitude }
    {
    }

    audio_waveform::audio_waveform(i_audio_device const& aDevice, float aAmplitude) :
        audio_waveform{ aDevice.data_format().sampleRate, aAmplitude }
    {
    }
    
    audio_waveform::~audio_waveform()
    {
    }

    audio_sample_rate audio_waveform::sample_rate() const
    {
        return iSampleRate;
    }

    void audio_waveform::set_sample_rate(audio_sample_rate aSampleRate)
    {
        iSampleRate = aSampleRate;
        for (auto& oscillator : iOscillators)
            oscillator->set_sample_rate(iSampleRate);
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

    float audio_waveform::amplitude() const
    {
        return iAmplitude;
    }

    void audio_waveform::set_amplitude(float aAmplitude)
    {
        iAmplitude = aAmplitude;
    }

    bool audio_waveform::has_envelope() const
    {
        return iEnvelope != std::nullopt;
    }

    adsr_envelope const& audio_waveform::envelope()
    {
        return iEnvelope.value();
    }

    void audio_waveform::clear_envelope()
    {
        iEnvelope = std::nullopt;
    }

    void audio_waveform::set_envelope(adsr_envelope const& aEnvelope)
    {
        iEnvelope = aEnvelope;
    }

    void audio_waveform::generate(audio_sample_count aSampleCount, float* aOutputSamples)
    {
        std::fill(aOutputSamples, aOutputSamples + aSampleCount, 0.0f);

        for (auto const& o : iOscillators)
        {
            thread_local std::vector<float> componentResult;
            componentResult.resize(aSampleCount);

            o->generate(aSampleCount, componentResult.data());

            auto outputSample = aOutputSamples;
            for (auto sampleComponent : componentResult)
                *(outputSample++) += (sampleComponent * amplitude());
        }
    }
}