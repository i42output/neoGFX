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

#include <neogfx/game/shape.hpp>
#include <card_games/card.hpp>

namespace neogames
{
	namespace card_games
	{
		template <typename GameTraits>
		class basic_card_shape : public neogfx::shape<>
		{
			// types
		public:
			typedef GameTraits game_traits;
			typedef basic_card<game_traits> card_type;
			// construction
		public:
			basic_card_shape(const card_type& aCard) :
				iCard{ aCard }
			{
			}
			// rendering
		public:
			void paint(neogfx::graphics_context& aGraphicsContext) const override
			{
				neogfx::shape<>::paint(aGraphicsContext);
				auto rect = bounding_box_2d();
				// todo
			}
			// attributes
		private:
			card_type iCard;
		};

		typedef basic_card_shape<default_game_traits> card_shape;
	}
}