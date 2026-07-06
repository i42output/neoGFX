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
    opengl_ssbo<T>::opengl_ssbo(i_string const& aName, ssbo_id aId, size_type aCapacity) :
        ssbo<T>{ aName, aId }, opengl_buffer<T>{ *this, true, static_cast<typename opengl_buffer<T>::size_type>(aCapacity) }
    {
        if (opengl_buffer<T>::handle() != 0)
            buffer_grown();
    }

    template <typename T>
    opengl_ssbo<T>::~opengl_ssbo()
    {
    }

    template <typename T>
    ssbo_range opengl_ssbo<T>::alloc(size_type aSize)
    {
        auto const maybeSpace = opengl_buffer<T>::find_space_for(aSize);
        if (maybeSpace != opengl_buffer<T>::size())
            return { static_cast<size_type>(maybeSpace), static_cast<size_type>(maybeSpace) + aSize };
        opengl_buffer<T>::resize(opengl_buffer<T>::size() + aSize);
        return { static_cast<size_type>(opengl_buffer<T>::size()) - aSize, static_cast<size_type>(opengl_buffer<T>::size()) };
    }

    template <typename T>
    void opengl_ssbo<T>::free(ssbo_range aRange)
    {
        opengl_buffer<T>::reclaim(aRange.first, aRange.last);
    }
    
    template <typename T>
    void* opengl_ssbo<T>::lock(ssbo_range aRange)
    {
        if (!opengl_buffer<T>::mapped())
            throw std::logic_error("neogfx::opengl_ssbo<T>::lock: not mapped");
        ++iLockCount;
        return opengl_buffer<T>::map() + aRange.first;
    }

    template <typename T>
    void opengl_ssbo<T>::unlock(ssbo_range aRange)
    {
        opengl_buffer<T>::flush(aRange.first, aRange.last - aRange.first);
        --iLockCount;
    }

    template <typename T>
    void opengl_ssbo<T>::reclaim()
    {
        opengl_buffer<T>::reclaim();
    }

    template <typename T>
    void opengl_ssbo<T>::flush()
    {
        opengl_buffer<T>::flush(0u, opengl_buffer<T>::size());
    }

    template <typename T>
    void opengl_ssbo<T>::buffer_grown()
    {
        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
            static_cast<GLuint>(this->id()), opengl_buffer<T>::handle()));
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