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

#include <neogfx/neogfx.hpp>
#include <chrono>
#include <neogfx/audio/audio_primitives.hpp>

#pragma once

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

	class i_audio_waveform;

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
		virtual void play(i_audio_waveform& aWaveform, std::chrono::duration<double> const& aDuration) = 0;
	};
}