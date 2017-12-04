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
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/gfx/i_texture_manager.hpp>
#include <video_poker/table.hpp>
#include <video_poker/poker.hpp>

namespace video_poker
{
	using namespace neogames::card_games;

	const int32_t STARTING_CREDITS = 10;
	const int32_t MAX_BET = 5;

	const std::map<poker_hand, int32_t> PAY_TABLE = 
	{
		{ Pair, 1 },
		{ TwoPair, 2 },
		{ ThreeOfAKind, 3 },
		{ Straight, 4 },
		{ Flush, 6 },
		{ FullHouse, 9 },
		{ FourOfAKind, 25 },
		{ StraightFlush, 50 },
		{ RoyalFlush, 250 }
	};

	table::table(neogfx::i_layout& aLayout, neogfx::sprite_plane& aSpritePlane) :
		neogfx::widget{ aLayout },
		iState{ table_state::TakeBet },
		iCredits{ STARTING_CREDITS },
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

		iTextures = neogfx::app::instance().rendering_engine().texture_manager().create_texture_atlas();
		auto& valueTextures = iTextures->create_sub_texture(neogfx::image{ ":/video_poker/resources/values.png" });
		iValueTextures.emplace(card::value::Joker, neogfx::sub_texture{ valueTextures, neogfx::rect{ valueTextures.atlas_location().position(), neogfx::size{36.0, 36.0} } });
		for (auto v = card::value::Two; v <= card::value::Ace; v = static_cast<card::value>(static_cast<uint32_t>(v) + 1))
			iValueTextures.emplace(v, neogfx::sub_texture{ valueTextures, neogfx::rect{ valueTextures.atlas_location().position() + neogfx::point{0.0, (static_cast<uint32_t>(v) - 1) * 36.0}, neogfx::size{ 36.0, 36.0 } } });
		iSuitTextures.emplace(card::suit::Club, iTextures->create_sub_texture(neogfx::image{ ":/video_poker/resources/club.png" }));
		iSuitTextures.emplace(card::suit::Diamond, iTextures->create_sub_texture(neogfx::image{ ":/video_poker/resources/diamond.png" }));
		iSuitTextures.emplace(card::suit::Spade, iTextures->create_sub_texture(neogfx::image{ ":/video_poker/resources/spade.png" }));
		iSuitTextures.emplace(card::suit::Heart, iTextures->create_sub_texture(neogfx::image{ ":/video_poker/resources/heart.png" }));
		iFaceTextures.emplace(card::value::Jack, iTextures->create_sub_texture(neogfx::image{ ":/video_poker/resources/jack.png" }));
		iFaceTextures.emplace(card::value::Queen, iTextures->create_sub_texture(neogfx::image{ ":/video_poker/resources/queen.png" }));
		iFaceTextures.emplace(card::value::King, iTextures->create_sub_texture(neogfx::image{ ":/video_poker/resources/king.png" }));
		update_widgets();
	}

	table_state table::state() const
	{
		return iState;
	}

	const i_card_textures& table::textures() const
	{
		return *this;
	}

	const neogfx::i_texture& table::value_texture(const card& aCard) const
	{
		auto vt = iValueTextures.find(aCard);
		if (vt != iValueTextures.end())
			return vt->second;
		throw texture_not_found();
	}

	const neogfx::i_texture& table::suit_texture(const card& aCard) const
	{
		auto st = iSuitTextures.find(aCard);
		if (st != iSuitTextures.end())
			return st->second;
		throw texture_not_found();
	}

	const neogfx::i_texture& table::face_texture(const card& aCard) const
	{
		auto ft = iFaceTextures.find(aCard);
		if (ft != iFaceTextures.end())
			return ft->second;
		throw texture_not_found();
	}

	void table::bet(int32_t aBet)
	{
		const int32_t minBet = -iStake;
		const int32_t maxBet = std::min(iCredits, MAX_BET - iStake);
		aBet = std::max(minBet, std::min(maxBet, aBet));
		if (aBet != 0)
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
	}

	void table::win(int32_t aWinnings)
	{
		// todo: win animation and sound
		iCredits += aWinnings;
	}

	void table::change_state(table_state aNewState)
	{
		if (iState != aNewState)
		{
			iState = aNewState;
			state_changed.trigger(iState);
			update_widgets();
			switch (iState)
			{
			case table_state::DealtSecond:
				{
					auto w = PAY_TABLE.find(video_poker::to_poker_hand(*iHand));
					if (w != PAY_TABLE.end() && (w->first != video_poker::poker_hand::Pair || most_frequent_card(*iHand) >= card::value::Jack))
						win(w->second * iStake);
					auto lastStake = iStake;
					iStake = 0;
					bet(lastStake);
					/* todo win/lose animation */
					change_state(iCredits + iStake > 0 ? table_state::TakeBet : table_state::GameOver);
				}
				break;
			case table_state::GameOver:
				if (neogfx::message_box::question(*this, "Out Of Credits - Game Over", "You have run out of credits!\n\nPlay again?", neogfx::standard_button::Yes | neogfx::standard_button::No) == neogfx::standard_button::Yes)
				{
					iCredits = STARTING_CREDITS;
					iHand.emplace();
					change_state(table_state::TakeBet);
				}
				else
					neogfx::app::instance().quit();
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