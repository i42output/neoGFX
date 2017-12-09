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

#include <neogfx/game/sprite.hpp>
#include <neogfx/game/shapes.hpp>
#include <card_games/card.hpp>
#include <card_games/i_card_textures.hpp>

namespace neogames
{
	namespace card_games
	{
		namespace
		{
			const neogfx::size kBridgeCardSize{ 57.15, 88.9 };
		}

		template <typename GameTraits>
		class basic_card_sprite : public neogfx::sprite
		{
			// types
		public:
			typedef GameTraits game_traits;
			typedef basic_card<game_traits> card_type;
			// construction
		public:
			basic_card_sprite(const i_card_textures& aCardTextures, const card_type& aCard) :
				sprite{ neogfx::colour::White }, iCardTextures { aCardTextures }, iCard{ aCard }
			{
				auto vlp = std::make_shared<neogfx::vertex_list>();
				auto cardVertices = neogfx::rounded_rect_vertices(neogfx::rect{ neogfx::point{}, neogfx::size{1.0, kBridgeCardSize.cy / kBridgeCardSize.cx } }.with_centred_origin(), 0.1, true, 20);
				vlp->reserve(cardVertices.end() - cardVertices.begin());
				for (const auto& v : cardVertices)
					vlp->push_back(neogfx::vertex{ v });
				set_vertices(vlp);
				neogfx::face_list faceList;
				for (neogfx::vertex_index vi = 1; vi + 1 < cardVertices.size(); ++vi)
					faceList.faces().push_back(neogfx::face{ {0, vi, vi + 1} });
				set_faces(faceList);
			}
			// geometry
		protected:
			neogfx::mat44 transformation_matrix() const override
			{
				return neogfx::mat44{ { extents()[0], 0.0, 0.0, 0.0 },{ 0.0, extents()[1] * kBridgeCardSize.cx / kBridgeCardSize.cy, 0.0, 0.0 },{ 0.0, 0.0, 1.0, 0.0 },{ position().x, position().y, position().z, 1.0 } };
			}
			// rendering
		protected:
			void paint(neogfx::graphics_context& aGraphicsContext) const override
			{
				sprite::paint(aGraphicsContext);
			}
			// attributes
		private:
			const i_card_textures& iCardTextures;
			card_type iCard;
		};

		typedef basic_card_sprite<default_game_traits> card_sprite;
	}
}