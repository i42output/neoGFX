// i_audio.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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
#include <neogfx/audio/i_audio_playback_device.hpp>

namespace neogfx
{
	class i_audio
	{
	public:
		struct failed_to_initialize_audio : std::runtime_error { failed_to_initialize_audio(const std::string& aReason) : std::runtime_error{"neogfx::i_audio::failed_to_initialize_audio: " + aReason} {} };
		struct bad_audio_device_index : std::logic_error { bad_audio_device_index() : std::logic_error{ "neogfx::i_audio::bad_audio_device_index" } {} };
	public:
		virtual void initialize(bool aOpenDefaultPlaybackDevice = true) = 0;
		virtual void uninitialize() = 0;
		virtual bool have_audio_playback_device() const = 0;
		virtual uint32_t audio_playback_device_count() const = 0;
		virtual i_audio_playback_device& audio_playback_device(uint32_t aDeviceIndex) = 0;
	};
}