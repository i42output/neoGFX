// opengl_helpers.cpp
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
#include "opengl_helpers.hpp"

namespace neogfx
{
    use_shader_program::use_shader_program(i_rendering_context& aContext, i_shader_program& aShaderProgram, const optional_mat44& aProjectionMatrix, const optional_mat44& aTransformationMatrix) :
        iRenderingContext{ aContext },
        iCurrentProgram{ aShaderProgram },
        iPreviousProgram{ service<i_rendering_engine>().is_shader_program_active() ? &service<i_rendering_engine>().active_shader_program() : nullptr }
    {
        iCurrentProgram.activate(iRenderingContext);
        if (iCurrentProgram.type() == shader_program_type::Standard)
        {
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
