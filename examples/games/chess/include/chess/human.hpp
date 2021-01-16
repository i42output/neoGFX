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
#include <chess/i_player.hpp>

namespace chess
{
    class human : public i_player 
    {
    public:
        define_declared_event(Moved, moved, move)
    public:
        human(chess::player aPlayer);
    public:
        player_type type() const override;
        chess::player player() const override;
    public:
        void greet(i_player& aOpponent) override;
        bool play(move const& aMove) override;
        void undo() override;
        void ready() override;
        void setup(matrix_board const& aSetup) override;
    private:
        ng::sink iSink;
        chess::player iPlayer;
        std::optional<move> iLastMove;
    };
}