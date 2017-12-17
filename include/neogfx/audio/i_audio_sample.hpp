// i_audio_sample.hpp
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

namespace neogfx
{
	class i_audio_sample
	{
	public:
		typedef uint32_t frame_index;
	public:
		virtual const audio_spec& spec() const = 0;
		virtual frame_index total_frames() const = 0;
	public:
		virtual frame_index read(frame_index aPosition, void* aBuffer, frame_index aBufferSize) const = 0;
		virtual frame_index write(frame_index aPosition, const void* aBuffer, frame_index aBufferSize) = 0;
	};
}