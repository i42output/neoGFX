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

#include <neogfx/core/geometrical.hpp>

#include <chess/piece.hpp>
#include <chess/player.hpp>

namespace chess
{
    typedef neogfx::point_u32 coordinates;
    typedef coordinates::coordinate_type coordinate;
    
    struct move
    {
        coordinates from;
        coordinates to;
        std::optional<piece> promoteTo;
        enum class castling_piece_index : uint32_t
        {
            QueensRook  = 0x0,
            King        = 0x1,
            KingsRook   = 0x2,

            COUNT
        };
        std::array<std::array<bool, static_cast<std::size_t>(castling_piece_index::COUNT)>, static_cast<std::size_t>(piece_color_cardinal::COUNT)> castlingState;
    };

    inline std::string to_string(move const& aMove)
    {
        return { static_cast<char>('a' + aMove.from.x), static_cast<char>('1' + aMove.from.y), static_cast<char>('a' + aMove.to.x), static_cast<char>('1' + aMove.to.y) };
    }

    typedef std::array<std::array<piece, 8>, 8> matrix;

    struct piece_bitboard
    {
        piece piece;
        uint64_t position;
    };
    typedef std::array<std::array<piece_bitboard, 16>, static_cast<std::size_t>(piece_color_cardinal::COUNT)> bitboard;

    template <typename Representation>
    struct basic_board
    {
        Representation position;
        std::array<coordinates, static_cast<std::size_t>(piece_color_cardinal::COUNT)> kings;
        player turn;
        std::optional<move> lastMove;
        mutable std::optional<move> checkTest;
    };

    using matrix_board = basic_board<matrix>;
    using bitboard_board = basic_board<bitboard>;

    using board = matrix_board;

    template <typename Representation>
    struct move_tables;
    template <typename Representation>
    move_tables<Representation> generate_move_tables();

    inline piece piece_at(matrix_board const& aBoard, coordinates aPosition)
    {
        auto const targetPiece = aBoard.position[aPosition.y][aPosition.x];
        if (!aBoard.checkTest)
            return targetPiece;
        if (aPosition == aBoard.checkTest->from)
            return piece::None;
        auto const movingPiece = !aBoard.checkTest->promoteTo ? aBoard.position[aBoard.checkTest->from.y][aBoard.checkTest->from.x] : *aBoard.checkTest->promoteTo;
        if (aPosition == aBoard.checkTest->to)
            return movingPiece;
        // en passant
        if (piece_type(targetPiece) == piece::Pawn && piece_type(movingPiece) == piece::Pawn &&
            piece_color(targetPiece) != piece_color(movingPiece))
        {
            if (aBoard.checkTest->from.x != aBoard.checkTest->to.x)
            {
                if (aBoard.lastMove && aPosition == aBoard.lastMove->to)
                {
                    if (std::abs(static_cast<int32_t>(aBoard.lastMove->from.y) - static_cast<int32_t>(aBoard.lastMove->to.y)) == 2)
                    {
                        if (aBoard.checkTest->to == coordinates{ aBoard.lastMove->to.x, piece_color(movingPiece) == piece::White ? aBoard.lastMove->to.y + 1 : aBoard.lastMove->to.y - 1 })
                        {
                            return piece::None;
                        }
                    }
                }
            }
        }
        return targetPiece;
    }

    inline coordinates king_position(matrix_board const& aBoard, piece aKing)
    {
        auto const kingPosition = aBoard.kings[as_color_cardinal<>(aKing)];
        if (aBoard.checkTest && aBoard.checkTest->from == kingPosition)
            return aBoard.checkTest->to;
        return kingPosition;
    }

    inline void move_piece(matrix_board& aBoard, chess::move const& aMove)
    {
        auto& source = aBoard.position[aMove.from.y][aMove.from.x];
        auto const movingPiece = source;
        auto& destination = aBoard.position[aMove.to.y][aMove.to.x];
        auto const targetPiece = destination;
        destination = (!aMove.promoteTo ? source : *aMove.promoteTo);
        source = piece::None;
        if (!aBoard.lastMove)
            aBoard.lastMove = aMove;
        else
        {
            aBoard.lastMove->from = aMove.from;
            aBoard.lastMove->to = aMove.to;
            aBoard.lastMove->promoteTo = aMove.promoteTo;
        }
        switch (piece_type(movingPiece))
        {
        case piece::King:
            aBoard.kings[as_color_cardinal<>(destination)] = aMove.to;
            aBoard.lastMove->castlingState[as_color_cardinal<>(movingPiece)][static_cast<std::size_t>(move::castling_piece_index::King)] = true;
            if (aMove.from.x - aMove.to.x == 2)
            {
                // queenside castling
                aBoard.lastMove->castlingState[as_color_cardinal<>(movingPiece)][static_cast<std::size_t>(move::castling_piece_index::QueensRook)] = true;
                aBoard.position[aMove.from.y][0u] = piece::None;
                aBoard.position[aMove.from.y][3u] = piece_color(movingPiece) | piece::Rook;
            }
            else if (aMove.to.x - aMove.from.x == 2)
            {
                // kingside castling
                aBoard.lastMove->castlingState[as_color_cardinal<>(movingPiece)][static_cast<std::size_t>(move::castling_piece_index::KingsRook)] = true;
                aBoard.position[aMove.from.y][7u] = piece::None;
                aBoard.position[aMove.from.y][5u] = piece_color(movingPiece) | piece::Rook;
            }
            break;
        case piece::Rook:
            if (aMove.from == coordinates{ 0u, 0u } || aMove.from == coordinates{ 0u, 7u })
                aBoard.lastMove->castlingState[as_color_cardinal<>(movingPiece)][static_cast<std::size_t>(move::castling_piece_index::QueensRook)] = true;
            else if (aMove.from == coordinates{ 7u, 0u } || aMove.from == coordinates{ 7u, 7u })
                aBoard.lastMove->castlingState[as_color_cardinal<>(movingPiece)][static_cast<std::size_t>(move::castling_piece_index::KingsRook)] = true;
            break;
        case piece::Pawn:
            // en passant
            if (targetPiece == piece::None && aMove.from.x != aMove.to.x)
                aBoard.position[piece_color(movingPiece) == piece::White ? aMove.to.y - 1 : aMove.to.y + 1][aMove.to.x] = piece::None;
            break;
        default:
            // do nothing
            break;
        }
        aBoard.turn = next_player(aBoard.turn);
    }

    inline void move_piece(bitboard_board& aBoard, chess::move const& aMove)
    {
        // todo
    }

    struct invalid_uci_move : std::runtime_error { invalid_uci_move() : std::runtime_error{ "chess::invalid_uci_move" } {} };

    inline move parse_uci_move(std::string const& aMove)
    {
        if (aMove.size() != 4 && aMove.size() != 5)
            throw invalid_uci_move();
        if (aMove[0] < 'a' || aMove[0] > 'h'|| aMove[1] < '1' || aMove[1] > '8' || aMove[2] < 'a' || aMove[2] > 'h' || aMove[3] < '1' || aMove[3] > '8')
            throw invalid_uci_move();
        move result;
        result.from.x = aMove[0] - 'a';
        result.from.y = aMove[1] - '1';
        result.to.x = aMove[2] - 'a';
        result.to.y = aMove[3] - '1';
        if (aMove.size() == 5)
            result.promoteTo = parse_piece_character(aMove[4]);
        return result;
    }

    template <typename Representation>
    struct setup;

    template <>
    struct setup<matrix>
    {
        static matrix_board const& position()
        {
            static constexpr matrix_board position
            {
                {{
                    { piece::WhiteRook, piece::WhiteKnight, piece::WhiteBishop, piece::WhiteQueen, piece::WhiteKing, piece::WhiteBishop, piece::WhiteKnight, piece::WhiteRook },
                    { piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn },
                    {}, {}, {}, {},
                    { piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn },
                    { piece::BlackRook, piece::BlackKnight, piece::BlackBishop, piece::BlackQueen, piece::BlackKing, piece::BlackBishop, piece::BlackKnight, piece::BlackRook },
                }},
                {{
                    { 4u, 0u }, { 4u, 7u }
                }},
                player::White
            };
            return position;
        }
    };

    template <>
    struct setup<bitboard>
    {
        static bitboard_board const& position()
        {
            static constexpr bitboard_board position
            {
                // todo
                {},
                {{
                    { 4u, 0u }, { 4u, 7u }
                }},
                player::White
            };
            return position;
        };
    };
}