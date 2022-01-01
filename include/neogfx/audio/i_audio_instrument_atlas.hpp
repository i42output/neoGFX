// i_audio_instrument_atlas.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/audio/audio_primitives.hpp>
#include <neogfx/audio/i_audio_bitstream.hpp>

#pragma once

namespace neogfx
{
	class i_audio_instrument_atlas
	{
	public:
		virtual ~i_audio_instrument_atlas() = default;
	public:
		virtual i_audio_bitstream& instrument(neogfx::instrument aInstrument, audio_sample_rate aSampleRate, note aNote) = 0;
	};
}