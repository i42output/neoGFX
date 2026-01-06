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

#pragma once

#include <neogfx/neogfx.hpp>

#include <neogfx/audio/audio_primitives.hpp>
#include <neogfx/audio/i_audio_bitstream.hpp>

namespace neogfx
{
	struct audio_instrument_atlas_file_found : std::runtime_error { audio_instrument_atlas_file_found() : std::runtime_error{ "Audio instrument atlas file not found" } {} };
	struct audio_instrument_not_found : std::runtime_error { audio_instrument_not_found(neogfx::instrument aInstrument) : std::runtime_error{"Audio instrument '" + to_string(aInstrument) + "' not found"} {} };
	struct audio_instrument_note_not_found : std::runtime_error { audio_instrument_note_not_found(neogfx::instrument aInstrument, neogfx::note aNote) : std::runtime_error{ "Audio instrument '" + to_string(aInstrument) + "' note '" + to_string(aNote) + "' not found" } {} };
	struct audio_instrument_sample_decode_failure : std::runtime_error { audio_instrument_sample_decode_failure() : std::runtime_error{ "audio_instrument_sample_decode_failure" } {} };

	class i_audio_instrument_atlas
	{
	public:
		virtual ~i_audio_instrument_atlas() = default;
	public:
		virtual bool load_instrument(neogfx::instrument aInstrument, audio_sample_rate aSampleRate) = 0;
		virtual i_audio_bitstream& instrument(neogfx::instrument aInstrument, audio_sample_rate aSampleRate, note aNote) = 0;
	};
}