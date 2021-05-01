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
    struct move_tables<mailbox_rep>
    {
        typedef neogfx::point_i32 move_coordinates;
        typedef std::array<std::array<std::vector<move_coordinates>, PIECES>, PIECE_COLORS> unit_moves;
        typedef std::array<bool, PIECES> can_move_multiple;
        typedef std::array<std::array<std::array<std::array<std::array<std::array<bool, 8u>, 8u>, 8u>, 8u>, PIECES>, PIECE_COLORS> valid_moves;
        typedef std::pair<std::size_t, std::array<move_coordinates, 8>> move_path;
        typedef std::array<std::array<std::array<std::array<bool, 8u>, 8u>, 8u>, 8u> trivial_moves;
        typedef std::array<std::array<std::array<std::array<move_path, 8u>, 8u>, 8u>, 8u> move_paths;
        unit_moves unitMoves;
        unit_moves unitCaptureMoves;
        can_move_multiple canMoveMultiple;
        trivial_moves trivialMoves;
        move_paths movePaths;
        valid_moves validMoves;
        valid_moves validCaptureMoves;
    };

    move_tables<mailbox_rep> generate_mailbox_move_tables();

    template <bool IntoCheckTest = false>
    inline bool in_check(move_tables<mailbox_rep> const& aTables, player aPlayer, mailbox_position const& aPosition);
        
    inline bool can_move_trivial(coordinates const& aFrom, coordinates const& aTo, bool& aKnight)
    {
        if (aFrom == aTo)
            return false;
        auto const delta = (aTo.as<int32_t>() - aFrom.as<int32_t>()).abs();
        if (delta.x != 0 && delta.y != 0 && delta.x != delta.y)
        {
            if ((delta.x == 1 && delta.y == 2) || (delta.x == 2 && delta.y == 1))
                aKnight = true;
            else
                return false;
        }
        return true;
    }

    inline bool can_move_trivial(move_tables<mailbox_rep> const& aTables, coordinates const& aFrom, coordinates const& aTo)
    {
        return aTables.trivialMoves[aFrom.y][aFrom.x][aTo.y][aTo.x];
    }

    template<bool CheckTest = false, bool IntoCheckTest = false, bool DefendTest = false>
    inline bool can_move(move_tables<mailbox_rep> const& aTables, player aTurn, mailbox_position const& aPosition, move const& aMove)
    {
        if (!can_move_trivial(aTables, aMove.from, aMove.to))
            return false;
        auto const movingPiece = piece_at(aPosition, aMove.from);
        auto const targetPiece = piece_at(aPosition, aMove.to);
        auto const movingPieceColor = piece_color(movingPiece);
        if (movingPieceColor != static_cast<piece>(aTurn))
            return false;
        auto const targetPieceColor = piece_color(targetPiece);
        if (!DefendTest && targetPieceColor == static_cast<piece>(aTurn))
            return false;
        auto const targetPieceType = piece_type(targetPiece);
        if (!CheckTest && targetPieceType == piece::King)
            return false;
        auto const movingPieceColorCardinal = as_color_cardinal<>(movingPiece);
        auto const movingPieceCardinal = as_cardinal<>(movingPiece);
        // non-capturing move...
        bool enPassant = false;
        bool castle = false;
        if (targetPieceType == piece::None)
        {
            if (!aTables.validMoves[movingPieceColorCardinal][movingPieceCardinal][aMove.from.y][aMove.from.x][aMove.to.y][aMove.to.x])
            {
                if (piece_type(movingPiece) == piece::Pawn &&
                    aTables.validCaptureMoves[movingPieceColorCardinal][movingPieceCardinal][aMove.from.y][aMove.from.x][aMove.to.y][aMove.to.x] &&
                    !aPosition.moveHistory.empty())
                {
                    auto const pieceLastMoved = piece_at(aPosition, aPosition.moveHistory.back().to);
                    if (piece_type(pieceLastMoved) == piece::Pawn && piece_color(pieceLastMoved) != static_cast<piece>(aTurn))
                    {
                        auto const delta = aPosition.moveHistory.back().to.as<int32_t>() - aPosition.moveHistory.back().from.as<int32_t>();
                        if (std::abs(delta.y) == 2)
                        {
                            auto const& deltaUnity = neogfx::delta_i32{ delta.x != 0 ? delta.x / std::abs(delta.x) : 0, delta.y != 0 ? delta.y / std::abs(delta.y) : 0 };
                            if (aPosition.moveHistory.back().to.as<int32_t>() - deltaUnity == aMove.to.as<int32_t>())
                                enPassant = true;
                        }
                    }
                }
                else if (piece_type(movingPiece) == piece::King && (aPosition.moveHistory.empty() || !aPosition.moveHistory.back().castlingState[movingPieceColorCardinal][static_cast<std::size_t>(move::castling_piece_index::King)]))
                {
                    if (aMove.to.y == aMove.from.y)
                    {
                        if ((aMove.to.x == 2 && (aPosition.moveHistory.empty() || !aPosition.moveHistory.back().castlingState[movingPieceColorCardinal][static_cast<std::size_t>(move::castling_piece_index::QueensRook)])) ||
                            (aMove.to.x == 6 && (aPosition.moveHistory.empty() || !aPosition.moveHistory.back().castlingState[movingPieceColorCardinal][static_cast<std::size_t>(move::castling_piece_index::KingsRook)])))
                            castle = !in_check(aTables, aTurn, aPosition);
                    }
                }
                if (!enPassant && !castle)
                    return false;
            }
            else if (movingPiece == piece::WhitePawn && aMove.to.y - aMove.from.y == 2u && piece_at(aPosition, coordinates{ aMove.from.x, aMove.from.y + 1u }) != piece::None)
                return false;
            else if (movingPiece == piece::BlackPawn && aMove.from.y - aMove.to.y == 2u && piece_at(aPosition, coordinates{ aMove.from.x, aMove.from.y - 1u }) != piece::None)
                return false;
        }
        // capturing move...
        if (targetPieceType != piece::None && !aTables.validCaptureMoves[movingPieceColorCardinal][movingPieceCardinal][aMove.from.y][aMove.from.x][aMove.to.y][aMove.to.x])
            return false;
        // any pieces in the way?
        if (aTables.canMoveMultiple[movingPieceCardinal] || castle)
        {
            auto const movePath = aTables.movePaths[aMove.from.y][aMove.from.x][aMove.to.y][!castle ? aMove.to.x : aMove.to.x == 2 ? 0 : 7];
            if (movePath.first > 1)
            {
                for (std::size_t i = 1; i < movePath.first - 1; ++i)
                {
                    auto const pos = movePath.second[i];
                    auto const inbetweenPiece = piece_at(aPosition, pos);
                    if (piece_type(inbetweenPiece) != piece::None)
                        return false;
                    if (castle)
                    {
                        aPosition.checkTest = move{ aMove.from, pos };
                        bool inCheck = in_check(aTables, aTurn, aPosition);
                        aPosition.checkTest = std::nullopt;
                        if (inCheck)
                            return false;
                    }
                }
            }
        }
        if (!IntoCheckTest)
        {
            aPosition.checkTest = aMove;
            bool inCheck = in_check<true>(aTables, aTurn, aPosition);
            aPosition.checkTest = std::nullopt;
            if (inCheck)
                return false;
        }
        if (draw(aPosition))
            return false;
        return true;
    }

    template <bool IntoCheckTest>
    inline bool in_check(move_tables<mailbox_rep> const& aTables, player aPlayer, mailbox_position const& aPosition)
    {
        auto const opponent = next_player(aPlayer);
        auto const kingPosition = king_position(aPosition, static_cast<piece>(aPlayer));
        for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
            for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
            {
                move const tryMove{ coordinates{ xFrom, yFrom }, kingPosition };
                if (tryMove.from == kingPosition)
                    continue;
                if (can_move<true, IntoCheckTest>(aTables, opponent, aPosition, tryMove))
                    return true;
            }
        return false;
    }

    template <player Player, typename ResultContainer>
    inline void sort_nodes(move_tables<mailbox_rep> const& aTables, mailbox_position const& aPosition, ResultContainer& aResult)
    {
        std::sort(as_valid_moves(aResult).begin(), as_valid_moves(aResult).end(), [&](auto const& lhs, auto const& rhs)
        {
            mailbox_position lhsBoard = aPosition;
            mailbox_position rhsBoard = aPosition;
            move_piece(lhsBoard, as_move(lhs));
            move_piece(rhsBoard, as_move(rhs));
            return eval<mailbox_rep, Player>{}(aTables, lhsBoard, 2.0).eval < eval<mailbox_rep, Player>{}(aTables, rhsBoard, 2.0).eval;
        });
    }

    template <player Player, typename ResultContainer>
    inline void valid_moves(move_tables<mailbox_rep> const& aTables, mailbox_position const& aPosition, ResultContainer& aResult)
    {
        as_valid_moves(aResult).clear();
        for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
            for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
                for (coordinate xTo = 0u; xTo <= 7u; ++xTo)
                    for (coordinate yTo = 0u; yTo <= 7u; ++yTo)
                    {
                        move candidateMove{ { xFrom, yFrom }, { xTo, yTo } };
                        if (can_move<>(aTables, Player, aPosition, candidateMove))
                        {
                            auto const movingPiece = piece_at(aPosition, candidateMove.from);
                            if (piece_type(movingPiece) == piece::Pawn)
                            {
                                auto const movingPieceColor = piece_color(movingPiece);
                                if ((movingPieceColor == piece::White && candidateMove.to.y == promotion_rank_v<player::White>) ||
                                    (movingPieceColor == piece::Black && candidateMove.to.y == promotion_rank_v<player::Black>))
                                {
                                    candidateMove.promoteTo = piece::Queen | movingPieceColor;
                                    as_valid_moves(aResult).emplace_back(candidateMove);
                                    candidateMove.promoteTo = piece::Rook | movingPieceColor;
                                    as_valid_moves(aResult).emplace_back(candidateMove);
                                    candidateMove.promoteTo = piece::Bishop | movingPieceColor;
                                    as_valid_moves(aResult).emplace_back(candidateMove);
                                    candidateMove.promoteTo = piece::Knight | movingPieceColor;
                                    as_valid_moves(aResult).emplace_back(candidateMove);
                                }
                                else
                                    as_valid_moves(aResult).emplace_back(candidateMove);
                            }
                            else
                                as_valid_moves(aResult).emplace_back(candidateMove);
                        }
                    }
    }
}