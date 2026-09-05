// audio_sample.cpp
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
#include <numeric>

#include <neogfx/audio/audio_sample.hpp>

#ifdef _WIN32
#define MA_ENABLE_WASAPI
#endif
#include "3rdparty/miniaudio/miniaudio.h"

namespace neogfx
{
	audio_sample::audio_sample(audio_sample_rate aSampleRate, std::vector<float>&& aPcmFrames, audio_channel aChannels) :
		audio_bitstream<i_audio_sample>{ aSampleRate },
		iPcmFrames{ std::move(aPcmFrames) },
		iChannels{ aChannels }
	{
	}

	audio_channel audio_sample::channels() const
	{
		return iChannels;
	}

	audio_frame_count audio_sample::length() const
	{
		auto const channels = channel_count(iChannels);
		return channels != 0ULL ? iPcmFrames.size() / channels : 0ULL;
	}

	void audio_sample::generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames)
	{
		generate_from(aChannel, iCursor, aFrameCount, aOutputFrames);
		iCursor = std::min(iCursor + aFrameCount, length());
	}

	void audio_sample::generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames)
	{
		auto const sourceChannels = channel_count(iChannels);
		auto const outputChannels = channel_count(aChannel);

		if (aFrameFrom >= length() || sourceChannels == 0ULL || outputChannels == 0ULL)
			return;
		auto const count = std::min(length() - aFrameFrom, aFrameCount);
		auto const sourceFrames = std::next(iPcmFrames.data(), static_cast<std::ptrdiff_t>(aFrameFrom * sourceChannels));

		// each requested channel is taken from the corresponding source channel; a mono source is present in all of
		// them, a mono request is the downmix of all of them, and anything the source hasn't got is left silent
		std::uint64_t outputChannelIndex = 0ULL;
		for (auto remaining = static_cast<std::uint64_t>(aChannel); remaining != 0ULL; ++outputChannelIndex)
		{
			auto const channel = static_cast<audio_channel>(remaining & (~remaining + 1ULL));
			remaining &= ~static_cast<std::uint64_t>(channel);

			auto output = std::next(aOutputFrames, static_cast<std::ptrdiff_t>(outputChannelIndex));
			if (sourceChannels == 1ULL)
			{
				for (audio_frame_count frame = 0ULL; frame < count; ++frame, output += outputChannels)
					*output += sourceFrames[frame];
			}
			else if ((iChannels & channel) != audio_channel::None)
			{
				auto source = std::next(sourceFrames, static_cast<std::ptrdiff_t>(channel_index(iChannels, channel)));
				for (audio_frame_count frame = 0ULL; frame < count; ++frame, output += outputChannels, source += sourceChannels)
					*output += *source;
			}
			else if (channel == audio_channel::Mono)
			{
				auto source = sourceFrames;
				for (audio_frame_count frame = 0ULL; frame < count; ++frame, output += outputChannels, source += sourceChannels)
					*output += std::accumulate(source, std::next(source, static_cast<std::ptrdiff_t>(sourceChannels)), 0.0f) / static_cast<float>(sourceChannels);
			}
		}
	}
}