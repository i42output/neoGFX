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
#include <string>

#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/layout/grid_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/spin_box.hpp>

#include <chess/chess.hpp>
#include <chess/i_player.hpp>
#include <chess/default_player_factory.hpp>

namespace chess::gui
{
    enum class game_type : std::uint32_t
    {
        HumanVsHuman,
        HumanVsCPU,
        CPUVsCPU
    };

    struct new_game_settings
    {
        struct player_settings
        {
            std::string name;
            std::uint32_t skillLevel = default_player_factory::DefaultSkillLevel;
        };

        // Player 1 is the human in Human vs CPU; player 2 always plays the opposite color to player 1.
        game_type type = game_type::HumanVsCPU;
        chess::player player1Color = chess::player::White;
        std::array<player_settings, 2u> players = { player_settings{ "Player 1" }, player_settings{ "Player 2" } };

        player_type type_of(std::size_t aPlayerIndex) const;
        chess::player color_of(std::size_t aPlayerIndex) const;
        std::size_t index_of(chess::player aColor) const;
        std::string description(std::size_t aPlayerIndex) const;
    };

    class new_game_dialog : public ng::dialog
    {
    private:
        struct player_box
        {
            ng::group_box box;
            ng::grid_layout grid;
            ng::label nameLabel;
            ng::line_edit name;
            ng::label colorLabel;
            ng::horizontal_layout colorLayout;
            ng::radio_button white;
            ng::radio_button black;
            ng::label skillLabel;
            ng::spin_box skill;

            player_box(ng::i_layout& aLayout, std::string const& aTitle);
        };
    public:
        new_game_dialog(ng::i_widget& aParent, new_game_settings const& aSettings);
    public:
        new_game_settings settings() const;
    private:
        game_type selected_type() const;
        void update_widgets();
    private:
        ng::group_box iGameTypeBox;
        ng::radio_button iHumanVsHuman;
        ng::radio_button iHumanVsCPU;
        ng::radio_button iCPUVsCPU;
        player_box iPlayer1;
        player_box iPlayer2;
    };
}
