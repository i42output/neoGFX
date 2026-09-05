// i_audio_clip.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

namespace neogfx
{
	class i_audio_clip : public i_reference_counted
	{
	public:
		using abstract_type = i_audio_clip;
	public:
		virtual ~i_audio_clip() = default;
	public:
		virtual i_string const& uri() const = 0;
		virtual audio_data_format const& data_format() const = 0;
		virtual audio_channel channels() const = 0;
		virtual audio_frame_count length() const = 0;
		virtual float const* cdata() const = 0;
	public:
		audio_sample_rate sample_rate() const { return data_format().sampleRate; }
	};
}
