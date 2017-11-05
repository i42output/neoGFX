/*
neogfx C++ GUI Library - Examples
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

#include <vector>
#include <algorithm>
#include <random>
#include <card_games/card.hpp>

namespace neogames
{
	namespace card_games
	{
		template <typename CardType>
		class basic_deck
		{
		public:
			typedef CardType card_type;
			typedef typename card_type::game_traits game_traits;
			typedef typename card_type::value card_value;
			typedef typename card_type::suit card_suit;
			typedef std::vector<card_type> cards;
		public:
			struct deck_empty : std::runtime_error { deck_empty() : std::runtime_error("neogames::card_games::basic_deck::deck_empty") {} };
		public:
			basic_deck() :
				iCards{
					{ card_value::Ace,		card_suit::Club },
					{ card_value::Two,		card_suit::Club },
					{ card_value::Three,	card_suit::Club },
					{ card_value::Four,		card_suit::Club },
					{ card_value::Five,		card_suit::Club },
					{ card_value::Six,		card_suit::Club },
					{ card_value::Seven,	card_suit::Club },
					{ card_value::Eight,	card_suit::Club },
					{ card_value::Nine,		card_suit::Club },
					{ card_value::Ten,		card_suit::Club },
					{ card_value::Jack,		card_suit::Club },
					{ card_value::Queen,	card_suit::Club },
					{ card_value::King,		card_suit::Club },
					{ card_value::Ace,		card_suit::Diamond },
					{ card_value::Two,		card_suit::Diamond },
					{ card_value::Three,	card_suit::Diamond },
					{ card_value::Four,		card_suit::Diamond },
					{ card_value::Five,		card_suit::Diamond },
					{ card_value::Six,		card_suit::Diamond },
					{ card_value::Seven,	card_suit::Diamond },
					{ card_value::Eight,	card_suit::Diamond },
					{ card_value::Nine,		card_suit::Diamond },
					{ card_value::Ten,		card_suit::Diamond },
					{ card_value::Jack,		card_suit::Diamond },
					{ card_value::Queen,	card_suit::Diamond },
					{ card_value::King,		card_suit::Diamond },
					{ card_value::Ace,		card_suit::Heart },
					{ card_value::Two,		card_suit::Heart },
					{ card_value::Three,	card_suit::Heart },
					{ card_value::Four,		card_suit::Heart },
					{ card_value::Five,		card_suit::Heart },
					{ card_value::Six,		card_suit::Heart },
					{ card_value::Seven,	card_suit::Heart },
					{ card_value::Eight,	card_suit::Heart },
					{ card_value::Nine,		card_suit::Heart },
					{ card_value::Ten,		card_suit::Heart },
					{ card_value::Jack,		card_suit::Heart },
					{ card_value::Queen,	card_suit::Heart },
					{ card_value::King,		card_suit::Heart },
					{ card_value::Ace,		card_suit::Spade },
					{ card_value::Two,		card_suit::Spade },
					{ card_value::Three,	card_suit::Spade },
					{ card_value::Four,		card_suit::Spade },
					{ card_value::Five,		card_suit::Spade },
					{ card_value::Six,		card_suit::Spade },
					{ card_value::Seven,	card_suit::Spade },
					{ card_value::Eight,	card_suit::Spade },
					{ card_value::Nine,		card_suit::Spade },
					{ card_value::Ten,		card_suit::Spade },
					{ card_value::Jack,		card_suit::Spade },
					{ card_value::Queen,	card_suit::Spade },
					{ card_value::King,		card_suit::Spade }
				}
			{
				if (game_traits::jokers_present::value)
					iCards.insert(iCards.end(), 2, card_type{ card_value::Joker, card_suit::Joker });
			}
		public:
			void shuffle()
			{
				std::random_device rd;
				std::mt19937 g(rd());
				std::shuffle(iCards.begin(), iCards.end(), g);
			}
			card_type deal_card()
			{
				if (iCards.empty())
					throw deck_empty();
				auto c = iCards.back();
				iCards.pop_back();
			}
			template <typename HandIter>
			void deal_hands(HandIter aFirstHand, HandIter aLastHand)
			{
				bool allHandsDelt = false;
				while (!allHandsDelt)
				{
					allHandsDelt = true;
					for (auto hand = aFirstHand; hand != aLastHand; ++hand)
						if (hand->take(deal_card()) && !hand->dealt())
							allHandsDelt = false;
				}

			}
		private:
			cards iCards;
		};

		typedef basic_deck<card> deck;
	}
}