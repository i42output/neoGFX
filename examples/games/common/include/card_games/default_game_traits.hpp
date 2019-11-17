/*
neogfx C++ GUI Library - Examples - Games
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

#include <cstdint>
#include <type_traits>

namespace neogames
{
    namespace card_games
    {
        struct default_game_traits
        {
            static inline constexpr uint32_t hand_size = 5u;
            static inline constexpr bool ace_high = true;
            static inline constexpr bool jokers_present = false;
        };

        typedef default_game_traits poker_game_traits;
    }
}