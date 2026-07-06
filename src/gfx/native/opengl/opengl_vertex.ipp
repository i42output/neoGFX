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

#include "opengl_buffer.hpp"
#include "opengl_vertex.hpp"

namespace neogfx
{
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
        vertex_buffer{ aProvider, aType }, iBuffer{ *this, aProvider.cacheable() }
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
        if (aShaderProgram.supports(vertex_buffer_type::Debug))
            iVertexDebugAttribArray.emplace(
                false,
                sizeof(vertex_type),
                vertex_type::offset::debug,
                aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Debug));
        else
            iVertexDebugAttribArray = std::nullopt;
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
    inline void opengl_vertex_buffer<V>::reclaim()
    {
        vertices().reclaim();
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
        if (iVertexDebugAttribArray)
            iVertexDebugAttribArray->update(iBuffer);
    }
}
