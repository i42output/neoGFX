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

#include <chess/chess.hpp>
#include <chess/piece.hpp>

namespace chess
{
    enum class player : uint8_t
    {
        Invalid = static_cast<uint8_t>(piece::None),

        White   = static_cast<uint8_t>(piece::White),
        Black   = static_cast<uint8_t>(piece::Black)
    };

    inline player next_player(player p)
    {
        return p == player::White ? player::Black : player::White;
    }

    template <player Player>
    constexpr player opponent_v;
    template <>
    constexpr player opponent_v<player::White> = player::Black;
    template <>
    constexpr player opponent_v<player::Black> = player::White;
}