// opengl_buffer.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/i_ssbo.hpp>
#include "opengl_buffer.hpp"
#include "opengl_buffer.ipp"

namespace neogfx
{
    template class opengl_buffer<ssbo_element_t<bool>>;
    template class opengl_buffer<ssbo_element_t<float>>;
    template class opengl_buffer<ssbo_element_t<double>>;
    template class opengl_buffer<ssbo_element_t<std::int32_t>>;
    template class opengl_buffer<ssbo_element_t<std::uint32_t>>;
    template class opengl_buffer<ssbo_element_t<vec2f>>;
    template class opengl_buffer<ssbo_element_t<vec2>>;
    template class opengl_buffer<ssbo_element_t<vec2i32>>;
    template class opengl_buffer<ssbo_element_t<vec2u32>>;
    template class opengl_buffer<ssbo_element_t<vec3f>>;
    template class opengl_buffer<ssbo_element_t<vec3>>;
    template class opengl_buffer<ssbo_element_t<vec3i32>>;
    template class opengl_buffer<ssbo_element_t<vec3u32>>;
    template class opengl_buffer<ssbo_element_t<vec4f>>;
    template class opengl_buffer<ssbo_element_t<vec4>>;
    template class opengl_buffer<ssbo_element_t<vec4i32>>;
    template class opengl_buffer<ssbo_element_t<vec4u32>>;
    template class opengl_buffer<ssbo_element_t<mat4f>>;
    template class opengl_buffer<ssbo_element_t<mat4>>;
}
