/*
neogfx C++ App/Game Engine - Examples - MOD Tracker
Copyright(C) 2026 Leigh Johnston

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

/*
Portions of this file (the lookup tables (OpenMPT's soundlib/Tables.cpp)) are derived from OpenMPT
(https://openmpt.org/).

OpenMPT is distributed under the BSD 3-Clause License:

Copyright (c) 2004-2026, OpenMPT Project Developers and Contributors
Copyright (c) 1997-2003, Olivier Lapicque
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the OpenMPT project nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

// The trackers' lookup tables: periods, waveforms and frequency slides. Those that are simply a formula
// are computed; those that came out of a tracker's source as they are (typos and all) are given as such.

#include <cmath>
#include <cstdint>
#include <array>

namespace mod_tracker::detail
{
    // ProTracker's periods from octave 0 (as FastTracker 2 numbers them) to octave 6
    inline constexpr std::uint16_t PROTRACKER_PERIODS[7u * 12u] =
    {
        2 * 1712, 2 * 1616, 2 * 1524, 2 * 1440, 2 * 1356, 2 * 1280, 2 * 1208, 2 * 1140, 2 * 1076, 2 * 1016, 2 * 960, 2 * 906,
        1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 907,
        856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
        428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
        214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
        107, 101, 95, 90, 85, 80, 75, 71, 67, 63, 60, 56,
        53, 50, 47, 45, 42, 40, 37, 35, 33, 31, 30, 28
    };

    // ProTracker's finetuned period tables, one octave for each of the 16 finetunes (0 .. 7, then -8 .. -1)
    inline constexpr std::uint16_t PROTRACKER_TUNED_PERIODS[16u * 12u] =
    {
        1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 907,
        1700, 1604, 1514, 1430, 1348, 1274, 1202, 1134, 1070, 1010, 954, 900,
        1688, 1592, 1504, 1418, 1340, 1264, 1194, 1126, 1064, 1004, 948, 894,
        1676, 1582, 1492, 1408, 1330, 1256, 1184, 1118, 1056, 996, 940, 888,
        1664, 1570, 1482, 1398, 1320, 1246, 1176, 1110, 1048, 990, 934, 882,
        1652, 1558, 1472, 1388, 1310, 1238, 1168, 1102, 1040, 982, 926, 874,
        1640, 1548, 1460, 1378, 1302, 1228, 1160, 1094, 1032, 974, 920, 868,
        1628, 1536, 1450, 1368, 1292, 1220, 1150, 1086, 1026, 968, 914, 862,
        1814, 1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960,
        1800, 1700, 1604, 1514, 1430, 1350, 1272, 1202, 1134, 1070, 1010, 954,
        1788, 1688, 1592, 1504, 1418, 1340, 1264, 1194, 1126, 1064, 1004, 948,
        1774, 1676, 1582, 1492, 1408, 1330, 1256, 1184, 1118, 1056, 996, 940,
        1762, 1664, 1570, 1482, 1398, 1320, 1246, 1176, 1110, 1048, 988, 934,
        1750, 1652, 1558, 1472, 1388, 1310, 1238, 1168, 1102, 1040, 982, 926,
        1736, 1640, 1548, 1460, 1378, 1302, 1228, 1160, 1094, 1032, 974, 920,
        1724, 1628, 1536, 1450, 1368, 1292, 1220, 1150, 1086, 1026, 968, 914
    };

    // Scream Tracker 3's periods for octave 4
    inline constexpr std::uint16_t S3M_PERIODS[12] = { 1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 907 };

    // the middle C frequencies of Scream Tracker 3's finetunes
    inline constexpr std::uint16_t S3M_FINETUNES[16] =
    {
        7895, 7941, 7985, 8046, 8107, 8169, 8232, 8280, 8363, 8413, 8463, 8529, 8581, 8651, 8723, 8757
    };

    // FastTracker 2's Amiga period table, eight steps per semitone
    inline constexpr std::uint16_t XM_PERIODS[104] =
    {
        907, 900, 894, 887, 881, 875, 868, 862, 856, 850, 844, 838, 832, 826, 820, 814,
        808, 802, 796, 791, 785, 779, 774, 768, 762, 757, 752, 746, 741, 736, 730, 725,
        720, 715, 709, 704, 699, 694, 689, 684, 678, 675, 670, 665, 660, 655, 651, 646,
        640, 636, 632, 628, 623, 619, 614, 610, 604, 601, 597, 592, 588, 584, 580, 575,
        570, 567, 563, 559, 555, 551, 547, 543, 538, 535, 532, 528, 524, 520, 516, 513,
        508, 505, 502, 498, 494, 491, 487, 484, 480, 477, 474, 470, 467, 463, 460, 457,
        453, 450, 447, 443, 440, 437, 434, 431
    };

    inline constexpr std::int8_t MOD_SINE[64] =
    {
        0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126,
        127, 126, 125, 122, 117, 112, 106, 98, 90, 81, 71, 60, 49, 37, 25, 12,
        0, -12, -25, -37, -49, -60, -71, -81, -90, -98, -106, -112, -117, -122, -125, -126,
        -127, -126, -125, -122, -117, -112, -106, -98, -90, -81, -71, -60, -49, -37, -25, -12
    };

    inline constexpr std::int8_t MOD_RANDOM[64] =
    {
        98, -127, -43, 88, 102, 41, -65, -94, 125, 20, -71, -86, -70, -32, -16, -96,
        17, 72, 107, -5, 116, -69, -62, -40, 10, -61, 65, 109, -18, -38, -13, -76,
        -23, 88, 21, -94, 8, 106, 21, -112, 6, 109, 20, -88, -30, 9, -127, 118,
        42, -34, 89, -4, -51, -72, 21, -29, 112, 123, 84, -101, -92, 98, -54, -95
    };

    // Impulse Tracker's sine table
    inline constexpr std::int8_t IT_SINE[256] =
    {
        0, 2, 3, 5, 6, 8, 9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
        24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
        45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
        59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
        59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
        45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
        24, 23, 22, 20, 19, 17, 16, 14, 12, 11, 9, 8, 6, 5, 3, 2,
        0, -2, -3, -5, -6, -8, -9, -11, -12, -14, -16, -17, -19, -20, -22, -23,
        -24, -26, -27, -29, -30, -32, -33, -34, -36, -37, -38, -39, -41, -42, -43, -44,
        -45, -46, -47, -48, -49, -50, -51, -52, -53, -54, -55, -56, -56, -57, -58, -59,
        -59, -60, -60, -61, -61, -62, -62, -62, -63, -63, -63, -64, -64, -64, -64, -64,
        -64, -64, -64, -64, -64, -64, -63, -63, -63, -62, -62, -62, -61, -61, -60, -60,
        -59, -59, -58, -57, -56, -56, -55, -54, -53, -52, -51, -50, -49, -48, -47, -46,
        -45, -44, -43, -42, -41, -39, -38, -37, -36, -34, -33, -32, -30, -29, -27, -26,
        -24, -23, -22, -20, -19, -17, -16, -14, -12, -11, -9, -8, -6, -5, -3, -2
    };

    // the volume changes of retriggers: multiplied by a sixteenth, or added
    inline constexpr std::int8_t RETRIGGER_MULTIPLY[16] = { 0, 0, 0, 0, 0, 0, 10, 8, 0, 0, 0, 0, 0, 0, 24, 32 };
    inline constexpr std::int8_t RETRIGGER_ADD[16] = { 0, -1, -2, -4, -8, -16, 0, 0, 0, 1, 2, 4, 8, 16, 0, 0 };

    // Impulse Tracker's volume column tone portamento speeds
    inline constexpr std::uint8_t IT_PORTAMENTO_VOLUME_COLUMN[16] =
    {
        0x00, 0x01, 0x04, 0x08, 0x10, 0x20, 0x40, 0x60, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
    };

    // fine linear slides as Impulse Tracker has them (65536 * 2^(n / 768), the downward table with its errors)
    inline constexpr std::uint32_t FINE_LINEAR_SLIDE_UP[16] =
    {
        65536, 65595, 65654, 65714, 65773, 65832, 65892, 65951, 66011, 66071, 66130, 66190, 66250, 66309, 66369, 66429
    };
    inline constexpr std::uint32_t FINE_LINEAR_SLIDE_DOWN[16] =
    {
        65535, 65477, 65418, 65359, 65300, 65241, 65182, 65123, 65065, 65006, 64947, 64888, 64830, 64772, 64713, 64645
    };

    struct computed_tables
    {
        std::array<std::uint32_t, 256u> linearSlideUp;      // round(65536 * 2^(n / 192))
        std::array<std::uint32_t, 256u> linearSlideDown;    // round(65536 * 2^(-n / 192))
        std::array<std::uint32_t, 768u> xmLinear;           // floor(8363 * 64 * 2^(-n / 768))
        std::array<std::uint16_t, 256u> xmPanning;          // round(65536 * sqrt(n / 256))

        computed_tables()
        {
            for (std::uint32_t n = 0u; n < 256u; ++n)
            {
                linearSlideUp[n] = static_cast<std::uint32_t>(std::lround(65536.0 * std::pow(2.0, n / 192.0)));
                linearSlideDown[n] = static_cast<std::uint32_t>(std::lround(65536.0 * std::pow(2.0, -static_cast<double>(n) / 192.0)));
                xmPanning[n] = static_cast<std::uint16_t>(std::lround(65536.0 * std::sqrt(n / 256.0)));
            }
            for (std::uint32_t n = 0u; n < 768u; ++n)
                xmLinear[n] = static_cast<std::uint32_t>(std::floor(8363.0 * 64.0 * std::pow(2.0, -static_cast<double>(n) / 768.0)));
        }
    };

    inline computed_tables const& tables()
    {
        static computed_tables const sTables;
        return sTables;
    }

    inline std::int32_t muldiv(std::int64_t aA, std::int64_t aB, std::int64_t aC)
    {
        return aC != 0 ? static_cast<std::int32_t>(aA * aB / aC) : 0;
    }

    inline std::int32_t muldivr(std::int64_t aA, std::int64_t aB, std::int64_t aC)
    {
        if (aC == 0)
            return 0;
        auto const product = aA * aB;
        // round half away from zero
        return static_cast<std::int32_t>((product + (product >= 0 ? aC / 2 : -aC / 2)) / aC);
    }

    inline std::int8_t mod_to_xm_finetune(std::uint32_t aValue)
    {
        return static_cast<std::int8_t>(static_cast<std::uint8_t>(aValue << 4u));
    }

    inline std::uint32_t xm_to_mod_finetune(std::int32_t aValue)
    {
        return static_cast<std::uint8_t>(aValue) >> 4u;
    }
}
