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

#include <thread>

#include <chess/primitives.hpp>

namespace chess
{
    struct best_move
    {
        move move;
        double value = 0.0;
    };

    template <typename Representation, player Player>
    class ai_thread
    {
    public:
        typedef Representation representation_type;
        typedef basic_board<representation_type> board_type;
    public:
        ai_thread() :
            iMoveTables{ generate_move_tables<representation_type>() }
        {
        }
    public:
        best_move eval(board_type const& aBoard, std::vector<move> const& aMoves)
        {
            std::optional<best_move> bestMove;
            for (auto const& m : aMoves)
            {
                board_type b = aBoard;
                move_piece(b, m);
                chess::eval<representation_type, Player> eval;
                auto value = eval(iMoveTables, Player, b, 1.0);
                if (!bestMove || bestMove->value < value)
                    bestMove = best_move(m, value);
            }
            return *bestMove;
        }
    private:
        move_tables<representation_type> const iMoveTables;
    };
}