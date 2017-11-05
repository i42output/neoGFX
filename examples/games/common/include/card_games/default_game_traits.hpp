#include <type_traits>

namespace neogames
{
	namespace card_games
	{
		struct default_game_traits
		{
			typedef std::true_type ace_high;
			typedef std::false_type jokers_present;
		};
	}
}