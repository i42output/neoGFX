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
    template <typename Representation>
    inline basic_board<Representation>& eval_board()
    {
        thread_local basic_board<Representation> sEvalBoard = {};
        return sEvalBoard;
    }

    template <player Player, typename Representation>
    double pvs(move_tables<Representation> const& tables, basic_board<Representation>& board, game_tree_node& node, int32_t depth, double alpha, double beta)
    {
        if (node.children == std::nullopt)
        {
            node.children.emplace();
            valid_moves<Player>(tables, board, *node.children);
        }
        auto& validMoves = *node.children;
        if (depth == 0 || validMoves.empty())
            return eval<Representation, Player>{}(tables, board, static_cast<double>(depth)).eval;
        for (auto& child : validMoves)
        {
            move_piece(board, child.move);
            double score = 0.0;
            if (&child == &validMoves[0])
                score = -pvs<opponent_v<Player>>(tables, board, child, depth - 1, -beta, -alpha);
            else
            {
                score = -pvs<opponent_v<Player>>(tables, board, child, depth - 1, -alpha - 1.0, -alpha);
                if (alpha < score && score < beta)
                    score = -pvs<opponent_v<Player>>(tables, board, child, depth - 1, -beta, -score);
            }
            undo(board);
            alpha = std::max(alpha, score);
            if (alpha >= beta)
                break;
        }
        return alpha;
    }

    template <typename Representation, player Player>
    ai_thread<Representation, Player>::ai_thread() :
        iMoveTables{ generate_move_tables<representation_type>() },
        iThread{ [&]() { process(); } }
    {
    }

    template <typename Representation, player Player>
    ai_thread<Representation, Player>::~ai_thread()
    {
        {
            std::lock_guard<std::mutex> lk{ iMutex };
            iFinished = true;
        }
        iSignal.notify_one();
        iThread.join();
    }
        
    template <typename Representation, player Player>
    std::promise<game_tree_node>& ai_thread<Representation, Player>::eval(board_type const& aBoard, game_tree_node&& aNode)
    {
        {
            std::lock_guard<std::mutex> lk{ iMutex };
            iQueue.emplace_back(aBoard, std::move(aNode));
        }
        return iQueue.back().result;
    }

    template <typename Representation, player Player>
    void ai_thread<Representation, Player>::start()
    {
        {
            std::unique_lock<std::mutex> lk{ iMutex };
            if (iQueue.empty())
                return;
        }
        iSignal.notify_one();
    }

    template <typename Representation, player Player>
    void ai_thread<Representation, Player>::process()
    {
        for (;;)
        {
            std::unique_lock<std::mutex> lk{ iMutex };
            iSignal.wait(lk, [&]() { return iFinished || !iQueue.empty(); });
            if (iFinished)
                return;
            for (auto& workItem : iQueue)
            {
                auto& evalBoard = eval_board<Representation>();
                evalBoard = workItem.board;
                auto& node = workItem.node;
                move_piece(evalBoard, node.move );
                node.eval = -pvs<opponent_v<Player>>(iMoveTables, evalBoard, node, 3, -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity());
                workItem.result.set_value(std::move(node));
            }
            iQueue.clear();
        }
    }

    template class ai_thread<matrix, player::White>;
    template class ai_thread<matrix, player::Black>;
    template class ai_thread<bitboard, player::White>;
    template class ai_thread<bitboard, player::Black>;
}