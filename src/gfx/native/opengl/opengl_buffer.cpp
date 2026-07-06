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

#include "opengl_buffer.hpp"
#include "opengl_buffer.ipp"

namespace neogfx
{
    template class opengl_buffer<bool>;
    template class opengl_buffer<float>;
    template class opengl_buffer<double>;
    template class opengl_buffer<std::int32_t>;
    template class opengl_buffer<std::uint32_t>;
    template class opengl_buffer<vec2f>;
    template class opengl_buffer<vec2>;
    template class opengl_buffer<vec2i32>;
    template class opengl_buffer<vec2u32>;
    template class opengl_buffer<vec3f>;
    template class opengl_buffer<vec3>;
    template class opengl_buffer<vec3i32>;
    template class opengl_buffer<vec3u32>;
    template class opengl_buffer<vec4f>;
    template class opengl_buffer<vec4>;
    template class opengl_buffer<vec4i32>;
    template class opengl_buffer<vec4u32>;
    template class opengl_buffer<mat4f>;
    template class opengl_buffer<mat4>;
}
