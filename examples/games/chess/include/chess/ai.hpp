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

#include <neogfx/core/async_thread.hpp>
#include <chess/i_player.hpp>
#include <chess/matrix.hpp>
#include <chess/bitboard.hpp>
#include <chess/ai_thread.hpp>

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
        typedef Representation representation_type;
    public:
        ai();
        ~ai();
    public:
        player_type type() const override;
        chess::player player() const override;
    public:
        void greet(i_player& aOpponent) override;
        void play() override;
        void stop() override;
        bool play(move const& aMove) override;
        bool playing() const override;
        void undo() override;
        void setup(matrix_board const& aSetup) override;
    private:
        bool do_work(neolib::yield_type aYieldType = neolib::yield_type::NoYield) override;
    private:
        std::optional<best_move> execute();
    private:
        move_tables<representation_type> const iMoveTables;
        std::recursive_mutex iBoardMutex;
        basic_board<representation_type> iBoard;
        std::vector<ai_thread<Representation, Player>> iThreads;
        std::mutex iSignalMutex;
        std::condition_variable iSignal;
        std::atomic<bool> iPlaying = false;
        std::atomic<bool> iFinished = false;
        ng::sink iSink;
    };

    extern template class ai<matrix, player::White>;
    extern template class ai<matrix, player::Black>;
    extern template class ai<bitboard, player::White>;
    extern template class ai<bitboard, player::Black>;
}