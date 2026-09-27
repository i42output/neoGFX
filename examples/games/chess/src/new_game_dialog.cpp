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

#include <chess/new_game_dialog.hpp>

namespace chess::gui
{
    player_type new_game_settings::type_of(std::size_t aPlayerIndex) const
    {
        switch (type)
        {
        case game_type::HumanVsHuman:
            return player_type::Human;
        case game_type::HumanVsCPU:
            return aPlayerIndex == 0u ? player_type::Human : player_type::AI;
        case game_type::CPUVsCPU:
        default:
            return player_type::AI;
        }
    }

    chess::player new_game_settings::color_of(std::size_t aPlayerIndex) const
    {
        return aPlayerIndex == 0u ? player1Color : opponent(player1Color);
    }

    std::size_t new_game_settings::index_of(chess::player aColor) const
    {
        return aColor == player1Color ? 0u : 1u;
    }

    std::string new_game_settings::description(std::size_t aPlayerIndex) const
    {
        std::string const color = color_of(aPlayerIndex) == chess::player::White ? "White" : "Black";
        if (type_of(aPlayerIndex) == player_type::AI)
            return "CPU Level " + std::to_string(players[aPlayerIndex].skillLevel) + " (" + color + ")";
        if (players[aPlayerIndex].name.empty())
            return color;
        return players[aPlayerIndex].name + " (" + color + ")";
    }

    new_game_dialog::player_box::player_box(ng::i_layout& aLayout, std::string const& aTitle) :
        box{ aLayout, aTitle },
        grid{ box.item_layout(), 3u, 2u, ng::alignment::Left | ng::alignment::VCenter },
        nameLabel{ grid, "Name:"_t },
        name{ grid },
        colorLabel{ grid, "Color:"_t },
        colorLayout{ grid },
        white{ colorLayout, "White"_t },
        black{ colorLayout, "Black"_t },
        skillLabel{ grid, "CPU skill level:"_t },
        skill{ grid }
    {
        name.set_size_policy(ng::size_constraint::Expanding, ng::size_constraint::Minimum);
        skill.set_minimum(static_cast<std::int32_t>(default_player_factory::MinSkillLevel));
        skill.set_maximum(static_cast<std::int32_t>(default_player_factory::MaxSkillLevel));
        skill.set_step(1);
    }

    new_game_dialog::new_game_dialog(ng::i_widget& aParent, new_game_settings const& aSettings) :
        ng::dialog{ aParent, "New Game"_t, ng::window_style::Dialog | ng::window_style::Modal | ng::window_style::TitleBar | ng::window_style::Close },
        iGameTypeBox{ client_layout(), "Game Type"_t },
        iHumanVsHuman{ iGameTypeBox.item_layout(), "Human vs Human"_t },
        iHumanVsCPU{ iGameTypeBox.item_layout(), "Human vs CPU"_t },
        iCPUVsCPU{ iGameTypeBox.item_layout(), "CPU vs CPU"_t },
        iPlayer1{ client_layout(), "Player 1"_t },
        iPlayer2{ client_layout(), "Player 2"_t }
    {
        switch (aSettings.type)
        {
        case game_type::HumanVsHuman:
            iHumanVsHuman.set_on();
            break;
        case game_type::HumanVsCPU:
            iHumanVsCPU.set_on();
            break;
        case game_type::CPUVsCPU:
            iCPUVsCPU.set_on();
            break;
        }

        if (aSettings.player1Color == chess::player::White)
        {
            iPlayer1.white.set_on();
            iPlayer2.black.set_on();
        }
        else
        {
            iPlayer1.black.set_on();
            iPlayer2.white.set_on();
        }

        iPlayer1.name.set_text(aSettings.players[0u].name);
        iPlayer2.name.set_text(aSettings.players[1u].name);
        iPlayer1.skill.set_value(static_cast<std::int32_t>(aSettings.players[0u].skillLevel));
        iPlayer2.skill.set_value(static_cast<std::int32_t>(aSettings.players[1u].skillLevel));

        iHumanVsHuman.On([this]() { update_widgets(); });
        iHumanVsCPU.On([this]() { update_widgets(); });
        iCPUVsCPU.On([this]() { update_widgets(); });

        // the two players always play opposite colors; choosing a color for either player sets the other
        iPlayer1.white.On([this]() { iPlayer2.black.set_on(); });
        iPlayer1.black.On([this]() { iPlayer2.white.set_on(); });
        iPlayer2.white.On([this]() { iPlayer1.black.set_on(); });
        iPlayer2.black.On([this]() { iPlayer1.white.set_on(); });

        button_box().add_button(ng::standard_button::Ok);
        button_box().add_button(ng::standard_button::Cancel);

        update_widgets();

        center_on_parent();
        set_ready_to_render(true);
    }

    new_game_settings new_game_dialog::settings() const
    {
        new_game_settings result;
        result.type = selected_type();
        result.player1Color = iPlayer1.white.is_on() ? chess::player::White : chess::player::Black;
        result.players[0u].name = iPlayer1.name.text().to_std_string();
        result.players[1u].name = iPlayer2.name.text().to_std_string();
        result.players[0u].skillLevel = static_cast<std::uint32_t>(iPlayer1.skill.value());
        result.players[1u].skillLevel = static_cast<std::uint32_t>(iPlayer2.skill.value());
        return result;
    }

    game_type new_game_dialog::selected_type() const
    {
        if (iHumanVsHuman.is_on())
            return game_type::HumanVsHuman;
        if (iCPUVsCPU.is_on())
            return game_type::CPUVsCPU;
        return game_type::HumanVsCPU;
    }

    void new_game_dialog::update_widgets()
    {
        new_game_settings current;
        current.type = selected_type();
        for (std::size_t i = 0u; i < 2u; ++i)
        {
            auto& playerBox = (i == 0u ? iPlayer1 : iPlayer2);
            bool const human = (current.type_of(i) == player_type::Human);
            playerBox.nameLabel.enable(human);
            playerBox.name.enable(human);
            playerBox.skillLabel.enable(!human);
            playerBox.skill.enable(!human);
        }
    }
}
