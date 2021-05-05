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

#include <chess/ai_thread.hpp>
#include <chess/mailbox.hpp>
#include <chess/bitboard.hpp>

namespace chess
{
    template <typename Representation>
    inline basic_position<Representation>& eval_board()
    {
        thread_local basic_position<Representation> sEvalBoard = {};
        return sEvalBoard;
    }

    // use stack to limit RAM usage... (todo: make configurable?)
    constexpr int32_t USE_STACK_DEPTH = 4;
    constexpr std::size_t STACK_NODE_STACK_CAPACITY = 32; // todo: what should this hard limit be?
    struct stack_node_stack_limit_exceeded : std::logic_error { stack_node_stack_limit_exceeded() : std::logic_error{ "chess::stack_node_stack_limit_exceeded" } {} };

    template <player Player, player Turn, typename Representation>
    double negascout(move_tables<Representation> const& tables, basic_position<Representation>& position, game_tree_node& node, int32_t ply, int32_t depth, double alpha, double beta)
    {
        typedef game_tree_node stack_node_t;
        typedef std::vector<stack_node_t> stack_node_stack_t;
        thread_local stack_node_stack_t stackNodeStack;
        auto const stackUsageDepth = ply - depth;
        bool const useStack = stackUsageDepth >= USE_STACK_DEPTH;
        auto const stackStackIndex = stackUsageDepth - USE_STACK_DEPTH;
        if (useStack && stackNodeStack.size() <= stackStackIndex)
        {
            stackNodeStack.reserve(STACK_NODE_STACK_CAPACITY);
            if (stackStackIndex >= stackNodeStack.capacity())
                throw stack_node_stack_limit_exceeded();
            stackNodeStack.resize(stackStackIndex + 1);
        }
        auto& use = (useStack ? stackNodeStack[stackStackIndex] : node);
        if (use.children == std::nullopt)
        {
            use.children.emplace();
            valid_moves<Turn>(tables, position, use);
        }
        else if (useStack)
            valid_moves<Turn>(tables, position, use);
        auto& validMoves = *use.children;
        if (depth == 0 || validMoves.empty())
            return eval<Representation, Turn>{}(tables, position, static_cast<double>(ply - depth)).eval;
        auto a = alpha;
        auto b = beta;
        for (auto& child : validMoves)
        {
            auto const& move = *child.move;
            move_piece(position, move);
            double t = -negascout<Player, opponent_v<Turn>>(tables, position, child, ply, depth - 1, -b, -a);
            if (t > a && t < beta && &child != &validMoves[0] && depth > 1)
                a = -negascout<Player, opponent_v<Turn>>(tables, position, child, ply, depth - 1, -beta, -t);
            undo(position);
            a = std::max(a, t);
            if (a >= beta)
                return a;
            auto const EPSILON = 0.000001;
            b = a + EPSILON;
        }
        return a;
    }

    template <player Player, typename Representation>
    void search(move_tables<Representation> const& tables, basic_position<Representation>& position, game_tree_node& node, int32_t ply)
    {
        ply = 2;

        double constexpr alpha = -std::numeric_limits<double>::max();
        double constexpr beta = std::numeric_limits<double>::max();
        // iterative deepening
        for (int32_t plyIteration = 1; plyIteration <= ply; ++plyIteration)
        {
            auto& candidateMoves = *node.children;
            for (auto& candidateMove : candidateMoves)
            {
                move_piece(position, *candidateMove.move);
                candidateMove.eval = -negascout<Player, opponent_v<Player>, Representation>(tables, position, candidateMove, plyIteration, plyIteration, alpha, beta);
                undo(position);
                if (candidateMove.children != std::nullopt)
                    std::stable_sort(candidateMove.children->begin(), candidateMove.children->end(),
                        [](auto const& m1, auto const& m2)
                        {
                            return m1.eval > m2.eval;
                        });
            }
        }
    }
            
        
    template <typename Representation, player Player>
    ai_thread<Representation, Player>::ai_thread(int32_t aPly) :
        iPly{ aPly },
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
    std::promise<game_tree_node>& ai_thread<Representation, Player>::eval(position_type const& aPosition, game_tree_node&& aNode)
    {
        {
            std::lock_guard<std::mutex> lk{ iMutex };
            iQueue.emplace_back(aPosition, *aNode.move, std::move(aNode));
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

            std::map<position_type, game_tree_node> work;
            
            for (auto& workItem : iQueue)
            {
                if (work[workItem.position].children == std::nullopt)
                    work[workItem.position].children.emplace();
                work[workItem.position].children->push_back(std::move(workItem.node));
            }
            
            auto& evalPosition = eval_board<Representation>();
            
            for (auto& workGroup : work)
            {
                evalPosition = workGroup.first;
                auto& node = workGroup.second;
                search<Player>(iMoveTables, evalPosition, node, iPly);

                for (auto& workItem : iQueue)
                {
                    if (workGroup.first == workItem.position)
                    {
                        for (auto& childNode : *node.children)
                            if (workItem.move == childNode.move)
                                workItem.result.set_value(std::move(childNode));
                    }
                }
            }
            
            iQueue.clear();
        }
    }

    template class ai_thread<mailbox_rep, player::White>;
    template class ai_thread<mailbox_rep, player::Black>;
    template class ai_thread<bitboard_rep, player::White>;
    template class ai_thread<bitboard_rep, player::Black>;
}