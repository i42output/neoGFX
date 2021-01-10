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
#include <stdexcept>

namespace chess
{
    enum class piece_cardinal : uint8_t
    {
        Pawn    = 0x00,
        Knight  = 0x01,
        Bishop  = 0x02,
        Rook    = 0x03,
        Queen   = 0x04,
        King    = 0x05,

        COUNT   = 0x06
    };

    enum class piece_color_cardinal : uint8_t
    {
        White   = 0x00,
        Black   = 0x01,

        COUNT   = 0x02
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

    template <typename T = std::size_t>
    inline T as_cardinal(piece p)
    {
        // todo: will bitwise operations be faster? profile and see.
        switch (p)
        {
        case piece::WhitePawn:
            return static_cast<T>(piece_cardinal::Pawn);
        case piece::WhiteKnight:
            return static_cast<T>(piece_cardinal::Knight);
        case piece::WhiteBishop:
            return static_cast<T>(piece_cardinal::Bishop);
        case piece::WhiteRook:
            return static_cast<T>(piece_cardinal::Rook);
        case piece::WhiteQueen:
            return static_cast<T>(piece_cardinal::Queen);
        case piece::WhiteKing:
            return static_cast<T>(piece_cardinal::King);
        case piece::BlackPawn:
            return static_cast<T>(piece_cardinal::Pawn);
        case piece::BlackKnight:
            return static_cast<T>(piece_cardinal::Knight);
        case piece::BlackBishop:
            return static_cast<T>(piece_cardinal::Bishop);
        case piece::BlackRook:
            return static_cast<T>(piece_cardinal::Rook);
        case piece::BlackQueen:
            return static_cast<T>(piece_cardinal::Queen);
        case piece::BlackKing:
            return static_cast<T>(piece_cardinal::King);
        default:
            throw std::logic_error("chess::as_cardinal");
        }
    }

    template <typename T = std::size_t>
    inline T as_color_cardinal(piece p)
    {
        // todo: will bitwise operations be faster? profile and see.
        switch (p)
        {
        case piece::White:
            return static_cast<T>(piece_color_cardinal::White);
        case piece::WhitePawn:
            return static_cast<T>(piece_color_cardinal::White);
        case piece::WhiteKnight:
            return static_cast<T>(piece_color_cardinal::White);
        case piece::WhiteBishop:
            return static_cast<T>(piece_color_cardinal::White);
        case piece::WhiteRook:
            return static_cast<T>(piece_color_cardinal::White);
        case piece::WhiteQueen:
            return static_cast<T>(piece_color_cardinal::White);
        case piece::WhiteKing:
            return static_cast<T>(piece_color_cardinal::White);
        case piece::Black:
            return static_cast<T>(piece_color_cardinal::Black);
        case piece::BlackPawn:
            return static_cast<T>(piece_color_cardinal::Black);
        case piece::BlackKnight:
            return static_cast<T>(piece_color_cardinal::Black);
        case piece::BlackBishop:
            return static_cast<T>(piece_color_cardinal::Black);
        case piece::BlackRook:
            return static_cast<T>(piece_color_cardinal::Black);
        case piece::BlackQueen:
            return static_cast<T>(piece_color_cardinal::Black);
        case piece::BlackKing:
            return static_cast<T>(piece_color_cardinal::Black);
        default:
            throw std::logic_error("chess::as_color_cardinal");
        }
    }

    inline piece operator&(piece lhs, piece rhs)
    {
        return static_cast<piece>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    inline piece operator|(piece lhs, piece rhs)
    {
        return static_cast<piece>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }

    inline piece piece_type(piece p)
    {
        return p & piece::TYPE_MASK;
    }

    inline piece piece_color(piece p)
    {
        return p & piece::COLOR_MASK;
    }

    struct invalid_piece_character : std::runtime_error { invalid_piece_character() : std::runtime_error{ "chess::invalid_piece_character" } {} };

    inline piece parse_piece_character(char aCharacter)
    {
        switch (aCharacter)
        {
        case 'p':
        case 'P':
            return piece::Pawn;
        case 'n':
        case 'N':
            return piece::Knight;
        case 'b':
        case 'B':
            return piece::Bishop;
        case 'r':
        case 'R':
            return piece::Rook;
        case 'q':
        case 'Q':
            return piece::Queen;
        case 'k':
        case 'K':
            return piece::King;
        default:
            throw invalid_piece_character();
        }
    }
}