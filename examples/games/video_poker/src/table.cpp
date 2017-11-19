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

#include <neogfx/neogfx.hpp>
#include <video_poker/table.hpp>

namespace video_poker
{
	using namespace neogames::card_games;

	table::table(neogfx::i_layout& aLayout, neogfx::sprite_plane& aSpritePlane) :
		neogfx::widget{ aLayout },
		iSpritePlane{ aSpritePlane },
		iMainLayout{ *this },
		iSpacer1{ iMainLayout },
		iSpacesLayout{ iMainLayout },
		iSpacer2{ iSpacesLayout },
		iSpaces{
			std::make_shared<card_space>(iSpacesLayout, iSpritePlane),
			std::make_shared<card_space>(iSpacesLayout, iSpritePlane),
			std::make_shared<card_space>(iSpacesLayout, iSpritePlane),
			std::make_shared<card_space>(iSpacesLayout, iSpritePlane),
			std::make_shared<card_space>(iSpacesLayout, iSpritePlane) },
		iSpacer3{ iSpacesLayout },
		iSpacer4{ iMainLayout },
		iInfoBarLayout{ iMainLayout },
		iCredits{ iInfoBarLayout, "Credits: " },
		iCreditsValue{ iInfoBarLayout, u8"£10" },
		iSpacer5{ iInfoBarLayout },
		iStake{ iInfoBarLayout, "Stake: " },
		iStakeValue{ iInfoBarLayout, u8"£0" }
	{
		iSpacer1.set_weight(neogfx::size{ 0.5 });
		iSpacer2.set_weight(neogfx::size{ 0.25 });
		iSpacer3.set_weight(neogfx::size{ 0.25 });
		iSpacer4.set_weight(neogfx::size{ 0.5 });
		iCredits.text().set_font(neogfx::font{ "Exo 2", "Black", 18.0 });
		iCredits.text().set_text_colour(neogfx::color::Yellow);
		iCreditsValue.text().set_font(neogfx::font{ "Exo 2", "Black", 18.0 });
		iCreditsValue.text().set_text_colour(neogfx::color::White);
		iStake.text().set_font(neogfx::font{ "Exo 2", "Black", 18.0 });
		iStake.text().set_text_colour(neogfx::color::Yellow);
		iStakeValue.text().set_font(neogfx::font{ "Exo 2", "Black", 18.0 });
		iStakeValue.text().set_text_colour(neogfx::color::White);
	}
}