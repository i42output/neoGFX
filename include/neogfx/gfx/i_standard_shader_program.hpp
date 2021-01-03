// i_standard_shader_program.hpp
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
#include <neogfx/gfx/i_shader_program.hpp>
#include <neogfx/gfx/i_fragment_shader.hpp>

namespace neogfx
{
    struct no_gradient_shader : std::logic_error { no_gradient_shader() : std::logic_error{ "neogfx::no_gradient_shader" } {} };
    struct no_filter_shader : std::logic_error { no_filter_shader() : std::logic_error{ "neogfx::no_filter_shader" } {} };
    struct no_glyph_shader : std::logic_error { no_glyph_shader() : std::logic_error{ "neogfx::no_glyph_shader" } {} };
    struct no_stipple_shader : std::logic_error { no_stipple_shader() : std::logic_error{ "neogfx::no_stipple_shader" } {} };
    struct no_shape_shader : std::logic_error { no_shape_shader() : std::logic_error{ "neogfx::no_shape_shader" } {} };

    class i_standard_shader_program : public i_shader_program
    {
    public:
        typedef i_standard_shader_program abstract_type;
        // operations
    public:
        virtual const i_gradient_shader& gradient_shader() const = 0;
        virtual i_gradient_shader& gradient_shader() = 0;
        virtual const i_texture_shader& texture_shader() const = 0;
        virtual i_texture_shader& texture_shader() = 0;
        virtual const i_filter_shader& filter_shader() const = 0;
        virtual i_filter_shader& filter_shader() = 0;
        virtual const i_glyph_shader& glyph_shader() const = 0;
        virtual i_glyph_shader& glyph_shader() = 0;
        virtual const i_stipple_shader& stipple_shader() const = 0;
        virtual i_stipple_shader& stipple_shader() = 0;
        virtual const i_shape_shader& shape_shader() const = 0;
        virtual i_shape_shader& shape_shader() = 0;
    };
}