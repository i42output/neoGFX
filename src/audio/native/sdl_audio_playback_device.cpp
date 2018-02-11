// sdl_audio.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include <neogfx/audio/audio_track.hpp>
#include <neogfx/audio/audio_beeper.hpp>
#include "sdl_audio_playback_device.hpp"

namespace neogfx
{
	sdl_audio_playback_device::sdl_audio_playback_device(const std::string& aName) : 
		sdl_audio_device{ aName }
	{
	}

	const std::string& sdl_audio_playback_device::name() const
	{
		return sdl_audio_device::name();
	}

	bool sdl_audio_playback_device::is_open() const
	{
		return sdl_audio_device::is_open();
	}

	void sdl_audio_playback_device::open(const audio_spec& aAudioSpec, audio_spec_requirements aRequirements)
	{
		sdl_audio_device::open(aAudioSpec, aRequirements);
	}

	void sdl_audio_playback_device::close()
	{
		sdl_audio_device::close();
	}

	const audio_spec& sdl_audio_playback_device::spec() const
	{
		return sdl_audio_device::spec();
	}
}