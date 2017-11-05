#include <type_traits>
#include <cstdint>

namespace neogames
{
	struct default_card_game_traits
	{
		typedef std::true_type ace_high;
		typedef std::false_type jokers_present;
	};

	template <typename CardGameTraits>
	class basic_playing_card
	{
	public:
		typedef CardGameTraits card_game_traits;
	public:
		enum class value : uint32_t
		{
			Joker	= 0,
			Ace		= card_game_traits::ace_high::value ? King + 1 : 1
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
		};
		enum class suite : uint32_t
		{
			Club	= 1,
			Diamond	= 2,
			Heart	= 3,
			Spade	= 4
		};
	};

	typedef basic_playing_card<default_card_game_traits> playing_card;
}