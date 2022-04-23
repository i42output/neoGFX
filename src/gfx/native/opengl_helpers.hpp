// opengl_helpers.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>
#include <vector>
#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/i_shader_program.hpp>
#include <neogfx/gfx/vertex_buffer.hpp>
#include "opengl.hpp"

namespace neogfx
{
    class opengl_vertex_array
    {
    public:
        opengl_vertex_array()
        {
            glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &iPreviousVertexArrayBindingHandle));
            glCheck(glGenVertexArrays(1, &iHandle));
            bind();
        }
        ~opengl_vertex_array()
        {
            glCheck(glBindVertexArray(iPreviousVertexArrayBindingHandle));
            glCheck(glDeleteVertexArrays(1, &iHandle));
        }
    public:
        void bind()
        {
            glCheck(glBindVertexArray(iHandle));
        }
    private:
        GLint iPreviousVertexArrayBindingHandle;
        GLuint iHandle;
    };

    class opengl_buffer_owner
    {
    public:
        virtual void buffer_grown() = 0;
    };

    template <typename T>
    class opengl_buffer
    {
    public:
        typedef T value_type;
        typedef value_type const& const_reference;
        typedef value_type& reference;
        typedef value_type const* const_pointer;
        typedef value_type* pointer;
        typedef const_pointer const_iterator;
        typedef pointer iterator;
        typedef std::size_t size_type;
    public:
        struct no_owner : std::logic_error { no_owner() : std::logic_error{ "neogfx::opengl_buffer::no_owner" } {} };
    public:
        opengl_buffer(size_type aCapacity)
        {
            if (aCapacity != 0)
            {
                glCheck(glCreateBuffers(1, &iBufferName));
                glCheck(glNamedBufferStorage(iBufferName, aCapacity * sizeof(value_type), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
                iCapacity = aCapacity;
            }
        }
        opengl_buffer(opengl_buffer_owner& aOwner, size_type aCapacity = 0) :
            opengl_buffer{ aCapacity }
        {
            iOwner = &aOwner;
        }
        ~opengl_buffer()
        {
            glCheck(glDeleteBuffers(1, &iBufferName));
        }
    public:
        size_type capacity() const
        {
            return iCapacity;
        }
        bool empty() const
        {
            return iSize == 0;
        }
        size_type size() const
        {
            return iSize;
        }
        const_iterator cbegin() const
        {
            return map();
        }
        const_iterator cend() const
        {
            return map() + size();
        }
        const_iterator begin() const
        {
            return cbegin();
        }
        const_iterator end() const
        {
            return cend();
        }
        iterator begin()
        {
            return map();
        }
        iterator end()
        {
            return map() + size();
        }
    public:
        void reserve(size_type aCapacity)
        {
            if (aCapacity > capacity())
                grow(aCapacity);
        }
    public:
        const_reference operator[](size_type aOffset) const
        {
            return *std::next(cbegin(), aOffset);
        }
        reference operator[](size_type aOffset)
        {
            return *std::next(begin(), aOffset);
        }
        const_reference back() const
        {
            return *std::prev(cend());
        }
        reference back()
        {
            return *std::prev(end());
        }
        std::size_t find_space_for(std::size_t aCount)
        {
             // todo: optimize
            for (auto space = iReclaimedSpace.begin(); space != iReclaimedSpace.end(); ++space)
                if (space->second - space->first >= aCount)
                {
                    auto result = space->first;
                    space->first += aCount;
                    if (space->first == space->second)
                        iReclaimedSpace.erase(space);
                    return result;
                }
            return size();
        }
        void push_back(const_reference aValue)
        {
            need(1);
            new (map() + iSize) value_type{ aValue };
            ++iSize;
        }
        template <typename... Args>
        void emplace_back(Args&&... aArgs)
        {
            need(1);
            new (map() + iSize) value_type{ std::forward<Args>(aArgs)... };
            ++iSize;
        }
        void pop_back()
        {
            --iSize;
        }
        void clear()
        {
            iSize = 0;
        }
    public:
        GLuint handle() const
        {
            return iBufferName;
        }
        const_pointer map() const
        {
            if (iMemory == nullptr)
            {
                glCheck(iMemory = static_cast<value_type*>(glMapNamedBufferRange(handle(), 0, capacity() * sizeof(value_type), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_COHERENT_BIT)));
            }
            return iMemory;
        }
        pointer map()
        {
            return const_cast<pointer>(to_const(*this).map());
        }
        void flush(size_type aElements)
        {
            if (iMemory != nullptr)
            {
                glCheck(glFlushMappedNamedBufferRange(handle(), 0, aElements * sizeof(value_type)));
            }
        }
        void unmap()
        {
            if (iMemory != nullptr)
            {
                flush(size());
                glCheck(glUnmapNamedBuffer(handle()));
                iMemory = nullptr;
            }
        }
    public:
        size_type room() const
        {
            return capacity() - size();
        }
        bool room_for(size_type aExtra) const
        {
            return aExtra <= room();
        }
        void need(size_type aExtra)
        {
            if (!room_for(aExtra))
                grow(std::max<size_type>(static_cast<size_type>((capacity() + aExtra) * 1.5), 16384));
        }
    public:
        void reclaim(std::size_t aStartIndex, std::size_t aEndIndex)
        {
            if (aEndIndex != aStartIndex)
                iReclaimedSpace.push_back(std::make_pair(aStartIndex, aEndIndex));
        }
    private:
        void grow(size_type aCapacity)
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
            std::swap(iReclaimedSpace, temp.iReclaimedSpace);
            iOwner->buffer_grown();
        }
    private:
        GLuint iBufferName = 0;
        size_type iCapacity = 0;
        size_type iSize = 0;
        mutable pointer iMemory = nullptr;
        opengl_buffer_owner* iOwner = nullptr;
        std::vector<std::pair<std::size_t, std::size_t>> iReclaimedSpace;
    };

    template <typename T>
    struct opengl_attrib_data_type {};
    template <>
    struct opengl_attrib_data_type<double> { static constexpr GLenum type = GL_DOUBLE; };
    template <>
    struct opengl_attrib_data_type<float> { static constexpr GLenum type = GL_FLOAT; };
    template <>
    struct opengl_attrib_data_type<uint8_t> { static constexpr GLenum type = GL_UNSIGNED_BYTE; };

    template <typename Vertex, typename Attrib>
    class opengl_vertex_attrib_array
    {
    public:
        struct cannot_get_attrib_location : std::logic_error { cannot_get_attrib_location(std::string const& aName) : std::logic_error("neogfx::opengl_vertex_attrib_array::cannot_get_attrib_location: " + aName) {} };
    public:
        typedef Vertex vertex_type;
        typedef Attrib attribute_type;
        typedef typename attribute_type::value_type value_type;
        static constexpr std::size_t arity = sizeof(attribute_type) / sizeof(value_type);
    public:
        opengl_vertex_attrib_array(bool aNormalized, std::size_t aStride, std::size_t aOffset, const i_shader_program& aShaderProgram, std::string const& aVariableName) :
            iNormalized{ aNormalized }, iStride{ aStride }, iOffset{ aOffset }, iShaderProgram{ aShaderProgram }, iVariableName{ aVariableName }
        {
        }
        ~opengl_vertex_attrib_array()
        {
        }
    public:
        void update(opengl_buffer<vertex_type>& aBuffer)
        {
            GLint previousBindingHandle;
            glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBindingHandle));
            if (previousBindingHandle != gl_handle_cast<GLint>(aBuffer.handle()))
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
    private:
        bool const iNormalized;
        std::size_t const iStride;
        std::size_t const iOffset;
        i_shader_program const& iShaderProgram;
        std::string const iVariableName;
    };

    inline vec4f color_to_vec4f(const avec4u8& aSource)
    {
        return vec4f{{ aSource[0] / 255.0f, aSource[1] / 255.0f, aSource[2] / 255.0f, aSource[3] / 255.0f }};
    }

    struct standard_vertex
    {
        vec3f xyz;
        vec4f rgba;
        vec2f st;
        vec4f xyzw;
        vec4f abcd;
        vec4f efgh;
        vec4f ijkl;
        standard_vertex(const vec3f& xyz = vec3f{}) :
            xyz{ xyz }
        {
        }
        standard_vertex(const vec3f& xyz, const vec4f& rgba, const vec2f& st = {}, const vec4f& xyzw = {}, const vec4f& abcd = {}, const vec4f& efgh = {}, const vec4f& ijkl = {}) :
            xyz{ xyz }, rgba{ rgba }, st{ st }, xyzw{ xyzw }, abcd{ abcd }, efgh{ efgh }, ijkl{ ijkl }
        {
        }
        struct offset
        {
            static constexpr std::size_t xyz = 0u;
            static constexpr std::size_t rgba = xyz + sizeof(decltype(standard_vertex::xyz));
            static constexpr std::size_t st = rgba + sizeof(decltype(standard_vertex::rgba));
            static constexpr std::size_t xyzw = st + sizeof(decltype(standard_vertex::st));
            static constexpr std::size_t abcd = xyzw + sizeof(decltype(standard_vertex::xyzw));
            static constexpr std::size_t efgh = abcd + sizeof(decltype(standard_vertex::abcd));
            static constexpr std::size_t ijkl = efgh + sizeof(decltype(standard_vertex::efgh));
        };
    };

    template <typename V = standard_vertex>
    class opengl_vertex_buffer : public vertex_buffer, private opengl_buffer_owner
    {
    public:
        typedef V vertex_type;
    public:
        typedef opengl_buffer<vertex_type> vertex_array;
        class use
        {
        public:
            use(opengl_vertex_buffer<V>& aParent) : iParent{ aParent }
            {
            }
            ~use()
            {
            }
        public:
            const vertex_array& vertices() const
            {
                return iParent.iBuffer;
            }
            vertex_array& vertices()
            {
                return iParent.iBuffer;
            }
            const optional_mat44& transformation() const
            {
                return iParent.iTransformation;
            }
            void set_transformation(const optional_mat44& aTransformation)
            {
                iParent.iTransformation = aTransformation;
            }
            void execute()
            {
                iParent.execute();
            }
        private:
            opengl_vertex_buffer<vertex_type>& iParent;
        };
    public:
        opengl_vertex_buffer(i_vertex_provider& aProvider, vertex_buffer_type aType) :
            vertex_buffer{ aProvider, aType }, iBuffer{ *this }
        {
        }
    public:
        void attach_shader(i_rendering_context& aContext, i_shader_program& aShaderProgram) override
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
            if (aShaderProgram.supports(vertex_buffer_type::Function0))
                iVertexFunction0AttribArray.emplace(
                    false,
                    sizeof(vertex_type),
                    vertex_type::offset::xyzw,
                    aShaderProgram,
                    standard_vertex_attribute_name(vertex_buffer_type::Function0));
            if (aShaderProgram.supports(vertex_buffer_type::Function1))
                iVertexFunction1AttribArray.emplace(
                    false,
                    sizeof(vertex_type),
                    vertex_type::offset::abcd,
                    aShaderProgram,
                    standard_vertex_attribute_name(vertex_buffer_type::Function1));
            if (aShaderProgram.supports(vertex_buffer_type::Function2))
                iVertexFunction2AttribArray.emplace(
                    false,
                    sizeof(vertex_type),
                    vertex_type::offset::efgh,
                    aShaderProgram,
                    standard_vertex_attribute_name(vertex_buffer_type::Function2));
            if (aShaderProgram.supports(vertex_buffer_type::Function3))
                iVertexFunction3AttribArray.emplace(
                    false,
                    sizeof(vertex_type),
                    vertex_type::offset::ijkl,
                    aShaderProgram,
                    standard_vertex_attribute_name(vertex_buffer_type::Function3));
            if (aShaderProgram.vertex_shader().has_standard_vertex_matrices())
            {
                auto& standardMatrices = aShaderProgram.vertex_shader().standard_vertex_matrices();
                standardMatrices.set_transformation_matrix(iTransformation);
            }
            vertex_buffer::attach_shader(aContext, aShaderProgram);
            update_attrib_arrays();
        }
        void detach_shader() override
        {
            vertex_buffer::detach_shader();
        }
    public:
        void reclaim(std::size_t aStartIndex, std::size_t aEndIndex)
        {
            vertices().reclaim(aStartIndex, aEndIndex);
        }
    public:
        void execute()
        {
            GLsync sync;
            glCheck(sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
            glCheck(glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, ~0ull));
            glCheck(glDeleteSync(sync));
        }
        void flush()
        {
            flush(vertices().size());
        }
        void flush(std::size_t aElements)
        {
            iBuffer.flush(aElements);
        }
        vertex_array& vertices()
        {
            return iBuffer;
        }
        std::size_t capacity() const
        {
            return iBuffer.capacity();
        }
    private:
        void buffer_grown() override
        {
            update_attrib_arrays();
        }
        void update_attrib_arrays()
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
        }
    private:
        opengl_buffer<vertex_type> iBuffer;
        optional_mat44 iTransformation;
        std::optional<opengl_vertex_array> iVao;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::xyz)>> iVertexPositionAttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::rgba)>> iVertexColorAttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::st)>> iVertexTextureCoordAttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::xyzw)>> iVertexFunction0AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::abcd)>> iVertexFunction1AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::efgh)>> iVertexFunction2AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::ijkl)>> iVertexFunction3AttribArray;
    };

    class use_shader_program
    {
    public:
        use_shader_program(i_rendering_context& aContext, i_shader_program& aShaderProgram, const optional_mat44& aProjectionMatrix = optional_mat44{}, const optional_mat44& aTransformationMatrix = optional_mat44{});
        ~use_shader_program();
    private:
        i_rendering_context& iRenderingContext;
        i_shader_program& iCurrentProgram;
        i_shader_program* iPreviousProgram;
    };
}
