// opengl_vertex.hpp
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
#include <bit>

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
        opengl_vertex_array();
        ~opengl_vertex_array();
    public:
        void bind();
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
        using value_type = T;
        using const_reference = value_type const&;
        using reference = value_type&;
        using const_pointer = value_type const*;
        using pointer = value_type*;
        using const_iterator = const_pointer;
        using iterator = pointer;
        using size_type = std::size_t;
    public:
        struct no_owner : std::logic_error { no_owner() : std::logic_error{ "neogfx::opengl_buffer::no_owner" } {} };
    private:
        using free_block = std::pair<size_type, size_type>;
        using free_blocks = std::vector<free_block>;
    public:
        opengl_buffer(size_type aCapacity);
        opengl_buffer(opengl_buffer_owner& aOwner, size_type aCapacity = 0);
        ~opengl_buffer();
    public:
        size_type capacity() const;
        bool empty() const;
        size_type size() const;
        const_iterator cbegin() const;
        const_iterator cend() const;
        const_iterator begin() const;
        const_iterator end() const;
        iterator begin();
        iterator end();
    public:
        void reserve(size_type aCapacity);
    public:
        const_reference operator[](size_type aOffset) const;
        reference operator[](size_type aOffset);
        const_reference back() const;
        reference back();
        size_type find_space_for(size_type aCount);
        void push_back(const_reference aValue);
        template <typename... Args>
        void emplace_back(Args&&... aArgs)
        {
            need(1);
            new (map() + iSize) value_type{ std::forward<Args>(aArgs)... };
            ++iSize;
        }
        void pop_back();
        void clear();
    public:
        GLuint handle() const;
        bool mapped() const;
        const_pointer map() const;
        pointer map();
        void flush(size_type aOffset, size_type aElements);
        void unmap();
    public:
        size_type room() const;
        bool room_for(size_type aExtra) const;
        void need(size_type aExtra);
    public:
        void reclaim(size_type aStartIndex, size_type aEndIndex);
    private:
        std::optional<std::pair<free_blocks const*, free_blocks::const_iterator>> find_free_block(size_type aCount) const;
        std::optional<std::pair<free_blocks*, free_blocks::iterator>> find_free_block(size_type aCount);
        void grow(size_type aCapacity);
    private:
        GLuint iBufferName = 0;
        size_type iCapacity = 0;
        size_type iSize = 0;
        mutable pointer iMemory = nullptr;
        opengl_buffer_owner* iOwner = nullptr;
        std::array<free_blocks, 32u> iFreeBlocks;
    };

    template <typename T>
    struct opengl_attrib_data_type {};
    template <>
    struct opengl_attrib_data_type<double> { static constexpr GLenum type = GL_DOUBLE; };
    template <>
    struct opengl_attrib_data_type<float> { static constexpr GLenum type = GL_FLOAT; };
    template <>
    struct opengl_attrib_data_type<std::uint8_t> { static constexpr GLenum type = GL_UNSIGNED_BYTE; };

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
        opengl_vertex_attrib_array(bool aNormalized, std::size_t aStride, std::size_t aOffset, const i_shader_program& aShaderProgram, std::string const& aVariableName);
        ~opengl_vertex_attrib_array();
    public:
        void update(opengl_buffer<vertex_type>& aBuffer);
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
        vec4f mnop;
        vec4f abcd2;
        vec4f efgh2;
        standard_vertex(const vec3f& xyz = vec3f{}) :
            xyz{ xyz }
        {
        }
        standard_vertex(const vec3f& xyz, const vec4f& rgba, const vec2f& st = {}, const vec4f& xyzw = {}, const vec4f& abcd = {}, const vec4f& efgh = {}, const vec4f& ijkl = {}, const vec4f mnop = {}, const vec4f abcd2 = {}, const vec4f efgh2 = {}) :
            xyz{ xyz }, rgba{ rgba }, st{ st }, xyzw{ xyzw }, abcd{ abcd }, efgh{ efgh }, ijkl{ ijkl }, mnop{ mnop }, abcd2{ abcd2 }, efgh2{ efgh2 }
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
            static constexpr std::size_t mnop = ijkl + sizeof(decltype(standard_vertex::ijkl));
            static constexpr std::size_t abcd2 = mnop + sizeof(decltype(standard_vertex::mnop));
            static constexpr std::size_t efgh2 = abcd2 + sizeof(decltype(standard_vertex::abcd2));
        };
    };

    template <typename V = standard_vertex>
    class opengl_vertex_buffer : public vertex_buffer, private opengl_buffer_owner
    {
    public:
        typedef V vertex_type;
    public:
        typedef opengl_buffer<vertex_type> vertex_array;
    public:
        opengl_vertex_buffer(i_vertex_provider& aProvider, vertex_buffer_type aType);
    public:
        void attach_shader(i_rendering_context& aContext, i_shader_program& aShaderProgram) override;
        void detach_shader() override;
    public:
        const optional_mat44& transformation() const;
        void set_transformation(const optional_mat44& aTransformation);
    public:
        void reclaim(std::size_t aStartIndex, std::size_t aEndIndex);
    public:
        void flush();
        void flush(std::size_t aCount);
        void flush(std::size_t aOffset, std::size_t aCount);
        vertex_array& vertices();
        std::size_t capacity() const;
    private:
        void buffer_grown() override;
        void update_attrib_arrays();
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
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::mnop)>> iVertexFunction4AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::abcd2)>> iVertexFunction5AttribArray;
        std::optional<opengl_vertex_attrib_array<vertex_type, decltype(vertex_type::efgh2)>> iVertexFunction6AttribArray;
    };

    class use_shader_program
    {
    public:
        use_shader_program(i_rendering_context& aContext, i_shader_program& aShaderProgram, scalar aOpacity = 1.0);
        ~use_shader_program();
    private:
        i_rendering_context& iRenderingContext;
        i_shader_program& iCurrentProgram;
        i_shader_program* iPreviousProgram;
    };
}
