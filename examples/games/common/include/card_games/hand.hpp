/*
neogfx C++ App/Game Engine - Examples - Games
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

#include <array>
#include <optional>
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
            typedef std::optional<card_type> optional_card;
            typedef std::array<optional_card, game_traits::hand_size> cards;
            typedef typename cards::size_type size_type;
        public:
            struct no_card_in_slot : std::logic_error { no_card_in_slot() : std::logic_error("neogames::card_games::basic_hand::no_card_in_slot") {} };
            struct bad_slot_index : std::logic_error { bad_slot_index() : std::logic_error("neogames::card_games::basic_hand::bad_slot_index") {} };
        public:
            basic_hand()
            {
            }
        public:
            bool have_card_at(size_type aSlotIndex) const
            {
                if (aSlotIndex >= iCards.size())
                    throw bad_slot_index();
                return iCards[aSlotIndex] != std::nullopt;
            }
            const card_type& card_at(size_type aSlotIndex) const
            {
                if (aSlotIndex >= iCards.size())
                    throw bad_slot_index();
                if (have_card_at(aSlotIndex))
                    return *iCards[aSlotIndex];
                throw no_card_in_slot();
            }
            card_type& card_at(size_type aSlotIndex)
            {
                return const_cast<card_type&>(neolib::to_const(*this).card_at(aSlotIndex));
            }
            bool contains(typename card_type::value aCardValue) const
            {
                for (auto& slot : iCards)
                    if (slot && *slot == aCardValue)
                        return true;
                return false;
            }
            bool fully_dealt() const
            {
                size_type totalDealt = 0;
                for (auto& slot : iCards)
                    if (slot && !slot->discarded())
                        ++totalDealt;
                return totalDealt == iCards.size();
            }
            bool pick(deck_type& aDeck)
            {
                for (auto& slot : iCards)
                    if (!slot || slot->discarded())
                    {
                        slot.emplace(aDeck.deal_card());
                        return true;
                    }
                return false;
            }
            void discard(size_type aSlotIndex)
            {
                if (aSlotIndex >= iCards.size())
                    throw bad_slot_index();
                iCards[aSlotIndex] = std::nullopt;
            }
        private:
            cards iCards;
        };

        typedef basic_hand<default_game_traits> hand;
    }
}