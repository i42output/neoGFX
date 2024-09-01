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
    opengl_ssbo<T>::opengl_ssbo(ssbo_id aId, i_shader_uniform& aSizeUniform) : 
        ssbo<T>{ aId, aSizeUniform } // todo
    {
        glCheck(glGenBuffers(1, &iHandle));
        glCheck(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, id(), iHandle));
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

        auto const existingCapacity = this->capacity();

        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, iHandle));

        try
        {
            thread_local std::vector<std::byte> existingData;
            existingData.clear();
            existingData.resize(sizeof(T) * aCapacity);
            if (existingCapacity != 0u)
            {
                scoped_ssbo_map ssm{ *this };
                std::copy(
                    static_cast<std::byte const*>(cdata()), 
                    static_cast<std::byte const*>(cdata()) + sizeof(T) * existingCapacity, 
                    existingData.data());
            }
            glCheck(glBufferData(GL_SHADER_STORAGE_BUFFER, aCapacity * sizeof(T), existingData.data(), GL_STATIC_DRAW));
        }
        catch (...)
        {
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
            throw;
        }
        
        glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));

        ssbo<T>::reserve(aCapacity);

        if (mapped())
        {
            auto const mappedCount = iMappedCount;
            try
            {
                iMappedCount = 0u;
                map();
                iMappedCount = mappedCount;
            }
            catch (...)
            {
                iMappedCount = mappedCount;
                throw;
            }
        }
    }

    template <typename T>
    void const* opengl_ssbo<T>::data() const
    {
        if (mapped())
            return iMapped;
        throw not_mapped();
    }

    template <typename T>
    void const* opengl_ssbo<T>::cdata() const
    {
        if (mapped())
            return iMapped;
        throw not_mapped();
    }

    template <typename T>
    void* opengl_ssbo<T>::data()
    {
        if (mapped())
            return iMapped;
        throw not_mapped();
    }

    template <typename T>
    bool opengl_ssbo<T>::mapped() const
    {
        return iMapped != nullptr;
    }

    template <typename T>
    void opengl_ssbo<T>::map() const
    {
        if (++iMappedCount == 1u)
        {
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, iHandle));
            glCheck(iMapped = static_cast<value_type*>(glMapBufferRange(
                GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * this->capacity(), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)));
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
        }
    }

    template <typename T>
    void opengl_ssbo<T>::unmap() const
    {
        if (--iMappedCount == 0u)
        {
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, iHandle));
            glCheck(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
            iMapped = nullptr;
            glCheck(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
        }
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