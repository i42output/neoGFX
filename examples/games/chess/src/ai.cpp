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

#include <atomic>
#include <chess/ai.hpp>

namespace chess
{
    extern std::atomic<std::uint64_t> sNodeCounter;

    namespace
    {
        template <typename T>
        void debug_moves(T const& moves, int ply, bool extra = false, bool extraExtra = false)
        {
            std::vector<std::pair<const game_tree_node*, const game_tree_node*>> debug;
            for (auto const& m : moves)
                debug.push_back(std::make_pair(&m, &m));

            std::sort(debug.begin(), debug.end(),
                [](auto const& d1, auto const& d2)
            {
                return std::forward_as_tuple(d1.second->move->from, d1.second->move->to) <
                    std::forward_as_tuple(d2.second->move->from, d2.second->move->to);
            });

            auto di = debug.begin();
            for (auto const& m : moves)
                (di++)->first = &m;

            for (auto const& d : debug)
            {
                std::cout << "(ply " << ply << "): " << to_string(*d.first->move) << ":  " << std::setw(12) << *d.first->eval;
                std::cout << "  " << to_string(*d.second->move) << ": " << std::setw(12) << *d.second->eval;
                std::cout << std::endl;
                if (extra)
                {
                    for (auto const& child : *d.first->children)
                    {
                        std::cout << "\t" << to_string(*child.move) << ":  " << std::setw(12) << *child.eval << std::endl;
                        if (extraExtra)
                        {
                            for (auto const& childChild : *child.children)
                            {
                                std::cout << "\t\t" << to_string(*childChild.move) << ":  " << std::setw(12) << *childChild.eval << std::endl;
                            }
                        }
                    }
                }
            }
        }
    }

    template <typename Representation, player Player>
    ai<Representation, Player>::ai(std::int32_t aPly) :
        async_thread{ "chess::ai" },
        iPly{ aPly },
        iMoveTables{ generate_move_tables<representation_type>() },
        iPosition{ chess::setup_position<representation_type>() },
        iTable{ DEFAULT_TABLE_SIZE, table_entry{} }
    {
        for (unsigned int t = 1u; t <= std::thread::hardware_concurrency(); ++t)
            iThreads.emplace_back(*this, iPly);
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
        iThreads.clear();
        async_task::cancel();
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
            std::unique_lock lk{ iMutex };
            make(iPosition, aMove);
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
        for (auto& thread : iThreads)
            thread.stop();
    }

    template <typename Representation, player Player>
    void ai<Representation, Player>::finish()
    {
        for (auto& thread : iThreads)
            thread.finish();
    }

    template <typename Representation, player Player>
    bool ai<Representation, Player>::play(move const& aMove)
    {
        std::unique_lock lk{ iMutex };
        make(iPosition, aMove);
        Moved(aMove);
        return true;
    }

    template <typename Representation, player Player>
    void ai<Representation, Player>::undo()
    {
        iRootNode = std::nullopt;
        unmake(iPosition);
    }

    template <typename Representation, player Player>
    bool ai<Representation, Player>::do_work(neolib::yield_type aYieldType)
    {
        bool didWork = async_task::do_work(aYieldType);

        std::unique_lock<std::mutex> lk{ iSignalMutex };
        if (!iFinished)
        {
            iSignal.wait_for(lk, std::chrono::seconds{ 1 }, [&]() { return iPlaying || iFinished; });
            lk.unlock();
            if (iPlaying && !iFinished)
            {
                auto bestMove = execute();
                iPlaying = false;
                if (bestMove)
                    Decided(*bestMove->move);
            }
        }

        return didWork;
    }

    template <typename Representation, player Player>
    game_tree_node const* ai<Representation, Player>::execute()
    {
        std::unique_lock lk{ iMutex };
        if (!iRootNode)
        {
            if (!iPosition.moveHistory.empty())
                iRootNode.emplace(iPosition.moveHistory.back());
            else
                iRootNode.emplace();
            iRootNode->children.emplace();
            valid_moves<Player>(iMoveTables, iPosition, *iRootNode);
        }
        else
        {
            if (iRootNode->children == std::nullopt)
            {
                iRootNode->children.emplace();
                valid_moves<Player>(iMoveTables, iPosition, *iRootNode);
            }
            auto existing = std::find_if(iRootNode->children->begin(), iRootNode->children->end(), [&](game_tree_node const& n) { return n.move == iPosition.moveHistory.back(); });
            if (existing == iRootNode->children->end())
                throw node_not_found();
            game_tree_node temp = std::move(*existing);
            iRootNode = std::move(temp);
        }
        sort_nodes<Player>(iMoveTables, iPosition, *iRootNode);

        auto& children = *(*iRootNode).children;

        // todo: opening book and/or sensible white first move...
        if (children.size() > 0u)
        {
            std::vector<game_tree_node> bestMoves;
            std::vector<std::future<game_tree_node>> futures;
            futures.reserve(children.size());
            auto iterThread = iThreads.begin();
            for (auto& child : children)
            {
                futures.emplace_back(iterThread->eval(iPosition, std::move(child)).get_future());
                if (++iterThread == iThreads.end())
                    iterThread = iThreads.begin();
            }
            
            sNodeCounter = 0;
            iNodesPerSecond = std::nullopt;
            iStartTime = std::chrono::steady_clock::now();

            lk.unlock();

            for (auto& t : iThreads)
                t.start();
            
            for (auto& future : futures)
                bestMoves.push_back(std::move(future.get()));

            lk.lock();
            iNodesPerSecond = nodes_per_second();
            iStartTime = std::nullopt;
            lk.unlock();

            std::sort(bestMoves.begin(), bestMoves.end(),
                [](auto const& m1, auto const& m2)
                {
                    return m1.eval > m2.eval;
                });

            debug_moves(bestMoves, iPly);

            auto const bestMoveEval = *bestMoves[0].eval;
            constexpr double MATE_CUTOFF = 1.0e10;
            bool const bestMoveIsMate = (bestMoveEval > MATE_CUTOFF);
            if (bestMoveIsMate || !iUseDecimator)
            {
                iRootNode = std::move(bestMoves[0]);
                return &*iRootNode;
            }
            else
            {
                auto const decimator = 0.125 * (iPosition.moveHistory.size() + 1); // todo: involve difficulty level?
                auto similarEnd = std::remove_if(bestMoves.begin(), bestMoves.end(),
                    [bestMoveEval, decimator](auto const& m)
                    {
                        return static_cast<std::int64_t>(*m.eval * decimator) != static_cast<std::int64_t>(bestMoveEval * decimator);
                    });
                thread_local std::random_device tEntropy;
                thread_local std::mt19937 tGenerator{ tEntropy() };
                std::uniform_int_distribution<std::ptrdiff_t> options{ 0, std::distance(bestMoves.begin(), similarEnd) - 1 };
                iRootNode = std::move(bestMoves[options(tGenerator)]);
            }
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
    void ai<Representation, Player>::setup(mailbox_position const& aSetup)
    {
        if constexpr (std::is_same_v<representation_type, mailbox_rep>)
        {
            std::unique_lock lk{ iMutex };
            iRootNode = std::nullopt;
            iPosition = aSetup;
        }
        else
        {
            std::unique_lock lk{ iMutex };
            iRootNode = std::nullopt;
            iPosition = bitboard_position{ {}, aSetup.turn, aSetup.moveHistory };
            for (coordinate x = 0u; x <= 7u; ++x)
                for (coordinate y = 0u; y <= 7u; ++y)
                    set_piece(iPosition.rep, coordinates{ x, y }, aSetup.rep[y][x]);
        }
        iUseDecimator = (iPosition == chess::setup_position<representation_type>());
    }

    template <typename Representation, player Player>
    std::uint64_t ai<Representation, Player>::nodes_per_second() const
    {
        std::unique_lock lk{ iMutex };
        if (iNodesPerSecond)
            return *iNodesPerSecond;
        else if (iStartTime)
            return static_cast<std::uint64_t>(sNodeCounter / std::chrono::duration<double>(std::chrono::steady_clock::now() - *iStartTime).count());
        else
            return 0;
    }

    template class ai<mailbox_rep, player::White>;
    template class ai<mailbox_rep, player::Black>;
    template class ai<bitboard_rep, player::White>;
    template class ai<bitboard_rep, player::Black>;
}