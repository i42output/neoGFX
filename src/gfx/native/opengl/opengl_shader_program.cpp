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
    opengl_ssbo<T>::opengl_ssbo(i_string const& aName, ssbo_id aId) :
        ssbo<T>{ aName, aId }
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
        if (iLockCount)
            throw ssbo_locked();

        if (aCapacity <= this->capacity())
            return;

        auto const existingCapacity = this->capacity();

        thread_local std::vector<T> existingData;
        existingData.clear();
        existingData.resize(this->size());
        if (existingCapacity != 0u)
        {
            std::copy(iMappedPtr, iMappedPtr + this->size(), existingData.begin());
            unmap();
            glCheck(glDeleteBuffers(1, &iHandle));
            glCheck(glGenBuffers(1, &iHandle));
            glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, this->id(), iHandle));
        }

        glCheck(glNamedBufferStorage(iHandle, aCapacity * sizeof(T), nullptr,
            GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | 
            GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
        
        ssbo<T>::reserve(aCapacity);
        map();

        if (existingCapacity != 0u)
            std::copy(existingData.begin(), existingData.end(), iMappedPtr);
    }

    template <typename T>
    void* opengl_ssbo<T>::lock(ssbo_range aRange)
    {
        if (!mapped())
            throw std::logic_error("neogfx::opengl_ssbo<T>::lock: not mapped");
        ++iLockCount;
        return iMappedPtr + aRange.first;
    }

    template <typename T>
    void opengl_ssbo<T>::unlock(ssbo_range aRange)
    {
        --iLockCount;
    }

    template <typename T>
    bool opengl_ssbo<T>::mapped() const
    {
        return iMappedPtr != nullptr;
    }

    template <typename T>
    void opengl_ssbo<T>::map() const
    {
        if (iMappedPtr != nullptr)
            throw std::logic_error("neogfx::opengl_ssbo<T>::map: already mapped");
        glCheck(iMappedPtr = static_cast<T*>(glMapNamedBufferRange(
            iHandle, 0, sizeof(T) * this->capacity(), 
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
            GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)));
    }

    template <typename T>
    void opengl_ssbo<T>::unmap() const
    {
        if (iMappedPtr == nullptr)
            throw std::logic_error("neogfx::opengl_ssbo<T>::map: already unmapped");
        if (!this->empty())
            glCheck(glFlushMappedNamedBufferRange(iHandle, 0, sizeof(T) * this->size()));
        glCheck(glUnmapNamedBuffer(iHandle));
        iMappedPtr = nullptr;
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