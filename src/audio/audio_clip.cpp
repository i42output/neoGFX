// audio_clip.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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
#include <filesystem>

#include <neolib/io/uri.hpp>

#include <neogfx/app/i_resource_manager.hpp>
#include <neogfx/audio/audio_clip.hpp>

#ifdef _WIN32
#define MA_ENABLE_WASAPI
#endif
#include "3rdparty/miniaudio/miniaudio.h"

namespace neogfx
{
    namespace
    {
        constexpr std::size_t DECODE_CHUNK_FRAMES = 16384u;

        audio_channel to_audio_channel(ma_channel aChannel)
        {
            if (aChannel == MA_CHANNEL_NONE || aChannel > 64u)
                return audio_channel::None;
            return static_cast<audio_channel>(1ULL << (aChannel - 1u));
        }

        audio_channel channels_of(ma_decoder& aDecoder)
        {
            auto const channels = aDecoder.outputChannels;
            ma_channel channelMap[MA_MAX_CHANNELS] = {};
            ma_format format;
            ma_uint32 channelCount;
            ma_uint32 sampleRate;
            auto build = [&]()
                {
                    auto result = audio_channel::None;
                    for (ma_uint32 channel = 0u; channel < channels; ++channel)
                        result = result | to_audio_channel(channelMap[channel]);
                    return result;
                };

            auto result = audio_channel::None;
            if (ma_decoder_get_data_format(&aDecoder, &format, &channelCount, &sampleRate, channelMap, sizeof(channelMap) / sizeof(channelMap[0])) == MA_SUCCESS)
                result = build();

            // a channel mask can't represent a map with duplicate or unknown channels, so fall back to the standard map
            if (channel_count(result) != channels)
            {
                ma_channel_map_init_standard(ma_standard_channel_map_default, channelMap, sizeof(channelMap) / sizeof(channelMap[0]), channels);
                result = build();
            }

            return result;
        }

        void read_all_pcm_frames(ma_decoder& aDecoder, audio_data_format& aDataFormat, audio_channel& aChannels, std::vector<float>& aPcmFrames)
        {
            aDataFormat = audio_data_format{ audio_sample_format::F32, aDecoder.outputChannels, static_cast<audio_sample_rate>(aDecoder.outputSampleRate) };
            aChannels = channels_of(aDecoder);

            ma_uint64 lengthInFrames = 0ULL;
            if (ma_decoder_get_length_in_pcm_frames(&aDecoder, &lengthInFrames) == MA_SUCCESS)
                aPcmFrames.reserve(static_cast<std::size_t>(lengthInFrames) * aDecoder.outputChannels);

            std::vector<float> chunk(DECODE_CHUNK_FRAMES * aDecoder.outputChannels);
            for (;;)
            {
                ma_uint64 framesRead = 0ULL;
                auto const result = ma_decoder_read_pcm_frames(&aDecoder, &chunk[0], DECODE_CHUNK_FRAMES, &framesRead);
                aPcmFrames.insert(aPcmFrames.end(), chunk.begin(), std::next(chunk.begin(), static_cast<std::ptrdiff_t>(framesRead * aDecoder.outputChannels)));
                if (result != MA_SUCCESS || framesRead < DECODE_CHUNK_FRAMES)
                    break;
            }
        }
    }

    audio_clip::audio_clip(string const& aUri, audio_sample_rate aSampleRate) :
        iUri{ aUri }
    {
        load(aSampleRate);
    }

    audio_clip::audio_clip(void const* aData, std::size_t aDataSize, audio_sample_rate aSampleRate)
    {
        decode(aData, aDataSize, aSampleRate);
    }

    audio_clip::~audio_clip()
    {}

    i_string const& audio_clip::uri() const
    {
        return iUri;
    }

    audio_data_format const& audio_clip::data_format() const
    {
        return iDataFormat;
    }

    audio_channel audio_clip::channels() const
    {
        return iChannels;
    }

    audio_frame_count audio_clip::length() const
    {
        return iDataFormat.channels != 0u ? iPcmFrames.size() / iDataFormat.channels : 0ULL;
    }

    float const* audio_clip::cdata() const
    {
        return iPcmFrames.data();
    }

    void audio_clip::load(audio_sample_rate aSampleRate)
    {
        auto const uri = iUri.to_std_string();

        // a scheme-less URI naming an existing file is decoded directly; anything else goes through the resource manager
        if (neolib::uri{ uri }.scheme().empty() && std::filesystem::exists(uri))
        {
            ma_decoder decoder;
            ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0u, static_cast<ma_uint32>(aSampleRate));
            if (ma_decoder_init_file(uri.c_str(), &config, &decoder) != MA_SUCCESS)
                throw failed_to_load_clip(uri);
            try
            {
                read_all_pcm_frames(decoder, iDataFormat, iChannels, iPcmFrames);
            }
            catch (...)
            {
                ma_decoder_uninit(&decoder);
                throw;
            }
            ma_decoder_uninit(&decoder);
        }
        else
        {
            auto resource = service<i_resource_manager>().load_resource(iUri);
            if (!resource->available())
                throw failed_to_load_clip(uri);
            decode(resource->cdata(), resource->size(), aSampleRate);
        }
    }

    void audio_clip::decode(void const* aData, std::size_t aDataSize, audio_sample_rate aSampleRate)
    {
        ma_decoder decoder;
        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0u, static_cast<ma_uint32>(aSampleRate));
        if (ma_decoder_init_memory(aData, aDataSize, &config, &decoder) != MA_SUCCESS)
            throw failed_to_decode_clip(iUri.to_std_string());
        try
        {
            read_all_pcm_frames(decoder, iDataFormat, iChannels, iPcmFrames);
        }
        catch (...)
        {
            ma_decoder_uninit(&decoder);
            throw;
        }
        ma_decoder_uninit(&decoder);
    }
}
