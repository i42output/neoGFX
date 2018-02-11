// sdl_audio.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present Leigh Johnston
  
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
#include <SDL.h>
#include "sdl_audio.hpp"
#include "sdl_audio_playback_device.hpp"

namespace neogfx
{
	sdl_audio::sdl_audio() : iInitialized{ false }
	{
	}

	sdl_audio::~sdl_audio()
	{
		uninitialize();
	}

	void sdl_audio::initialize(bool aOpenDefaultPlaybackDevice)
	{
		if (!iInitialized)
		{
			if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
				throw failed_to_initialize_audio(SDL_GetError());
			int deviceCount = SDL_GetNumAudioDevices(0);
			for (int device = 0; device < deviceCount; ++device)
				iAudioPlaybackDevices.push_back(std::make_unique<sdl_audio_playback_device>(SDL_GetAudioDeviceName(device, 0)));
			iInitialized = true;
			if (aOpenDefaultPlaybackDevice && have_audio_playback_device())
				audio_playback_device(0).open();
		}
	}

	void sdl_audio::uninitialize()
	{
		if (iInitialized)
		{
			iAudioPlaybackDevices.clear();
			SDL_QuitSubSystem(SDL_INIT_AUDIO);
			iInitialized = false;
		}
	}

	bool sdl_audio::have_audio_playback_device() const
	{
		return !iAudioPlaybackDevices.empty();
	}

	uint32_t sdl_audio::audio_playback_device_count() const
	{
		return iAudioPlaybackDevices.size();
	}

	i_audio_playback_device& sdl_audio::audio_playback_device(uint32_t aDeviceIndex)
	{
		if (aDeviceIndex < iAudioPlaybackDevices.size())
			return *iAudioPlaybackDevices[aDeviceIndex];
		throw bad_audio_device_index();
	}
}