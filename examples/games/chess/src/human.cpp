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

#include <chess/human.hpp>

namespace chess
{
    human::human(chess::player aPlayer) :
        iPlayer{ aPlayer }
    {
    }

    player_type human::type() const
    {
        return player_type::Human;
    }

    player human::player() const
    {
        return iPlayer;
    }

    void human::greet(i_player& aOpponent)
    {
    }

    void human::play()
    {
    }

    void human::stop()
    {
    }

    void human::finish()
    {
    }

    bool human::play(move const& aMove)
    {
        iLastMove = aMove;
        Moved(aMove);
        return true;
    }

    bool human::playing() const
    {
        return false;
    }

    void human::undo()
    {
    }

    void human::setup(mailbox_position const& aSetup)
    {
    }

    std::uint64_t human::nodes_per_second() const
    {
        return 0;
    }
}