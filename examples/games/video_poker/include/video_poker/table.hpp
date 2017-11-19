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
#include <video_poker/card_space.hpp>

namespace video_poker
{
	using namespace neogames::card_games;

	class table : public neogfx::widget
	{
	private:
		typedef std::shared_ptr<card_space> card_space_pointer;
	public:
		table(neogfx::i_layout& aLayout, neogfx::sprite_plane& aSpritePlane);
	public:
		neogfx::sprite_plane& iSpritePlane;
		neogfx::vertical_layout iMainLayout;
		neogfx::vertical_spacer iSpacer1;
		neogfx::horizontal_layout iSpacesLayout;
		neogfx::horizontal_spacer iSpacer2;
		std::array<card_space_pointer, 5> iSpaces;
		neogfx::horizontal_spacer iSpacer3;
		neogfx::vertical_spacer iSpacer4;
		neogfx::horizontal_layout iInfoBarLayout;
		neogfx::label iCredits;
		neogfx::label iCreditsValue;
		neogfx::horizontal_spacer iSpacer5;
		neogfx::label iStake;
		neogfx::label iStakeValue;
	};
}