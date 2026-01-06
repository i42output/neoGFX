// i_audio_bitstream.hpp
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

namespace neogfx
{
    class i_audio_bitstream : public i_reference_counted
    {
    public:
        typedef i_audio_bitstream abstract_type;
    public:
        typedef void* handle;
    public:
        virtual ~i_audio_bitstream() = default;
    public:
        virtual audio_sample_rate sample_rate() const = 0;
        virtual void set_sample_rate(audio_sample_rate aSampleRate) = 0;
    public:
        virtual float amplitude() const = 0;
        virtual void set_amplitude(float aAmplitude) = 0;
        virtual bool has_envelope() const = 0;
        virtual adsr_envelope const& envelope() const = 0;
        virtual void clear_envelope() = 0;
        virtual void set_envelope(adsr_envelope const& aEnvelope) = 0;
    public:
        virtual audio_frame_count length() const = 0;
        virtual void generate(audio_channel aChannel, audio_frame_count aFrameCount, float* aOutputFrames) = 0;
        virtual void generate_from(audio_channel aChannel, audio_frame_index aFrameFrom, audio_frame_count aFrameCount, float* aOutputFrames) = 0;
    };
}