// audio_instrument_atlas.hpp
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
#include <neogfx/audio/i_audio_instrument_atlas.hpp>

#pragma once

namespace neogfx
{
	class audio_instrument_atlas : public i_audio_instrument_atlas
	{
	private:
		typedef std::tuple<neogfx::instrument, audio_sample_rate, note> note_key;
		struct sample_info
		{
			std::string sampleFile;
			note midiKeyLow;
			note midiKeyPitchCentre;
			note midiKeyHigh;
		};
	public:
		audio_instrument_atlas();
	public:
		bool load_instrument(neogfx::instrument aInstrument, audio_sample_rate aSampleRate) override;
		i_audio_bitstream& instrument(neogfx::instrument aInstrument, audio_sample_rate aSampleRate, note aNote) override;
	private:
		std::map<neogfx::instrument, std::map<note, sample_info>> iSamples;
		std::map<note_key, ref_ptr<i_audio_bitstream>> iNotes;
	};
}