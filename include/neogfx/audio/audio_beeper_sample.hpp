// i_audio_beeper_sample.hpp
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
#include <neogfx/audio/audio_spec.hpp>
#include <neogfx/audio/i_audio_beeper_sample.hpp>

namespace neogfx
{
	class audio_beeper_sample : public i_audio_beeper_sample
	{
	public:
		audio_beeper_sample(const audio_spec& aSpec);
	public:
		const audio_spec& spec() const override;
		frame_index total_frames() const override;
	public:
		frame_index read(frame_index aPosition, void* aBuffer, frame_index aBufferSize) const override;
		frame_index write(frame_index aPosition, const void* aBuffer, frame_index aBufferSize) override;
	public:
		void beep(double aDuration, double aFrequency) override;
		void beep(const envelope& aEnvelope, double aFrequency) override;
		void silence(double aDuration) override;
		void repeat_start(uint32_t aRepeatCount) override;
		void repeat_end() override;
	private:
		audio_spec iSpec;
	};
}