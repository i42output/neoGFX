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

#include <neogfx/gfx/i_texture.hpp>
#include <card_games/card.hpp>

namespace neogames
{
	namespace card_games
	{
		template <typename GameTraits>
		class i_basic_card_textures
		{
		public:
			typedef basic_card<GameTraits> card_type;
		public:
			struct texture_not_found : std::logic_error { texture_not_found() : std::logic_error("neogames::card_games::i_basic_card_textures::texture_not_found") {} };
		public:
			virtual const neogfx::i_texture& value_texture(const card_type& aCard) const = 0;
			virtual const neogfx::i_texture& suit_texture(const card_type& aCard) const = 0;
			virtual const neogfx::i_texture& face_texture(const card_type& aCard) const = 0;
		};

		typedef i_basic_card_textures<default_game_traits> i_card_textures;
	}
}