// audio.cpp
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

#include <neogfx/audio/audio.hpp>
#include <neogfx/audio/audio_device.hpp>

#define MINIAUDIO_IMPLEMENTATION
#ifdef _WIN32
#define MA_ENABLE_WASAPI
#endif
#include "3rdparty/miniaudio/miniaudio.h"

template<> neogfx::i_audio& services::start_service<neogfx::i_audio>()
{
    static neogfx::audio sAudio;
    return sAudio;
}

namespace neogfx
{
    inline audio_device_type to_audio_device_type(ma_device_type aType)
    {
        switch (aType)
        {
        case ma_device_type_playback:
            return audio_device_type::Playback;
        case ma_device_type_capture:
            return audio_device_type::Capture;
        case ma_device_type_duplex:
            return audio_device_type::Duplex;
        case ma_device_type_loopback:
            return audio_device_type::Loopback;
        default:
            throw std::logic_error("neogfx::to_audio_device_type");
        }
    }

    inline audio_sample_format to_audio_sample_format(ma_format aFormat)
    {
        switch (aFormat)
        {
        case ma_format_unknown:
            return audio_sample_format::Unknown;
        case ma_format_u8:
            return audio_sample_format::U8;
        case ma_format_s16:
            return audio_sample_format::S16;
        case ma_format_s24:
            return audio_sample_format::S24;
        case ma_format_s32:
            return audio_sample_format::S32;
        case ma_format_f32:
            return audio_sample_format::F32;
        default:
            throw std::logic_error("neogfx::to_audio_sample_format");
        }
    }
        
    audio::audio() : iContext{ ma_context{} }
    {
        if (ma_context_init(NULL, 0, NULL, std::any_cast<ma_context>(&iContext)) != MA_SUCCESS)
            throw std::runtime_error{ "neogfx::audio::audio" };

        auto callback = [](ma_context* pContext, ma_device_type deviceType, const ma_device_info* pInfo, void* pUserData) -> ma_bool32
        {
            vector<audio_data_format> dataFormats;
            for (std::size_t dataFormatIndex = 0; dataFormatIndex < pInfo->nativeDataFormatCount; ++dataFormatIndex)
                dataFormats.emplace_back(
                    to_audio_sample_format(pInfo->nativeDataFormats[dataFormatIndex].format),
                    pInfo->nativeDataFormats[dataFormatIndex].channels,
                    static_cast<audio_sample_rate>(pInfo->nativeDataFormats[dataFormatIndex].sampleRate));
            auto& audioObj = *static_cast<audio*>(pUserData);
            audioObj.iDeviceInfos.emplace_back(pInfo->id, to_audio_device_type(deviceType), string{ pInfo->name }, pInfo->isDefault, dataFormats);
            return MA_TRUE;
        };

        ma_context_enumerate_devices(std::any_cast<ma_context>(&iContext), callback, this);
    }

    audio::~audio()
    {
    }

    i_vector<i_audio_device_info> const& audio::available_devices()
    {
        return iDeviceInfos;
    }

    i_audio_device& audio::create_device(i_audio_device_info const& aDeviceInfo, audio_data_format const& aDataFormat)
    {
        iDevices.push_back(make_ref<audio_device>(std::any_cast<ma_context>(&iContext), aDeviceInfo, aDataFormat));
        return *iDevices.back();
    }

    i_audio_device& audio::create_playback_device(audio_data_format const& aDataFormat)
    {
        auto deviceInfo = find_device(audio_device_type::Playback, aDataFormat);
        if (deviceInfo != nullptr)
            return create_device(*deviceInfo, aDataFormat);
        else
            throw std::runtime_error("neogfx::audo::create_playback_device");
    }

    i_audio_device& audio::create_capture_device(audio_data_format const& aDataFormat)
    {
        auto deviceInfo = find_device(audio_device_type::Capture, aDataFormat);
        if (deviceInfo != nullptr)
            return create_device(*deviceInfo, aDataFormat);
        else
            throw std::runtime_error("neogfx::audo::create_playback_device");
    }

    void audio::destroy_device(i_audio_device& aDevice)
    {
    }

    i_audio_instrument_atlas& audio::instrument_atlas()
    {
        return iInstrumentAtlas;
    }

    i_audio_device_info const* audio::find_device(audio_device_type aType, audio_data_format const& aDataFormat)
    {
        i_audio_device_info const* bestMatch = nullptr;
        for (auto const& deviceInfo : iDeviceInfos)
        {
            if (deviceInfo.type() != aType)
                continue;
            if (deviceInfo.data_formats().empty())
            {
                if (bestMatch == nullptr || !bestMatch->is_default() && deviceInfo.is_default())
                    bestMatch = &deviceInfo;
            }
            else
            {
                for (auto const& dataFormat : deviceInfo.data_formats())
                {
                    if (dataFormat.sampleRate >= aDataFormat.sampleRate)
                    {
                        if (bestMatch == nullptr || !bestMatch->is_default() && deviceInfo.is_default())
                            bestMatch = &deviceInfo;
                    }
                }
            }
        }
        return bestMatch;
    }

}