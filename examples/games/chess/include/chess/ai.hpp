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

#include <chess/i_player.hpp>

namespace chess
{
    template <typename Representation>
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
        void greet(chess::player aPlayer, i_player& aOpponent) override;
        bool play(move const& aMove) override;
    private:
        chess::player iPlayer;
        basic_board<representation_type> iBoard;
        ng::sink iSink;
    };

    extern template class ai<matrix>;
    extern template class ai<bitboard>;
}