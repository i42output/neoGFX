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
Portions of this file (the loaders' declarations) are derived from OpenMPT (https://openmpt.org/).

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

// What the format loaders share: reading binary data, decoding sample data and translating effects.

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <mod_tracker/module.hpp>

namespace mod_tracker::detail
{
    // Bounds-checked access to a file in memory. Reading past the end throws, except through bytes(),
    // which returns whatever is there (a truncated file keeps as much sample data as it has).
    class reader
    {
    public:
        reader(std::span<std::byte const> aData) :
            iData{ aData }
        {
        }
    public:
        std::size_t size() const
        {
            return iData.size();
        }
        bool has(std::size_t aOffset, std::size_t aCount) const
        {
            return aOffset <= iData.size() && aCount <= iData.size() - aOffset;
        }
        std::uint8_t u8(std::size_t aOffset) const
        {
            if (!has(aOffset, 1u))
                throw module_load_error{ "file is truncated" };
            return static_cast<std::uint8_t>(iData[aOffset]);
        }
        std::int8_t i8(std::size_t aOffset) const
        {
            return static_cast<std::int8_t>(u8(aOffset));
        }
        std::uint16_t u16le(std::size_t aOffset) const
        {
            return static_cast<std::uint16_t>(u8(aOffset) | (u8(aOffset + 1u) << 8u));
        }
        std::uint16_t u16be(std::size_t aOffset) const
        {
            return static_cast<std::uint16_t>((u8(aOffset) << 8u) | u8(aOffset + 1u));
        }
        std::int16_t i16le(std::size_t aOffset) const
        {
            return static_cast<std::int16_t>(u16le(aOffset));
        }
        std::int16_t i16be(std::size_t aOffset) const
        {
            return static_cast<std::int16_t>(u16be(aOffset));
        }
        std::uint32_t u32le(std::size_t aOffset) const
        {
            return static_cast<std::uint32_t>(u16le(aOffset)) | (static_cast<std::uint32_t>(u16le(aOffset + 2u)) << 16u);
        }
        std::uint32_t u32be(std::size_t aOffset) const
        {
            return (static_cast<std::uint32_t>(u16be(aOffset)) << 16u) | static_cast<std::uint32_t>(u16be(aOffset + 2u));
        }
        bool magic(std::size_t aOffset, std::string const& aMagic) const
        {
            if (!has(aOffset, aMagic.size()))
                return false;
            for (std::size_t index = 0u; index < aMagic.size(); ++index)
                if (static_cast<char>(iData[aOffset + index]) != aMagic[index])
                    return false;
            return true;
        }
        // a fixed-length name: stops at a null, shows anything unprintable as a space, trims trailing spaces
        std::string text(std::size_t aOffset, std::size_t aLength) const;
        std::span<std::byte const> bytes(std::size_t aOffset, std::size_t aCount) const
        {
            if (aOffset >= iData.size())
                return {};
            return iData.subspan(aOffset, std::min(aCount, iData.size() - aOffset));
        }
        std::span<std::byte const> data() const
        {
            return iData;
        }
    private:
        std::span<std::byte const> iData;
    };

    // sequential reading on top of a reader
    class cursor
    {
    public:
        cursor(reader const& aReader, std::size_t aOffset = 0u) :
            iReader{ aReader }, iOffset{ aOffset }
        {
        }
    public:
        std::size_t offset() const
        {
            return iOffset;
        }
        void seek(std::size_t aOffset)
        {
            iOffset = aOffset;
        }
        void skip(std::size_t aCount)
        {
            iOffset += aCount;
        }
        bool can_read(std::size_t aCount) const
        {
            return iReader.has(iOffset, aCount);
        }
        std::uint8_t u8()
        {
            auto const result = iReader.u8(iOffset);
            iOffset += 1u;
            return result;
        }
        std::uint16_t u16le()
        {
            auto const result = iReader.u16le(iOffset);
            iOffset += 2u;
            return result;
        }
        std::uint16_t u16be()
        {
            auto const result = iReader.u16be(iOffset);
            iOffset += 2u;
            return result;
        }
        std::uint32_t u32le()
        {
            auto const result = iReader.u32le(iOffset);
            iOffset += 4u;
            return result;
        }
        std::uint32_t u32be()
        {
            auto const result = iReader.u32be(iOffset);
            iOffset += 4u;
            return result;
        }
    private:
        reader const& iReader;
        std::size_t iOffset;
    };

    // how sample data is stored
    struct sample_encoding
    {
        enum class bits : std::uint8_t { Eight, Sixteen, ThirtyTwo };
        bits width = bits::Eight;
        bool isSigned = true;
        bool bigEndian = false;
        bool delta = false;
        bool stereo = false;
        bool stereoInterleaved = false;     // L R L R ... rather than all of L then all of R

        std::size_t bytes_per_sample() const
        {
            return width == bits::Eight ? 1u : width == bits::Sixteen ? 2u : 4u;
        }
    };

    // decode aFrames frames of PCM into aSample (setting its data, length and stereo flag); returns the
    // number of bytes the encoded data takes up
    std::size_t decode_pcm(std::span<std::byte const> aData, std::uint32_t aFrames, sample_encoding const& aEncoding, sample& aSample);
    // Impulse Tracker 2.14 / 2.15 compressed samples; returns the number of bytes consumed
    std::size_t decode_it_compressed(std::span<std::byte const> aData, std::uint32_t aFrames, bool a16Bit, bool aStereo, bool aIT215, sample& aSample);

    // effect translation
    std::pair<effect, std::uint8_t> convert_mod_effect(std::uint8_t aCommand, std::uint8_t aParameter);
    std::pair<effect, std::uint8_t> convert_s3m_effect(std::uint8_t aCommand, std::uint8_t aParameter, bool aFromIT);
    // put a volume-column-capable effect into the volume column, as trackers that have two effect columns
    // are played; returns the effect that didn't fit, if any
    std::pair<effect, std::uint8_t> fill_two_effects(cell& aCell, effect aEffect1, std::uint8_t aParameter1, effect aEffect2, std::uint8_t aParameter2);

    // the loaders, each given a file that has already been identified as its format
    bool is_s3m(reader const& aFile);
    bool is_xm(reader const& aFile);
    bool is_it(reader const& aFile);
    bool is_dbm(reader const& aFile);
    bool is_mo3(reader const& aFile);
    module_data load_mod(reader const& aFile);
    module_data load_s3m(reader const& aFile);
    module_data load_xm(reader const& aFile);
    module_data load_it(reader const& aFile);
    module_data load_dbm(reader const& aFile);
    module_data load_mo3(reader const& aFile);

    // OpenMPT's extensions: its song properties (from "STPM"), its instrument properties ("XTPM"), and the
    // adjustments for modules saved by older versions
    void read_song_extensions(reader const& aFile, std::size_t aOffset, module_data& aModule);
    std::size_t read_instrument_extensions(reader const& aFile, std::size_t aOffset, std::vector<instrument>& aInstruments);
    void apply_openmpt_upgrades(module_data& aModule);

    // pan positions for MOD-style channel layouts (left, right, right, left, ...)
    void setup_amiga_panning(module_data& aModule);
}
