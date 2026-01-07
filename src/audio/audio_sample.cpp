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

#include <neogfx/audio/audio_sample.hpp>

#ifdef _WIN32
#define MA_ENABLE_WASAPI
#endif
#include "3rdparty/miniaudio/miniaudio.h"

namespace neogfx
{
	audio_sample::audio_sample(audio_sample_rate aSampleRate, std::vector<float>&& aPcmFrames) :
		audio_bitstream<i_audio_sample>{ aSampleRate },
		iPcmFrames{ aPcmFrames }
	{
	}

	audio_frame_count audio_sample::length() const
	{
		return iPcmFrames.size();
	}

	void audio_sample::generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames)
	{
		generate_from(aChannel, iCursor, aFrameCount, aOutputFrames);
	}
	
	void audio_sample::generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames)
	{
		// todo multiple channels

		std::fill(aOutputFrames, aOutputFrames + aFrameCount, 0.0f);
		if (aFrameFrom >= iPcmFrames.size())
			return;
		auto count = std::min(iPcmFrames.size() - aFrameFrom, aFrameCount);
		std::copy(std::next(iPcmFrames.begin(), aFrameFrom), std::next(iPcmFrames.begin(), aFrameFrom + count), aOutputFrames);
		iCursor = aFrameFrom + count;
	}
}