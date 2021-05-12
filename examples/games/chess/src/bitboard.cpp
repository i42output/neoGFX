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

#include <vector>

#include <chess/bitboard.hpp>

namespace chess
{
    template <>
    bitboard_position const& setup_position<bitboard_rep>()
    {
        static const bitboard_position position
        {
            // same information structured differently based on how the bitboard algorithms use the information...
            {
                {{ // byColor
                    { 0b0000000000000000000000000000000000000000000000001111111111111111ull }, // white
                    { 0b1111111111111111000000000000000000000000000000000000000000000000ull }  // black
                }},
                {{ // byPiece
                    { 0b0000000011111111000000000000000000000000000000001111111100000000ull }, // pawn
                    { 0b0100001000000000000000000000000000000000000000000000000001000010ull }, // knight
                    { 0b0010010000000000000000000000000000000000000000000000000000100100ull }, // bishop
                    { 0b1000000100000000000000000000000000000000000000000000000010000001ull }, // rook
                    { 0b0000100000000000000000000000000000000000000000000000000000001000ull }, // queen
                    { 0b0001000000000000000000000000000000000000000000000000000000010000ull }  // king
                }}, 
                {{ // bySquare
                    piece::WhiteRook, piece::WhiteKnight, piece::WhiteBishop, piece::WhiteQueen, piece::WhiteKing, piece::WhiteBishop, piece::WhiteKnight, piece::WhiteRook,
                    piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn, piece::WhitePawn,
                    {}, {}, {}, {}, {}, {}, {}, {},
                    {}, {}, {}, {}, {}, {}, {}, {},
                    {}, {}, {}, {}, {}, {}, {}, {},
                    {}, {}, {}, {}, {}, {}, {}, {},
                    piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn, piece::BlackPawn,
                    piece::BlackRook, piece::BlackKnight, piece::BlackBishop, piece::BlackQueen, piece::BlackKing, piece::BlackBishop, piece::BlackKnight, piece::BlackRook,
                }}
            },
            {{ // kings
                { 4u, 0u }, { 4u, 7u }
            }},
            player::White
        };
        return position;
    }

    template<>
    move_tables<bitboard_rep> generate_move_tables<bitboard_rep>()
    {
        move_tables<bitboard_rep> result{ generate_move_tables<mailbox_rep>() };
        // todo
        return result;
    }

    template <player Player>
    struct eval<bitboard_rep, Player>
    {
        eval_result operator()(move_tables<bitboard_rep> const& aTables, bitboard_position const& aPosition, double aPly, eval_info* aEvalInfo = nullptr)
        {
            // todo
            if (aEvalInfo)
                *aEvalInfo = eval_info{};
            return { eval_node::Terminal, 0.0 };
        }
        eval_result operator()(move_tables<bitboard_rep> const& aTables, bitboard_position const& aPosition, double aPly, eval_info& aEvalInfo)
        {
            return eval{}(aTables, aPosition, aPly, &aEvalInfo);
        }
    };

    template struct eval<bitboard_rep, player::White>;
    template struct eval<bitboard_rep, player::Black>;
}