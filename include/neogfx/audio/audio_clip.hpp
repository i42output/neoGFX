// audio_clip.hpp
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

#include <vector>

#include <neogfx/audio/i_audio_clip.hpp>

namespace neogfx
{
	class audio_clip : public reference_counted<i_audio_clip>
	{
	public:
		struct failed_to_load_clip : std::runtime_error { failed_to_load_clip(std::string const& aUri) : std::runtime_error{ "neogfx::audio_clip: failed to load '" + aUri + "'" } {} };
		struct failed_to_decode_clip : std::runtime_error { failed_to_decode_clip(std::string const& aUri) : std::runtime_error{ "neogfx::audio_clip: failed to decode '" + aUri + "'" } {} };
	public:
		// aSampleRate of 0 means decode at the clip's native sample rate.
		audio_clip(string const& aUri, audio_sample_rate aSampleRate = 0ULL);
		audio_clip(void const* aData, std::size_t aDataSize, audio_sample_rate aSampleRate = 0ULL);
		~audio_clip();
	public:
		i_string const& uri() const final;
		audio_data_format const& data_format() const final;
		audio_channel channels() const final;
		audio_frame_count length() const final;
		float const* cdata() const final;
	private:
		void load(audio_sample_rate aSampleRate);
		void decode(void const* aData, std::size_t aDataSize, audio_sample_rate aSampleRate);
	private:
		string iUri;
		audio_data_format iDataFormat = { audio_sample_format::F32, 0u, 0ULL };
		audio_channel iChannels = audio_channel::None;
		std::vector<float> iPcmFrames;
	};
}
