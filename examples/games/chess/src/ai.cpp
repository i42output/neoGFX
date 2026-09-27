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

#include <algorithm>
#include <thread>

#include <stockparrot/chess.hpp>

#include <chess/ai.hpp>

namespace chess
{
    namespace
    {
        // Castling rights are inferred from king/rook home squares as the position
        // has no move history at this point (matching move_validator behaviour).
        std::string to_fen(mailbox_position const& aPosition)
        {
            auto const at = [&](coordinate x, coordinate y) { return piece_at(aPosition.rep, coordinates{ x, y }); };
            std::string result;
            for (coordinate y = 8u; y-- > 0u;)
            {
                std::uint32_t empty = 0u;
                for (coordinate x = 0u; x <= 7u; ++x)
                {
                    auto const p = at(x, y);
                    if (p == piece::None)
                    {
                        ++empty;
                        continue;
                    }
                    if (empty != 0u)
                    {
                        result += static_cast<char>('0' + empty);
                        empty = 0u;
                    }
                    result += to_string(p);
                }
                if (empty != 0u)
                    result += static_cast<char>('0' + empty);
                if (y > 0u)
                    result += '/';
            }
            result += aPosition.turn == player::White ? " w " : " b ";
            std::string castling;
            if (at(4u, 0u) == piece::WhiteKing && at(7u, 0u) == piece::WhiteRook)
                castling += 'K';
            if (at(4u, 0u) == piece::WhiteKing && at(0u, 0u) == piece::WhiteRook)
                castling += 'Q';
            if (at(4u, 7u) == piece::BlackKing && at(7u, 7u) == piece::BlackRook)
                castling += 'k';
            if (at(4u, 7u) == piece::BlackKing && at(0u, 7u) == piece::BlackRook)
                castling += 'q';
            result += castling.empty() ? "-" : castling;
            result += " - 0 1";
            return result;
        }

        std::string setup_fen(mailbox_position aPosition)
        {
            while (unmake(aPosition));
            return to_fen(aPosition);
        }
    }

    struct ai::engine_client : uci::i_uci_client
    {
        ai& owner;
        std::optional<std::string> bestMove;

        engine_client(ai& aOwner) :
            owner{ aOwner }
        {
        }

        void response(uci::i_uci&, std::string const&) override
        {
        }
        void info(uci::i_uci&, std::int64_t, std::chrono::milliseconds, std::int64_t, std::int64_t aNodesPerSecond, std::int64_t, std::string const&) override
        {
            owner.iNodesPerSecond = static_cast<std::uint64_t>(aNodesPerSecond);
        }
        void bestmove(uci::i_uci&, std::string const& aBestMove) override
        {
            bestMove = aBestMove;
        }
    };

    ai::ai(chess::player aPlayer, std::chrono::milliseconds aMoveTime, std::optional<std::int32_t> aMaxDepth) :
        async_thread{ "chess::ai" },
        iPlayer{ aPlayer },
        iMoveTime{ aMoveTime },
        iMaxDepth{ aMaxDepth },
        iPosition{ chess::setup_position<mailbox_rep>() },
        iSetupFen{ setup_fen(iPosition) },
        iEngineClient{ std::make_unique<engine_client>(*this) },
        iEngine{ std::make_unique<stockparrot::Engine>() }
    {
        iEngine->connect(*iEngineClient);
        iEngine->setoption("Threads", std::to_string(std::max(1u, std::thread::hardware_concurrency())));
        Decided([&](move const& aBestMove)
        {
            play(aBestMove);
        });
        start();
    }

    ai::~ai()
    {
        {
            std::lock_guard<std::mutex> lk{ iSignalMutex };
            iFinished = true;
        }
        iSignal.notify_one();
        async_task::cancel(); // waits for any in-progress search to complete
    }
        
    player_type ai::type() const
    {
        return player_type::AI;
    }

    player ai::player() const
    {
        return iPlayer;
    }

    void ai::greet(i_player& aOpponent)
    {
        iSink = aOpponent.moved([&](move const& aMove)
        {
            std::unique_lock lk{ iMutex };
            make(iPosition, aMove);
        });
    }

    void ai::play()
    {
        {
            std::unique_lock<std::mutex> lk{ iSignalMutex };
            iPlaying = true;
        }
        iSignal.notify_one();
    }

    void ai::stop()
    {
        // stockparrot searches are bounded by iMoveTime and cannot be interrupted;
        // clearing iPlaying causes the result of an in-progress search to be discarded.
        iPlaying = false;
    }

    void ai::finish()
    {
        {
            std::lock_guard<std::mutex> lk{ iSignalMutex };
            iFinished = true;
        }
        iSignal.notify_one();
    }

    bool ai::play(move const& aMove)
    {
        std::unique_lock lk{ iMutex };
        make(iPosition, aMove);
        Moved(aMove);
        return true;
    }

    void ai::undo()
    {
        std::unique_lock lk{ iMutex };
        unmake(iPosition);
    }

    bool ai::do_work(neolib::yield_type aYieldType)
    {
        bool didWork = async_task::do_work(aYieldType);

        std::unique_lock<std::mutex> lk{ iSignalMutex };
        if (!iFinished)
        {
            iSignal.wait_for(lk, std::chrono::seconds{ 1 }, [&]() { return iPlaying || iFinished; });
            lk.unlock();
            if (iPlaying && !iFinished)
            {
                auto const bestMove = execute();
                bool const stopped = !iPlaying.exchange(false);
                if (bestMove && !stopped && !iFinished)
                    Decided(*bestMove);
            }
        }

        return didWork;
    }

    std::optional<move> ai::execute()
    {
        std::unique_lock lk{ iMutex };
        auto const snapshot = iPosition;
        auto const setupFen = iSetupFen;
        lk.unlock();

        std::string moves;
        for (auto const& m : snapshot.moveHistory)
            moves += (moves.empty() ? "" : " ") + to_string(m);

        iEngineClient->bestMove = std::nullopt;
        iNodesPerSecond = 0;
        iEngine->position(uci::fen{ setupFen }, moves);
        uci::go_params goParams{ uci::movetime{ static_cast<std::int32_t>(iMoveTime.count()) } };
        if (iMaxDepth)
            goParams.push_back(uci::depth{ *iMaxDepth });
        iEngine->go(goParams);

        lk.lock();
        // discard the result if the position changed (undo/setup) while searching or there is no legal move
        if (iPosition != snapshot || !iEngineClient->bestMove || *iEngineClient->bestMove == "0000")
            return {};
        auto result = parse_uci_move(*iEngineClient->bestMove);
        if (result.promoteTo)
            result.promoteTo = static_cast<piece>(iPlayer) | *result.promoteTo;
        return result;
    }

    bool ai::playing() const
    {
        return iPlaying;
    }

    void ai::setup(mailbox_position const& aSetup)
    {
        std::unique_lock lk{ iMutex };
        iPosition = aSetup;
        iSetupFen = setup_fen(aSetup);
    }

    std::uint64_t ai::nodes_per_second() const
    {
        return iNodesPerSecond;
    }
}
