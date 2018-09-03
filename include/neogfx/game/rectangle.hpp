// rectangle.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/i_image.hpp>
#include <neogfx/game/entity.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/material.hpp>
#include <neogfx/game/shape_factory.hpp>

namespace neogfx::game
{
	struct rectangle_parameters
	{
		vec3 position;
		vec2 extents;
		settings_updater updater = 
			[this](i_ecs& aEcs, entity_id aEntity)
			{
				auto& m = aEcs.component<mesh>().have_entity_record(aEntity) ?
					aEcs.component<mesh>().entity_record(aEntity) : 
					aEcs.component<mesh>().populate(aEntity, mesh
					{
						{},
						{ 
							vec2{ 0.0, 0.0 }, vec2{ 1.0, 0.0 }, vec2{ 0.0, 1.0 }, 
							vec2{ 1.0, 0.0 }, vec2{ 1.0, 1.0 }, vec2{ 0.0, 1.0 } 
						},
						{	
							face{ 0u, 1u, 2u }, 
							face{ 3u, 4u, 5u } 
						}
					});
				// todo: outline support
				m.vertices = rect_vertices(rect{ point{ position }, size{ extents } }, 0, rect_type::FilledTriangles);
				aEcs.component<mesh>().populate(aEntity, m);
			};

		struct meta : i_component_data::meta
		{
			static const neolib::uuid& id()
			{
				static const neolib::uuid sId = { 0x6f45d8be, 0xba9c, 0x4a32, 0xa99e, { 0x37, 0xd3, 0xf2, 0xb4, 0xe7, 0x53 } };
				return sId;
			}
			static const neolib::i_string& name()
			{
				static const neolib::string sName = "Rectangle";
				return sName;
			}
			static uint32_t field_count()
			{
				return 3;
			}
			static component_data_field_type field_type(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
					return component_data_field_type::Vec3;
				case 1:
					return component_data_field_type::Vec2;
				case 2:
					return component_data_field_type::Updater;
				default:
					throw invalid_field_index();
				}
			}
			static const neolib::i_string& field_name(uint32_t aFieldIndex)
			{
				static const neolib::string sFieldNames[] =
				{
					"Position",
					"Extents",
					"Updater"
				};
				return sFieldNames[aFieldIndex];
			}
		};
	};

	class rectangle : public entity
	{
	public:
		static const entity_archetype& archetype()
		{
			static const entity_archetype sArchetype
			{
				{ 0xce3d930, 0x6b18, 0x403b, 0x9680, { 0x89, 0xed, 0x54, 0x83, 0xd5, 0x72 } },
				"Rectangle",
				{ rectangle_parameters::meta::id(), mesh::meta::id() }
			};
			return sArchetype;
		}
	public:
		rectangle(i_ecs& aEcs, const vec3& aPosition, const vec2& aExtents);
		rectangle(i_ecs& aEcs, const vec3& aPosition, const vec2& aExtents, const neogfx::colour& aColour);
		rectangle(i_ecs& aEcs, const vec3& aPosition, const vec2& aExtents, const i_texture& aTexture);
		rectangle(i_ecs& aEcs, const vec3& aPosition, const vec2& aExtents, const i_image& aImage);
		rectangle(i_ecs& aEcs, const vec3& aPosition, const vec2& aExtents, const i_texture& aTexture, const rect& aTextureRect);
		rectangle(i_ecs& aEcs, const vec3& aPosition, const vec2& aExtents, const i_image& aImage, const rect& aTextureRect);
		rectangle(const rectangle& aOther);
	};
}