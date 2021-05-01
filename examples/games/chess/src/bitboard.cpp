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
    bitboard_position const& setup_position<bitboard>()
    {
        static const bitboard_position position
        {
            // todo
            {},
            {{
                { 4u, 0u }, { 4u, 7u }
            }},
            player::White
        };
        return position;
    }

    template<>
    move_tables<bitboard> generate_move_tables<bitboard>()
    {
        move_tables<bitboard> result{};
        // todo
        return result;
    }

    template <player Player>
    struct eval<bitboard, Player>
    {
        eval_result operator()(move_tables<bitboard> const& aTables, bitboard_position const& aPosition, double aPly, eval_info* aEvalInfo = nullptr)
        {
            // todo
            if (aEvalInfo)
                *aEvalInfo = eval_info{};
            return { eval_node::Terminal, 0.0 };
        }
        eval_result operator()(move_tables<bitboard> const& aTables, bitboard_position const& aPosition, double aPly, eval_info& aEvalInfo)
        {
            return eval{}(aTables, aPosition, aPly, &aEvalInfo);
        }
    };

    template struct eval<bitboard, player::White>;
    template struct eval<bitboard, player::Black>;
}