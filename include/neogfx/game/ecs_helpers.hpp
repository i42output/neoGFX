// ecs_helpers.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/color.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/shapes.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/color.hpp>
#include <neogfx/game/gradient.hpp>
#include <neogfx/game/material.hpp>
#include <neogfx/game/rigid_body.hpp>

namespace neogfx
{
    inline mat44 to_transformation_matrix(const game::rigid_body& aRigidBody, bool aIncludeTranslation = true)
    {
        scalar az = aRigidBody.angle.z;
        // todo: following rotation is 2D, make it 3D...
        return aIncludeTranslation ? 
            mat44
            {
                { std::cos(az), std::sin(az), 0.0, 0.0 },
                { -std::sin(az), std::cos(az), 0.0, 0.0 },
                { 0.0, 0.0, 1.0, 0.0 },
                { aRigidBody.position.x, aRigidBody.position.y, aRigidBody.position.z, 1.0 } 
            } : 
            mat44
            {
                { std::cos(az), std::sin(az), 0.0, 0.0 },
                { -std::sin(az), std::cos(az), 0.0, 0.0 },
                { 0.0, 0.0, 1.0, 0.0 },
                { 0.0, 0.0, 0.0, 1.0 }
            };
    }

    template <typename CoordinateType, logical_coordinate_system CoordinateSystem>
    inline game::mesh to_ecs_component(const basic_rect<CoordinateType, CoordinateSystem>& aRect, mesh_type aMeshType = mesh_type::Triangles, scalar aZpos = 0.0, uint32_t aOffset = 0)
    {
        auto const rectVertices = rect_vertices(aRect, aMeshType, aZpos);
        return game::mesh
        {
            {
                rectVertices.begin(), rectVertices.end()
            },
            {
                vec2{ 0.0, 1.0 }, vec2{ 1.0, 1.0 }, vec2{ 0.0, 0.0 },
                vec2{ 1.0, 1.0 }, vec2{ 1.0, 0.0 }, vec2{ 0.0, 0.0 }
            },
            {
                game::face{ aOffset + 0u, aOffset + 1u, aOffset + 2u },
                game::face{ aOffset + 3u, aOffset + 4u, aOffset + 5u }    
            }
        };
    }

    inline game::mesh to_ecs_component(const vertices& aVertices, mesh_type aSourceMeshType = mesh_type::TriangleFan, mesh_type aDestinationMeshType = mesh_type::Triangles)
    {
        // todo
        return game::mesh{};
    }

    inline game::color to_ecs_component(const color& aColor)
    {
        return game::color{ aColor.to_vec4() };
    }

    inline game::gradient to_ecs_component(const gradient& aGradient)
    {
        game::gradient result;
        result.colorStops.reserve(aGradient.color_stop_count());
        result.colorStopPositions.reserve(aGradient.color_stop_count());
        result.alphaStops.reserve(aGradient.alpha_stop_count());
        result.alphaStopPositions.reserve(aGradient.alpha_stop_count());
        for (auto cs = aGradient.color_begin(); cs != aGradient.color_end(); ++cs)
        {
            result.colorStops.push_back(to_ecs_component(cs->second));
            result.colorStopPositions.push_back(cs->first);
        }
        for (auto as = aGradient.alpha_begin(); as != aGradient.alpha_end(); ++as)
        {
            result.alphaStops.push_back(as->second / 255.0);
            result.alphaStopPositions.push_back(as->first);
        }
        return result;
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

    inline game::material to_ecs_component(const brush& aBrush)
    {
        if (std::holds_alternative<color>(aBrush))
            return game::material{ to_ecs_component(std::get<color>(aBrush)) };
        else if (std::holds_alternative<gradient>(aBrush))
            return game::material{ {}, to_ecs_component(std::get<gradient>(aBrush)) };
        else
            return game::material{ game::color{} };
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

    inline void add_patch(game::mesh& aMesh, game::mesh_renderer& aMeshRenderer, const rect& aRect, scalar aZpos, const neogfx::i_texture& aTexture, const mat33& aTextureTransform = mat33::identity())
    {
        auto patchMesh = to_ecs_component(aRect, mesh_type::Triangles, aZpos, static_cast<uint32_t>(aMesh.vertices.size())); 
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