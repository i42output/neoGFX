// i_audio_beeper.hpp
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
#pragma once

#include <neogfx/neogfx.hpp>

namespace neogfx
{
	struct audio_envelope
	{
		double attack;
		double decay;
		double sustain;
		double release;
	};

	class i_audio_beeper
	{
	public:
		virtual void beep(double aDuration, double aFrequency) = 0;
		virtual void beep(const audio_envelope& aEnvelope, double aFrequency) = 0;
		virtual void silence(double aDuration) = 0;
		virtual void repeat_start(uint32_t aRepeatCount) = 0;
		virtual void repeat_end() = 0;
		virtual void clear() = 0;
	};
}