// audio_device.cpp
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

#include <algorithm>
#include <bit>

#include <neogfx/audio/audio_device.hpp>

#ifdef _WIN32
#define MA_ENABLE_WASAPI
#endif
#include "3rdparty/miniaudio/miniaudio.h"

namespace neogfx
{
	inline ma_device_type from_audio_device_type(audio_device_type aType)
	{
		switch (aType)
		{
		case audio_device_type::Playback:
			return ma_device_type_playback;
		case audio_device_type::Capture:
			return ma_device_type_capture;
		case audio_device_type::Duplex:
			return ma_device_type_duplex;
		case audio_device_type::Loopback:
			return ma_device_type_loopback;
		default:
			throw std::logic_error("neogfx::from_audio_device_type");
		}
	}

	inline audio_channel to_audio_channel(ma_channel aChannel)
	{
		if (aChannel == MA_CHANNEL_NONE || aChannel > 64u)
			return audio_channel::None;
		return static_cast<audio_channel>(1ULL << (aChannel - 1u));
	}

	inline audio_channel to_audio_channels(ma_channel const* aChannelMap, ma_uint32 aChannels)
	{
		auto result = audio_channel::None;
		for (ma_uint32 channel = 0u; channel < aChannels; ++channel)
			result = result | to_audio_channel(aChannelMap[channel]);
		return result;
	}

	inline ma_format from_audio_sample_format(audio_sample_format aFormat)
	{
		switch (aFormat)
		{
		case audio_sample_format::Unknown:
			return ma_format_unknown;
		case audio_sample_format::U8:
			return ma_format_u8;
		case audio_sample_format::S16:
			return ma_format_s16;
		case audio_sample_format::S24:
			return ma_format_s24;
		case audio_sample_format::S32:
			return ma_format_s32;
		case audio_sample_format::F32:
			return ma_format_f32;
		default:
			throw std::logic_error("neogfx::from_audio_sample_format");
		}
	}

	audio_device_info::audio_device_info(audio_device_id aId, audio_device_type aType, i_string const& aName, bool aIsDefault, vector<audio_data_format> const& aDataFormats) :
		iId{ aId }, iType{ aType }, iName{ aName }, iIsDefault{ aIsDefault }, iDataFormats{ aDataFormats }
	{}

	audio_device_info::audio_device_info(i_audio_device_info const& aOther) :
		iId{ aOther.id() }, iType{ aOther.type() }, iName{ aOther.name() }, iIsDefault{ aOther.is_default() }
	{}

	audio_device_info::~audio_device_info()
	{}

	audio_device_id audio_device_info::id() const
	{
		return std::any_cast<ma_device_id>(&iId);
	}

	audio_device_type audio_device_info::type() const
	{
		return iType;
	}

	i_string const& audio_device_info::name() const
	{
		return iName;
	}

	bool audio_device_info::is_default() const
	{
		return iIsDefault;
	}

	i_vector<audio_data_format> const& audio_device_info::data_formats() const
	{
		return iDataFormats;
	}

	audio_device::audio_device(audio_context aContext, i_audio_device_info const& aDeviceInfo, audio_data_format const& aDataFormat) :
		iInfo{ aDeviceInfo }, iDataFormat{ aDataFormat }
	{
		auto callback = [](ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
			{
				auto& device = *static_cast<audio_device*>(pDevice->pUserData);
				auto const channels = to_audio_channels(pDevice->playback.channelMap, pDevice->playback.channels);
				auto const now = std::chrono::steady_clock::now();
				auto const output = static_cast<float*>(pOutput);
				std::fill(output, output + frameCount * channel_count(channels), 0.0f);
				std::unique_lock lock{ device.iMutex };
				for (auto& source : device.iSources)
				{
					// a source without an expiry time plays until it is stopped
					if (source.expiryTime != std::nullopt && *source.expiryTime <= now)
						continue;
					// the cursor lives here rather than in the bitstream so that the same bitstream can back several sources at once
					source.bitstream->generate_from(channels, source.cursor, frameCount, output);
					source.cursor += frameCount;
				}
			};

		iConfig = ma_device_config_init(from_audio_device_type(aDeviceInfo.type()));
		auto& config = *std::any_cast<ma_device_config>(&iConfig);
		config.playback.format = from_audio_sample_format(aDataFormat.sampleFormat);
		config.playback.channels = aDataFormat.channels;
		config.capture.format = from_audio_sample_format(aDataFormat.sampleFormat);
		config.capture.channels = aDataFormat.channels;
		config.sampleRate = static_cast<decltype(config.sampleRate)>(aDataFormat.sampleRate);
		config.dataCallback = callback;
		config.pUserData = this;

		iHandle = ma_device{};
		if (ma_device_init(NULL, &config, std::any_cast<ma_device>(&iHandle)) != MA_SUCCESS)
			throw std::runtime_error("neogfx::audio_device::audio_device");
	}

	audio_device::~audio_device()
	{
		ma_device_uninit(std::any_cast<ma_device>(&iHandle));
	}

	i_audio_device_info const& audio_device::info() const
	{
		return iInfo;
	}

	audio_data_format const& audio_device::data_format() const
	{
		return iDataFormat;
	}

	void audio_device::start()
	{
		ma_device_start(std::any_cast<ma_device>(&iHandle));
	}

	void audio_device::stop()
	{
		ma_device_stop(std::any_cast<ma_device>(&iHandle));
		std::unique_lock lock{ iMutex };
		iSources.clear();
	}

	void audio_device::play(i_audio_bitstream& aBitstream)
	{
		auto const duration = std::chrono::duration<double>{ static_cast<double>(aBitstream.length()) / aBitstream.sample_rate() };
		play(aBitstream, duration);
	}

	void audio_device::play(i_audio_bitstream& aBitstream, std::chrono::duration<double> const& aDuration)
	{
		auto const now = std::chrono::steady_clock::now();
		std::unique_lock lock{ iMutex };
		// expired sources are removed here rather than in the audio callback so that nothing is destroyed on the audio thread
		std::erase_if(iSources, [&](auto const& aSource) { return aSource.expiryTime != std::nullopt && *aSource.expiryTime <= now; });
		iSources.push_back(source{
			ref_ptr<i_audio_bitstream>{ ref_ptr<i_audio_bitstream>{}, &aBitstream },
			now + std::chrono::duration_cast<std::chrono::milliseconds>(aDuration) });
	}
}