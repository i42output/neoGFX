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

#include <chess/ai.hpp>

namespace chess
{
    template <typename Representation, player Player>
    ai<Representation, Player>::ai() :
        async_thread{ "chess::ai" },
        iMoveTables{ generate_move_tables<representation_type>() },
        iBoard{ chess::setup_position<representation_type>() },
        iThreads{ std::max(1u, std::thread::hardware_concurrency()) }
    {
        start();
        Decided([&](move const& aBestMove)
        {
            play(aBestMove);
        });
    }

    template <typename Representation, player Player>
    ai<Representation, Player>::~ai()
    {
        {
            std::lock_guard<std::mutex> lk{ iSignalMutex };
            iFinished = true;
        }
        iSignal.notify_one();
    }
        
    template <typename Representation, player Player>
    player_type ai<Representation, Player>::type() const
    {
        return player_type::AI;
    }

    template <typename Representation, player Player>
    player ai<Representation, Player>::player() const
    {
        return Player;
    }

    template <typename Representation, player Player>
    void ai<Representation, Player>::greet(i_player& aOpponent)
    {
        iSink = aOpponent.moved([&](move const& aMove)
        {
            std::lock_guard<std::recursive_mutex> lk{ iBoardMutex };
            move_piece(iBoard, aMove);
        });
    }

    template <typename Representation, player Player>
    void ai<Representation, Player>::play()
    {
        {
            std::unique_lock<std::mutex> lk{ iSignalMutex };
            iPlaying = true;
        }
        iSignal.notify_one();
    }

    template <typename Representation, player Player>
    void ai<Representation, Player>::stop()
    {
        // todo
    }

    template <typename Representation, player Player>
    bool ai<Representation, Player>::play(move const& aMove)
    {
        std::lock_guard<std::recursive_mutex> lk{ iBoardMutex };
        move_piece(iBoard, aMove);
        Moved.trigger(aMove);
        return true;
    }

    template <typename Representation, player Player>
    void ai<Representation, Player>::undo()
    {
        iRootNode = std::nullopt;
        chess::undo(iBoard);
    }

    template <typename Representation, player Player>
    bool ai<Representation, Player>::do_work(neolib::yield_type aYieldType)
    {
        bool didWork = async_task::do_work(aYieldType);

        std::unique_lock<std::mutex> lk{ iSignalMutex };
        if (!iFinished)
        {
            iSignal.wait_for(lk, std::chrono::seconds{ 1 }, [&]() { return iPlaying || iFinished; });
            if (iPlaying)
            {
                auto bestMove = execute();
                iPlaying = false;
                if (bestMove)
                    Decided.trigger(*bestMove->move);
            }
        }

        return didWork;
    }

    template <typename Representation, player Player>
    game_tree_node const* ai<Representation, Player>::execute()
    {
        std::optional<std::lock_guard<std::recursive_mutex>> lk{ iBoardMutex };
        if (!iRootNode)
        {
            if (iBoard.moveHistory.empty())
                iRootNode.emplace(iBoard.moveHistory.back());
            else
                iRootNode.emplace();
            iRootNode->children.emplace();
            valid_moves<Player>(iMoveTables, iBoard, *iRootNode);
        }
        else
        {
            auto existing = std::find_if(iRootNode->children->begin(), iRootNode->children->end(), [&](game_tree_node const& n) { return n.move == iBoard.moveHistory.back(); });
            if (existing == iRootNode->children->end())
                throw node_not_found();
            game_tree_node temp = std::move(*existing);
            iRootNode = std::move(temp);
        }
        sort_nodes<Player>(iMoveTables, iBoard, *iRootNode);

        auto& children = *(*iRootNode).children;

        // todo: opening book and/or sensible white first move...
        if (children.size() > 0u)
        {
            ai_thread<representation_type, Player> thread;
            std::vector<std::future<game_tree_node>> futures;
            futures.reserve(children.size());
            auto iterThread = iThreads.begin();
            for (auto& child : children)
            {
                futures.emplace_back(iterThread->eval(iBoard, std::move(child)).get_future());
                if (++iterThread == iThreads.end())
                    iterThread = iThreads.begin();
            }
            lk = std::nullopt;
            for (auto& t : iThreads)
                t.start();
            std::vector<game_tree_node> bestMoves;
            for (auto& future : futures)
                bestMoves.push_back(std::move(future.get()));
            std::sort(bestMoves.begin(), bestMoves.end(),
                [](auto const& m1, auto const& m2)
                {
                    return m1.eval >= m2.eval;
                });
            auto const bestMoveEval = *bestMoves[0].eval;
            auto const decimator = 0.125 * (iBoard.moveHistory.size() + 1); // todo: involve difficulty level?
            auto similarEnd = std::remove_if(bestMoves.begin(), bestMoves.end(),
                [bestMoveEval, decimator](auto const& m)
                {
                    return static_cast<int64_t>(*m.eval * decimator) != static_cast<int64_t>(bestMoveEval * decimator);
                });
            thread_local std::random_device tEntropy;
            thread_local std::mt19937 tGenerator{ tEntropy() };
            std::uniform_int_distribution<std::ptrdiff_t> options{ 0, std::distance(bestMoves.begin(), similarEnd) };
            iRootNode = std::move(bestMoves[options(tGenerator)]);
            return &*iRootNode;
        }
        return nullptr;
    }

    template <typename Representation, player Player>
    bool ai<Representation, Player>::playing() const
    {
        return iPlaying;
    }

    template <typename Representation, player Player>
    void ai<Representation, Player>::setup(matrix_board const& aSetup)
    {
        if constexpr (std::is_same_v<representation_type, matrix>)
        {
            std::lock_guard<std::recursive_mutex> lk{ iBoardMutex };
            iRootNode = std::nullopt;
            iBoard = aSetup;
        }
        else
            ; // todo (convert to bitboard representation)
    }

    template class ai<matrix, player::White>;
    template class ai<matrix, player::Black>;
    template class ai<bitboard, player::White>;
    template class ai<bitboard, player::Black>;
}