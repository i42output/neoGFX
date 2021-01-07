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

#include <vector>

#include <chess/move_validator.hpp>

namespace chess
{
    move_table generate_move_table()
    {
        std::array<std::vector<neogfx::point_i32>, static_cast<std::size_t>(piece_cardinal::COUNT)> const unitMoves =
        { {
            { { 0, 1 } }, // pawn
            { { 2, 1 }, { 2, -1 }, { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }, { -2, 1 }, { -2, -1 } }, // knight
            { { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } }, // bishop
            { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } }, // rook
            { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } }, // queen
            { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 } } // king
        } };
        std::array<bool, static_cast<std::size_t>(piece_cardinal::COUNT)> const canMoveMultiple =
        {
            false, // pawn
            false, // knight
            true, // bishop
            true, // rook
            true, // queen
            false // king
        };
        return {}; // todo
    }

    bool move_validator::can_move(player aTurn, position const& aPosition, move const& aMove) const
    {
        return true;
    }
}