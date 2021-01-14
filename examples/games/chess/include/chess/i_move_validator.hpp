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

#include <chess/primitives.hpp>
#include <chess/matrix.hpp>

namespace chess
{
    class i_move_validator
    {
    public:
        virtual ~i_move_validator() = default;
    public:
        virtual bool can_move(player aTurn, matrix_board const& aBoard, move const& aMove) const = 0;
        virtual bool has_moves(player aTurn, matrix_board const& aBoard, coordinates const& aMovePosition) const = 0;
        virtual bool in_check(player aTurn, matrix_board const& aBoard) const = 0;
        virtual bool check_if_moved(player aTurn, matrix_board const& aBoard, coordinates const& aMovePosition) const = 0;
        virtual double eval(player aTurn, matrix_board const& aBoard, eval_info& aInfo) const = 0;
    };
}