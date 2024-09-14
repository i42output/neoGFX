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
    opengl_ssbo<T>::opengl_ssbo(i_string const& aName, ssbo_id aId, i_shader_uniform& aMetaUniform, bool aTripleBuffer) :
        ssbo<T>{ aName, aId, aMetaUniform, aTripleBuffer }
    {
        glCheck(glGenBuffers(1, &iHandle));
        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->id(), iHandle));
    }

    template <typename T>
    opengl_ssbo<T>::~opengl_ssbo()
    {
        glCheck(glDeleteBuffers(1, &iHandle));
    }

    template <typename T>
    void opengl_ssbo<T>::reserve(std::size_t aCapacity)
    {
        if (aCapacity <= this->capacity())
            return;

        auto const mappedCount = iMappedCount;

        auto const existingCapacity = this->capacity();

        thread_local std::vector<T> existingData;
        existingData.clear();
        existingData.resize(aCapacity * (this->triple_buffer() ? 3u : 1u));
        if (existingCapacity != 0u)
        {
            scoped_ssbo_map ssm{ *this };
            auto dest = existingData.begin();
            for (std::uint32_t frameIndex = 0; frameIndex < (this->triple_buffer() ? 3u : 1u); ++frameIndex)
            {
                std::copy(
                    iMappedPtr,
                    iMappedPtr + existingCapacity * frameIndex,
                    dest);
                dest += aCapacity;
            }
        }
        
        if (mapped())
        {
            iMappedCount = 1u;
            unmap();
        }
        
        glCheck(glNamedBufferStorage(iHandle, aCapacity * (this->triple_buffer() ? 3u : 1u) * sizeof(T), nullptr,
            GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | 
            (this->triple_buffer() ? (GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT) : 0)));
        
        if (mappedCount)
        {
            map();
            iMappedCount = mappedCount;
        }

        if (existingCapacity != 0u)
        {
            scoped_ssbo_map ssm{ *this };
            std::copy(
                existingData.begin(),
                existingData.end(),
                iMappedPtr);
        }

        ssbo<T>::reserve(aCapacity);
    }

    template <typename T>
    void const* opengl_ssbo<T>::data() const
    {
        if (mapped())
            return iMappedPtr + (this->triple_buffer_frame() * this->capacity());
        throw not_mapped();
    }

    template <typename T>
    void const* opengl_ssbo<T>::cdata() const
    {
        if (mapped())
            return iMappedPtr + (this->triple_buffer_frame() * this->capacity());;
        throw not_mapped();
    }

    template <typename T>
    void* opengl_ssbo<T>::data()
    {
        if (mapped())
            return iMappedPtr + (this->triple_buffer_frame() * this->capacity());;
        throw not_mapped();
    }

    template <typename T>
    bool opengl_ssbo<T>::mapped() const
    {
        return iMappedCount != 0u;
    }

    template <typename T>
    void opengl_ssbo<T>::map() const
    {
        if (++iMappedCount == 1u)
        {
            if (this->triple_buffer())
                ++iMappedCount;
            glCheck(iMappedPtr = static_cast<T*>(glMapNamedBufferRange(
                iHandle, 0, sizeof(T) * this->capacity(), 
                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                (this->triple_buffer() ? (GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT) : 0))));
        }
    }

    template <typename T>
    void opengl_ssbo<T>::unmap() const
    {
        if (--iMappedCount == 0u)
        {
            glCheck(glFlushMappedNamedBufferRange(iHandle, 0, sizeof(T) * this->size()));
            glCheck(glUnmapNamedBuffer(iHandle));
            iMappedPtr = nullptr;
        }
    }

    template <typename T>
    void opengl_ssbo<T>::sync() const
    {
        ssbo<T>::sync();
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