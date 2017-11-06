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

#include <unordered_map>
#include <card_games/card.hpp>
#include <card_games/deck.hpp>
#include <card_games/hand.hpp>

namespace video_poker
{
	using namespace neogames::card_games;

	enum poker_hand
	{
		HighCard,
		Pair,
		TwoPair,
		ThreeOfAKind,
		Straight,
		Flush,
		FullHouse,
		FourOfAKind,
		StraightFlush,
		RoyalFlush
	};

	inline double poker_hand_probability(poker_hand aPockerHand)
	{
		static const std::unordered_map<poker_hand, double> sProbabilites
		{
			{ HighCard,			50.1177 },
			{ Pair,				42.2569 },
			{ TwoPair,			4.7539 },
			{ ThreeOfAKind,		2.1128 },
			{ Straight,			0.3925 },
			{ Flush,			0.1965 },
			{ FullHouse,		0.1441 },
			{ FourOfAKind,		0.0240 },
			{ StraightFlush,	0.00139 },
			{ RoyalFlush,		0.000154 }
		};
		return sProbabilites.find(aPockerHand)->second;
	}

	template <typename GameTraits>
	inline poker_hand to_poker_hand(const basic_hand<GameTraits>& aHand)
	{
		// todo
		return poker_hand::HighCard;
	}
}