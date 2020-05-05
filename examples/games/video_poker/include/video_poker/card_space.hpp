/*
neogfx C++ GUI Library - Examples - Games - Video Poker
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

#include <neogfx/neogfx.hpp>
#include <map>
#include <unordered_map>
#include <set>
#include <boost/pool/pool_alloc.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/game/canvas.hpp>
#include <neogfx/game/sprite.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <card_games/card.hpp>
#include <video_poker/flashing_button.hpp>
#include <video_poker/i_table.hpp>

namespace video_poker
{
    using namespace neogames::card_games;

    class card_widget : public neogfx::widget
    {
    public:
        card_widget(neogfx::i_layout& aLayout, neogfx::game::canvas& aCanvas, const i_card_textures& aCardTextures);
    protected:
        neogfx::size minimum_size(const neogfx::optional_size& aAvailableSpace = {}) const override;
        neogfx::size maximum_size(const neogfx::optional_size& aAvailableSpace = {}) const override;
    protected:
        void paint(neogfx::i_graphics_context& aGc) const override;
    public:
        bool has_card() const;
        video_poker::card& card() const;
        void set_card(video_poker::card& aCard);
        void clear_card();
    private:
        void update_sprite_geometry();
        void toggle_hold();
    private:
        neogfx::game::canvas& iCanvas;
        const i_card_textures& iCardTextures;
        neogfx::sink iSink;
        video_poker::card* iCard;
        neogfx::game::entity_id iCardSprite;
    };

    class card_space : neogfx::widget
    {
    public:
        struct no_card : std::runtime_error { no_card() : std::runtime_error("video_poker::card_space::no_card") {} };
    public:
        card_space(neogfx::i_layout& aLayout, neogfx::game::canvas& aCanvas, i_table& aTable);
    public:
        bool has_card() const;
        const video_poker::card& card() const;
        video_poker::card& card();
        void set_card(video_poker::card& aCard);
        void clear_card();
    private:
        void update_widgets();
    public:
        neogfx::game::canvas& iCanvas;
        i_table& iTable;
        neogfx::vertical_layout iVerticalLayout;
        card_widget iCardWidget;
        flashing_button iHoldButton;
        video_poker::card* iCard;
        neogfx::sink iSink;
    };
}