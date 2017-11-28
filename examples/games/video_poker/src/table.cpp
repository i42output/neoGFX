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

	const int32_t MAX_BET = 5;

	table::table(neogfx::i_layout& aLayout, neogfx::sprite_plane& aSpritePlane) :
		neogfx::widget{ aLayout },
		iState{ table_state::TakeBet },
		iCredits{ 10 },
		iStake{ 0 },
		iSpritePlane{ aSpritePlane },
		iMainLayout{ *this, neogfx::alignment::Centre },
		iLabelTitle{ iMainLayout, "VIDEO POKER" },
		iSpacer1{ iMainLayout },
		iSpacesLayout{ iMainLayout },
		iSpacer2{ iSpacesLayout },
		iSpaces{
			std::make_shared<card_space>(iSpacesLayout, iSpritePlane, *this),
			std::make_shared<card_space>(iSpacesLayout, iSpritePlane, *this),
			std::make_shared<card_space>(iSpacesLayout, iSpritePlane, *this),
			std::make_shared<card_space>(iSpacesLayout, iSpritePlane, *this),
			std::make_shared<card_space>(iSpacesLayout, iSpritePlane, *this) },
		iSpacer3{ iSpacesLayout },
		iSpacer4{ iMainLayout },
		iGambleLayout{ iMainLayout },
		iBetMinus{ iGambleLayout, "BET\n-" },
		iBetPlus{ iGambleLayout, "BET\n+" },
		iBetMax{ iGambleLayout, "MAX\nBET" },
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
		auto set_bet_button_apperance = [](neogfx::push_button& aButton)
		{
			aButton.set_size_policy(neogfx::size_policy::Minimum, neogfx::size{ 1.0 });
			aButton.set_weight(neogfx::size{});
			aButton.set_foreground_colour(neogfx::colour::White);
			aButton.text().set_size_hint("MAX\nBET");
			aButton.text().set_text_colour(neogfx::colour::Black);
			aButton.text().set_font(neogfx::font{ "Exo 2", "Black", 24.0 });
		};
		set_bet_button_apperance(iBetMinus);
		set_bet_button_apperance(iBetPlus);
		set_bet_button_apperance(iBetMax);
		set_bet_button_apperance(iDeal);
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
		iBetMax.clicked([this]() { bet(MAX_BET); });
		iDeal.clicked([this]() { deal(); });

		update_widgets();
	}

	table_state table::state() const
	{
		return iState;
	}

	void table::bet(int32_t aBet)
	{
		aBet = std::min(aBet, MAX_BET - iStake);
		if ((aBet > 0 && iCredits > 0) || (aBet < 0 && iStake > 0))
		{
			iCredits -= aBet;
			iStake += aBet;
			update_widgets();
		}
	}

	void table::deal()
	{
		switch (iState)
		{
		case table_state::TakeBet:
			iDeck.emplace();
			iDeck->shuffle();
			iHand.emplace();
			iDeck->deal_hand(*iHand);
			for (std::size_t i = 0; i < 5; ++i)
			{
				auto& card = iHand->card_at(i);
				iSpaces[i]->set_card(card);
				card.discard();
			}
			change_state(table_state::DealtFirst);
			break;
		case table_state::DealtFirst:
			iDeck->exchange_cards(*iHand);
			for (std::size_t i = 0; i < 5; ++i)
				iSpaces[i]->set_card(iHand->card_at(i));
			change_state(table_state::DealtSecond);
			break;
		}
		update_widgets();
	}

	void table::change_state(table_state aNewState)
	{
		if (iState != aNewState)
		{
			iState = aNewState;
			state_changed.trigger(iState);
			switch (iState)
			{
			case table_state::DealtSecond:
				{
					auto lastStake = iStake;
					iStake = 0;
					bet(lastStake);
					/* todo win/lose animation */
					change_state(table_state::TakeBet);
				}
				break;
			default:
				// do nothing
				break;
			}
		}
	}

	void table::update_widgets()
	{
		iLabelCreditsValue.text().set_text(u8"£" + boost::lexical_cast<std::string>(iCredits));
		iLabelStakeValue.text().set_text(u8"£" + boost::lexical_cast<std::string>(iStake));
		iBetMinus.enable(iState == table_state::TakeBet && iStake > 0);
		iBetPlus.enable(iState == table_state::TakeBet && iCredits > 0 && iStake < MAX_BET);
		iBetMax.enable(iState == table_state::TakeBet && iCredits > 0 && iStake < MAX_BET);
		iDeal.enable(iState != table_state::DealtSecond && iStake > 0);
	}
}