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
#include <card_games/card.hpp>
#include <card_games/deck.hpp>

namespace neogames
{
	namespace card_games
	{
		template <typename GameTraits>
		class basic_hand
		{
		public:
			typedef GameTraits game_traits;
			typedef basic_card<game_traits> card_type;
			typedef basic_deck<game_traits> deck_type;
		private:
			typedef std::vector<card_type> cards;
		public:
			basic_hand()
			{
				iCards.reserve(game_traits::hand_size);
			}
		public:
			void pick(deck_type& aDeck)
			{
				iCards.push_back(aDeck.deal_card());
			}
		private:
			cards iCards;
		};

		typedef basic_hand<default_game_traits> hand;
	}
}