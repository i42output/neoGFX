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

#include <map>
#include <unordered_map>
#include <set>
#include <boost/pool/pool_alloc.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/game/sprite_plane.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <neogfx/gfx/texture_atlas.hpp>
#include <card_games/deck.hpp>
#include <card_games/hand.hpp>
#include <video_poker/flashing_button.hpp>
#include <video_poker/card_space.hpp>
#include <video_poker/i_table.hpp>

namespace video_poker
{
	using namespace neogames::card_games;

	class table : public neogfx::widget, public i_table, private i_card_textures
	{
	private:
		typedef std::shared_ptr<card_space> card_space_pointer;
	public:
		table(neogfx::i_layout& aLayout, neogfx::sprite_plane& aSpritePlane);
	public:
		table_state state() const override;
	public:
		const i_card_textures& textures() const override;
	private:
		const neogfx::i_texture& value_texture(const card& aCard) const override;
		const neogfx::i_texture& suit_texture(const card& aCard) const override;
		const neogfx::i_texture& face_texture(const card& aCard) const override;
	private:
		void bet(int32_t aBet);
		void deal();
		void win(int32_t aWinnings);
		void change_state(table_state aNewState);
		void update_widgets();
	private:
		table_state iState;
		int32_t iCredits;
		int32_t iStake;
		boost::optional<deck> iDeck;
		boost::optional<hand> iHand;
		neogfx::sprite_plane& iSpritePlane;
		neogfx::vertical_layout iMainLayout;
		neogfx::label iLabelTitle;
		neogfx::vertical_spacer iSpacer1;
		neogfx::horizontal_layout iSpacesLayout;
		neogfx::horizontal_spacer iSpacer2;
		std::array<card_space_pointer, 5> iSpaces;
		neogfx::horizontal_spacer iSpacer3;
		neogfx::vertical_spacer iSpacer4;
		neogfx::horizontal_layout iGambleLayout;
		flashing_button iBetMinus;
		flashing_button iBetPlus;
		flashing_button iBetMax;
		neogfx::horizontal_spacer iSpacerGamble;
		flashing_button iDeal;
		neogfx::horizontal_layout iInfoBarLayout;
		neogfx::label iLabelCredits;
		neogfx::label iLabelCreditsValue;
		neogfx::horizontal_spacer iSpacer5;
		neogfx::label iLabelStake;
		neogfx::label iLabelStakeValue;
		std::unique_ptr<neogfx::i_texture_atlas> iTextures;
		std::map<card::value, neogfx::sub_texture> iValueTextures;
		std::map<card::suit, neogfx::sub_texture> iSuitTextures;
		std::map<card::value, neogfx::sub_texture> iFaceTextures;
	};
}