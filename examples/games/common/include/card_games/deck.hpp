#include <vector>
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
			}
			card_type deal_card()
			{
			}
			template <typename HandIter>
			void deal_hands(HandIter aFirst, HandIter aLast)
			{
			}
		private:
			cards iCards;
		};

		typedef basic_deck<card> deck;
	}
}