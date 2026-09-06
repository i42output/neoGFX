// i_audio_device.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <chrono>
#include <optional>

#include <neogfx/audio/audio_primitives.hpp>
#include <neogfx/audio/i_audio_bitstream.hpp>

namespace neogfx
{
	typedef std::any audio_device_id;
	typedef std::any audio_device_config;
	typedef std::any audio_device_handle;

	enum class audio_device_type : std::int32_t
	{
		Playback	= 1,
		Capture		= 2,
		Duplex		= Playback | Capture,
		Loopback	= 4
	};

	class i_audio_device_info
	{
	public:
		typedef i_audio_device_info abstract_type;
	public:
		virtual ~i_audio_device_info() = default;
	public:
		virtual audio_device_id id() const = 0;
		virtual audio_device_type type() const = 0;
		virtual i_string const& name() const = 0;
		virtual bool is_default() const = 0;
	public:
		virtual i_vector<audio_data_format> const& data_formats() const = 0;
	};

	class i_audio_bitstream;

	class i_audio_device : public i_reference_counted
	{
	public:
		typedef i_audio_device abstract_type;
	public:
		virtual ~i_audio_device() = default;
	public:
		virtual i_audio_device_info const& info() const = 0;
		virtual audio_data_format const& data_format() const = 0;
	public:
		virtual void start() = 0;
		virtual void stop() = 0;
	public:
		// a duration of std::nullopt plays until stopped; aGain scales this playback only, leaving
		// the bitstream's own amplitude alone, so one bitstream can play at several volumes at once
		virtual audio_playback_id play(i_audio_bitstream& aBitstream, audio_frame_index aFrom, std::optional<std::chrono::duration<double>> const& aDuration, float aGain) = 0;
		virtual void stop(audio_playback_id aPlayback) = 0;
	public:
		audio_playback_id play(i_audio_bitstream& aBitstream, audio_frame_index aFrom, std::optional<std::chrono::duration<double>> const& aDuration)
		{
			return play(aBitstream, aFrom, aDuration, 1.0f);
		}
		audio_playback_id play(i_audio_bitstream& aBitstream, std::chrono::duration<double> const& aDuration)
		{
			return play(aBitstream, 0ULL, aDuration, 1.0f);
		}
		// plays the whole of aBitstream from the beginning; a bitstream with no length of its own,
		// such as a waveform, plays until it is stopped
		audio_playback_id play(i_audio_bitstream& aBitstream)
		{
			return play(aBitstream, 0ULL, duration_of(aBitstream), 1.0f);
		}
		audio_playback_id play(i_audio_bitstream& aBitstream, float aGain)
		{
			return play(aBitstream, 0ULL, duration_of(aBitstream), aGain);
		}
	public:
		static std::optional<std::chrono::duration<double>> duration_of(i_audio_bitstream const& aBitstream, audio_frame_index aFrom = 0ULL)
		{
			if (aBitstream.length() == 0ULL || aBitstream.sample_rate() == 0ULL || aFrom >= aBitstream.length())
				return {};
			return std::chrono::duration<double>{
				static_cast<double>(aBitstream.length() - aFrom) / static_cast<double>(aBitstream.sample_rate()) };
		}
	};
}