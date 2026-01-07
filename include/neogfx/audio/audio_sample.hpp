// audio_sample.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <neogfx/audio/audio_bitstream.hpp>
#include <neogfx/audio/i_audio_sample.hpp>

namespace neogfx
{
	class audio_sample : public audio_bitstream<i_audio_sample>
	{
	public:
		audio_sample(audio_sample_rate aSampleRate, std::vector<float>&& aPcmFrames);
	public:
		audio_frame_count length() const override;
		void generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames) override;
		void generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames) override;
	private:
		std::vector<float> iPcmFrames;
		audio_frame_index iCursor = 0ULL;
	};
}