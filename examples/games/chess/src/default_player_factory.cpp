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

#include <chess/default_player_factory.hpp>
#include <chess/human.hpp>
#include <chess/ai.hpp>

namespace chess
{
    std::unique_ptr<i_player> default_player_factory::create_player(player_type aType, chess::player aPlayer)
    {
        switch (aType) 
        {
        case player_type::Human:
            return std::make_unique<human>(aPlayer);
        case player_type::NetworkedHuman:
            throw not_implemented_yet{ "default_player_factory::create_player" };
        case player_type::AI:
            if (aPlayer == chess::player::White)
                return std::make_unique<ai<mailbox_rep, chess::player::White>>();
            else
                return std::make_unique<ai<mailbox_rep, chess::player::Black>>();
        default:
            throw std::invalid_argument{ "default_player_factory::create_player" };
        }
    }
}