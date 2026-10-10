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

#include <mod_tracker/module.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>

namespace mod_tracker
{
    namespace
    {
        constexpr std::size_t TITLE_LENGTH = 20u;
        constexpr std::size_t SAMPLE_HEADER_SIZE = 30u;
        constexpr std::size_t SAMPLE_NAME_LENGTH = 22u;
        constexpr std::size_t ORDER_TABLE_SIZE = 128u;
        constexpr std::size_t CELL_SIZE = 4u;

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
            std::uint16_t u16be(std::size_t aOffset) const
            {
                return static_cast<std::uint16_t>((u8(aOffset) << 8) | u8(aOffset + 1u));
            }
            std::string text(std::size_t aOffset, std::size_t aLength) const
            {
                std::string result;
                for (std::size_t index = 0u; index < aLength; ++index)
                {
                    auto const ch = static_cast<char>(u8(aOffset + index));
                    if (ch == '\0')
                        break;
                    // MOD names are 7-bit ASCII in practice; anything else is shown as a space
                    result.push_back(ch >= ' ' && ch <= '~' ? ch : ' ');
                }
                while (!result.empty() && result.back() == ' ')
                    result.pop_back();
                return result;
            }
            std::span<std::byte const> bytes(std::size_t aOffset, std::size_t aCount) const
            {
                if (aOffset >= iData.size())
                    return {};
                return iData.subspan(aOffset, std::min(aCount, iData.size() - aOffset));
            }
        private:
            std::span<std::byte const> iData;
        };

        std::optional<std::uint32_t> channels_from_signature(std::string const& aSignature)
        {
            if (aSignature.size() != 4u)
                return {};
            if (aSignature == "M.K." || aSignature == "M!K!" || aSignature == "M&K!" || aSignature == "N.T." ||
                aSignature == "FLT4" || aSignature == "4CHN")
                return 4u;
            if (aSignature == "FLT8" || aSignature == "OCTA" || aSignature == "OKTA" || aSignature == "CD81")
                return 8u;
            auto const digit = [](char aCh) { return aCh >= '0' && aCh <= '9'; };
            // nCHN: 1 to 9 channels
            if (digit(aSignature[0]) && aSignature.substr(1) == "CHN" && aSignature[0] != '0')
                return static_cast<std::uint32_t>(aSignature[0] - '0');
            // nnCH: 10 to 32 channels
            if (digit(aSignature[0]) && digit(aSignature[1]) && aSignature.substr(2) == "CH")
            {
                auto const channels = static_cast<std::uint32_t>((aSignature[0] - '0') * 10 + (aSignature[1] - '0'));
                if (channels >= 1u && channels <= 32u)
                    return channels;
            }
            // TDZn: TakeTracker, 1 to 3 channels
            if (aSignature.substr(0, 3) == "TDZ" && digit(aSignature[3]) && aSignature[3] != '0')
                return static_cast<std::uint32_t>(aSignature[3] - '0');
            return {};
        }

        char hex_digit(std::uint32_t aValue)
        {
            return "0123456789ABCDEF"[aValue & 0x0Fu];
        }

        cell decode_cell(reader const& aReader, std::size_t aOffset)
        {
            auto const b0 = aReader.u8(aOffset);
            auto const b1 = aReader.u8(aOffset + 1u);
            auto const b2 = aReader.u8(aOffset + 2u);
            auto const b3 = aReader.u8(aOffset + 3u);
            cell result;
            result.sample = static_cast<std::uint8_t>((b0 & 0xF0u) | (b2 >> 4u));
            result.period = static_cast<std::uint16_t>(((b0 & 0x0Fu) << 8u) | b1);
            result.effect = static_cast<std::uint8_t>(b2 & 0x0Fu);
            result.parameter = b3;
            if (result.period != 0u)
            {
                auto const note = period_note(result.period, 0);
                // only a period that is actually in the table gets a note name
                if (std::abs(static_cast<int>(note_period(note, 0)) - static_cast<int>(result.period)) <= 2)
                    result.note = static_cast<std::uint8_t>(note);
            }
            return result;
        }
    }

    module_data load_module(std::span<std::byte const> aData)
    {
        reader const data{ aData };

        // 31-sample modules carry a signature at offset 1080; without one this is (at best) a 15-sample module
        std::size_t sampleCount = 31u;
        std::string signature;
        std::optional<std::uint32_t> channels;
        if (data.has(1080u, 4u))
        {
            for (std::size_t index = 0u; index < 4u; ++index)
                signature.push_back(static_cast<char>(data.u8(1080u + index)));
            channels = channels_from_signature(signature);
        }
        if (channels == std::nullopt)
        {
            sampleCount = 15u;
            signature.clear();
            channels = 4u;
        }

        module_data result;
        result.channels = *channels;
        result.signature = signature;
        result.title = data.text(0u, TITLE_LENGTH);

        std::size_t offset = TITLE_LENGTH;
        result.samples.resize(sampleCount);
        std::vector<std::uint32_t> sampleLengths(sampleCount);
        for (std::size_t index = 0u; index < sampleCount; ++index, offset += SAMPLE_HEADER_SIZE)
        {
            auto& s = result.samples[index];
            s.name = data.text(offset, SAMPLE_NAME_LENGTH);
            sampleLengths[index] = static_cast<std::uint32_t>(data.u16be(offset + 22u)) * 2u;
            auto const finetune = static_cast<std::int8_t>(data.u8(offset + 24u) & 0x0Fu);
            s.finetune = static_cast<std::int8_t>(finetune >= 8 ? finetune - 16 : finetune);
            auto const volume = data.u8(offset + 25u);
            // a 15-sample module has no signature to identify it, so be strict about what else is plausible
            if (sampleCount == 15u && volume > 64u)
                throw module_load_error{ "not a MOD file" };
            s.volume = std::min<std::uint8_t>(volume, 64u);
            s.loopStart = static_cast<std::uint32_t>(data.u16be(offset + 26u)) * 2u;
            s.loopLength = static_cast<std::uint32_t>(data.u16be(offset + 28u)) * 2u;
        }

        auto const songLength = std::min<std::size_t>(data.u8(offset), ORDER_TABLE_SIZE);
        result.restart = data.u8(offset + 1u);
        offset += 2u;
        if (songLength == 0u)
            throw module_load_error{ "the song is empty" };
        // ProTracker counts every entry in the order table, not just those in the song, when working out
        // how many patterns are stored
        std::uint32_t patternCount = 0u;
        bool songEnded = false;
        for (std::size_t index = 0u; index < ORDER_TABLE_SIZE; ++index)
        {
            auto const order = data.u8(offset + index);
            // 0xFF ("---") ends the song and 0xFE ("+++") is skipped: markers written by some PC trackers
            if (index < songLength && !songEnded)
            {
                if (order == 0xFFu && !result.orders.empty())
                    songEnded = true;
                else if (order != 0xFEu && order != 0xFFu)
                    result.orders.push_back(order);
            }
            if (order < 128u)
                patternCount = std::max<std::uint32_t>(patternCount, order + 1u);
        }
        if (result.orders.empty())
            throw module_load_error{ "the song is empty" };
        offset += ORDER_TABLE_SIZE;
        if (sampleCount == 31u)
            offset += 4u; // the signature
        if (sampleCount == 15u && patternCount > 64u)
            throw module_load_error{ "not a MOD file" };

        // Startrekker's FLT8 stores each 8-channel pattern as a pair of 4-channel patterns (channels 1-4
        // then 5-8), and its order list counts in those halves
        bool const pairedPatterns = (signature == "FLT8");
        if (pairedPatterns)
            patternCount = (patternCount + 1u) & ~1u;
        auto const storedChannels = pairedPatterns ? 4u : result.channels;
        auto const patternSize = ROWS_PER_PATTERN * storedChannels * CELL_SIZE;
        if (!data.has(offset, patternSize * patternCount))
            throw module_load_error{ "pattern data is truncated" };
        std::vector<pattern> stored(patternCount);
        for (auto& p : stored)
        {
            p.cells.reserve(ROWS_PER_PATTERN * storedChannels);
            for (std::uint32_t index = 0u; index < ROWS_PER_PATTERN * storedChannels; ++index, offset += CELL_SIZE)
                p.cells.push_back(decode_cell(data, offset));
        }
        if (pairedPatterns)
        {
            for (std::uint32_t index = 0u; index < patternCount; index += 2u)
            {
                pattern merged;
                merged.cells.reserve(ROWS_PER_PATTERN * result.channels);
                for (std::uint32_t row = 0u; row < ROWS_PER_PATTERN; ++row)
                    for (auto const* half : { &stored[index], &stored[index + 1u] })
                        merged.cells.insert(merged.cells.end(),
                            std::next(half->cells.begin(), row * storedChannels), std::next(half->cells.begin(), (row + 1u) * storedChannels));
                result.patterns.push_back(std::move(merged));
            }
            for (auto& order : result.orders)
                order = static_cast<std::uint8_t>(order / 2u);
        }
        else
            result.patterns = std::move(stored);
        for (auto& order : result.orders)
            if (order >= result.patterns.size())
                order = 0u;

        // sample data follows the patterns; a truncated file keeps whatever sample data it does have
        for (std::size_t index = 0u; index < sampleCount; ++index)
        {
            auto& s = result.samples[index];
            auto const bytes = data.bytes(offset, sampleLengths[index]);
            s.data.resize(bytes.size());
            std::transform(bytes.begin(), bytes.end(), s.data.begin(), [](std::byte aByte) { return static_cast<std::int8_t>(aByte); });
            offset += sampleLengths[index];
            auto const length = static_cast<std::uint32_t>(s.data.size());
            // Soundtracker stored the loop start in bytes rather than words
            if (sampleCount == 15u && s.looped() && s.loop_end() > length && s.loopStart / 2u + s.loopLength <= length)
                s.loopStart /= 2u;
            if (s.loopStart >= length)
                s.loopStart = 0u, s.loopLength = 0u;
            else if (s.loop_end() > length)
                s.loopLength = length - s.loopStart;
        }
        return result;
    }

    module_data load_module(std::string const& aPath)
    {
        std::ifstream file{ aPath, std::ios::binary };
        if (!file)
            throw module_load_error{ "cannot open '" + aPath + "'" };
        std::vector<char> const contents{ std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
        return load_module(std::span<std::byte const>{ reinterpret_cast<std::byte const*>(contents.data()), contents.size() });
    }

    std::array<std::uint16_t, PERIOD_TABLE_NOTES> const& base_periods()
    {
        static constexpr std::array<std::uint16_t, PERIOD_TABLE_NOTES> sPeriods =
        {
            856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
            428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
            214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113
        };
        return sPeriods;
    }

    std::uint16_t note_period(std::uint32_t aNote, std::int32_t aFinetune)
    {
        static auto const sTable = []()
            {
                // ProTracker's finetuned tables are, to within a unit, the base table scaled by 2^(-finetune/96)
                std::array<std::array<std::uint16_t, PERIOD_TABLE_NOTES>, 16u> table = {};
                for (std::int32_t finetune = -8; finetune <= 7; ++finetune)
                    for (std::uint32_t note = 0u; note < PERIOD_TABLE_NOTES; ++note)
                        table[static_cast<std::size_t>(finetune + 8)][note] = finetune == 0 ? base_periods()[note] :
                            static_cast<std::uint16_t>(std::lround(base_periods()[note] * std::pow(2.0, -finetune / 96.0)));
                return table;
            }();
        return sTable[static_cast<std::size_t>(std::clamp(aFinetune, -8, 7) + 8)][std::min(aNote, PERIOD_TABLE_NOTES - 1u)];
    }

    std::uint32_t period_note(std::uint32_t aPeriod, std::int32_t aFinetune)
    {
        std::uint32_t best = 0u;
        auto bestDistance = std::abs(static_cast<int>(note_period(0u, aFinetune)) - static_cast<int>(aPeriod));
        for (std::uint32_t note = 1u; note < PERIOD_TABLE_NOTES; ++note)
        {
            auto const distance = std::abs(static_cast<int>(note_period(note, aFinetune)) - static_cast<int>(aPeriod));
            if (distance < bestDistance)
            {
                best = note;
                bestDistance = distance;
            }
        }
        return best;
    }

    std::string note_text(cell const& aCell)
    {
        static constexpr char const* sNames[12] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" };
        if (aCell.period == 0u)
            return "...";
        if (aCell.note == NO_NOTE)
            return "???";
        return std::string{ sNames[aCell.note % 12u] } + static_cast<char>('1' + aCell.note / 12u);
    }

    std::string sample_text(cell const& aCell)
    {
        if (aCell.sample == 0u)
            return "..";
        return std::string{ hex_digit(aCell.sample >> 4u), hex_digit(aCell.sample) };
    }

    std::string effect_text(cell const& aCell)
    {
        if (!aCell.has_effect())
            return "...";
        return std::string{ hex_digit(aCell.effect), hex_digit(aCell.parameter >> 4u), hex_digit(aCell.parameter) };
    }
}
