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

        void read_all_pcm_frames(ma_decoder& aDecoder, audio_data_format& aDataFormat, std::vector<float>& aPcmFrames)
        {
            aDataFormat = audio_data_format{ audio_sample_format::F32, aDecoder.outputChannels, static_cast<audio_sample_rate>(aDecoder.outputSampleRate) };

            ma_uint64 lengthInFrames = 0ULL;
            if (ma_decoder_get_length_in_pcm_frames(&aDecoder, &lengthInFrames) == MA_SUCCESS)
                aPcmFrames.reserve(static_cast<std::size_t>(lengthInFrames) * aDecoder.outputChannels);

            std::vector<float> chunk(DECODE_CHUNK_FRAMES * aDecoder.outputChannels);
            for (;;)
            {
                ma_uint64 framesRead = 0ULL;
                auto const result = ma_decoder_read_pcm_frames(&aDecoder, &chunk[0], DECODE_CHUNK_FRAMES, &framesRead);
                aPcmFrames.insert(aPcmFrames.end(), chunk.begin(), std::next(chunk.begin(), framesRead * aDecoder.outputChannels));
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
                read_all_pcm_frames(decoder, iDataFormat, iPcmFrames);
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
            read_all_pcm_frames(decoder, iDataFormat, iPcmFrames);
        }
        catch (...)
        {
            ma_decoder_uninit(&decoder);
            throw;
        }
        ma_decoder_uninit(&decoder);
    }
}
