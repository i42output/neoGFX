// audio_beeper_sample.cpp
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
#include <neogfx/audio/audio_beeper_sample.hpp>

namespace neogfx
{
	audio_beeper_sample::audio_beeper_sample(const audio_spec& aSpec) : 
		iSpec{ aSpec }
	{
	}

	const audio_spec& audio_beeper_sample::spec() const
	{
		return iSpec;
	}

	audio_beeper_sample::frame_index audio_beeper_sample::total_frames() const
	{
		// todo
		throw std::logic_error("audio_beeper_sample::total_frames: not implemented");
	}

	audio_beeper_sample::frame_index audio_beeper_sample::read(frame_index aPosition, void* aBuffer, frame_index aBufferSize) const
	{
		// todo
		throw std::logic_error("audio_beeper_sample::read: not implemented");
	}

	audio_beeper_sample::frame_index audio_beeper_sample::write(frame_index aPosition, const void* aBuffer, frame_index aBufferSize)
	{
		// todo
		throw std::logic_error("audio_beeper_sample::write: not implemented");
	}

	void audio_beeper_sample::beep(double aDuration, double aFrequency)
	{
		// todo
		throw std::logic_error("audio_beeper_sample::beep: not implemented");
	}

	void audio_beeper_sample::beep(const envelope& aEnvelope, double aFrequency)
	{
		// todo
		throw std::logic_error("audio_beeper_sample::beep: not implemented");
	}

	void audio_beeper_sample::silence(double aDuration)
	{
		// todo
		throw std::logic_error("audio_beeper_sample::silence: not implemented");
	}

	void audio_beeper_sample::repeat_start(uint32_t aRepeatCount)
	{
		// todo
		throw std::logic_error("audio_beeper_sample::repeat_start: not implemented");
	}

	void audio_beeper_sample::repeat_end()
	{
		// todo
		throw std::logic_error("audio_beeper_sample::repeat_end: not implemented");
	}
}