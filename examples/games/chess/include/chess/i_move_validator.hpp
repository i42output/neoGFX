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

#include <chess/primitives.hpp>

namespace chess
{
    struct move_tables
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

    inline bool in_check(move_tables const& aTables, player aPlayer, board const& aBoard);
        
    inline bool can_move(move_tables const& aTables, player aTurn, board const& aBoard, move const& aMove, bool aCheckTest = false)
    {
        auto const movingPiece = piece_at(aBoard, aMove.from);
        auto const targetPiece = piece_at(aBoard, aMove.to);
        if (piece_color(movingPiece) != static_cast<piece>(aTurn) ||
            piece_color(targetPiece) == static_cast<piece>(aTurn))
            return false;
        if (piece_type(targetPiece) == piece::King && !aCheckTest)
            return false;
        // normal move...
        if (piece_type(targetPiece) == piece::None && !aTables.validMoves[static_cast<std::size_t>(as_color_cardinal(movingPiece))][static_cast<std::size_t>(as_cardinal(movingPiece))][aMove.from.y][aMove.from.x][aMove.to.y][aMove.to.x])
            return false;
        // capturing move...
        if (piece_type(targetPiece) != piece::None && !aTables.validCaptureMoves[static_cast<std::size_t>(as_color_cardinal(movingPiece))][static_cast<std::size_t>(as_cardinal(movingPiece))][aMove.from.y][aMove.from.x][aMove.to.y][aMove.to.x])
            return false;
        // any pieces in the way?
        if (aTables.canMoveMultiple[static_cast<std::size_t>(as_cardinal(movingPiece))])
        {
            auto const delta = move_tables::move_coordinates{ static_cast<int32_t>(aMove.to.x), static_cast<int32_t>(aMove.to.y) } - move_tables::move_coordinates{ static_cast<int32_t>(aMove.from.x), static_cast<int32_t>(aMove.from.y) };
            auto const& deltaUnity = neogfx::delta_i32{ delta.dx != 0 ? delta.dx / std::abs(delta.dx) : 0, delta.dy != 0 ? delta.dy / std::abs(delta.dy) : 0 };
            auto const start = aMove.from.as<int32_t>() + deltaUnity;
            auto const end = aMove.to.as<int32_t>();
            for (move_tables::move_coordinates pos = start; pos != end; pos += deltaUnity)
                if (piece_type(piece_at(aBoard, pos)) != piece::None)
                    return false;
        }
        if (!aCheckTest)
        {
            aBoard.checkTest = aMove;
            bool inCheck = in_check(aTables, aTurn, aBoard);
            aBoard.checkTest = std::nullopt;
            if (inCheck)
                return false;
        }
        // todo: corner cases (check, en passant, castling)
        return true;
    }

    inline bool in_check(move_tables const& aTables, player aPlayer, board const& aBoard)
    {
        auto const opponent = next_player(aPlayer);
        auto const kingPosition = king_position(aBoard, static_cast<piece>(aPlayer));
        for (coordinates::coordinate_type yFrom = 0u; yFrom <= 7u; ++yFrom)
            for (coordinates::coordinate_type xFrom = 0u; xFrom <= 7u; ++xFrom)
                if (can_move(aTables, opponent, aBoard, move{ coordinates{ xFrom, yFrom }, kingPosition }, true))
                    return true;
        return false;
    }
    
    class i_move_validator
    {
    public:
        virtual ~i_move_validator() = default;
    public:
        virtual bool can_move(player aTurn, board const& aBoard, move const& aMove) const = 0;
    };
}