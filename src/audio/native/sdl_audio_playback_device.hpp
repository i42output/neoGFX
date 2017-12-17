// sdl_audio_playback_device.hpp
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
#pragma once

#include <neogfx/neogfx.hpp>
#include <SDL.h>
#include <neogfx/audio/i_audio_playback_device.hpp>
#include "sdl_audio_device.hpp"

namespace neogfx
{
	class sdl_audio_playback_device : public sdl_audio_device<i_audio_playback_device>
	{
	public:
		sdl_audio_playback_device(const std::string& aName);
	public:
		i_audio_sample& load_sample(const std::string& aUri) override;
		i_audio_sample& create_sample(double aDuration) override;
		i_audio_beeper_sample& create_beeper_sample() override;
		void destroy_sample(i_audio_sample& aSample) override;
	public:
		i_audio_track& create_track() override;
		void destroy_track(i_audio_track& aTrack) override;
	public:
		void beep(double aDuration, double aFrequency) override;
		void beep(const envelope& aEnvelope, double aFrequency) override;
		void silence(double aDuration) override;
		void repeat_start(uint32_t aRepeatCount) override;
		void repeat_end() override;
	private:
		i_audio_track& beeper_track();
	private:
		std::vector<std::shared_ptr<i_audio_sample>> iSamples;
		std::vector<std::shared_ptr<i_audio_track>> iTracks;
		std::weak_ptr<i_audio_track> iBeeperTrack;
	};
}