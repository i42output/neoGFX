// opengl_vertex.ipp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015-2026 Leigh Johnston.  All Rights Reserved.
  
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

#include "opengl_vertex.hpp"

namespace neogfx
{
    template <typename T>
    inline opengl_buffer<T>::opengl_buffer(size_type aCapacity)
    {
        if (aCapacity != 0)
        {
            glCheck(glCreateBuffers(1, &iBufferName));
            glCheck(glNamedBufferStorage(iBufferName, aCapacity * sizeof(value_type), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
            iCapacity = aCapacity;
        }
    }

    template <typename T>
    inline opengl_buffer<T>::opengl_buffer(opengl_buffer_owner& aOwner, size_type aCapacity) :
        opengl_buffer{ aCapacity }
    {
        iOwner = &aOwner;
    }

    template <typename T>
    inline opengl_buffer<T>::~opengl_buffer()
    {
        glCheck(glDeleteBuffers(1, &iBufferName));
    }

    template <typename T>
    inline typename opengl_buffer<T>::size_type opengl_buffer<T>::capacity() const
    {
        return iCapacity;
    }

    template <typename T>
    inline bool opengl_buffer<T>::empty() const
    {
        return iSize == 0;
    }

    template <typename T>
    inline typename opengl_buffer<T>::size_type opengl_buffer<T>::size() const
    {
        return iSize;
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_iterator opengl_buffer<T>::cbegin() const
    {
        return map();
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_iterator opengl_buffer<T>::cend() const
    {
        return map() + size();
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_iterator opengl_buffer<T>::begin() const
    {
        return cbegin();
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_iterator opengl_buffer<T>::end() const
    {
        return cend();
    }
    
    template <typename T>
    inline typename opengl_buffer<T>::iterator opengl_buffer<T>::begin()
    {
        return map();
    }

    template <typename T>
    inline typename opengl_buffer<T>::iterator opengl_buffer<T>::end()
    {
        return map() + size();
    }

    template <typename T>
    inline void opengl_buffer<T>::reserve(size_type aCapacity)
    {
        if (aCapacity > capacity())
            grow(aCapacity);
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_reference opengl_buffer<T>::operator[](size_type aOffset) const
    {
        return *std::next(cbegin(), aOffset);
    }

    template <typename T>
    inline typename opengl_buffer<T>::reference opengl_buffer<T>::operator[](size_type aOffset)
    {
        return *std::next(begin(), aOffset);
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_reference opengl_buffer<T>::back() const
    {
        return *std::prev(cend());
    }

    template <typename T>
    inline typename opengl_buffer<T>::reference opengl_buffer<T>::back()
    {
        return *std::prev(end());
    }

    template <typename T>
    inline typename opengl_buffer<T>::size_type opengl_buffer<T>::find_space_for(size_type aCount)
    {
        auto freeBlockRef = find_free_block(aCount);

        if (!freeBlockRef)
            return size();

        auto const freeBlock = *freeBlockRef->second;
        std::swap(*freeBlockRef->second, freeBlockRef->first->back());
        freeBlockRef->first->pop_back();

        auto result = freeBlock.first;

        auto leftover = (freeBlock.second - freeBlock.first) - aCount;
        if (leftover > 0)
            iFreeBlocks[std::countr_zero(std::bit_ceil(leftover))].emplace_back(freeBlock.first + aCount, freeBlock.first + aCount + leftover);

        return result;
    }

    template <typename T>
    inline void opengl_buffer<T>::push_back(const_reference aValue)
    {
        need(1);
        new (map() + iSize) value_type{ aValue };
        ++iSize;
    }

    template <typename T>
    inline void opengl_buffer<T>::pop_back()
    {
        --iSize;
    }

    template <typename T>
    inline void opengl_buffer<T>::clear()
    {
        iSize = 0;
        iFreeBlocks = {};
    }

    template <typename T>
    inline GLuint opengl_buffer<T>::handle() const
    {
        return iBufferName;
    }

    template <typename T>
    inline bool opengl_buffer<T>::mapped() const
    {
        return iMemory != nullptr;
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_pointer opengl_buffer<T>::map() const
    {
        if (iMemory == nullptr)
        {
            glCheck(iMemory = static_cast<value_type*>(glMapNamedBufferRange(handle(), 0, capacity() * sizeof(value_type), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT)));
        }
        return iMemory;
    }

    template <typename T>
    inline typename opengl_buffer<T>::pointer opengl_buffer<T>::map()
    {
        return const_cast<pointer>(to_const(*this).map());
    }

    template <typename T>
    inline void opengl_buffer<T>::flush(size_type aOffset, size_type aElements)
    {
        if (mapped())
        {
            glCheck(glFlushMappedNamedBufferRange(handle(), aOffset, aElements * sizeof(value_type)));
        }
        else
            throw std::logic_error("neogfx::opengl_buffer<T>::flush: buffer not mapped!");
    }

    template <typename T>
    inline void opengl_buffer<T>::unmap()
    {
        if (iMemory != nullptr)
        {
            flush(0, size());
            glCheck(glUnmapNamedBuffer(handle()));
            iMemory = nullptr;
        }
    }

    template <typename T>
    inline typename opengl_buffer<T>::size_type opengl_buffer<T>::room() const
    {
        return capacity() - size();
    }

    template <typename T>
    inline bool opengl_buffer<T>::room_for(size_type aExtra) const
    {
        if (aExtra <= room())
            return true;
        if (find_free_block(aExtra))
            return true;
        return false;
    }

    template <typename T>
    inline void opengl_buffer<T>::need(size_type aExtra)
    {
        if (!room_for(aExtra))
            grow(std::max<size_type>(static_cast<size_type>((capacity() + aExtra) * 1.5), 16384));
    }

    template <typename T>
    inline void opengl_buffer<T>::reclaim(size_type aStartIndex, size_type aEndIndex)
    {
        if (aEndIndex != aStartIndex)
            iFreeBlocks[std::countr_zero(std::bit_ceil(aEndIndex - aStartIndex))].emplace_back(aStartIndex, aEndIndex);
    }

    template <typename T>
    inline auto opengl_buffer<T>::find_free_block(size_type aCount) const -> std::optional<std::pair<typename opengl_buffer<T>::free_blocks const*, typename opengl_buffer<T>::free_blocks::const_iterator>>
    {
        auto probe = std::bit_ceil(aCount);
        bool peek = std::countr_zero(probe * 2) < iFreeBlocks.size();
        bool peeked = false;
        while (std::countr_zero(probe) < iFreeBlocks.size())
        {
            auto& freeBlocksProbe = iFreeBlocks[std::countr_zero(peek ? probe * 2 : probe)];
            for (auto freeBlockProbe = freeBlocksProbe.begin(); freeBlockProbe != freeBlocksProbe.end(); ++freeBlockProbe)
                if (freeBlockProbe->second - freeBlockProbe->first >= aCount)
                    return std::make_pair(&freeBlocksProbe, freeBlockProbe);
            if (peek)
            {
                peek = false;
                peeked = true;
            }
            else if (peeked)
            {
                peeked = false;
                probe *= 4;
            }
            else
                probe *= 2;
        }
        return {};
    }

    template <typename T>
    inline auto opengl_buffer<T>::find_free_block(size_type aCount) -> std::optional<std::pair<typename opengl_buffer<T>::free_blocks*, typename opengl_buffer<T>::free_blocks::iterator>>
    {
        auto const result = const_cast<opengl_buffer const&>(*this).find_free_block(aCount);
        if (!result)
            return {};
        auto freeBlocks = const_cast<free_blocks*>(result->first);
        return std::make_pair(freeBlocks, std::next(freeBlocks->begin(), std::distance(freeBlocks->cbegin(), result->second)));
    }

    template <typename T>
    inline void opengl_buffer<T>::grow(size_type aCapacity)
    {
        opengl_buffer<T> temp{ aCapacity };
        if (!empty())
        {
            map();
            std::copy(begin(), end(), std::back_inserter(temp));
            unmap();
        }
        std::swap(iBufferName, temp.iBufferName);
        std::swap(iCapacity, temp.iCapacity);
        std::swap(iSize, temp.iSize);
        std::swap(iMemory, temp.iMemory);
        std::swap(iFreeBlocks, temp.iFreeBlocks);
        iOwner->buffer_grown();
    }

    template <typename Vertex, typename Attrib>
    inline opengl_vertex_attrib_array<Vertex, Attrib>::opengl_vertex_attrib_array(bool aNormalized, std::size_t aStride, std::size_t aOffset, const i_shader_program& aShaderProgram, std::string const& aVariableName) :
        iNormalized{ aNormalized }, iStride{ aStride }, iOffset{ aOffset }, iShaderProgram{ aShaderProgram }, iVariableName{ aVariableName }
    {
    }

    template <typename Vertex, typename Attrib>
    inline opengl_vertex_attrib_array<Vertex, Attrib>::~opengl_vertex_attrib_array()
    {
    }

    template <typename Vertex, typename Attrib>
    inline void opengl_vertex_attrib_array<Vertex, Attrib>::update(opengl_buffer<vertex_type>& aBuffer)
    {
        GLint previousBindingHandle;
        glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBindingHandle));
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, aBuffer.handle()));
        GLuint index;
        glCheck(index = glGetAttribLocation(to_gl_handle<GLuint>(iShaderProgram.handle()), iVariableName.c_str()));
        if (index != -1)
        {
            glCheck(glVertexAttribPointer(
                index,
                static_cast<GLint>(arity),
                opengl_attrib_data_type<value_type>::type,
                iNormalized ? GL_TRUE : GL_FALSE,
                static_cast<GLsizei>(iStride),
                reinterpret_cast<const GLvoid*>(iOffset)));
            glCheck(glEnableVertexAttribArray(index));
        }
        if (previousBindingHandle != gl_handle_cast<GLint>(aBuffer.handle()))
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, previousBindingHandle));
    }

    template <typename V>
    inline opengl_vertex_buffer<V>::opengl_vertex_buffer(i_vertex_provider& aProvider, vertex_buffer_type aType) :
        vertex_buffer{ aProvider, aType }, iBuffer{ *this }
    {
    }

    template <typename V>
    inline void opengl_vertex_buffer<V>::attach_shader(i_rendering_context& aContext, i_shader_program& aShaderProgram)
    {
        if (iVao == std::nullopt)
            iVao.emplace();
        else
            iVao->bind();
        iVertexPositionAttribArray.emplace(
            false,
            sizeof(vertex_type),
            vertex_type::offset::xyz,
            aShaderProgram,
            standard_vertex_attribute_name(vertex_buffer_type::Vertices));
        iVertexColorAttribArray.emplace(
            false,
            sizeof(vertex_type),
            vertex_type::offset::rgba,
            aShaderProgram,
            standard_vertex_attribute_name(vertex_buffer_type::Color));
        if (aShaderProgram.supports(vertex_buffer_type::UV))
            iVertexTextureCoordAttribArray.emplace(
                false,
                sizeof(vertex_type),
                vertex_type::offset::st,
                aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::UV));
        else
            iVertexTextureCoordAttribArray = std::nullopt;
        if (aShaderProgram.supports(vertex_buffer_type::Function0))
            iVertexFunction0AttribArray.emplace(
                false,
                sizeof(vertex_type),
                vertex_type::offset::xyzw,
                aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Function0));
        else
            iVertexFunction0AttribArray = std::nullopt;
        if (aShaderProgram.supports(vertex_buffer_type::Function1))
            iVertexFunction1AttribArray.emplace(
                false,
                sizeof(vertex_type),
                vertex_type::offset::abcd,
                aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Function1));
        else
            iVertexFunction1AttribArray = std::nullopt;
        if (aShaderProgram.supports(vertex_buffer_type::Function2))
            iVertexFunction2AttribArray.emplace(
                false,
                sizeof(vertex_type),
                vertex_type::offset::efgh,
                aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Function2));
        else
            iVertexFunction2AttribArray = std::nullopt;
        if (aShaderProgram.supports(vertex_buffer_type::Function3))
            iVertexFunction3AttribArray.emplace(
                false,
                sizeof(vertex_type),
                vertex_type::offset::ijkl,
                aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Function3));
        else
            iVertexFunction3AttribArray = std::nullopt;
        if (aShaderProgram.supports(vertex_buffer_type::Function4))
            iVertexFunction4AttribArray.emplace(
                false,
                sizeof(vertex_type),
                vertex_type::offset::mnop,
                aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Function4));
        else
            iVertexFunction4AttribArray = std::nullopt;
        if (aShaderProgram.supports(vertex_buffer_type::Function5))
            iVertexFunction5AttribArray.emplace(
                false,
                sizeof(vertex_type),
                vertex_type::offset::abcd2,
                aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Function5));
        else
            iVertexFunction5AttribArray = std::nullopt;
        if (aShaderProgram.supports(vertex_buffer_type::Function6))
            iVertexFunction6AttribArray.emplace(
                false,
                sizeof(vertex_type),
                vertex_type::offset::efgh2,
                aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Function6));
        else
            iVertexFunction6AttribArray = std::nullopt;
        if (aShaderProgram.type() == shader_program_type::Standard)
            static_cast<i_standard_vertex_shader&>(aShaderProgram.vertex_shader()).set_transformation_matrix(iTransformation);
        vertex_buffer::attach_shader(aContext, aShaderProgram);
        update_attrib_arrays();
    }

    template <typename V>
    inline void opengl_vertex_buffer<V>::detach_shader()
    {
        vertex_buffer::detach_shader();
    }

    template <typename V>
    inline const optional_mat44& opengl_vertex_buffer<V>::transformation() const
    {
        return iTransformation;
    }

    template <typename V>
    inline void opengl_vertex_buffer<V>::set_transformation(const optional_mat44& aTransformation)
    {
        iTransformation = aTransformation;
    }

    template <typename V>
    inline void opengl_vertex_buffer<V>::reclaim(std::size_t aStartIndex, std::size_t aEndIndex)
    {
        vertices().reclaim(aStartIndex, aEndIndex);
    }

    template <typename V>
    inline void opengl_vertex_buffer<V>::flush()
    {
        flush(vertices().size());
    }

    template <typename V>
    inline void opengl_vertex_buffer<V>::flush(std::size_t aCount)
    {
        flush(0, aCount);
    }

    template <typename V>
    inline void opengl_vertex_buffer<V>::flush(std::size_t aOffset, std::size_t aCount)
    {
        iBuffer.flush(aOffset, aCount);
    }

    template <typename V>
    inline opengl_vertex_buffer<V>::vertex_array& opengl_vertex_buffer<V>::vertices()
    {
        return iBuffer;
    }

    template <typename V>
    inline std::size_t opengl_vertex_buffer<V>::capacity() const
    {
        return iBuffer.capacity();
    }

    template <typename V>
    inline void opengl_vertex_buffer<V>::buffer_grown()
    {
        update_attrib_arrays();
    }

    template <typename V>
    inline void opengl_vertex_buffer<V>::update_attrib_arrays()
    {
        if (iVao)
            iVao->bind();
        if (iVertexPositionAttribArray)
            iVertexPositionAttribArray->update(iBuffer);
        if (iVertexColorAttribArray)
            iVertexColorAttribArray->update(iBuffer);
        if (iVertexTextureCoordAttribArray)
            iVertexTextureCoordAttribArray->update(iBuffer);
        if (iVertexFunction0AttribArray)
            iVertexFunction0AttribArray->update(iBuffer);
        if (iVertexFunction1AttribArray)
            iVertexFunction1AttribArray->update(iBuffer);
        if (iVertexFunction2AttribArray)
            iVertexFunction2AttribArray->update(iBuffer);
        if (iVertexFunction3AttribArray)
            iVertexFunction3AttribArray->update(iBuffer);
        if (iVertexFunction4AttribArray)
            iVertexFunction4AttribArray->update(iBuffer);
        if (iVertexFunction5AttribArray)
            iVertexFunction5AttribArray->update(iBuffer);
        if (iVertexFunction6AttribArray)
            iVertexFunction6AttribArray->update(iBuffer);
    }
}
