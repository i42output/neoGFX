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
    template <typename Representation>
    ai<Representation>::ai() :
        iPlayer{ chess::player::Invalid }
    {
    }

    template <typename Representation>
    player_type ai<Representation>::type() const
    {
        return player_type::AI;
    }

    template <typename Representation>
    player ai<Representation>::player() const
    {
        return iPlayer;
    }

    template <typename Representation>
    void ai<Representation>::greet(chess::player aPlayer, i_player& aOpponent)
    {
        iPlayer = aPlayer;
        iBoard = chess::setup<representation_type>::position();
        iSink = aOpponent.moved([&](move const& aMove)
        {
            // todo
        });
    }

    template <typename Representation>
    bool ai<Representation>::play(move const& aMove)
    {
        Moved.trigger(aMove);
    }


    template class ai<matrix>;
    template class ai<bitboard>;
}