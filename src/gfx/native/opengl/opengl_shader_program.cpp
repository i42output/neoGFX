// opengl_shader_program.cpp
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

#include "opengl_shader_program.ipp"

namespace neogfx
{
    template <typename T>
    opengl_ssbo<T>::opengl_ssbo(i_shader_uniform& aSizeUniform) : ssbo<T>{ neolib::cookie{}, aSizeUniform } // todo
    {
    }

    template class opengl_ssbo<bool>;
    template class opengl_ssbo<float>;
    template class opengl_ssbo<double>;
    template class opengl_ssbo<std::int32_t>;
    template class opengl_ssbo<std::uint32_t>;
    template class opengl_ssbo<vec2f>;
    template class opengl_ssbo<vec2>;
    template class opengl_ssbo<vec2i32>;
    template class opengl_ssbo<vec2u32>;
    template class opengl_ssbo<vec3f>;
    template class opengl_ssbo<vec3>;
    template class opengl_ssbo<vec3i32>;
    template class opengl_ssbo<vec3u32>;
    template class opengl_ssbo<vec4f>;
    template class opengl_ssbo<vec4>;
    template class opengl_ssbo<vec4i32>;
    template class opengl_ssbo<vec4u32>;
    template class opengl_ssbo<mat4f>;
    template class opengl_ssbo<mat4>;
    
    template class basic_opengl_shader_program<>;
    template class basic_opengl_shader_program<standard_shader_program>;
}