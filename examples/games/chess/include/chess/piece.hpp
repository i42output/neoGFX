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

#include <cstdint>

namespace chess
{
    enum class piece_cardinal : uint8_t
    {
        Pawn    = 0x00,
        Knight  = 0x01,
        Bishop  = 0x02,
        Rook    = 0x03,
        Queen   = 0x04,
        King    = 0x05
    };

    enum class piece : uint8_t
    {
        None        = 0x00,

        Pawn        = 0x01,
        Knight      = 0x02,
        Bishop      = 0x04,
        Rook        = 0x08,
        Queen       = 0x10,
        King        = 0x20,

        White       = 0x40,
        Black       = 0x80,

        WhitePawn   = White | Pawn,
        WhiteKnight = White | Knight,
        WhiteBishop = White | Bishop,
        WhiteRook   = White | Rook,
        WhiteQueen  = White | Queen,
        WhiteKing   = White | King,

        BlackPawn   = Black | Pawn,
        BlackKnight = Black | Knight,
        BlackBishop = Black | Bishop,
        BlackRook   = Black | Rook,
        BlackQueen  = Black | Queen,
        BlackKing   = Black | King,

        TYPE_MASK   = Pawn | Knight | Bishop | Rook | Queen | King,
        COLOR_MASK  = White | Black
    };

    inline piece operator&(piece lhs, piece rhs)
    {
        return static_cast<piece>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    inline piece operator|(piece lhs, piece rhs)
    {
        return static_cast<piece>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    inline piece piece_type(piece p)
    {
        return p & piece::TYPE_MASK;
    }

    inline piece piece_color(piece p)
    {
        return p & piece::COLOR_MASK;
    }
}