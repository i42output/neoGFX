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

#include <neogfx/game/i_ecs.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/standard_archetypes.hpp>
#include <neogfx/game/ecs_helpers.hpp>
#include <neogfx/gfx/shapes.hpp>
#include <card_games/card.hpp>
#include <card_games/i_card_textures.hpp>

namespace neogames
{
    namespace card_games
    {
        namespace
        {
            const neogfx::size kBridgeCardSize{ 57.15, 88.9 };

            template <typename CardValue>
            std::vector<std::pair<neogfx::rect, bool>> face_texture_rects(const neogfx::rect& aAabb, CardValue aValue)
            {
                std::vector<std::pair<neogfx::rect, bool>> result;
                auto faceRect = aAabb;
                faceRect.deflate(neogfx::size{ 0.1 });
                if (aValue >= CardValue::Jack)
                {
                    faceRect.deflate(neogfx::size{ 0.0, (faceRect.cy - faceRect.cx) / 2.0 });
                    result.push_back(std::make_pair(faceRect, false));
                }
                else
                {
                    faceRect.deflate(neogfx::size{ 0.1, 0.0 });
                    switch (aValue)
                    {
                    case CardValue::Two:
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.top() }, neogfx::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.bottom() - 0.2 }, neogfx::size{ 0.2 } }, true));
                        break;
                    case CardValue::Three:
                        result = face_texture_rects(aAabb, CardValue::Two);
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.centre().y - 0.1 }, neogfx::size{ 0.2 } }, false));
                        break;
                    case CardValue::Four:
                        result.push_back(std::make_pair(neogfx::rect{ faceRect.top_left(), neogfx::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(neogfx::rect{ faceRect.top_right() + neogfx::delta{ -0.2, 0.0 }, neogfx::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(neogfx::rect{ faceRect.bottom_left() + neogfx::delta{ 0.0, -0.2 }, neogfx::size{ 0.2 } }, true));
                        result.push_back(std::make_pair(neogfx::rect{ faceRect.bottom_right() + neogfx::delta{ -0.2, -0.2 }, neogfx::size{ 0.2 } }, true));
                        break;
                    case CardValue::Five:
                        result = face_texture_rects(aAabb, CardValue::Four);
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.centre().y - 0.1 }, neogfx::size{ 0.2 } }, false));
                        break;
                    case CardValue::Six:
                        result = face_texture_rects(aAabb, CardValue::Four);
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.left(), faceRect.centre().y - 0.1 }, neogfx::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.right() - 0.2, faceRect.centre().y - 0.1 }, neogfx::size{ 0.2 } }, false));
                        break;
                    case CardValue::Seven:
                        result = face_texture_rects(aAabb, CardValue::Six);
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.top() + faceRect.cy * 2.0 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, false));
                        break;
                    case CardValue::Eight:
                        result = face_texture_rects(aAabb, CardValue::Seven);
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.bottom() - faceRect.cy * 2.0 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, true));
                        break;
                    case CardValue::Nine:
                        result = face_texture_rects(aAabb, CardValue::Four);
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.left(), faceRect.top() + faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.right() - 0.2, faceRect.top() + faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.centre().x - 0.1, faceRect.centre().y - 0.1 }, neogfx::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.left(), faceRect.bottom() - faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, true));
                        result.push_back(std::make_pair(neogfx::rect{ neogfx::point{ faceRect.right() - 0.2, faceRect.bottom() - faceRect.cy * 2.5 / 7.0 - 0.1 }, neogfx::size{ 0.2 } }, true));
                        break;
                    case CardValue::Ten:
                        result = face_texture_rects(aAabb, CardValue::Four);
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
        }

        template <typename GameTraits>
        neogfx::game::entity_id create_card_sprite(neogfx::game::i_ecs& aEcs, const i_card_textures& aCardTextures, const basic_card<GameTraits>& aCard)
        {
            // We could have a separate texture for each of the 52 cards but instead we build the
            // card mesh up here using texture atlas textures.

            static const neogfx::game::sprite_archetype cardArchetype{ "Card" };

            // Card background shape...
            auto cardBackgroundVertices = neogfx::rounded_rect_vertices(neogfx::rect{ neogfx::point{}, neogfx::size{1.0, kBridgeCardSize.cy / kBridgeCardSize.cx } }.with_centred_origin(), 0.1, neogfx::mesh_type::Triangles, 20);

            neogfx::game::mesh mesh{ cardBackgroundVertices, neogfx::vertices_2d{cardBackgroundVertices.size(), neogfx::vec2{} }, neogfx::game::default_faces(cardBackgroundVertices) };
            neogfx::game::mesh_renderer meshRenderer{ neogfx::game::material{ neogfx::to_ecs_component( neogfx::color::White) } };

            auto aabb = neogfx::game::bounding_rect(mesh);
            aabb.deflate(neogfx::size{ 0.025, 0.05 });

            // Card value textures...
            neogfx::add_patch(mesh, meshRenderer, neogfx::rect{ aabb.top_left(), neogfx::size{ 0.2 } }, 0.0, aCardTextures.value_texture(aCard));
            meshRenderer.patches.back().material.color = neogfx::to_ecs_component(aCard == basic_card<GameTraits>::color::Black ? neogfx::color::Black : neogfx::color{ 213, 0, 0 });

            const neogfx::mat33 uvRotate180{ { -1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }, { 1.0, 1.0, 1.0 } };

            neogfx::add_patch(mesh, meshRenderer, neogfx::rect{ aabb.bottom_right() + neogfx::size{ -0.2 }, neogfx::size{ 0.2 } }, 0.0, aCardTextures.value_texture(aCard), uvRotate180);
            meshRenderer.patches.back().material.color = neogfx::to_ecs_component(aCard == basic_card<GameTraits>::color::Black ? neogfx::color::Black : neogfx::color{ 213, 0, 0 });

            // Card suit textures under card value textures...
            neogfx::add_patch(mesh, meshRenderer, neogfx::rect{ aabb.top_left() + neogfx::delta{0.025, 0.4 - 0.15}, neogfx::size{ 0.15 } }, 0.0, aCardTextures.suit_texture(aCard));
            neogfx::add_patch(mesh, meshRenderer, neogfx::rect{ aabb.bottom_right() + neogfx::size{ -0.2 } +neogfx::delta{ 0.025, -0.2 }, neogfx::size{ 0.15 } }, 0.0, aCardTextures.suit_texture(aCard), uvRotate180);

            auto faceTextureRects = face_texture_rects(aabb, static_cast<typename basic_card<GameTraits>::value>(aCard));
            for (auto const& r : faceTextureRects)
                neogfx::add_patch(mesh, meshRenderer, r.first, 0.0, aCardTextures.face_texture(aCard), r.second ? uvRotate180 : neogfx::mat33::identity());

            return aEcs.create_entity(cardArchetype, neogfx::game::mesh_filter{ {}, mesh, {} }, meshRenderer);
        }
    }
}