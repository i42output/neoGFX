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

#include <vector>
#include <random>
#include <chrono>

#include <neogfx/core/async_thread.hpp>
#include <neogfx/core/i_event.hpp>
#include <chess/i_player.hpp>
#include <chess/mailbox.hpp>
#include <chess/bitboard.hpp>
#include <chess/ai_thread.hpp>
#include <chess/node.hpp>
#include <chess/table.hpp>

namespace chess
{
    template <typename Representation, player Player>
    class ai : public i_player, public neogfx::async_thread
    {
    public:
        define_declared_event(Moved, moved, move)
    private:
        define_event(Decided, decided, move)
    public:
        struct node_not_found : std::logic_error { node_not_found() : std::logic_error{ "chess::ai::node_not_found" } {} };
    public:
        typedef Representation representation_type;
    public:
        ai(int32_t aPly = 4);
        ~ai();
    public:
        player_type type() const override;
        chess::player player() const override;
    public:
        void greet(i_player& aOpponent) override;
        void play() override;
        void stop() override;
        void finish() override;
        bool play(move const& aMove) override;
        bool playing() const override;
        void undo() override;
        void setup(mailbox_position const& aSetup) override;
    public:
        uint64_t nodes_per_second() const override;
    private:
        bool do_work(neolib::yield_type aYieldType = neolib::yield_type::NoYield) override;
    private:
        game_tree_node const* execute();
    private:
        int32_t iPly;
        move_tables<representation_type> const iMoveTables;
        mutable std::recursive_mutex iMutex;
        basic_position<representation_type> iPosition;
        table iTable;
        std::list<ai_thread<Representation, Player>> iThreads;
        std::mutex iSignalMutex;
        std::condition_variable iSignal;
        std::atomic<bool> iPlaying = false;
        std::atomic<bool> iFinished = false;
        std::optional<game_tree_node> iRootNode;
        ng::sink iSink;
        std::optional<std::chrono::steady_clock::time_point> iStartTime;
        std::optional<uint64_t> iNodesPerSecond;
        bool iUseDecimator = false;
    };

    extern template class ai<mailbox_rep, player::White>;
    extern template class ai<mailbox_rep, player::Black>;
    extern template class ai<bitboard_rep, player::White>;
    extern template class ai<bitboard_rep, player::Black>;
}