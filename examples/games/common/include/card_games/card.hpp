/*
neogfx C++ GUI Library - Examples - Games
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

#include <neogfx/core/event.hpp>
#include <card_games/default_game_traits.hpp>

namespace neogames
{
    namespace card_games
    {
        template <typename GameTraits>
        class basic_card
        {
            typedef basic_card<GameTraits> self_type;
        public:
            neogfx::event<basic_card&> changed;
            neogfx::event<basic_card&> destroyed;
        public:
            typedef GameTraits game_traits;
        public:
            enum class value : uint32_t
            {
                Joker   = 0,
                LowAce  = 1,
                Two     = 2,
                Three   = 3,
                Four    = 4,
                Five    = 5,
                Six     = 6,
                Seven   = 7,
                Eight   = 8,
                Nine    = 9,
                Ten     = 10,
                Jack    = 11,
                Queen   = 12,
                King    = 13,
                Ace     = game_traits::ace_high ? King + 1 : Two - 1
            };
            enum class suit : uint32_t
            {
                Joker   = 0,
                Club    = 1,
                Diamond = 2,
                Heart   = 3,
                Spade   = 4
            };
            enum class color
            {
                Red,
                Black
            };
        public:
            struct bad_value : std::logic_error { bad_value() : std::logic_error("neogames::basic_card::bad_value") {} };
            struct bad_suit : std::logic_error { bad_suit() : std::logic_error("neogames::basic_card::bad_suit") {} };
        public:
            basic_card(value aValue, suit aSuit) :
                iValue{ aValue }, iSuit{ aSuit }, iDiscarded{ false }
            {
            }
            basic_card(const self_type& aOther) :
                iValue{ aOther.iValue }, iSuit{ aOther.iSuit }, iDiscarded{ aOther.iDiscarded }
            {
            }
            ~basic_card()
            {
                destroyed.trigger(*this);
            }
            self_type& operator=(const self_type& aOther)
            {
                iValue = aOther.iValue;
                iSuit = aOther.iSuit;
                iDiscarded = aOther.iDiscarded;
                return *this;
            }
        public:
            void swap(self_type& aOther)
            {
                std::swap(iValue, aOther.iValue);
                std::swap(iSuit, aOther.iSuit);
                std::swap(iDiscarded, aOther.iDiscarded);
            }
        public:
            operator value() const 
            { 
                return iValue; 
            }
            operator suit() const 
            { 
                return iSuit; 
            }
            operator color() const 
            { 
                return iSuit == suit::Club || iSuit == suit::Spade ? color::Black : color::Red;
            }
            bool is_face_card() const
            {
                switch (iValue)
                {
                case value::Jack:
                case value::Queen:
                case value::King:
                    return true;
                case value::Ace:
                    return value::Ace > value::King;
                default:
                    return false;
                }
            }
            bool discarded() const
            {
                return iDiscarded;
            }
            void discard()
            {
                iDiscarded = true;
                changed.trigger(*this);
            }
            void undiscard()
            {
                iDiscarded = false;
                changed.trigger(*this);
            }
        public:
            std::string to_string() const
            {
                return to_string(iValue) + " of " + to_string(iSuit) + "s";
            }
        public:
            bool operator==(const basic_card& aOther) const
            {
                return iValue == aOther.iValue && iSuit == aOther.iSuit;
            }
            bool operator!=(const basic_card& aOther) const
            {
                return !(*this == aOther);
            }
            bool operator<(const basic_card& aRhs) const
            {
                return std::tie(iValue, iSuit) < std::tie(aRhs.iValue, aRhs.iSuit);
            }
        public:
            static const std::string& to_string(value aValue)
            {
                static const std::unordered_map<value, std::string> sValueStrings
                {
                    { value::Joker, "Joker" },
                    { value::Two, "2" },
                    { value::Three, "3" },
                    { value::Four, "4" },
                    { value::Five, "5" },
                    { value::Six, "6" },
                    { value::Seven, "7" },
                    { value::Eight, "8" },
                    { value::Nine, "9" },
                    { value::Ten, "10" },
                    { value::Jack, "Jack" },
                    { value::Queen, "Queen" },
                    { value::King, "King" },
                    { value::Ace, "Ace" }
                };
                auto result = sValueStrings.find(aValue);
                if (result != sValueStrings.end())
                    return result->second;
                throw bad_value();
            }
            static const std::string& to_string(suit aSuit)
            {
                static const std::unordered_map<suit, std::string> sSuitStrings
                {
                    { suit::Joker, "Joker" },
                    { suit::Club, "Club" },
                    { suit::Diamond, "Diamond" },
                    { suit::Heart, "Heart" },
                    { suit::Spade, "Spade" }
                };
                auto result = sSuitStrings.find(aSuit);
                if (result != sSuitStrings.end())
                    return result->second;
                throw bad_suit();
            }
        private:
            value iValue;
            suit iSuit;
            bool iDiscarded;
        };

        template <typename GameTraits>
        inline void swap(basic_card<GameTraits>& aLhs, basic_card<GameTraits>& aRhs)
        {
            aLhs.swap(aRhs);
        }

        typedef basic_card<default_game_traits> card;
    }
}
