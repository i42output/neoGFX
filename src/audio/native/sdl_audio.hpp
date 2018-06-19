// sdl_audio.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/audio/i_audio.hpp>
#include <neogfx/audio/i_audio_device.hpp>

namespace neogfx
{
	class sdl_audio : public i_audio
	{
	public:
		sdl_audio();
		~sdl_audio();
	public:
		void initialize(bool aOpenDefaultPlaybackDevice = true) override;
		void uninitialize() override;
		bool have_audio_playback_device() const override;
		uint32_t audio_playback_device_count() const override;
		i_audio_playback_device& audio_playback_device(uint32_t aDeviceIndex) override;
	private:
		bool iInitialized;
		std::vector<std::unique_ptr<i_audio_playback_device>> iAudioPlaybackDevices;
	};
}