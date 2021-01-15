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

#include <limits>
#include <chrono>

#include <chess/primitives.hpp>

namespace chess
{
    template<>
    struct move_tables<matrix>
    {
        typedef neogfx::point_i32 move_coordinates;
        typedef std::array<std::array<std::vector<move_coordinates>, static_cast<std::size_t>(piece_cardinal::COUNT)>, static_cast<std::size_t>(piece_color_cardinal::COUNT)> unit_moves;
        typedef std::array<bool, static_cast<std::size_t>(piece_cardinal::COUNT)> can_move_multiple;
        typedef std::array<std::array<std::array<std::array<std::array<std::array<bool, 8u>, 8u>, 8u>, 8u>, static_cast<std::size_t>(piece_cardinal::COUNT)>, static_cast<std::size_t>(piece_color_cardinal::COUNT)> valid_moves;
        unit_moves unitMoves;
        unit_moves unitCaptureMoves;
        can_move_multiple canMoveMultiple;
        valid_moves validMoves;
        valid_moves validCaptureMoves;
    };

    move_tables<matrix> generate_matrix_move_tables();

    template <bool IntoCheckTest = false>
    inline bool in_check(move_tables<matrix> const& aTables, player aPlayer, matrix_board const& aBoard);
        
    template<bool CheckTest = false, bool IntoCheckTest = false, bool DefendTest = false>
    inline bool can_move(move_tables<matrix> const& aTables, player aTurn, matrix_board const& aBoard, move const& aMove)
    {
        auto const movingPiece = piece_at(aBoard, aMove.from);
        auto const targetPiece = piece_at(aBoard, aMove.to);
        if (piece_color(movingPiece) != static_cast<piece>(aTurn))
            return false;
        if (!DefendTest && piece_color(targetPiece) == static_cast<piece>(aTurn))
            return false;
        if (!CheckTest && piece_type(targetPiece) == piece::King)
            return false;
        // non-capturing move...
        bool enPassant = false;
        bool castle = false;
        if (piece_type(targetPiece) == piece::None)
        {
            if (!aTables.validMoves[as_color_cardinal<>(movingPiece)][as_cardinal<>(movingPiece)][aMove.from.y][aMove.from.x][aMove.to.y][aMove.to.x])
            {
                if (piece_type(movingPiece) == piece::Pawn &&
                    aTables.validCaptureMoves[as_color_cardinal<>(movingPiece)][as_cardinal<>(movingPiece)][aMove.from.y][aMove.from.x][aMove.to.y][aMove.to.x] &&
                    aBoard.lastMove)
                {
                    auto const pieceLastMoved = piece_at(aBoard, aBoard.lastMove->to);
                    if (piece_type(pieceLastMoved) == piece::Pawn && piece_color(pieceLastMoved) != static_cast<piece>(aTurn))
                    {
                        auto const delta = aBoard.lastMove->to.as<int32_t>() - aBoard.lastMove->from.as<int32_t>();
                        if (std::abs(delta.dy) == 2)
                        {
                            auto const& deltaUnity = neogfx::delta_i32{ delta.dx != 0 ? delta.dx / std::abs(delta.dx) : 0, delta.dy != 0 ? delta.dy / std::abs(delta.dy) : 0 };
                            if (aBoard.lastMove->to.as<int32_t>() - deltaUnity == aMove.to.as<int32_t>())
                                enPassant = true;
                        }
                    }
                }
                else if (piece_type(movingPiece) == piece::King && (!aBoard.lastMove || !aBoard.lastMove->castlingState[as_color_cardinal<>(movingPiece)][static_cast<std::size_t>(move::castling_piece_index::King)]))
                {
                    if (aMove.to.y == aMove.from.y)
                    {
                        if ((aMove.to.x == 2 && !aBoard.lastMove->castlingState[as_color_cardinal<>(movingPiece)][static_cast<std::size_t>(move::castling_piece_index::QueensRook)]) ||
                            (aMove.to.x == 6 && !aBoard.lastMove->castlingState[as_color_cardinal<>(movingPiece)][static_cast<std::size_t>(move::castling_piece_index::KingsRook)]))
                            castle = !in_check(aTables, aTurn, aBoard);
                    }
                }
                if (!enPassant && !castle)
                    return false;
            }
            else if (movingPiece == piece::WhitePawn && aMove.to.y - aMove.from.y == 2u && piece_at(aBoard, coordinates{ aMove.from.x, aMove.from.y + 1u }) != piece::None)
                return false;
            else if (movingPiece == piece::BlackPawn && aMove.from.y - aMove.to.y == 2u && piece_at(aBoard, coordinates{ aMove.from.x, aMove.from.y - 1u }) != piece::None)
                return false;
        }
        // capturing move...
        if (piece_type(targetPiece) != piece::None && !aTables.validCaptureMoves[as_color_cardinal<>(movingPiece)][as_cardinal<>(movingPiece)][aMove.from.y][aMove.from.x][aMove.to.y][aMove.to.x])
            return false;
        // any pieces in the way?
        if (aTables.canMoveMultiple[as_cardinal<>(movingPiece)] || castle)
        {
            // todo: would a move array be faster than calculating/using deltas? profile and see.
            auto const delta = move_tables<matrix>::move_coordinates{ static_cast<int32_t>(aMove.to.x), static_cast<int32_t>(aMove.to.y) } - move_tables<matrix>::move_coordinates{ static_cast<int32_t>(aMove.from.x), static_cast<int32_t>(aMove.from.y) };
            auto const& deltaUnity = neogfx::delta_i32{ delta.dx != 0 ? delta.dx / std::abs(delta.dx) : 0, delta.dy != 0 ? delta.dy / std::abs(delta.dy) : 0 };
            auto const start = aMove.from.as<int32_t>() + deltaUnity;
            auto const end = (!castle ? aMove.to.as<int32_t>() : aMove.to.as<int32_t>().with_x(aMove.to.x == 2 ? 0 : 7));
            for (move_tables<matrix>::move_coordinates pos = start; pos != end; pos += deltaUnity)
            {
                auto const inbetweenPiece = piece_at(aBoard, pos);
                if (piece_type(inbetweenPiece) != piece::None)
                    return false;
                if (castle)
                {
                    aBoard.checkTest = move{ aMove.from, pos };
                    bool inCheck = in_check(aTables, aTurn, aBoard);
                    aBoard.checkTest = std::nullopt;
                    if (inCheck)
                        return false;
                }
            }
        }
        if (!IntoCheckTest)
        {
            aBoard.checkTest = aMove;
            bool inCheck = in_check<true>(aTables, aTurn, aBoard);
            aBoard.checkTest = std::nullopt;
            if (inCheck)
                return false;
        }
        // todo: corner cases (check, en passant, castling)
        return true;
    }

    template <bool IntoCheckTest>
    inline bool in_check(move_tables<matrix> const& aTables, player aPlayer, matrix_board const& aBoard)
    {
        auto const opponent = next_player(aPlayer);
        auto const kingPosition = king_position(aBoard, static_cast<piece>(aPlayer));
        for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
            for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
            {
                move const tryMove{ coordinates{ xFrom, yFrom }, kingPosition };
                if (tryMove.from == kingPosition)
                    continue;
                if (can_move<true, IntoCheckTest>(aTables, opponent, aBoard, tryMove))
                    return true;
            }
        return false;
    }

    template <player Player, typename ResultContainer>
    inline void valid_moves(move_tables<matrix> const& aTables, matrix_board const& aBoard, ResultContainer& aResult, bool aSort = false)
    {
        aResult.clear();
        for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
            for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
                for (coordinate xTo = 0u; xTo <= 7u; ++xTo)
                    for (coordinate yTo = 0u; yTo <= 7u; ++yTo)
                    {
                        move candidateMove{ { xFrom, yFrom }, { xTo, yTo } };
                        if (can_move<>(aTables, Player, aBoard, candidateMove))
                        {
                            auto const movingPiece = piece_at(aBoard, candidateMove.from);
                            if (piece_type(movingPiece) == piece::Pawn)
                            {
                                auto const movingPieceColor = piece_color(movingPiece);
                                if ((movingPieceColor == piece::White && candidateMove.to.y == promotion_rank_v<player::White>) ||
                                    (movingPieceColor == piece::Black && candidateMove.to.y == promotion_rank_v<player::Black>))
                                {
                                    candidateMove.promoteTo = piece::Queen | movingPieceColor;
                                    aResult.push_back(candidateMove);
                                    candidateMove.promoteTo = piece::Knight | movingPieceColor;
                                    aResult.push_back(candidateMove);
                                    // todo: do we care about bishop and rook promotion?
                                }
                                else
                                    aResult.push_back(candidateMove);
                            }
                            else
                                aResult.push_back(candidateMove);
                        }
                    }
        if (aSort)
            std::sort(aResult.begin(), aResult.end(), [&](auto const& lhs, auto const& rhs)
            {
                matrix_board lhsBoard = aBoard;
                matrix_board rhsBoard = aBoard;
                move_piece(lhsBoard, lhs);
                move_piece(rhsBoard, rhs);
                return eval<matrix, Player>{}(aTables, lhsBoard, 2.0).eval < eval<matrix, Player>{}(aTables, rhsBoard, 2.0).eval;
            });
    }

}