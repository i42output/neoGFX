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
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/game/shape_factory.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/colour.hpp>
#include <neogfx/game/gradient.hpp>
#include <neogfx/game/material.hpp>
#include <neogfx/game/rigid_body.hpp>

namespace neogfx
{
	inline mat44 to_transformation_matrix(const game::rigid_body& aRigidBody)
	{
		scalar az = aRigidBody.angle.z;
		// todo: following rotation is 2D, make it 3D...
		return mat44{ { std::cos(az), std::sin(az), 0.0, 0.0 }, { -std::sin(az), std::cos(az), 0.0, 0.0 }, { 0.0, 0.0, 1.0, 0.0 }, { aRigidBody.position.x, aRigidBody.position.y, aRigidBody.position.z, 1.0 } };
	}

	inline game::mesh to_ecs_component(const rect& aRect, dimension aPixelAdjust = 0, rect_type aType = rect_type::FilledTriangles)
	{
		return game::mesh
		{
			{
				rect_vertices(aRect, aPixelAdjust, aType)
			},
			{
				vec2{ 0.0, 0.0 }, vec2{ 1.0, 0.0 }, vec2{ 0.0, 1.0 },
				vec2{ 1.0, 0.0 }, vec2{ 1.0, 1.0 }, vec2{ 0.0, 1.0 }
			},
			{
				game::face{ 0u, 1u, 2u },
				game::face{ 3u, 4u, 5u }
			}
		};
	}

	inline game::colour to_ecs_component(const colour& aColour)
	{
		return game::colour{ aColour.to_vec4() };
	}

	inline game::gradient to_ecs_component(const gradient& aGradient)
	{
		game::gradient result;
		result.colourStops.reserve(aGradient.colour_stop_count());
		result.colourStopPositions.reserve(aGradient.colour_stop_count());
		result.alphaStops.reserve(aGradient.alpha_stop_count());
		result.alphaStopPositions.reserve(aGradient.alpha_stop_count());
		for (auto cs = aGradient.colour_begin(); cs != aGradient.colour_end(); ++cs)
		{
			result.colourStops.push_back(to_ecs_component(cs->second));
			result.colourStopPositions.push_back(cs->first);
		}
		for (auto as = aGradient.alpha_begin(); as != aGradient.alpha_end(); ++as)
		{
			result.alphaStops.push_back(as->second / 255.0);
			result.alphaStopPositions.push_back(as->first);
		}
		return result;
	}

	inline game::material to_ecs_component(const brush& aBrush)
	{
		if (std::holds_alternative<colour>(aBrush))
			return game::material{ to_ecs_component(std::get<colour>(aBrush)) };
		else if (std::holds_alternative<gradient>(aBrush))
			return game::material{ {}, to_ecs_component(std::get<gradient>(aBrush)) };
		else
			return game::material{ game::colour{} };
	}

	inline game::texture to_ecs_component(const i_texture& aTexture)
	{
		return game::texture
		{
			neolib::cookie_auto_ref{ service<i_texture_manager>::instance(), aTexture.id() },
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
			neolib::cookie_auto_ref{ service<i_texture_manager>::instance(), aTexture.id() },
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
			neolib::cookie_auto_ref{ service<i_texture_manager>::instance(), newTexture.id() },
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
			neolib::cookie_auto_ref{ service<i_texture_manager>::instance(), newTexture.id() },
			newTexture.type(),
			newTexture.sampling(),
			newTexture.dpi_scale_factor(),
			newTexture.extents().to_vec2(),
			aTextureRect.to_aabb_2d()
		};
	}

}