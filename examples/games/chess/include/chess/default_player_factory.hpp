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

#include <chess/chess.hpp>
#include <chess/i_player.hpp>

namespace chess
{
    class default_player_factory : public i_player_factory
    {
    public:
        static constexpr std::uint32_t MinSkillLevel = 1u;
        static constexpr std::uint32_t MaxSkillLevel = 10u;
        static constexpr std::uint32_t DefaultSkillLevel = 8u;
    public:
        void set_ai_skill_level(chess::player aPlayer, std::uint32_t aSkillLevel);
        std::unique_ptr<i_player> create_player(player_type aType, chess::player aPlayer) override;
    private:
        std::array<std::uint32_t, 2u> iAiSkillLevel = { DefaultSkillLevel, DefaultSkillLevel };
    };
}