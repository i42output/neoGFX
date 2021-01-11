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
    template <typename Representation>
    engine<Representation>::engine() :
        iTurn{ player::Invalid }
    {
    }

    template <typename Representation>
    void engine<Representation>::reset()
    {
        setup(player::White, chess::setup<representation_type>::position());
    }

    template <typename Representation>
    void engine<Representation>::setup(player aTurn, basic_board<representation_type> const& aBoard)
    {
        iBoard = aBoard;
        iTurn = aTurn;
    }

    template <typename Representation>
    bool engine<Representation>::play(move const& aMove)
    {
        // todo
        return false;
    }

    template class engine<matrix>;
    template class engine<bitboard>;
}