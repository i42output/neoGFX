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

#include <algorithm>

namespace chess
{
    namespace
    {
        struct ai_skill
        {
            std::chrono::milliseconds moveTime;
            std::optional<std::int32_t> maxDepth;
        };

        // index is skill level - 1; level 8 (the default) matches the previous fixed AI settings
        ai_skill const sAiSkills[default_player_factory::MaxSkillLevel] =
        {
            { std::chrono::milliseconds{ 250 }, 1 },
            { std::chrono::milliseconds{ 500 }, 2 },
            { std::chrono::milliseconds{ 750 }, 3 },
            { std::chrono::milliseconds{ 1000 }, 4 },
            { std::chrono::milliseconds{ 1500 }, 5 },
            { std::chrono::milliseconds{ 2000 }, 6 },
            { std::chrono::milliseconds{ 2500 }, 8 },
            { std::chrono::milliseconds{ 3000 }, std::nullopt },
            { std::chrono::milliseconds{ 5000 }, std::nullopt },
            { std::chrono::milliseconds{ 10000 }, std::nullopt }
        };
    }

    void default_player_factory::set_ai_skill_level(chess::player aPlayer, std::uint32_t aSkillLevel)
    {
        iAiSkillLevel[as_cardinal(aPlayer)] = std::clamp(aSkillLevel, MinSkillLevel, MaxSkillLevel);
    }

    std::unique_ptr<i_player> default_player_factory::create_player(player_type aType, chess::player aPlayer)
    {
        switch (aType) 
        {
        case player_type::Human:
            return std::make_unique<human>(aPlayer);
        case player_type::NetworkedHuman:
            throw not_implemented_yet{ "default_player_factory::create_player" };
        case player_type::AI:
        {
            auto const& skill = sAiSkills[iAiSkillLevel[as_cardinal(aPlayer)] - 1u];
            return std::make_unique<ai>(aPlayer, skill.moveTime, skill.maxDepth);
        }
        default:
            throw std::invalid_argument{ "default_player_factory::create_player" };
        }
    }
}