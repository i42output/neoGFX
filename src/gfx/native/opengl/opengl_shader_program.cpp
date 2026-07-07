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
        ssbo<T>{ aName, aId }, buffer_type{ *this, true, static_cast<typename buffer_type::size_type>(aCapacity) }
    {
        if (buffer_type::handle() != 0)
            buffer_grown();
    }

    template <typename T>
    opengl_ssbo<T>::~opengl_ssbo()
    {
    }

    template <typename T>
    ssbo_range opengl_ssbo<T>::alloc(size_type aSize)
    {
        auto const maybeSpace = buffer_type::find_space_for(aSize);
        
        if (maybeSpace != buffer_type::size())
            return { static_cast<size_type>(maybeSpace), static_cast<size_type>(maybeSpace) + aSize };

        if (iLockCount != 0u && aSize > buffer_type::room())
            throw ssbo_locked{};

        buffer_type::resize(buffer_type::size() + aSize);

        return { static_cast<size_type>(buffer_type::size()) - aSize, static_cast<size_type>(buffer_type::size()) };
    }

    template <typename T>
    void opengl_ssbo<T>::free(ssbo_range aRange)
    {
        buffer_type::reclaim(aRange.first, aRange.last);
    }
    
    template <typename T>
    void* opengl_ssbo<T>::lock(ssbo_range aRange)
    {
        if (!buffer_type::mapped())
            throw std::logic_error("neogfx::opengl_ssbo<T>::lock: not mapped");
        ++iLockCount;
        return buffer_type::map() + aRange.first;
    }

    template <typename T>
    void opengl_ssbo<T>::unlock(ssbo_range aRange)
    {
        buffer_type::flush(aRange.first, aRange.last - aRange.first);
        --iLockCount;
    }

    template <typename T>
    void opengl_ssbo<T>::reclaim()
    {
        buffer_type::reclaim();
    }

    template <typename T>
    void opengl_ssbo<T>::flush()
    {
        buffer_type::flush(0u, buffer_type::size());
    }

    template <typename T>
    void opengl_ssbo<T>::buffer_grown()
    {
        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
            static_cast<GLuint>(this->id()), buffer_type::handle()));
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