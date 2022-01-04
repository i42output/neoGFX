// i_audio_instrument.hpp
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
#include <chrono>
#include <neogfx/audio/audio_primitives.hpp>
#include <neogfx/audio/i_audio_bitstream.hpp>

#pragma once

namespace neogfx
{
    class i_audio_instrument : public i_audio_bitstream
    {
    public:
        typedef i_audio_instrument abstract_type;
    public:
        typedef audio_frame_index time_point;
        typedef audio_frame_count time_interval;
    public:
        virtual ~i_audio_instrument() = default;
    public:
        virtual time_point play_note(note aNote, std::chrono::duration<double> const& aDuration, float aAmplitude = 1.0f) = 0;
        virtual time_point play_note(time_point aWhen, note aNote, std::chrono::duration<double> const& aDuration, float aAmplitude = 1.0f) = 0;
        virtual time_point play_silence(std::chrono::duration<double> const& aDuration) = 0;
        virtual time_point play_silence(time_point aWhen, std::chrono::duration<double> const& aDuration) = 0;
    };
}