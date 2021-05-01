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

#include <chess/move_validator.hpp>

namespace chess
{
    move_validator::move_validator() : 
        iMoveTables{ generate_move_tables<mailbox>() }
    {
    }

    bool move_validator::can_move(player aTurn, mailbox_position const& aPosition, move const& aMove) const
    {
        return chess::can_move<>(iMoveTables, aTurn, aPosition, aMove);
    }

    bool move_validator::has_moves(player aTurn, mailbox_position const& aPosition, coordinates const& aMovePosition) const
    {
        for (coordinate y = 0u; y <= 7u; ++y)
            for (coordinate x = 0u; x <= 7u; ++x)
                if (chess::can_move<>(iMoveTables, aTurn, aPosition, move{ aMovePosition, coordinates{x, y} }))
                    return true;
        return false;
    }

    bool move_validator::in_check(player aTurn, mailbox_position const& aPosition) const
    {
        return chess::in_check(iMoveTables, aTurn, aPosition);
    }

    bool move_validator::check_if_moved(player aTurn, mailbox_position const& aPosition, coordinates const& aMovePosition) const
    {
        for (coordinate y = 0u; y <= 7u; ++y)
            for (coordinate x = 0u; x <= 7u; ++x)
                if (!chess::can_move<>(iMoveTables, aTurn, aPosition, move{ aMovePosition, coordinates{x, y} }))
                    if (chess::can_move<true>(iMoveTables, aTurn, aPosition, move{ aMovePosition, coordinates{x, y} }))
                        return true;
        return false;
    }

    double move_validator::eval(player aTurn, mailbox_position const& aPosition, eval_info& aInfo) const
    {
        if (aTurn == player::White)
            return chess::eval<mailbox, player::White>{}(iMoveTables, aPosition, 1.0, aInfo).eval;
        else if (aTurn == player::Black)
            return chess::eval<mailbox, player::Black>{}(iMoveTables, aPosition, 1.0, aInfo).eval;
        else
            return 0.0;
    }
}