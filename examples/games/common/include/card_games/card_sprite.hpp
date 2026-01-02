/*
neogfx C++ App/Game Engine - Examples - Games
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
            const ng::size kBridgeCardSize{ 57.15, 88.9 };

            template <typename CardValue>
            std::vector<std::pair<ng::rect, bool>> face_texture_rects(const ng::rect& aAabb, CardValue aValue)
            {
                std::vector<std::pair<ng::rect, bool>> result;
                auto faceRect = aAabb;
                faceRect.deflate(ng::size{ 0.1 });
                if (aValue >= CardValue::Jack)
                {
                    faceRect.deflate(ng::size{ 0.0, (faceRect.cy - faceRect.cx) / 2.0 });
                    result.push_back(std::make_pair(faceRect, false));
                }
                else
                {
                    faceRect.deflate(ng::size{ 0.1, 0.0 });
                    switch (aValue)
                    {
                    case CardValue::Two:
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.center().x - 0.1, faceRect.top() }, ng::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.center().x - 0.1, faceRect.bottom() - 0.2 }, ng::size{ 0.2 } }, true));
                        break;
                    case CardValue::Three:
                        result = face_texture_rects(aAabb, CardValue::Two);
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.center().x - 0.1, faceRect.center().y - 0.1 }, ng::size{ 0.2 } }, false));
                        break;
                    case CardValue::Four:
                        result.push_back(std::make_pair(ng::rect{ faceRect.top_left(), ng::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(ng::rect{ faceRect.top_right() + ng::delta{ -0.2, 0.0 }, ng::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(ng::rect{ faceRect.bottom_left() + ng::delta{ 0.0, -0.2 }, ng::size{ 0.2 } }, true));
                        result.push_back(std::make_pair(ng::rect{ faceRect.bottom_right() + ng::delta{ -0.2, -0.2 }, ng::size{ 0.2 } }, true));
                        break;
                    case CardValue::Five:
                        result = face_texture_rects(aAabb, CardValue::Four);
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.center().x - 0.1, faceRect.center().y - 0.1 }, ng::size{ 0.2 } }, false));
                        break;
                    case CardValue::Six:
                        result = face_texture_rects(aAabb, CardValue::Four);
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.left(), faceRect.center().y - 0.1 }, ng::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.right() - 0.2, faceRect.center().y - 0.1 }, ng::size{ 0.2 } }, false));
                        break;
                    case CardValue::Seven:
                        result = face_texture_rects(aAabb, CardValue::Six);
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.center().x - 0.1, faceRect.top() + faceRect.cy * 2.0 / 7.0 - 0.1 }, ng::size{ 0.2 } }, false));
                        break;
                    case CardValue::Eight:
                        result = face_texture_rects(aAabb, CardValue::Seven);
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.center().x - 0.1, faceRect.bottom() - faceRect.cy * 2.0 / 7.0 - 0.1 }, ng::size{ 0.2 } }, true));
                        break;
                    case CardValue::Nine:
                        result = face_texture_rects(aAabb, CardValue::Four);
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.left(), faceRect.top() + faceRect.cy * 2.5 / 7.0 - 0.1 }, ng::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.right() - 0.2, faceRect.top() + faceRect.cy * 2.5 / 7.0 - 0.1 }, ng::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.center().x - 0.1, faceRect.center().y - 0.1 }, ng::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.left(), faceRect.bottom() - faceRect.cy * 2.5 / 7.0 - 0.1 }, ng::size{ 0.2 } }, true));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.right() - 0.2, faceRect.bottom() - faceRect.cy * 2.5 / 7.0 - 0.1 }, ng::size{ 0.2 } }, true));
                        break;
                    case CardValue::Ten:
                        result = face_texture_rects(aAabb, CardValue::Four);
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.center().x - 0.1, faceRect.top() + faceRect.cy * 1.5 / 7.0 - 0.1 }, ng::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.left(), faceRect.top() + faceRect.cy * 2.5 / 7.0 - 0.1 }, ng::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.right() - 0.2, faceRect.top() + faceRect.cy * 2.5 / 7.0 - 0.1 }, ng::size{ 0.2 } }, false));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.left(), faceRect.bottom() - faceRect.cy * 2.5 / 7.0 - 0.1 }, ng::size{ 0.2 } }, true));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.right() - 0.2, faceRect.bottom() - faceRect.cy * 2.5 / 7.0 - 0.1 }, ng::size{ 0.2 } }, true));
                        result.push_back(std::make_pair(ng::rect{ ng::point{ faceRect.center().x - 0.1, faceRect.bottom() - faceRect.cy * 1.5 / 7.0 - 0.1 }, ng::size{ 0.2 } }, true));
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
        ng::game::entity_id create_card_sprite(ng::game::i_ecs& aEcs, const i_card_textures& aCardTextures, const basic_card<GameTraits>& aCard)
        {
            // We could have a separate texture for each of the 52 cards but instead we build the
            // card mesh up here using texture atlas textures.

            static const ng::game::sprite_2d_archetype cardArchetype{ "Card" };

            // Card background shape...
            auto cardBackgroundVertices = ng::rounded_rect_vertices<ng::vec3f>(ng::rect{ ng::point{}, ng::size{1.0, kBridgeCardSize.cy / kBridgeCardSize.cx } }.with_centered_origin(), 0.1, ng::mesh_type::Triangles, 20);

            ng::game::mesh mesh{ cardBackgroundVertices, ng::game::vertices_2d{cardBackgroundVertices.size(), ng::vec2f{} }, ng::game::default_faces(cardBackgroundVertices) };
            ng::game::mesh_renderer meshRenderer{ ng::game::material{ ng::to_ecs_component( ng::color::White) } };

            auto aabb = ng::game::bounding_rect(mesh);
            aabb.deflate(ng::size{ 0.025, 0.05 });

            // Card value textures...
            ng::add_patch(mesh, meshRenderer, ng::rect{ aabb.top_left(), ng::size{ 0.2 } }, aCardTextures.value_texture(aCard));
            meshRenderer.patches.back().material.color = ng::to_ecs_component(aCard == basic_card<GameTraits>::color::Black ? ng::color::Black : ng::color{ 213, 0, 0 });

            const ng::mat33 uvRotate180{ { -1.0, 0.0, 0.0 }, { 0.0, -1.0, 0.0 }, { 1.0, 1.0, 1.0 } };

            ng::add_patch(mesh, meshRenderer, ng::rect{ aabb.bottom_right() + ng::size{ -0.2 }, ng::size{ 0.2 } }, aCardTextures.value_texture(aCard), uvRotate180);
            meshRenderer.patches.back().material.color = ng::to_ecs_component(aCard == basic_card<GameTraits>::color::Black ? ng::color::Black : ng::color{ 213, 0, 0 });

            // Card suit textures under card value textures...
            ng::add_patch(mesh, meshRenderer, ng::rect{ aabb.top_left() + ng::delta{0.025, 0.4 - 0.15}, ng::size{ 0.15 } }, aCardTextures.suit_texture(aCard));
            ng::add_patch(mesh, meshRenderer, ng::rect{ aabb.bottom_right() + ng::size{ -0.2 } +ng::delta{ 0.025, -0.2 }, ng::size{ 0.15 } }, aCardTextures.suit_texture(aCard), uvRotate180);

            auto faceTextureRects = face_texture_rects(aabb, static_cast<typename basic_card<GameTraits>::value>(aCard));
            for (auto const& r : faceTextureRects)
                ng::add_patch(mesh, meshRenderer, r.first, aCardTextures.face_texture(aCard), r.second ? uvRotate180 : ng::mat33::identity());

            return aEcs.create_entity(cardArchetype, ng::game::box_collider_2d{ {}, { cardBackgroundVertices.begin(), cardBackgroundVertices.end() } }, ng::game::mesh_filter{{}, mesh, {}}, meshRenderer);
        }
    }
}