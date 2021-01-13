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
    template <typename Representation, chess::player Player>
    ai<Representation, Player>::ai() :
        iMoveTables{ generate_move_tables<representation_type>() },
        iBoard{ chess::setup<representation_type>::position() }
    {
    }

    template <typename Representation, chess::player Player>
    player_type ai<Representation, Player>::type() const
    {
        return player_type::AI;
    }

    template <typename Representation, chess::player Player>
    player ai<Representation, Player>::player() const
    {
        return Player;
    }

    template <typename Representation, chess::player Player>
    void ai<Representation, Player>::greet(i_player& aOpponent)
    {
        iSink = aOpponent.moved([&](move const& aMove)
        {
            move_piece(iBoard, aMove);
            play();
        });
    }

    template <typename Representation, chess::player Player>
    bool ai<Representation, Player>::play(move const& aMove)
    {
        Moved.trigger(aMove);
        return true;
    }

    template <typename Representation, chess::player Player>
    void ai<Representation, Player>::play()
    {
        thread_local std::vector<move> validMoves;
        validMoves.clear();
        for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
            for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
                for (coordinate xTo = 0u; xTo <= 7u; ++xTo)
                    for (coordinate yTo = 0u; yTo <= 7u; ++yTo)
                    {
                        move const canidateMove{ { xFrom, yFrom }, { xTo, yTo } };
                        if (can_move(iMoveTables, Player, iBoard, canidateMove))
                        {
                            validMoves.push_back(canidateMove);
                            // todo: pawn promotion
                        }
                    }
    }

    template class ai<matrix, chess::player::White>;
    template class ai<matrix, chess::player::Black>;
    template class ai<bitboard, chess::player::White>;
    template class ai<bitboard, chess::player::Black>;
}