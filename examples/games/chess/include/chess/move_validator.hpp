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

#pragma once

#include <array>
#include <vector>

#include <chess/i_move_validator.hpp>

namespace chess
{
    move_tables generate_move_tables();
        
    class move_validator : public i_move_validator
    {
    public:
        move_validator();
    public:
        bool can_move(player aTurn, board const& aBoard, move const& aMove) const override;
    private:
        move_tables iMoveTables;
    };
}