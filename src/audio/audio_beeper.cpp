// audio_beeper.cpp
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
#include <neogfx/audio/audio_beeper.hpp>

namespace neogfx
{
	audio_beeper::audio_beeper(i_audio_playback_device& aDevice) : 
		iDevice{ aDevice }, iTrack{ aDevice.create_track() }, iSample{ aDevice.spec() }
	{
		iTrack.add_sample(iSample);
	}

	void audio_beeper::beep(double aDuration, double aFrequency)
	{
		iSample.beep(aDuration, aFrequency);
	}

	void audio_beeper::beep(const audio_envelope& aEnvelope, double aFrequency)
	{
		iSample.beep(aEnvelope, aFrequency);
	}

	void audio_beeper::silence(double aDuration)
	{
		iSample.silence(aDuration);
	}

	void audio_beeper::repeat_start(uint32_t aRepeatCount)
	{
		iSample.repeat_start(aRepeatCount);
	}

	void audio_beeper::repeat_end()
	{
		iSample.repeat_end();
	}

	void audio_beeper::clear()
	{
		iSample.clear();
	}
}