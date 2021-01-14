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

#include <random>
#include <chess/i_player.hpp>
#include <chess/matrix.hpp>
#include <chess/bitboard.hpp>

namespace chess
{
    template <typename Representation, chess::player Player>
    class ai : public i_player
    {
    public:
        define_declared_event(Moved, moved, move const&)
    public:
        typedef Representation representation_type;
    public:
        ai();
    public:
        player_type type() const override;
        chess::player player() const override;
    public:
        void greet(i_player& aOpponent) override;
        bool play(move const& aMove) override;
        void ready() override;
        void setup(matrix_board const& aSetup) override;
    private:
        void play();
    private:
        move_tables<representation_type> const iMoveTables;
        basic_board<representation_type> iBoard;
        ng::sink iSink;
    };

    extern template class ai<matrix, chess::player::White>;
    extern template class ai<matrix, chess::player::Black>;
    extern template class ai<bitboard, chess::player::White>;
    extern template class ai<bitboard, chess::player::Black>;
}