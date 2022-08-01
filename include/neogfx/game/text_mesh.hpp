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
#include <neogfx/gfx/i_graphics_context.hpp>
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
        vec2 extents;
        scalar border;
        vec4 padding;
        neogfx::alignment alignment;
        shared<font> font;
        material ink;
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
            static const i_string& name()
            {
                static const string sName = "Text Mesh";
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
                    return component_data_field_type::Vec2;
                case 2:
                    return component_data_field_type::Scalar;
                case 3:
                    return component_data_field_type::Vec4;
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
            static const i_string& field_name(uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Text",
                    "Extents",
                    "Border",
                    "Padding",
                    "Alignment",
                    "Font",
                    "Ink",
                    "Text Effect",
                    "Text Effect Material",
                    "Text Effect Width"
                };
                return sFieldNames[aFieldIndex];
            }
            static constexpr bool has_updater = true;
            static void update(const text_mesh& aData, i_ecs& aEcs, i_graphics_context const& aGc, entity_id aEntity)
            {
                auto& mf = aEcs.component<mesh_filter>().has_entity_record(aEntity) ?
                    aEcs.component<mesh_filter>().entity_record(aEntity) :
                    aEcs.component<mesh_filter>().populate(aEntity, mesh_filter{});
                auto& mr = aEcs.component<mesh_renderer>().has_entity_record(aEntity) ?
                    aEcs.component<mesh_renderer>().entity_record(aEntity) :
                    aEcs.component<mesh_renderer>().populate(aEntity, mesh_renderer{});
                mf.mesh = mesh{};
                mr.patches = patches{};
                auto glyphText = aGc.to_multiline_glyph_text(aData.text, service<i_font_manager>().font_from_id(aData.font.ptr->id.cookie()), aData.extents.x, aData.alignment);
                for (auto const& line : glyphText.lines)
                {
                    auto mesh_line = [&](const point& aOffset, const material& aMaterial) 
                    {
                        auto pos = line.pos + aOffset;
                        for (auto i = line.begin; i < line.end; ++i)
                        {
                            auto const& glyph = *std::next(glyphText.glyphText.begin(), i);
                            if (!is_whitespace(glyph))
                            {
                                auto const& glyphFont = glyphText.glyphText.glyph_font(glyph);
                                auto const& glyphTexture = glyphText.glyphText.glyph_texture(glyph);
                                vec3 glyphOrigin(
                                    pos.x + glyphTexture.placement().x,
                                    aGc.logical_coordinates().is_game_orientation() ?
                                        pos.y + (glyphTexture.placement().y + -glyphFont.descender()) :
                                        pos.y + glyphFont.height() - (glyphTexture.placement().y + -glyphFont.descender()) - glyphTexture.texture().extents().cy,
                                    0.0);
                                add_patch(*mf.mesh, mr, rect{ glyphOrigin, glyphTexture.texture().extents() }, glyphTexture.texture());
                                mr.patches.back().material = material{ aMaterial.color, aMaterial.gradient, aMaterial.sharedTexture, mr.patches.back().material.texture, aMaterial.shaderEffect };
                            }
                            pos.x += advance(glyph).cx;
                        }
                    };
                    switch(aData.textEffect)
                    {
                    case text_effect_type::Outline:
                        for (scalar oy = -aData.textEffectWidth; oy <= aData.textEffectWidth; ++oy)
                            for (scalar ox = -aData.textEffectWidth; ox <= aData.textEffectWidth; ++ox)
                                mesh_line(point{ ox, oy }, aData.textEffectMaterial);
                        break;
                    default:
                        // todo
                        break;
                    }
                    mesh_line(point{}, aData.ink);
                }
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
            text(i_ecs& aEcs, i_graphics_context const& aGc, std::string const& aText, const neogfx::font& aFont, const neogfx::text_attributes& aTextAttributes, neogfx::alignment aAlignment = alignment::Left);
            text(const text& aOther);
        };
    }
}