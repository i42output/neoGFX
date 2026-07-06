// opengl_vertex.cpp
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

#include "opengl_buffer.ipp"
#include "opengl_vertex.hpp"
#include "opengl_vertex.ipp"

namespace neogfx
{
    template class opengl_buffer<standard_vertex>;
    template class opengl_vertex_buffer<>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::xyz)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::rgba)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::st)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::xyzw)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::abcd)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::efgh)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::ijkl)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::mnop)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::abcd2)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::efgh2)>;
    template class opengl_vertex_attrib_array<standard_vertex, decltype(standard_vertex::debug)>;

    opengl_vertex_array::opengl_vertex_array()
    {
        glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &iPreviousVertexArrayBindingHandle));
        glCheck(glGenVertexArrays(1, &iHandle));
        bind();
    }

    opengl_vertex_array::~opengl_vertex_array()
    {
        glCheck(glBindVertexArray(iPreviousVertexArrayBindingHandle));
        glCheck(glDeleteVertexArrays(1, &iHandle));
    }

    void opengl_vertex_array::bind()
    {
        glCheck(glBindVertexArray(iHandle));
    }

    use_shader_program::use_shader_program(i_rendering_context& aContext, i_shader_program& aShaderProgram, scalar aOpacity) :
        iRenderingContext{ aContext },
        iCurrentProgram{ aShaderProgram },
        iPreviousProgram{ service<i_rendering_engine>().is_shader_program_active() ? &service<i_rendering_engine>().active_shader_program() : nullptr }
    {
        iCurrentProgram.activate(iRenderingContext);
        if (iCurrentProgram.type() == shader_program_type::Standard)
        {
            iCurrentProgram.as<i_standard_shader_program>().standard_vertex_shader().set_opacity(aOpacity);
            if (iRenderingContext.gradient_set())
                iRenderingContext.apply_gradient(iCurrentProgram.as<i_standard_shader_program>().gradient_shader());
            else
                iCurrentProgram.as<i_standard_shader_program>().gradient_shader().clear_gradient();
        }
    }

    use_shader_program::~use_shader_program()
    {
        if (&iCurrentProgram != iPreviousProgram)
        {
            iCurrentProgram.deactivate();
            if (iPreviousProgram != nullptr)
                iPreviousProgram->activate(iRenderingContext);
        }
        if (iCurrentProgram.type() == shader_program_type::Standard)
        {
            if (!iRenderingContext.gradient_set())
                iCurrentProgram.as<i_standard_shader_program>().gradient_shader().clear_gradient();
            iCurrentProgram.as<i_standard_shader_program>().texture_shader().clear_texture();
            iCurrentProgram.as<i_standard_shader_program>().filter_shader().clear_filter();
            iCurrentProgram.as<i_standard_shader_program>().glyph_shader().clear_glyph();
            iCurrentProgram.as<i_standard_shader_program>().stipple_shader().clear_stipple();
            iCurrentProgram.as<i_standard_shader_program>().shape_shader().clear_shape();
        }
    }
}
