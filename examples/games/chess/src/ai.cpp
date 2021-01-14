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
#include <chess/ai_thread.hpp>

namespace chess
{
    template <typename Representation, player Player>
    ai<Representation, Player>::ai() :
        iMoveTables{ generate_move_tables<representation_type>() },
        iBoard{ chess::setup_position<representation_type>() }
    {
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
            move_piece(iBoard, aMove);
        });
    }

    template <typename Representation, player Player>
    bool ai<Representation, Player>::play(move const& aMove)
    {
        move_piece(iBoard, aMove);
        Moved.trigger(aMove);
        return true;
    }

    template <typename Representation, player Player>
    void ai<Representation, Player>::play()
    {
        thread_local std::vector<move> tValidMoves;
        tValidMoves.clear();
        for (coordinate xFrom = 0u; xFrom <= 7u; ++xFrom)
            for (coordinate yFrom = 0u; yFrom <= 7u; ++yFrom)
                for (coordinate xTo = 0u; xTo <= 7u; ++xTo)
                    for (coordinate yTo = 0u; yTo <= 7u; ++yTo)
                    {
                        move candidateMove{ { xFrom, yFrom }, { xTo, yTo } };
                        if (can_move(iMoveTables, Player, iBoard, candidateMove))
                        {
                            auto const movingPiece = piece_at(iBoard, candidateMove.from);
                            if (piece_type(movingPiece) == piece::Pawn)
                            {
                                auto const movingPieceColor = piece_color(movingPiece);
                                if ((movingPieceColor == piece::White && candidateMove.to.y == promotion_rank_v<player::White>) ||
                                    (movingPieceColor == piece::Black && candidateMove.to.y == promotion_rank_v<player::Black>))
                                {
                                    candidateMove.promoteTo = piece::Queen | movingPieceColor;
                                    tValidMoves.push_back(candidateMove);
                                    candidateMove.promoteTo = piece::Knight | movingPieceColor;
                                    tValidMoves.push_back(candidateMove);
                                    // todo: do we care about bishop and rook promotion?
                                }
                                else
                                    tValidMoves.push_back(candidateMove);
                            }
                            else
                                tValidMoves.push_back(candidateMove);
                        }
                    }
        thread_local std::random_device tEntropy;
        thread_local std::mt19937 tGenerator(tEntropy());
        if (tValidMoves.size() > 0u)
        {
            ai_thread<representation_type, Player> thread;
            auto e = thread.eval(iBoard, tValidMoves);
            play(e.move);
        }
    }

    template <typename Representation, player Player>
    void ai<Representation, Player>::ready()
    {
        play();
    }

    template <typename Representation, player Player>
    void ai<Representation, Player>::setup(matrix_board const& aSetup)
    {
        if constexpr (std::is_same_v<representation_type, matrix>)
            iBoard = aSetup;
        else
            ; // todo (convert to bitboard representation)
    }

    template class ai<matrix, player::White>;
    template class ai<matrix, player::Black>;
    template class ai<bitboard, player::White>;
    template class ai<bitboard, player::Black>;
}