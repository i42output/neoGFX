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
    enum class player : std::uint8_t
    {
        Invalid = static_cast<std::uint8_t>(piece::None),

        White   = static_cast<std::uint8_t>(piece::White),
        Black   = static_cast<std::uint8_t>(piece::Black)
    };

    template <player Player>
    inline double player_piece_value(chess::piece piece);

    template <>
    inline double player_piece_value<chess::player::White>(chess::piece piece)
    {
        return piece_value<chess::piece::White>(piece);
    }

    template <>
    inline double player_piece_value<chess::player::Black>(chess::piece piece)
    {
        return piece_value<chess::piece::Black>(piece);
    }

    template <typename T = std::size_t>
    inline T as_cardinal(player p)
    {
        return as_color_cardinal<T>(static_cast<piece>(p));
    }

    template <player Player>
    constexpr player opponent_v;
    template <>
    constexpr player opponent_v<player::White> = player::Black;
    template <>
    constexpr player opponent_v<player::Black> = player::White;

    inline player opponent(player p)
    {
        return p == player::White ? opponent_v<player::White> : opponent_v<player::Black>;
    }
}