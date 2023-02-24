/*
neogfx C++ App/Game Engine - Examples - Games - Video Poker
Copyright(C) 2017 Leigh Johnston

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

#include <neogfx/core/event.hpp>
#include <card_games/i_card_textures.hpp>

namespace video_poker
{
    using namespace neogames::card_games;

    enum class table_state
    {
        TakeBet,
        DealtFirst,
        DealtSecond,
        GameOver
    };

    class i_table
    {
    public:
        ng::event<table_state> state_changed;
    public:
        virtual table_state state() const = 0;
    public:
        virtual const i_card_textures& textures() const = 0;
    };
}