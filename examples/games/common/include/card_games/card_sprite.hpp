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
				// We could have a separate texture for each of the 52 cards but instead we build the
				// card mesh up here using texture atlas textures.

				auto vlp = std::make_shared<neogfx::vertex_list>();
				neogfx::face_list faceList;

				// Card background shape...
				auto cardBackgroundVertices = neogfx::rounded_rect_vertices(neogfx::rect{ neogfx::point{}, neogfx::size{1.0, kBridgeCardSize.cy / kBridgeCardSize.cx } }.with_centred_origin(), 0.1, true, 20);
				vlp->reserve(cardBackgroundVertices.end() - cardBackgroundVertices.begin() + 28 * 3);
				neogfx::add_faces(vlp, faceList, cardBackgroundVertices);
				iCardBackgroundFaces = faceList.faces().size();

				auto tlp = std::make_shared<neogfx::texture_list>();

				auto aabb = neogfx::bounding_rect(*vlp);
				aabb.deflate(neogfx::size{ 0.025, 0.05 });

				// Card value textures...
				neogfx::add_faces(vlp, tlp, faceList, neogfx::rect{ aabb.top_left(), neogfx::size{ 0.2 } }, iCardTextures.value_texture(iCard));
				neogfx::add_faces(vlp, tlp, faceList, neogfx::rect{ aabb.bottom_right() + neogfx::size{ -0.2 }, neogfx::size{ 0.2 } }, iCardTextures.value_texture(iCard), true);
				iCardValueFaces = faceList.faces().size();
				
				// Card suit textures under card value textures...
				neogfx::add_faces(vlp, tlp, faceList, neogfx::rect{ aabb.top_left() + neogfx::delta{0.025, 0.4 - 0.15}, neogfx::size{ 0.15 } }, iCardTextures.suit_texture(iCard));
				neogfx::add_faces(vlp, tlp, faceList, neogfx::rect{ aabb.bottom_right() + neogfx::size{ -0.2 } + neogfx::delta{ 0.025, -0.2 }, neogfx::size{ 0.15 } }, iCardTextures.suit_texture(iCard), true);

				auto faceTextureRects = face_texture_rects(aabb, iCard);
				for (const auto& r : faceTextureRects)
					neogfx::add_faces(vlp, tlp, faceList, r.first, iCardTextures.face_texture(iCard), r.second);

				// Finish up...
				set_vertices(vlp);
				set_textures(tlp);
				set_faces(faceList);
			}
			// geometry
		protected:
			neogfx::mat44 transformation_matrix() const override
			{
				return neogfx::mat44{ { extents()[0], 0.0, 0.0, 0.0 },{ 0.0, extents()[1] * kBridgeCardSize.cx / kBridgeCardSize.cy, 0.0, 0.0 },{ 0.0, 0.0, 1.0, 0.0 },{ position().x, position().y, position().z, 1.0 } };
			}
			// updates
		protected:
			bool update(const optional_time_interval&, const neogfx::vec3&)
			{
				return true;
			}
			// rendering
		protected:
			void paint(neogfx::graphics_context& aGraphicsContext) const override
			{
				{
					neogfx::scoped_faces sf{ *this, faces().begin(), faces().begin() + iCardBackgroundFaces };
					aGraphicsContext.fill_shape(*this, neogfx::to_brush(*current_frame().colour()));
				}
				{
					neogfx::scoped_faces sf{ *this, faces().begin() + iCardBackgroundFaces, faces().begin() + iCardValueFaces };
					aGraphicsContext.draw_textures(*this, textures(), iCard == card_type::colour::Black ? neogfx::colour::Black : neogfx::colour{ 213, 0, 0 });
				}
				{
					neogfx::scoped_faces sf{ *this, faces().begin() + iCardValueFaces, faces().end() };
					aGraphicsContext.draw_textures(*this, textures());
				}
			}
		private:
			std::vector<std::pair<neogfx::rect, bool>> face_texture_rects(const neogfx::rect& aAabb, typename card_type::value aValue)
			{
				std::vector<std::pair<neogfx::rect, bool>> result;
				auto faceRect = aAabb;
				faceRect.deflate(neogfx::size{ 0.1 });
				if (aValue >= card_type::value::Jack)
				{
					faceRect.deflate(neogfx::size{ 0.0, (faceRect.cy - faceRect.cx) / 2.0 });
					result.push_back(std::make_pair(faceRect, false));
				}
				else
				{
					faceRect.deflate(neogfx::size{ 0.1, 0.0 });
					switch (aValue)
					{
					case card_type::value::Two:
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.top() }, neogfx::size{ 0.2 } }, false));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.bottom() - 0.2 }, neogfx::size{ 0.2 } }, true));
						break;
					case card_type::value::Three:
						result = face_texture_rects(aAabb, card_type::value::Two);
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.centre().y - 0.1 }, neogfx::size{ 0.2 } }, false));
						break;
					case card_type::value::Four:
						result.push_back(std::make_pair(neogfx::rect{ faceRect.top_left(), neogfx::size{ 0.2 } }, false));
						result.push_back(std::make_pair(neogfx::rect{ faceRect.top_right() + neogfx::delta{ -0.2, 0.0 }, neogfx::size{ 0.2 } }, false));
						result.push_back(std::make_pair(neogfx::rect{ faceRect.bottom_left() + neogfx::delta{ 0.0, -0.2 }, neogfx::size{ 0.2 } }, true));
						result.push_back(std::make_pair(neogfx::rect{ faceRect.bottom_right() + neogfx::delta{ -0.2, -0.2 }, neogfx::size{ 0.2 } }, true));
						break;
					case card_type::value::Five:
						result = face_texture_rects(aAabb, card_type::value::Four);
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.centre().y - 0.1 }, neogfx::size{ 0.2 } }, false));
						break;
					case card_type::value::Six:
						result = face_texture_rects(aAabb, card_type::value::Four);
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.left(), faceRect.centre().y - 0.1 }, neogfx::size{ 0.2 } }, false));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.right() - 0.2, faceRect.centre().y - 0.1 }, neogfx::size{ 0.2 } }, false));
						break;
					case card_type::value::Seven:
						result = face_texture_rects(aAabb, card_type::value::Six);
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.top() + faceRect.cy * 2.0 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, false));
						break;
					case card_type::value::Eight:
						result = face_texture_rects(aAabb, card_type::value::Seven);
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.bottom() - faceRect.cy * 2.0 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, true));
						break;
					case card_type::value::Nine:
						result = face_texture_rects(aAabb, card_type::value::Four);
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.left(), faceRect.top() + faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, false));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.right() - 0.2, faceRect.top() + faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, false));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.centre().y - 0.1 }, neogfx::size{ 0.2 } }, false));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.left(), faceRect.bottom() - faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, true));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.right() - 0.2, faceRect.bottom() - faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, true));
						break;
					case card_type::value::Ten:
						result = face_texture_rects(aAabb, card_type::value::Four);
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.top() + faceRect.cy * 1.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, false));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.left(), faceRect.top() + faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, false));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.right() - 0.2, faceRect.top() + faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, false));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.left(), faceRect.bottom() - faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, true));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.right() - 0.2, faceRect.bottom() - faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, true));
						result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.bottom() - faceRect.cy * 1.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, true));
						break;
					default:
						// do nothing
						break;
					}
				}
				return result;
			}
			// attributes
		private:
			const i_card_textures& iCardTextures;
			card_type iCard;
			std::size_t iCardBackgroundFaces;
			std::size_t iCardValueFaces;
		};

		typedef basic_card_sprite<default_game_traits> card_sprite;
	}
}