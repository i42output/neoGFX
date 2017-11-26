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
		iCredits{ 10 },
		iStake{ 0 },
		iSpritePlane{ aSpritePlane },
		iMainLayout{ *this, neogfx::alignment::Centre },
		iLabelTitle{ iMainLayout, "VIDEO POKER" },
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
		iGambleLayout{ iMainLayout },
		iBetMinus{ iGambleLayout, "BET\n-" },
		iBetPlus{ iGambleLayout, "BET\n+" },
		iSpacerGamble{ iGambleLayout },
		iDeal{ iGambleLayout, "DEAL" },
		iInfoBarLayout{ iMainLayout },
		iLabelCredits{ iInfoBarLayout, "Credits: " },
		iLabelCreditsValue{ iInfoBarLayout, "" },
		iSpacer5{ iInfoBarLayout },
		iLabelStake{ iInfoBarLayout, "Stake: " },
		iLabelStakeValue{ iInfoBarLayout, "" }
	{
		iMainLayout.set_spacing(neogfx::size{ 16.0 });
		iLabelTitle.text().set_font(neogfx::font{ "Exo 2", "Black", 48.0 });
		iLabelTitle.text().set_text_colour(neogfx::color::Green);
		iSpacer1.set_weight(neogfx::size{ 0.1 });
		iSpacer2.set_weight(neogfx::size{ 0.25 });
		iSpacer3.set_weight(neogfx::size{ 0.25 });
		iSpacer4.set_weight(neogfx::size{ 0.1 });
		iBetMinus.set_weight(neogfx::size{});
		iBetMinus.set_foreground_colour(neogfx::colour::White);
		iBetMinus.text().set_text_colour(neogfx::colour::Black);
		iBetMinus.text().set_font(neogfx::font{ "Exo 2", "Black", 24.0 });
		iBetPlus.set_weight(neogfx::size{});
		iBetPlus.set_foreground_colour(neogfx::colour::White);
		iBetPlus.text().set_text_colour(neogfx::colour::Black);
		iBetPlus.text().set_font(neogfx::font{ "Exo 2", "Black", 24.0 });
		iDeal.set_weight(neogfx::size{});
		iDeal.set_foreground_colour(neogfx::colour::White);
		iDeal.text().set_text_colour(neogfx::colour::Black);
		iDeal.text().set_font(neogfx::font{ "Exo 2", "Black", 24.0 });
		iLabelCredits.text().set_font(neogfx::font{ "Exo 2", "Black", 36.0 });
		iLabelCredits.text().set_text_colour(neogfx::color::Yellow);
		iLabelCreditsValue.text().set_font(neogfx::font{ "Exo 2", "Black", 36.0 });
		iLabelCreditsValue.text().set_text_colour(neogfx::color::White);
		iLabelStake.text().set_font(neogfx::font{ "Exo 2", "Black", 36.0 });
		iLabelStake.text().set_text_colour(neogfx::color::Yellow);
		iLabelStakeValue.text().set_font(neogfx::font{ "Exo 2", "Black", 36.0 });
		iLabelStakeValue.text().set_text_colour(neogfx::color::White);

		iBetMinus.clicked([this]() { bet(-1); });
		iBetPlus.clicked([this]() { bet(+1); });

		update_widgets();
	}

	void table::bet(int32_t aBet)
	{
		if ((aBet > 0 && iCredits > 0) || (aBet < 0 && iStake > 0))
		{
			iCredits -= aBet;
			iStake += aBet;
			update_widgets();
		}
	}

	void table::update_widgets()
	{
		iLabelCreditsValue.text().set_text(u8"£" + boost::lexical_cast<std::string>(iCredits));
		iLabelStakeValue.text().set_text(u8"£" + boost::lexical_cast<std::string>(iStake));
		iBetMinus.enable(iStake > 0);
		iBetPlus.enable(iCredits > 0);
		iDeal.enable(iStake > 0);
	}
}