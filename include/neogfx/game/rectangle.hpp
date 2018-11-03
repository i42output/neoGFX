// rectangle.hpp
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
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/i_image.hpp>
#include <neogfx/game/ecs_helpers.hpp>
#include <neogfx/game/renderable_entity_archetype.hpp>
#include <neogfx/game/entity.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/core/shapes.hpp>

namespace neogfx::game
{
	struct rectangle
	{
		vec3 position;
		vec2 extents;
		material material;

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
					return component_data_field_type::ComponentData | component_data_field_type::Optional;
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
					"Material"
				};
				return sFieldNames[aFieldIndex];
			}
			static constexpr bool has_updater = true;
			static void update(const rectangle& aData, i_ecs& aEcs, entity_id aEntity)
			{
				using neogfx::game::material;
				aEcs.component<mesh>().populate(aEntity, to_ecs_component(rect{ point{~aData.position.xy - aData.extents / 2.0 }, size{aData.extents} }));
				aEcs.component<material>().populate(aEntity, aData.material);
			}
		};
	};

	namespace shape
	{
		class rectangle : public entity
		{
		public:
			static const entity_archetype& archetype(i_ecs& aEcs)
			{
				using neogfx::game::rectangle;
				static const renderable_entity_archetype sArchetype
				{
					{ 0xce3d930, 0x6b18, 0x403b, 0x9680, { 0x89, 0xed, 0x54, 0x83, 0xd5, 0x72 } },
					"Rectangle",
					{ rectangle::meta::id(), mesh_renderer::meta::id(), mesh_filter::meta::id() }
				};
				if (!aEcs.archetype_registered(sArchetype))
					aEcs.register_archetype(sArchetype);
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
}