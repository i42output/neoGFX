// ecs_helpers.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/i_gradient_manager.hpp>
#include <neogfx/gfx/shapes.hpp>
#include <neogfx/game/i_ecs.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/color.hpp>
#include <neogfx/game/gradient.hpp>
#include <neogfx/game/material.hpp>
#include <neogfx/game/filter.hpp>
#include <neogfx/game/animation_filter.hpp>
#include <neogfx/game/rigid_body.hpp>

namespace neogfx
{
    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline game::mesh const& to_ecs_component(game::mesh& aResult, const basic_rect<CoordinateType, CoordinateSystem>& aRect, mesh_type aMeshType = mesh_type::Triangles, optional_mat44 const& aTransformation = {}, uint32_t aOffset = 0)
    {
        auto const& rectVertices = rect_vertices(aRect, aMeshType, aTransformation);
        aResult.vertices.assign(rectVertices.begin(), rectVertices.end());
        aResult.uv = {
                vec2{ 0.0, 1.0 }, vec2{ 1.0, 1.0 }, vec2{ 0.0, 0.0 },
                vec2{ 1.0, 1.0 }, vec2{ 1.0, 0.0 }, vec2{ 0.0, 0.0 } };
        aResult.faces = {
                game::face{ aOffset + 0u, aOffset + 1u, aOffset + 2u },
                game::face{ aOffset + 3u, aOffset + 4u, aOffset + 5u } };
        return aResult;
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline game::mesh const& to_ecs_component(const basic_rect<CoordinateType, CoordinateSystem>& aRect, mesh_type aMeshType = mesh_type::Triangles, optional_mat44 const& aTransformation = {}, uint32_t aOffset = 0)
    {
        thread_local game::mesh result;
        return to_ecs_component(result, aRect, aMeshType, aTransformation, aOffset);
    }

    inline game::mesh to_ecs_component(const vertices& aVertices, mesh_type aSourceMeshType = mesh_type::TriangleFan, mesh_type aDestinationMeshType = mesh_type::Triangles)
    {
        // todo
        return game::mesh{};
    }

    inline game::color to_ecs_component(const color& aColor)
    {
        return game::color{ aColor };
    }

    inline game::gradient to_ecs_component(const gradient& aGradient)
    {
        return game::gradient
        {
            neolib::cookie_ref_ptr{ service<i_gradient_manager>(), aGradient.id() }, 
            aGradient.bounding_box() ? aGradient.bounding_box()->to_aabb_2d() : std::optional<aabb_2d>{}
        };
    }

    inline game::material to_ecs_component(const text_color& aTextColor)
    {
        if (std::holds_alternative<color>(aTextColor))
            return game::material{ to_ecs_component(std::get<color>(aTextColor)) };
        else if (std::holds_alternative<gradient>(aTextColor))
            return game::material{ {}, to_ecs_component(std::get<gradient>(aTextColor)) };
        else
            return game::material{ game::color{} };
    }

    inline shader_effect to_ecs_component(text_effect_type aEffectType)
    {
        switch (aEffectType)
        {
        case text_effect_type::None:
            return shader_effect::None;
        case text_effect_type::Outline:
            return shader_effect::None;
        case text_effect_type::Glow:
            return shader_effect::ColorizeSpot;
        case text_effect_type::Shadow:
            return shader_effect::ColorizeSpot;
        default:
            return shader_effect::None;
        }
    }

    inline game::material to_ecs_component(const brush& aBrush)
    {
        if (std::holds_alternative<color>(aBrush))
            return game::material{ to_ecs_component(std::get<color>(aBrush)) };
        else if (std::holds_alternative<gradient>(aBrush))
            return game::material{ {}, to_ecs_component(std::get<gradient>(aBrush)) };
        else
            return game::material{ game::color{} };
    }

    inline std::optional<game::filter> to_ecs_component(blurring_algorithm aAlgorithm, scalar aParameter1, scalar aParameter2)
    {
        switch (aAlgorithm)
        {
        case blurring_algorithm::None:
            return {};
        case blurring_algorithm::Gaussian:
            return game::filter{ shader_filter::GaussianBlur, aParameter1, aParameter2 };
        default:
            return {};
        }
    }

    inline game::texture to_ecs_component(const i_texture& aTexture)
    {
        return game::texture
        {
            neolib::cookie_ref_ptr{ service<i_texture_manager>(), aTexture.id() },
            aTexture.type(),
            aTexture.sampling(),
            aTexture.dpi_scale_factor(),
            aTexture.extents().to_vec2(),
            aTexture.type() == texture_type::Texture ? optional_aabb_2d{} : aTexture.as_sub_texture().atlas_location().to_aabb_2d()
        };
    }

    inline game::texture to_ecs_component(const i_texture& aTexture, const rect& aTextureRect)
    {
        return game::texture
        {
            neolib::cookie_ref_ptr{ service<i_texture_manager>(), aTexture.id() },
            aTexture.type(),
            aTexture.sampling(),
            aTexture.dpi_scale_factor(),
            aTexture.extents().to_vec2(),
            aTextureRect.to_aabb_2d()
        };
    }

    inline game::texture to_ecs_component(const i_image& aImage)
    {
        texture newTexture{ aImage };
        return game::texture
        {
            neolib::cookie_ref_ptr{ service<i_texture_manager>(), newTexture.id() },
            newTexture.type(),
            newTexture.sampling(),
            newTexture.dpi_scale_factor(),
            newTexture.extents().to_vec2(),
            {}
        };
    }

    inline game::texture to_ecs_component(const i_image& aImage, const rect& aTextureRect)
    {
        texture newTexture{ aImage };
        return game::texture
        {
            neolib::cookie_ref_ptr{ service<i_texture_manager>(), newTexture.id() },
            newTexture.type(),
            newTexture.sampling(),
            newTexture.dpi_scale_factor(),
            newTexture.extents().to_vec2(),
            aTextureRect.to_aabb_2d()
        };
    }

    inline game::material image_to_material(game::i_ecs& aEcs, std::string const& aName, const neogfx::image& aImage)
    {
        return game::material{ {}, {}, aEcs.shared_component<game::texture>().populate(aName, to_ecs_component(aImage)) };
    }

    inline game::material image_to_material(game::i_ecs& aEcs, std::string const& aName, std::string const& aImageResource)
    {
        return image_to_material(aEcs, aName, neogfx::image{ aImageResource });
    }

    inline game::animation regular_sprite_sheet_to_animation(const vec2u32& aSpriteSheetExtents, const vec2u32& aCells, const vec2u32& aCellIndexTopLeft, const vec2u32& aCellIndexBottomRight, scalar aDefaultDuration = 0.0)
    {
        vec2 const uvCellExtents{ 1.0 / aCells.as<scalar>().x, 1.0 / aCells.as<scalar>().y };
        game::animation results;
        for (u32 y = aCellIndexTopLeft.y; y <= aCellIndexBottomRight.y; ++y)
        {
            for (u32 x = aCellIndexTopLeft.x; x <= aCellIndexBottomRight.x; ++x)
            {
                vec2 const uvOffset{ x * uvCellExtents.x, 1.0 - (y + 1) * uvCellExtents.y };
                results.frames.push_back(
                    game::animation_frame
                    {
                        aDefaultDuration,
                        game::mesh_filter
                        {
                            {},
                            game::mesh
                            {
                                { vec3{ -1.0, -1.0, 0.0 }, vec3{ 1.0, -1.0, 0.0 }, vec3{ 1.0, 1.0, 0.0 }, vec3{ -1.0, 1.0, 0.0 } },
                                { uvOffset, uvOffset + vec2{ uvCellExtents.x, 0.0 }, uvOffset + vec2{ uvCellExtents.x, uvCellExtents.y }, uvOffset + vec2{ 0.0, uvCellExtents.y } },
                                { game::face{ 3u, 2u, 0u }, game::face{ 2u, 1u, 0u } }
                            }
                        }
                    });
            }
        }
        return results;
    }

    inline game::animation regular_sprite_sheet_to_animation(const vec2u32& aSpriteSheetExtents, const vec2u32& aCells, scalar aDefaultFrameDuration = 0.0)
    {
        return regular_sprite_sheet_to_animation(aSpriteSheetExtents, aCells, vec2u32{}, vec2u32{ aCells.x - 1u, aCells.y - 1u }, aDefaultFrameDuration);
    }

    inline game::animation regular_sprite_sheet_to_animation(const game::material& aSpriteSheet, const vec2u32& aCells, const vec2u32& aCellIndexTopLeft, const vec2u32& aCellIndexBottomRight, scalar aDefaultFrameDuration = 0.0)
    {
        return regular_sprite_sheet_to_animation(
            aSpriteSheet.sharedTexture ? aSpriteSheet.sharedTexture->ptr->extents.as<u32>() : aSpriteSheet.texture->extents.as<u32>(),
            aCells, aCellIndexTopLeft, aCellIndexBottomRight, aDefaultFrameDuration);
    }

    inline game::animation regular_sprite_sheet_to_animation(const game::material& aSpriteSheet, const vec2u32& aCells, scalar aDefaultFrameDuration = 0.0)
    {
        return regular_sprite_sheet_to_animation(
            aSpriteSheet.sharedTexture ? aSpriteSheet.sharedTexture->ptr->extents.as<u32>() : aSpriteSheet.texture->extents.as<u32>(),
            aCells, aDefaultFrameDuration);
    }

    struct renderable_animation
    {
        game::material material;
        game::animation_filter filter;
    };
    
    inline renderable_animation regular_sprite_sheet_to_renderable_animation(game::i_ecs& aEcs, std::string const& aName, const neogfx::image& aSpriteSheet, const vec2u32& aCells, scalar aDefaultFrameDuration = 0.0)
    {
        renderable_animation result;
        result.material = image_to_material(aEcs, aName, aSpriteSheet);
        result.filter = game::animation_filter{ aEcs.shared_component<game::animation>().populate(aName, regular_sprite_sheet_to_animation(result.material, aCells, aDefaultFrameDuration)) };
        return result;
    }

    inline renderable_animation regular_sprite_sheet_to_renderable_animation(game::i_ecs& aEcs, std::string const& aName, std::string const& aSpriteSheetResource, const vec2u32& aCells, scalar aDefaultFrameDuration = 0.0)
    {
        return regular_sprite_sheet_to_renderable_animation(aEcs, aName, neogfx::image{ aSpriteSheetResource }, aCells, aDefaultFrameDuration);
    }

    inline void add_patch(game::mesh& aMesh, game::mesh_renderer& aMeshRenderer, const rect& aRect, const neogfx::i_texture& aTexture, const mat33& aTextureTransform = mat33::identity())
    {
        thread_local game::mesh patchMesh;
        to_ecs_component(patchMesh, aRect, mesh_type::Triangles, std::nullopt, static_cast<uint32_t>(aMesh.vertices.size()));
        aMesh.vertices.insert(aMesh.vertices.end(), patchMesh.vertices.begin(), patchMesh.vertices.end());
        if (!aTextureTransform.is_identity())
            for (auto& uv : patchMesh.uv)
                uv = (aTextureTransform * vec3{ uv.x, uv.y, 1.0 }).xy;
        aMesh.uv.insert(aMesh.uv.end(), patchMesh.uv.begin(), patchMesh.uv.end());
        auto& patch = aMeshRenderer.patches.emplace_back();
        patch.faces.insert(patch.faces.end(), patchMesh.faces.begin(), patchMesh.faces.end());
        patch.material.texture = to_ecs_component(aTexture);
    }
}