// standard_shader_program.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/shader_program.hpp>
#include <neogfx/gfx/i_standard_shader_program.hpp>

namespace neogfx
{
    class standard_shader_program : public shader_program<i_standard_shader_program>
    {
    public:
        standard_shader_program(std::string const& aName = "standard_shader_program");
    public:
        shader_program_type type() const final;
        const i_standard_vertex_shader& standard_vertex_shader() const final;
        i_standard_vertex_shader& standard_vertex_shader() final;
        const i_gradient_shader& gradient_shader() const final;
        i_gradient_shader& gradient_shader() final;
        const i_texture_shader& texture_shader() const final;
        i_texture_shader& texture_shader() final;
        const i_filter_shader& filter_shader() const final;
        i_filter_shader& filter_shader() final;
        const i_glyph_shader& glyph_shader() const final;
        i_glyph_shader& glyph_shader() final;
        const i_stipple_shader& stipple_shader() const final;
        i_stipple_shader& stipple_shader() final;
        const i_shape_shader& shape_shader() const final;
        i_shape_shader& shape_shader() final;
    private:
        ref_ptr<i_fragment_shader> iDefaultShader;
        ref_ptr<i_gradient_shader> iGradientShader;
        ref_ptr<i_texture_shader> iTextureShader;
        ref_ptr<i_filter_shader> iFilterShader;
        ref_ptr<i_glyph_shader> iGlyphShader;
        ref_ptr<i_stipple_shader> iStippleShader;
        ref_ptr<i_shape_shader> iShapeShader;
    };
}