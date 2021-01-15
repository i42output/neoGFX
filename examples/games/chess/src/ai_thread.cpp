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

#include <neolib/core/vecarray.hpp>
#include <chess/ai_thread.hpp>
#include <chess/matrix.hpp>
#include <chess/bitboard.hpp>

namespace chess
{
    template <player Player, typename Representation>
    double pvs(move_tables<Representation> const& tables, basic_board<Representation> const& node, int32_t depth, double alpha, double beta)
    {
        neolib::vecarray<move, 64> validMoves;
        valid_moves<Player>(tables, node, validMoves);
        if (depth == 0 || validMoves.empty())
            return eval<Representation, Player>{}(tables, node, static_cast<double>(depth)).eval;
        for (auto const& m : validMoves)
        {
            basic_board<Representation> child = node; // todo: making a copy of the entire board certainly isn't optimal.
            move_piece(child, m);
            double score = 0.0;
            if (&m == &validMoves[0])
                score = -pvs<opponent_v<Player>>(tables, child, depth - 1, -beta, -alpha);
            else
            {
                score = -pvs<opponent_v<Player>>(tables, child, depth - 1, -alpha - 1.0, -alpha);
                if (alpha < score && score < beta)
                    score = -pvs<opponent_v<Player>>(tables, child, depth - 1, -beta, -score);
            }
            alpha = std::max(alpha, score);
            if (alpha >= beta)
                break;
        }
        return alpha;
    }
    
    template <typename Representation, player Player>
    ai_thread<Representation, Player>::ai_thread() :
        iMoveTables{ generate_move_tables<representation_type>() }
    {
    }

    template <typename Representation, player Player>
    best_move ai_thread<Representation, Player>::eval(board_type const& aBoard, std::vector<move> const& aMoves)
    {
        std::optional<best_move> bestMove;
        for (auto const& m : aMoves)
        {
            board_type b = aBoard;
            move_piece(b, m);
            auto value = -pvs<opponent_v<Player>>(iMoveTables, b, 3, -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
            if (!bestMove || bestMove->value < value)
                bestMove = best_move(m, value);
        }
        return *bestMove;
    }

    template class ai_thread<matrix, player::White>;
    template class ai_thread<matrix, player::Black>;
    template class ai_thread<bitboard, player::White>;
    template class ai_thread<bitboard, player::Black>;
}