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
#include <random>

#include <chess/position.hpp>

namespace chess::zobrist
{
    enum class piece_index : std::size_t
    {
        WhitePawn               = 0x00,
        WhiteKnight             = 0x01,
        WhiteBishop             = 0x02,
        WhiteRook               = 0x03,
        WhiteQueen              = 0x04,
        WhiteKing               = 0x05,
        BlackPawn               = 0x06,
        BlackKnight             = 0x07,
        BlackBishop             = 0x08,
        BlackRook               = 0x09,
        BlackQueen              = 0x0A,
        BlackKing               = 0x0B,

        COUNT
    };

    inline piece_index operator~(piece_index lhs)
    {
        return static_cast<piece_index>(~static_cast<std::size_t>(lhs));
    }

    inline piece_index operator&(piece_index lhs, piece_index rhs)
    {
        return static_cast<piece_index>(static_cast<std::size_t>(lhs) & static_cast<std::size_t>(rhs));
    }

    inline piece_index operator|(piece_index lhs, piece_index rhs)
    {
        return static_cast<piece_index>(static_cast<std::size_t>(lhs) | static_cast<std::size_t>(rhs));
    }

    inline piece_index to_piece_index(piece aPiece)
    {
        switch (aPiece)
        {
        case piece::WhitePawn:
            return piece_index::WhitePawn;
        case piece::WhiteKnight:
            return piece_index::WhiteKnight;
        case piece::WhiteBishop:
            return piece_index::WhiteBishop;
        case piece::WhiteRook:
            return piece_index::WhiteRook;
        case piece::WhiteQueen:
            return piece_index::WhiteQueen;
        case piece::WhiteKing:
            return piece_index::WhiteKing;
        case piece::BlackPawn:
            return piece_index::BlackPawn;
        case piece::BlackKnight:
            return piece_index::BlackKnight;
        case piece::BlackBishop:
            return piece_index::BlackBishop;
        case piece::BlackRook:
            return piece_index::BlackRook;
        case piece::BlackQueen:
            return piece_index::BlackQueen;
        case piece::BlackKing:
            return piece_index::BlackKing;
        default:
            throw std::logic_error{ "chess::zobrist::to_piece_index" };
        }
    }

    inline constexpr std::size_t to_index(piece_index aPieceIndex)
    {
        return static_cast<std::size_t>(aPieceIndex);
    }

    typedef std::uint64_t bitstring_t;

    struct keys
    {
        std::array<std::array<bitstring_t, to_index(piece_index::COUNT)>, SQUARES> pieces;
        bitstring_t blackToMove;
        bitstring_t enPassant[8];
        bitstring_t castling[4];
    };

    inline keys const& create_keys()
    {
        keys result;
        
        std::mt19937_64 rand64;
        for (std::size_t sq = 0u; sq < SQUARES; ++sq)
        {
            result.pieces[sq][to_index(piece_index::WhitePawn)] = rand64();
            result.pieces[sq][to_index(piece_index::WhiteKnight)] = rand64();
            result.pieces[sq][to_index(piece_index::WhiteBishop)] = rand64();
            result.pieces[sq][to_index(piece_index::WhiteRook)] = rand64();
            result.pieces[sq][to_index(piece_index::WhiteQueen)] = rand64();
            result.pieces[sq][to_index(piece_index::WhiteKing)] = rand64();
            result.pieces[sq][to_index(piece_index::BlackPawn)] = rand64();
            result.pieces[sq][to_index(piece_index::BlackKnight)] = rand64();
            result.pieces[sq][to_index(piece_index::BlackBishop)] = rand64();
            result.pieces[sq][to_index(piece_index::BlackRook)] = rand64();
            result.pieces[sq][to_index(piece_index::BlackQueen)] = rand64();
            result.pieces[sq][to_index(piece_index::BlackKing)] = rand64();
        }
        result.blackToMove = rand64();
        for (std::size_t file = 0u; file < 8u; ++file)
            result.enPassant[file] = rand64();
        for (std::size_t cr = 0u; cr < 4u; ++cr)
            result.castling[cr] = rand64();
        return result;
    }

    inline keys const& get_keys()
    {
        static keys const sKeys = create_keys();
        return sKeys;
    }

    typedef bitstring_t hash_t;

    inline hash_t hash(bitboard_position const& aBoard)
    {
        hash_t hash = 0ull;

        for (std::size_t sq = 0u; sq < SQUARES; ++sq)
            if (aBoard.rep.bySquare[sq] != piece::None)
                hash ^= get_keys().pieces[sq][to_index(to_piece_index(aBoard.rep.bySquare[sq]))];

        if (aBoard.turn == player::Black)
            hash ^= get_keys().blackToMove;

        // todo: en passant and castling

        return hash;
    }

    inline hash_t hash(mailbox_position const& aBoard)
    {
        hash_t hash = 0ull;

        for (std::size_t sq = 0u; sq < SQUARES; ++sq)
            if (aBoard.rep[sq / 8u][sq % 8u] != piece::None)
                hash ^= get_keys().pieces[sq][to_index(to_piece_index(aBoard.rep[sq / 8u][sq % 8u]))];

        if (aBoard.turn == player::Black)
            hash ^= get_keys().blackToMove;

        // todo: en passant and castling

        return hash;
    }
}