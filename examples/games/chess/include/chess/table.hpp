/*
neogfx C++ App/Game Engine - Examples - Games - Chess
Copyright(C) 2020 Leigh Johnston

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

#include <vector>
#include <chrono>

#include <chess/zobrist.hpp>

namespace chess
{
    struct table_entry
    {
        std::atomic<zobrist::hash_t> signature = zobrist::hash_t{};
        std::atomic<std::chrono::steady_clock::time_point> age = std::chrono::steady_clock::time_point{};
        std::atomic<std::int32_t> depth = 0;
        std::atomic<double> score = 0.0;

        table_entry() = default;
        table_entry(table_entry const& other) :
            signature{ other.signature.load() },
            age{ other.age.load() },
            depth{ other.depth.load() },
            score{ other.score.load() }
        {
        }

        zobrist::hash_t combined_hash(zobrist::hash_t aHash) const
        {
            auto result = aHash ^ age.load().time_since_epoch().count() ^ depth.load();
            double s = score.load();
            std::uint64_t n;
            std::memcpy(&n, &s, sizeof s);
            result ^= n;
            return result;
        }
    };

    std::size_t constexpr DEFAULT_TABLE_SIZE = 1000000000 / sizeof(table_entry);

    typedef std::vector<table_entry> table;
}