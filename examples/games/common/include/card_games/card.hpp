#include <card_games/default_game_traits.hpp>

namespace neogames
{
	namespace card_games
	{
		template <typename GameTraits>
		class basic_card
		{
		public:
			typedef GameTraits game_traits;
		public:
			enum class value : uint32_t
			{
				Joker	= 0,
				Two		= 2,
				Three	= 3,
				Four	= 4,
				Five	= 5,
				Six		= 6,
				Seven	= 7,
				Eight	= 8,
				Nine	= 9,
				Ten		= 10,
				Jack	= 11,
				Queen	= 12,
				King	= 13,
				Ace		= game_traits::ace_high::value ? King + 1 : Two - 1
			};
			enum class suit : uint32_t
			{
				Joker   = 0,
				Club	= 1,
				Diamond = 2,
				Heart	= 3,
				Spade	= 4
			};
			enum class colour
			{
				Red,
				Black
			};
		public:
			basic_card(value aValue, suit aSuit) :
				iValue{ aValue }, iSuit{ aSuit }
			{
			}
		public:
			operator value() const { return iValue; }
			operator suit() const { return iSuit; }
			operator colour() const { return iSuit == Club || iSuit == Spade ? colour::Black : colour::Red; }
		private:
			value iValue;
			suit iSuit;
		};

		typedef basic_card<default_game_traits> card;
	}
}