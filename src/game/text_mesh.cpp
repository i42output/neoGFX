// text_mesh.cpp
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

#include <neogfx/neogfx.hpp>

#include <neogfx/game/rectangle.hpp>
#include <neogfx/game/text_mesh.hpp>

namespace neogfx::game
{
    namespace shape
    {
        text::text(i_ecs& aEcs, i_graphics_context const& aGc, i_string const& aText, const neogfx::font& aFont, const neogfx::text_format& aTextFormat, neogfx::alignment aAlignment) :
            entity{ aEcs, archetype().id() }
        {
            neogfx::game::scoped_component_data_lock<game::text_mesh> lock{ aEcs };
            auto const& font = aEcs.shared_component<game::font>().populate(to_string(neolib::generate_uuid()).to_std_string(), 
                game::font{{service<i_font_manager>(), aFont.id()}, aFont.family_name(), aFont.style_name(), aFont.size(), aFont.underline()});
            auto& textMesh = aEcs.component<game::text_mesh>().populate(id(), game::text_mesh
                {
                    aText,
                    {},
                    {},
                    {},
                    aAlignment,
                    { font },
                    { to_ecs_component(aTextFormat.ink()) },
                    aTextFormat.effect() ? aTextFormat.effect()->type() : text_effect_type::None,
                    { aTextFormat.effect() ? to_ecs_component(aTextFormat.effect()->color()) : game::material{} },
                    aTextFormat.effect() ? static_cast<float>(aTextFormat.effect()->width()) : 0.0f
                });
            game::text_mesh::meta::update(textMesh, aEcs, aGc, id());
        }

        text::text(const text& aOther) :
            entity{ aOther.ecs(), archetype().id() }
        {
            // todo: clone entity
        }

        text::~text()
        {
        }
    }

    void text_mesh::meta::update(const text_mesh& aData, i_ecs& aEcs, i_graphics_context const& aGc, entity_id aEntity)
    {
        auto& mf = aEcs.component<mesh_filter>().has_entity_record(aEntity) ?
            aEcs.component<mesh_filter>().entity_record(aEntity) :
            aEcs.component<mesh_filter>().populate(aEntity, mesh_filter{});
        auto& mr = aEcs.component<mesh_renderer>().has_entity_record(aEntity) ?
            aEcs.component<mesh_renderer>().entity_record(aEntity) :
            aEcs.component<mesh_renderer>().populate(aEntity, mesh_renderer{});
        
        mf.mesh = mesh{};
        mr.patches = patches{};
        
        neogfx::font font = service<i_font_manager>().font_from_id(aData.font->id.cookie());
        auto multilineGlyphText = aGc.to_multiline_glyph_text(aData.text, font, aData.extents.x, aData.alignment);

        bool const renderToPatch = (aData.textEffect != text_effect_type::None || aData.renderToPatch);

        if (!renderToPatch)
        {
            for (auto const& line : multilineGlyphText.lines)
            {
                auto const glyphs = std::ranges::subrange(std::next(multilineGlyphText.glyphText.cbegin(), line.begin), std::next(multilineGlyphText.glyphText.cbegin(), line.end));
                if (glyphs.empty())
                    continue;
                auto const pos = line.bbox[0] - vec3f{ glyphs.begin()->cell[0] };
                for (auto const& glyphChar : glyphs)
                {
                    if (is_whitespace(glyphChar))
                        continue;
                    else if (!is_emoji(glyphChar))
                    {
                        auto const& glyphTexture = multilineGlyphText.glyphText.glyph(glyphChar);
                        auto& patch = *add_patch(*mf.mesh, mr, pos + vec3f{ glyphChar.cell[0] } + quadf{ glyphChar.shape[0], glyphChar.shape[1], glyphChar.shape[2], glyphChar.shape[3] }, glyphTexture.texture());
                        patch.material = game::material{
                            aData.material.color,
                            aData.material.gradient,
                            aData.material.sharedTexture,
                            patch.material.texture,
                            aData.material.shaderEffect };
                    }
                    else
                    {
                        auto const& emojiAtlas = service<i_font_manager>().emoji_atlas();
                        auto const& emojiTexture = emojiAtlas.emoji_texture(glyphChar.value).as_sub_texture();
                        auto& patch = *add_patch(*mf.mesh, mr, pos + vec3f{ glyphChar.cell[0] } + quadf{ glyphChar.shape[0], glyphChar.shape[1], glyphChar.shape[2], glyphChar.shape[3] }, emojiTexture);
                        patch.material = game::material{
                            {},
                            {},
                            aData.material.sharedTexture,
                            patch.material.texture,
                            {} };
                    }
                }
            }
        }
        else // renderToPatch 
        {
            size extents{ multilineGlyphText.bbox[2] - multilineGlyphText.bbox[0] };

            if (extents.cx == 0.0 || extents.cy == 0.0)
                return;

            auto ink = aData.material.color ?
                text_color{ neogfx::color{ aData.material.color->rgba } } :
                aData.material.gradient ?
                    text_color{ neogfx::gradient{ service<i_gradient_manager>().find_gradient(aData.material.gradient->id.cookie()) } } :
                    text_color{ neogfx::color::White };
            if (aData.material.gradient && aData.material.gradient->boundingBox)
                apply_bounding_box(ink, *aData.material.gradient->boundingBox);

            auto effectInk = aData.textEffectMaterial.color ?
                text_color{ neogfx::color{ aData.textEffectMaterial.color->rgba } } :
                aData.textEffectMaterial.gradient ?
                    text_color{ neogfx::gradient{ service<i_gradient_manager>().find_gradient(aData.textEffectMaterial.gradient->id.cookie()) } } :
                    text_color{ neogfx::color::White };
            if (aData.textEffectMaterial.gradient && aData.textEffectMaterial.gradient->boundingBox)
                apply_bounding_box(effectInk, *aData.textEffectMaterial.gradient->boundingBox);

            text_format const textFormat{
                ink,
                text_effect{
                    aData.textEffect,
                    effectInk,
                    aData.textEffectWidth } };

            scalar textEffectOutset = 0.0;

            if (textFormat.effect())
                textEffectOutset = std::max(textEffectOutset, textFormat.effect()->outset());
            if (textFormat.effect2())
                textEffectOutset = std::max(textEffectOutset, textFormat.effect2()->outset());

            extents += size{ textEffectOutset * 2.0 };
            extents = extents.ceil();

            neogfx::texture tex{ extents, 1.0, texture_sampling::Multisample };
            {
                graphics_context gcTex{ tex };
                gcTex.draw_multiline_glyph_text(vec3{}, multilineGlyphText, textFormat);
            }

            auto& patch = *add_patch(*mf.mesh, mr, game_rect{ point{}, extents }, tex);
            patch.material = game::material{
                {},
                {},
                aData.material.sharedTexture,
                patch.material.texture,
                {} };
        }

        auto const& boundingBox = renderToPatch ?
            game::bounding_rect(*mf.mesh).as<float>() :
            rect{ point{}, size{ multilineGlyphText.bbox[2] - multilineGlyphText.bbox[0] } }.as<float>();
        auto const& boundingBoxCentre = boundingBox.as<float>().center().to_vec3();
        for (auto& v : mf.mesh->vertices)
            v -= boundingBoxCentre;
    }

}