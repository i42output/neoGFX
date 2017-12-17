// sdl_audio.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neogfx/audio/audio_beeper_sample.hpp>
#include "sdl_audio_playback_device.hpp"

namespace neogfx
{
	sdl_audio_playback_device::sdl_audio_playback_device(const std::string& aName) : 
		sdl_audio_device<i_audio_playback_device>{ aName }
	{
	}

	i_audio_sample& sdl_audio_playback_device::load_sample(const std::string& aUri)
	{
		// todo
		throw std::logic_error("sdl_audio_playback_device::load_sample: not yet implemented");
	}

	i_audio_sample& sdl_audio_playback_device::create_sample(double aDuration)
	{
		// todo
		throw std::logic_error("sdl_audio_playback_device::create_sample: not yet implemented");
	}

	i_audio_beeper_sample& sdl_audio_playback_device::create_beeper_sample()
	{
		auto newSample = std::make_shared<audio_beeper_sample>(spec());
		iSamples.push_back(newSample);
		return *newSample;
	}

	void sdl_audio_playback_device::destroy_sample(i_audio_sample& aSample)
	{
		for (auto i = iSamples.begin(); i != iSamples.end(); ++i)
			if (&**i == &aSample)
			{
				iSamples.erase(i);
				return;
			}
	}

	i_audio_track& sdl_audio_playback_device::create_track()
	{
		auto newTrack = std::make_shared<audio_track>(spec());
		iTracks.push_back(newTrack);
		return *newTrack;
	}

	void sdl_audio_playback_device::destroy_track(i_audio_track& aTrack)
	{
		for (auto i = iTracks.begin(); i != iTracks.end(); ++i)
			if (&**i == &aTrack)
			{
				iTracks.erase(i);
				return;
			}
	}

	void sdl_audio_playback_device::beep(double aDuration, double aFrequency)
	{
		auto beeperSample = std::make_shared<audio_beeper_sample>(spec());
		beeperSample->beep(aDuration, aFrequency);
		beeper_track().add_sample(beeperSample);
	}

	void sdl_audio_playback_device::beep(const envelope& aEnvelope, double aFrequency)
	{
		auto beeperSample = std::make_shared<audio_beeper_sample>(spec());
		beeperSample->beep(aEnvelope, aFrequency);
		beeper_track().add_sample(beeperSample);
	}

	void sdl_audio_playback_device::silence(double aDuration)
	{
		beeper_track().add_silence(aDuration);
	}

	void sdl_audio_playback_device::repeat_start(uint32_t aRepeatCount)
	{
		beeper_track().repeat_start(aRepeatCount);
	}

	void sdl_audio_playback_device::repeat_end()
	{
		beeper_track().repeat_end();
	}

	i_audio_track& sdl_audio_playback_device::beeper_track()
	{
		if (iBeeperTrack.expired())
		{
			iTracks.push_back(std::make_shared<audio_track>(spec()));
			iBeeperTrack = iTracks.back();
		}
		return *iBeeperTrack.lock();
	}
}