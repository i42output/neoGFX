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

#include <chess/engine.hpp>

namespace chess
{
    engine::engine() : 
        iTurn{ player::Invalid }
    {
    }

    void engine::reset()
    {
        setup(player::White, matrix_board_setup);
    }

    void engine::setup(player aTurn, matrix_board const& aBoard)
    {
        iBoard = aBoard;
        iTurn = aTurn;
    }

    bool engine::play(move const& aMove)
    {
        // todo
        return false;
    }
}