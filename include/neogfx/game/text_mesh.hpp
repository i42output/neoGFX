// text_mesh.hpp
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
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/game/component.hpp>
#include <neogfx/game/entity.hpp>
#include <neogfx/game/renderable_entity_archetype.hpp>
#include <neogfx/gfx/shapes.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/material.hpp>
#include <neogfx/game/font.hpp>

namespace neogfx::game
{
	struct text_mesh
	{
		string text;
		vec3 position;
		vec2 extents;
		scalar border;
		vec4 margins;
		neogfx::alignment alignment;
		shared<font> font;
		text_effect_type textEffect;
		material textEffectMaterial;
		scalar textEffectWidth;

		struct meta : i_component_data::meta
		{
			static const neolib::uuid& id()
			{
				static const neolib::uuid sId = { 0x6f45d8be, 0xba9c, 0x4a32, 0xa99e,{ 0x37, 0xd3, 0xf2, 0xb4, 0xe7, 0x53 } };
				return sId;
			}
			static const neolib::i_string& name()
			{
				static const neolib::string sName = "Text Mesh";
				return sName;
			}
			static uint32_t field_count()
			{
				return 10;
			}
			static component_data_field_type field_type(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
					return component_data_field_type::String;
				case 1:
					return component_data_field_type::Vec3;
				case 2:
					return component_data_field_type::Vec2;
				case 3:
					return component_data_field_type::Scalar;
				case 4:
					return component_data_field_type::Vec4;
				case 5:
					return component_data_field_type::Enum | component_data_field_type::Uint32;
				case 6:
					return component_data_field_type::ComponentData | component_data_field_type::Shared;
				case 7:
					return component_data_field_type::Enum | component_data_field_type::Uint32;
				case 8:
					return component_data_field_type::ComponentData;
				case 9:
					return component_data_field_type::Scalar;
				default:
					throw invalid_field_index();
				}
			}
			static neolib::uuid field_type_id(uint32_t aFieldIndex)
			{
				switch (aFieldIndex)
				{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 7:
				case 9:
					return neolib::uuid{};
				case 6:
					return font::meta::id();
				case 8:
					return material::meta::id();
				default:
					throw invalid_field_index();
				}
			}
			static const neolib::i_string& field_name(uint32_t aFieldIndex)
			{
				static const neolib::string sFieldNames[] =
				{
					"Text",
					"Position",
					"Extents",
					"Border",
					"Margins",
					"Alignment",
					"Font",
					"Text Effect",
					"Text Effect Material",
					"Text Effect Width"
				};
				return sFieldNames[aFieldIndex];
			}
			static constexpr bool has_updater = true;
			static void update(const text_mesh& aData, i_ecs& aEcs, entity_id aEntity)
			{
				auto& m = aEcs.component<mesh_filter>().has_entity_record(aEntity) ?
					aEcs.component<mesh_filter>().entity_record(aEntity) :
					aEcs.component<mesh_filter>().populate(aEntity, mesh_filter
						{
							{},
							mesh
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
							},
							{}
						});
				m.mesh->vertices = rect_vertices(rect{ point{ aData.position }, size{ aData.extents } }, 0, mesh_type::Triangles);
				for (auto& v : m.mesh->vertices)
					v.z = aData.position.z;
			}
		};
	};

	namespace shape
	{
		class text : public entity
		{
		public:
			static const entity_archetype& archetype()
			{
				using neogfx::game::text_mesh;
				static const renderable_entity_archetype sArchetype
				{
					{ 0xe3115152, 0x90ad, 0x4f7a, 0x83b0, { 0x7a, 0x59, 0xce, 0xea, 0x47, 0xb } },
					"Text Mesh",
					{ text_mesh::meta::id(), mesh::meta::id() }
				};
				return sArchetype;
			}
		public:
			text(i_ecs& aEcs, const vec3& aPosition, const std::string& aText, const neogfx::font& aFont, const neogfx::text_appearance& aAppearance, neogfx::alignment aAlignment = alignment::Left);
			text(const text& aOther);
		};
	}
}