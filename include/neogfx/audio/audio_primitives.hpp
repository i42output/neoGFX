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
    enum class note : std::uint32_t
    {
        A0  = 1,
        Bb0 = 2,
        B0  = 3,
        C1  = 4,
        Db1 = 5,
        D1  = 6,
        Eb1 = 7,
        E1  = 8,
        F1  = 9,
        Gb1 = 10,
        G1  = 11,
        Ab1 = 12,
        A1  = 13,
        Bb1 = 14,
        B1  = 15,
        C2  = 16,
        Db2 = 17,
        D2  = 18,
        Eb2 = 19,
        E2  = 20,
        F2  = 21,
        Gb2 = 22,
        G2  = 23,
        Ab2 = 24,
        A2  = 25,
        Bb2 = 26,
        B2  = 27,
        C3  = 28,
        Db3 = 29,
        D3  = 30,
        Eb3 = 31,
        E3  = 32,
        F3  = 33,
        Gb3 = 34,
        G3  = 35,
        Ab3 = 36,
        A3  = 37,
        Bb3 = 38,
        B3  = 39,
        C4  = 40,
        Db4 = 41,
        D4  = 42,
        Eb4 = 43,
        E4  = 44,
        F4  = 45,
        Gb4 = 46,
        G4  = 47,
        Ab4 = 48,
        A4  = 49,
        Bb4 = 50,
        B4  = 51,
        C5  = 52,
        Db5 = 53,
        D5  = 54,
        Eb5 = 55,
        E5  = 56,
        F5  = 57,
        Gb5 = 58,
        G5  = 59,
        Ab5 = 60,
        A5  = 61,
        Bb5 = 62,
        B5  = 63,
        C6  = 64,
        Db6 = 65,
        D6  = 66,
        Eb6 = 67,
        E6  = 68,
        F6  = 69,
        Gb6 = 70,
        G6  = 71,
        Ab6 = 72,
        A6  = 73,
        Bb6 = 74,
        B6  = 75,
        C7  = 76,
        Db7 = 77,
        D7  = 78,
        Eb7 = 79,
        E7  = 80,
        F7  = 81,
        Gb7 = 82,
        G7  = 83,
        Ab7 = 84,
        A7  = 85,
        Bb7 = 86,
        B7  = 87,
        C8  = 88
    };

    inline float frequency(neogfx::note note)
    {
        return std::pow(2.0f, (static_cast<float>(note) - 49.0f) / 12.0f) * 440.0f;
    }

    template <neogfx::note Note>
    inline float frequency()
    {
        return frequency(Note);
    }

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