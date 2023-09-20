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

#include <chess/chess.hpp>

namespace chess
{
    enum class piece_cardinal : std::uint8_t
    {
        Pawn    = 0x00,
        Knight  = 0x01,
        Bishop  = 0x02,
        Rook    = 0x03,
        Queen   = 0x04,
        King    = 0x05,

        COUNT   = 0x06
    };

    constexpr std::size_t PIECE_TYPES = static_cast<std::size_t>(piece_cardinal::COUNT);

    enum class piece_color_cardinal : std::uint8_t
    {
        White   = 0x00,
        Black   = 0x01,

        COUNT   = 0x02
    };

    constexpr std::size_t PIECE_COLORS = static_cast<std::size_t>(piece_color_cardinal::COUNT);

    enum class piece : std::uint8_t
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

    std::array<piece_cardinal, 256> constexpr sPieceCardinals = []()
    {
        std::array<piece_cardinal, 256> pieceCardinals = {};
        pieceCardinals[static_cast<std::size_t>(piece::Pawn)] = piece_cardinal::Pawn;
        pieceCardinals[static_cast<std::size_t>(piece::WhitePawn)] = piece_cardinal::Pawn;
        pieceCardinals[static_cast<std::size_t>(piece::BlackPawn)] = piece_cardinal::Pawn;
        pieceCardinals[static_cast<std::size_t>(piece::Knight)] = piece_cardinal::Knight;
        pieceCardinals[static_cast<std::size_t>(piece::WhiteKnight)] = piece_cardinal::Knight;
        pieceCardinals[static_cast<std::size_t>(piece::BlackKnight)] = piece_cardinal::Knight;
        pieceCardinals[static_cast<std::size_t>(piece::Bishop)] = piece_cardinal::Bishop;
        pieceCardinals[static_cast<std::size_t>(piece::WhiteBishop)] = piece_cardinal::Bishop;
        pieceCardinals[static_cast<std::size_t>(piece::BlackBishop)] = piece_cardinal::Bishop;
        pieceCardinals[static_cast<std::size_t>(piece::Rook)] = piece_cardinal::Rook;
        pieceCardinals[static_cast<std::size_t>(piece::WhiteRook)] = piece_cardinal::Rook;
        pieceCardinals[static_cast<std::size_t>(piece::BlackRook)] = piece_cardinal::Rook;
        pieceCardinals[static_cast<std::size_t>(piece::Queen)] = piece_cardinal::Queen;
        pieceCardinals[static_cast<std::size_t>(piece::WhiteQueen)] = piece_cardinal::Queen;
        pieceCardinals[static_cast<std::size_t>(piece::BlackQueen)] = piece_cardinal::Queen;
        pieceCardinals[static_cast<std::size_t>(piece::King)] = piece_cardinal::King;
        pieceCardinals[static_cast<std::size_t>(piece::WhiteKing)] = piece_cardinal::King;
        pieceCardinals[static_cast<std::size_t>(piece::BlackKing)] = piece_cardinal::King;
        return pieceCardinals;
    }();

    template <typename T = std::size_t>
    inline T as_cardinal(piece p)
    {
        return static_cast<T>(sPieceCardinals[static_cast<std::size_t>(p)]);
    }

    std::array<piece, PIECE_TYPES> constexpr sCardinalPieceTypes = []()
    {
        std::array<piece, PIECE_TYPES> pieceTypes = {};
        pieceTypes[static_cast<std::size_t>(piece_cardinal::Pawn)] = piece::Pawn;
        pieceTypes[static_cast<std::size_t>(piece_cardinal::Knight)] = piece::Knight;
        pieceTypes[static_cast<std::size_t>(piece_cardinal::Bishop)] = piece::Bishop;
        pieceTypes[static_cast<std::size_t>(piece_cardinal::Rook)] = piece::Rook;
        pieceTypes[static_cast<std::size_t>(piece_cardinal::Queen)] = piece::Queen;
        pieceTypes[static_cast<std::size_t>(piece_cardinal::King)] = piece::King;
        return pieceTypes;
    }();

    template <typename T>
    inline piece cardinal_to_piece(T c)
    {
        return sCardinalPieceTypes[static_cast<std::size_t>(c)];
    }

    std::array<piece_color_cardinal, 256> constexpr sPieceColorCardinals = []()
    {
        std::array<piece_color_cardinal, 256> pieceColorCardinals = {};
        pieceColorCardinals[static_cast<std::size_t>(piece::White)] = piece_color_cardinal::White;
        pieceColorCardinals[static_cast<std::size_t>(piece::Black)] = piece_color_cardinal::Black;
        pieceColorCardinals[static_cast<std::size_t>(piece::WhitePawn)] = piece_color_cardinal::White;
        pieceColorCardinals[static_cast<std::size_t>(piece::BlackPawn)] = piece_color_cardinal::Black;
        pieceColorCardinals[static_cast<std::size_t>(piece::WhiteKnight)] = piece_color_cardinal::White;
        pieceColorCardinals[static_cast<std::size_t>(piece::BlackKnight)] = piece_color_cardinal::Black;
        pieceColorCardinals[static_cast<std::size_t>(piece::WhiteBishop)] = piece_color_cardinal::White;
        pieceColorCardinals[static_cast<std::size_t>(piece::BlackBishop)] = piece_color_cardinal::Black;
        pieceColorCardinals[static_cast<std::size_t>(piece::WhiteRook)] = piece_color_cardinal::White;
        pieceColorCardinals[static_cast<std::size_t>(piece::BlackRook)] = piece_color_cardinal::Black;
        pieceColorCardinals[static_cast<std::size_t>(piece::WhiteQueen)] = piece_color_cardinal::White;
        pieceColorCardinals[static_cast<std::size_t>(piece::BlackQueen)] = piece_color_cardinal::Black;
        pieceColorCardinals[static_cast<std::size_t>(piece::WhiteKing)] = piece_color_cardinal::White;
        pieceColorCardinals[static_cast<std::size_t>(piece::BlackKing)] = piece_color_cardinal::Black;
        return pieceColorCardinals;
    }();

    template <typename T = std::size_t>
    inline T as_color_cardinal(piece p)
    {
        return static_cast<T>(sPieceColorCardinals[static_cast<std::size_t>(p)]);
    }

    inline piece operator&(piece lhs, piece rhs)
    {
        return static_cast<piece>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
    }

    inline piece operator|(piece lhs, piece rhs)
    {
        return static_cast<piece>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
    }

    std::array<piece, 256> constexpr sPieceTypes = []()
    {
        std::array<piece, 256> pieceTypes = {};
        pieceTypes[static_cast<std::size_t>(piece::Pawn)] = piece::Pawn;
        pieceTypes[static_cast<std::size_t>(piece::WhitePawn)] = piece::Pawn;
        pieceTypes[static_cast<std::size_t>(piece::BlackPawn)] = piece::Pawn;
        pieceTypes[static_cast<std::size_t>(piece::Knight)] = piece::Knight;
        pieceTypes[static_cast<std::size_t>(piece::WhiteKnight)] = piece::Knight;
        pieceTypes[static_cast<std::size_t>(piece::BlackKnight)] = piece::Knight;
        pieceTypes[static_cast<std::size_t>(piece::Bishop)] = piece::Bishop;
        pieceTypes[static_cast<std::size_t>(piece::WhiteBishop)] = piece::Bishop;
        pieceTypes[static_cast<std::size_t>(piece::BlackBishop)] = piece::Bishop;
        pieceTypes[static_cast<std::size_t>(piece::Rook)] = piece::Rook;
        pieceTypes[static_cast<std::size_t>(piece::WhiteRook)] = piece::Rook;
        pieceTypes[static_cast<std::size_t>(piece::BlackRook)] = piece::Rook;
        pieceTypes[static_cast<std::size_t>(piece::Queen)] = piece::Queen;
        pieceTypes[static_cast<std::size_t>(piece::WhiteQueen)] = piece::Queen;
        pieceTypes[static_cast<std::size_t>(piece::BlackQueen)] = piece::Queen;
        pieceTypes[static_cast<std::size_t>(piece::King)] = piece::King;
        pieceTypes[static_cast<std::size_t>(piece::WhiteKing)] = piece::King;
        pieceTypes[static_cast<std::size_t>(piece::BlackKing)] = piece::King;
        return pieceTypes;
    }();

    std::array<piece, 256> constexpr sPieceColors = []()
    {
        std::array<piece, 256> pieceColors = {};
        pieceColors[static_cast<std::size_t>(piece::WhitePawn)] = piece::White;
        pieceColors[static_cast<std::size_t>(piece::BlackPawn)] = piece::Black;
        pieceColors[static_cast<std::size_t>(piece::WhiteKnight)] = piece::White;
        pieceColors[static_cast<std::size_t>(piece::BlackKnight)] = piece::Black;
        pieceColors[static_cast<std::size_t>(piece::WhiteBishop)] = piece::White;
        pieceColors[static_cast<std::size_t>(piece::BlackBishop)] = piece::Black;
        pieceColors[static_cast<std::size_t>(piece::WhiteRook)] = piece::White;
        pieceColors[static_cast<std::size_t>(piece::BlackRook)] = piece::Black;
        pieceColors[static_cast<std::size_t>(piece::WhiteQueen)] = piece::White;
        pieceColors[static_cast<std::size_t>(piece::BlackQueen)] = piece::Black;
        pieceColors[static_cast<std::size_t>(piece::WhiteKing)] = piece::White;
        pieceColors[static_cast<std::size_t>(piece::BlackKing)] = piece::Black;
        return pieceColors;
    }();

    std::array<double, 256> constexpr sPieceValuesWhite = []()
    {
        std::array<double, 256> pieceValuesWhite = {};
        pieceValuesWhite[static_cast<std::size_t>(piece::Pawn)] = 1.0;
        pieceValuesWhite[static_cast<std::size_t>(piece::WhitePawn)] = 1.0;
        pieceValuesWhite[static_cast<std::size_t>(piece::BlackPawn)] = -1.0;
        pieceValuesWhite[static_cast<std::size_t>(piece::Knight)] = 3.05;
        pieceValuesWhite[static_cast<std::size_t>(piece::WhiteKnight)] = 3.05;
        pieceValuesWhite[static_cast<std::size_t>(piece::BlackKnight)] = -3.05;
        pieceValuesWhite[static_cast<std::size_t>(piece::Bishop)] = 3.33;
        pieceValuesWhite[static_cast<std::size_t>(piece::WhiteBishop)] = 3.33;
        pieceValuesWhite[static_cast<std::size_t>(piece::BlackBishop)] = -3.33;
        pieceValuesWhite[static_cast<std::size_t>(piece::Rook)] = 5.63;
        pieceValuesWhite[static_cast<std::size_t>(piece::WhiteRook)] = 5.63;
        pieceValuesWhite[static_cast<std::size_t>(piece::BlackRook)] = -5.63;
        pieceValuesWhite[static_cast<std::size_t>(piece::Queen)] = 9.5;
        pieceValuesWhite[static_cast<std::size_t>(piece::WhiteQueen)] = 9.5;
        pieceValuesWhite[static_cast<std::size_t>(piece::BlackQueen)] = -9.5;
        return pieceValuesWhite;
    }();

    std::array<double, 256> constexpr sPieceValuesBlack = []()
    {
        std::array<double, 256> pieceValuesBlack = {};
        pieceValuesBlack[static_cast<std::size_t>(piece::Pawn)] = 1.0;
        pieceValuesBlack[static_cast<std::size_t>(piece::WhitePawn)] = -1.0;
        pieceValuesBlack[static_cast<std::size_t>(piece::BlackPawn)] = 1.0;
        pieceValuesBlack[static_cast<std::size_t>(piece::Knight)] = 3.05;
        pieceValuesBlack[static_cast<std::size_t>(piece::WhiteKnight)] = -3.05;
        pieceValuesBlack[static_cast<std::size_t>(piece::BlackKnight)] = 3.05;
        pieceValuesBlack[static_cast<std::size_t>(piece::Bishop)] = 3.33;
        pieceValuesBlack[static_cast<std::size_t>(piece::WhiteBishop)] = -3.33;
        pieceValuesBlack[static_cast<std::size_t>(piece::BlackBishop)] = 3.33;
        pieceValuesBlack[static_cast<std::size_t>(piece::Rook)] = 5.63;
        pieceValuesBlack[static_cast<std::size_t>(piece::WhiteRook)] = -5.63;
        pieceValuesBlack[static_cast<std::size_t>(piece::BlackRook)] = 5.63;
        pieceValuesBlack[static_cast<std::size_t>(piece::Queen)] = 9.5;
        pieceValuesBlack[static_cast<std::size_t>(piece::WhiteQueen)] = -9.5;
        pieceValuesBlack[static_cast<std::size_t>(piece::BlackQueen)] = 9.5;
        return pieceValuesBlack;
    }();

    inline piece piece_type(piece p)
    {
        return sPieceTypes[static_cast<std::size_t>(p)];
    }

    inline piece piece_color(piece p)
    {
        return sPieceColors[static_cast<std::size_t>(p)];
    }

    inline piece piece_opponent_color(piece p)
    {
        return piece_color(p) == piece::White ? piece::Black : piece::White;
    }

    template <chess::piece Color>
    inline double piece_value(chess::piece piece);

    template <>
    inline double piece_value<chess::piece::White>(chess::piece piece)
    {
        return sPieceValuesWhite[static_cast<std::size_t>(piece)];
    }

    template <>
    inline double piece_value<chess::piece::Black>(chess::piece piece)
    {
        return sPieceValuesBlack[static_cast<std::size_t>(piece)];
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