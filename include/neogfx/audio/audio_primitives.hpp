// audio_primitives.hpp
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
#include <array>

#pragma once

namespace neogfx
{
    enum class audio_channel : std::uint64_t
    {
        None                               = 0x0000000000000000ULL,
        Mono                               = 0x0000000000000001ULL,
        FrontLeft                          = 0x0000000000000002ULL,
        FrontRight                         = 0x0000000000000004ULL,
        FrontCenter                        = 0x0000000000000008ULL,
        Lfe                                = 0x0000000000000010ULL,
        BackLeft                           = 0x0000000000000020ULL,
        BackRight                          = 0x0000000000000040ULL,
        FrontLeftCenter                    = 0x0000000000000080ULL,
        FrontRightCenter                   = 0x0000000000000100ULL,
        BackCenter                         = 0x0000000000000200ULL,
        SideLeft                           = 0x0000000000000400ULL,
        SideRight                          = 0x0000000000000800ULL,
        TopCenter                          = 0x0000000000001000ULL,
        TopFrontLeft                       = 0x0000000000002000ULL,
        TopFrontCenter                     = 0x0000000000004000ULL,
        TopFrontRight                      = 0x0000000000008000ULL,
        TopBackLeft                        = 0x0000000000010000ULL,
        TopBackCenter                      = 0x0000000000020000ULL,
        TopBackRight                       = 0x0000000000040000ULL,
        Aux0                               = 0x0000000000080000ULL,
        Aux1                               = 0x0000000000100000ULL,
        Aux2                               = 0x0000000000200000ULL,
        Aux3                               = 0x0000000000400000ULL,
        Aux4                               = 0x0000000000800000ULL,
        Aux5                               = 0x0000000001000000ULL,
        Aux6                               = 0x0000000002000000ULL,
        Aux7                               = 0x0000000004000000ULL,
        Aux8                               = 0x0000000008000000ULL,
        Aux9                               = 0x0000000010000000ULL,
        Aux10                              = 0x0000000020000000ULL,
        Aux11                              = 0x0000000040000000ULL,
        Aux12                              = 0x0000000080000000ULL,
        Aux13                              = 0x0000000100000000ULL,
        Aux14                              = 0x0000000200000000ULL,
        Aux15                              = 0x0000000400000000ULL,
        Aux16                              = 0x0000000800000000ULL,
        Aux17                              = 0x0000001000000000ULL,
        Aux18                              = 0x0000002000000000ULL,
        Aux19                              = 0x0000004000000000ULL,
        Aux20                              = 0x0000008000000000ULL,
        Aux21                              = 0x0000010000000000ULL,
        Aux22                              = 0x0000020000000000ULL,
        Aux23                              = 0x0000040000000000ULL,
        Aux24                              = 0x0000080000000000ULL,
        Aux25                              = 0x0000100000000000ULL,
        Aux26                              = 0x0000200000000000ULL,
        Aux27                              = 0x0000400000000000ULL,
        Aux28                              = 0x0000800000000000ULL,
        Aux29                              = 0x0001000000000000ULL,
        Aux30                              = 0x0002000000000000ULL,
        Aux31                              = 0x0004000000000000ULL,
                                           
        Left                               = FrontLeft,
        Right                              = FrontRight
    };

    inline audio_channel operator&(audio_channel lhs, audio_channel rhs)
    {
        return static_cast<audio_channel>(static_cast<std::uint64_t>(lhs) & static_cast<std::uint64_t>(rhs));
    }

    inline audio_channel operator|(audio_channel lhs, audio_channel rhs)
    {
        return static_cast<audio_channel>(static_cast<std::uint64_t>(lhs) | static_cast<std::uint64_t>(rhs));
    }

    inline audio_channel operator^(audio_channel lhs, audio_channel rhs)
    {
        return static_cast<audio_channel>(static_cast<std::uint64_t>(lhs) ^ static_cast<std::uint64_t>(rhs));
    }

    inline audio_channel operator~(audio_channel lhs)
    {
        return static_cast<audio_channel>(~static_cast<std::uint64_t>(lhs));
    }

    enum class audio_stream_format : std::uint32_t
    {
        Unknown = 0,

        Pcm     = 1
    };

    enum class audio_sample_format : std::uint32_t
    {
        Unknown = 0,

        U8      = 1,
        S16     = 2,
        S24     = 3,
        S32     = 4,
        F32     = 5
    };

    typedef std::uint64_t audio_sample_rate;

	enum class audio_standard_sample_rate : std::uint64_t
	{
        Rate48000   = 48000,
        Rate44100   = 44100,

        Rate32000   = 32000,
        Rate24000   = 24000,
        Rate22050   = 22050,

        Rate88200   = 88200,
        Rate96000   = 96000,
        Rate176400  = 176400,
        Rate192000  = 192000,

        Rate16000   = 16000,
        Rate11025   = 11250,
        Rate8000    = 8000,

        Rate352800  = 352800,
        Rate384000  = 384000,
	};

    struct audio_data_format
    {
        typedef audio_data_format abstract_type;

        audio_sample_format sampleFormat;
        std::uint32_t channels;
        audio_sample_rate sampleRate;
    };

    typedef std::uint64_t audio_sample_index;
    typedef std::uint64_t audio_sample_count;

    typedef std::uint64_t audio_frame_index;
    typedef std::uint64_t audio_frame_count;

    template <typename SampleType, std::size_t Channels>
    using audio_frame = std::array<SampleType, Channels>;

    struct adsr_envelope
    {
        float attack;
        float decay;
        float sustain;
        float release;
    };
}