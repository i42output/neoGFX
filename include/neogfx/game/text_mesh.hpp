// text_mesh.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>
#include <neogfx/game/component.hpp>
#include <neogfx/game/entity.hpp>
#include <neogfx/game/renderable_entity_archetype.hpp>
#include <neogfx/gfx/shapes.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/material.hpp>
#include <neogfx/game/font.hpp>
#include <neogfx/game/ecs_helpers.hpp>

namespace neogfx::game
{
    struct text_mesh
    {
        string text;
        vec2f extents;
        float border;
        vec4f padding;
        neogfx::alignment alignment;
        shared<font> font;
        game::material material;
        text_effect_type textEffect;
        game::material textEffectMaterial;
        float textEffectWidth;

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x6f45d8be, 0xba9c, 0x4a32, 0xa99e,{ 0x37, 0xd3, 0xf2, 0xb4, 0xe7, 0x53 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Text Mesh";
                return sName;
            }
            static std::uint32_t field_count()
            {
                return 10;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::String;
                case 1:
                    return component_data_field_type::Vec2f;
                case 2:
                    return component_data_field_type::Float32;
                case 3:
                    return component_data_field_type::Vec4f;
                case 4:
                    return component_data_field_type::Enum | component_data_field_type::Uint32;
                case 5:
                    return component_data_field_type::ComponentData | component_data_field_type::Shared;
                case 6:
                    return component_data_field_type::ComponentData;
                case 7:
                    return component_data_field_type::Enum | component_data_field_type::Uint32;
                case 8:
                    return component_data_field_type::ComponentData;
                case 9:
                    return component_data_field_type::Float32;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 7:
                case 9:
                    return neolib::uuid{};
                case 5:
                    return font::meta::id();
                case 6:
                case 8:
                    return material::meta::id();
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Text",
                    "Extents",
                    "Border",
                    "Padding",
                    "Alignment",
                    "Font",
                    "Material",
                    "Text Effect",
                    "Text Effect Material",
                    "Text Effect Width"
                };
                return sFieldNames[aFieldIndex];
            }
            static constexpr bool has_updater = true;
            static void update(const text_mesh& aData, i_ecs& aEcs, i_graphics_context const& aGc, entity_id aEntity);
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
            text(i_ecs& aEcs, i_graphics_context const& aGc, i_string const& aText, const neogfx::font& aFont, const neogfx::text_format& aTextFormat, neogfx::alignment aAlignment = alignment::Left);
            text(const text& aOther);
            ~text();
        };
    }
}