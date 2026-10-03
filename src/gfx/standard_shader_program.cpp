// standard_shader_program.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2019, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/standard_shader_program.hpp>
#include <neogfx/gfx/vertex_shader.hpp>
#include <neogfx/gfx/fragment_shader.hpp>

namespace neogfx
{
    standard_shader_program::standard_shader_program(std::string const& aName) :
        shader_program<i_standard_shader_program>{ aName }
    {
    }

    void standard_shader_program::create_standard_shaders()
    {
        add_shader<standard_texture_vertex_shader>();
        iDefaultShader = static_cast<i_fragment_shader&>(add_shader<standard_fragment_shader<>>());
        iGradientShader = static_cast<i_gradient_shader&>(add_shader<standard_gradient_shader>());
        iTextureShader = static_cast<i_texture_shader&>(add_shader<standard_texture_shader>());
        // n.b. after the texture shader: it shades the base colour (see i_pbr_shader)
        iPbrShader = static_cast<i_pbr_shader&>(add_shader<standard_pbr_shader>());
        iFilterShader = static_cast<i_filter_shader&>(add_shader<standard_filter_shader>());
        iGlyphShader = static_cast<i_glyph_shader&>(add_shader<standard_glyph_shader>());
        iShapeShader = static_cast<i_shape_shader&>(add_shader<standard_shape_shader>());
        iStippleShader = static_cast<i_stipple_shader&>(add_shader<standard_stipple_shader>());
        iModelMatrices = static_cast<i_shader_program&>(*this).create_ssbo<mat4f>("bModelMatrices"_s);
        iModelTable = static_cast<i_shader_program&>(*this).create_ssbo<std::uint32_t>("bModelTable"_s);
        iSceneLights = static_cast<i_shader_program&>(*this).create_ssbo<vec4f>("bLights"_s);
        iShadowMatrices = static_cast<i_shader_program&>(*this).create_ssbo<mat4f>("bShadowMatrices"_s);
        // ensure the SSBOs are mapped (see standard_shape_shader)
        iModelMatrices->alloc(1);
        iModelTable->alloc(1);
        iSceneLights->alloc(1);
        iShadowMatrices->alloc(1);
    }

    i_ssbo& standard_shader_program::model_matrices()
    {
        return *iModelMatrices;
    }

    i_ssbo& standard_shader_program::model_table()
    {
        return *iModelTable;
    }

    i_ssbo& standard_shader_program::scene_lights()
    {
        return *iSceneLights;
    }

    i_ssbo& standard_shader_program::shadow_matrices()
    {
        return *iShadowMatrices;
    }

    shader_program_type standard_shader_program::type() const
    {
        return shader_program_type::Standard;
    }

    const i_standard_vertex_shader& standard_shader_program::standard_vertex_shader() const
    {
        return static_cast<const i_standard_vertex_shader&>(vertex_shader());
    }

    i_standard_vertex_shader& standard_shader_program::standard_vertex_shader()
    {
        return static_cast<i_standard_vertex_shader&>(vertex_shader());
    }

    const i_gradient_shader& standard_shader_program::gradient_shader() const
    {
        if (iGradientShader != nullptr)
            return *iGradientShader;
        throw no_gradient_shader();
    }

    i_gradient_shader& standard_shader_program::gradient_shader()
    {
        if (iGradientShader != nullptr)
            return *iGradientShader;
        throw no_gradient_shader();
    }

    const i_texture_shader& standard_shader_program::texture_shader() const
    {
        return *iTextureShader;
    }

    i_texture_shader& standard_shader_program::texture_shader()
    {
        return *iTextureShader;
    }

    const i_pbr_shader& standard_shader_program::pbr_shader() const
    {
        return *iPbrShader;
    }

    i_pbr_shader& standard_shader_program::pbr_shader()
    {
        return *iPbrShader;
    }

    const i_filter_shader& standard_shader_program::filter_shader() const
    {
        if (iFilterShader != nullptr)
            return *iFilterShader;
        throw no_filter_shader();
    }

    i_filter_shader& standard_shader_program::filter_shader()
    {
        if (iFilterShader != nullptr)
            return *iFilterShader;
        throw no_filter_shader();
    }

    const i_glyph_shader& standard_shader_program::glyph_shader() const
    {
        if (iGlyphShader != nullptr)
            return *iGlyphShader;
        throw no_glyph_shader();
    }

    i_glyph_shader& standard_shader_program::glyph_shader()
    {
        if (iGlyphShader != nullptr)
            return *iGlyphShader;
        throw no_glyph_shader();
    }

    const i_stipple_shader& standard_shader_program::stipple_shader() const
    {
        if (iStippleShader != nullptr)
            return *iStippleShader;
        throw no_stipple_shader();
    }

    i_stipple_shader& standard_shader_program::stipple_shader()
    {
        if (iStippleShader != nullptr)
            return *iStippleShader;
        throw no_stipple_shader();
    }

    const i_shape_shader& standard_shader_program::shape_shader() const
    {
        if (iShapeShader != nullptr)
            return *iShapeShader;
        throw no_shape_shader();
    }

    i_shape_shader& standard_shader_program::shape_shader()
    {
        if (iShapeShader != nullptr)
            return *iShapeShader;
        throw no_shape_shader();
    }
}