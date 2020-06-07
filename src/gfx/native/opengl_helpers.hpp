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
            glCheck(glBindVertexArray(iHandle));
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

    template <typename T>
    class opengl_buffer;
        
    template <typename T>
    class opengl_buffer_owner
    {
    public:
        virtual void update(opengl_buffer<T>& aBuffer) = 0;
    };

    template <typename T>
    class opengl_buffer
    {
    public:
        typedef T value_type;
    public:
        struct no_owner : std::logic_error { no_owner() : std::logic_error{ "neogfx::opengl_buffer::no_owner" } {} };
    public:
        opengl_buffer(std::size_t aSize) :
            iOwner{ nullptr }, iSize { aSize }, iMemory{ nullptr }
        {
            glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &iPreviousBindingHandle));
            glCheck(glGenBuffers(1, &iHandle));
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, iHandle));
            glCheck(glBufferStorage(GL_ARRAY_BUFFER, size() * sizeof(value_type), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
        }
        ~opengl_buffer()
        {
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, iPreviousBindingHandle));
            glCheck(glDeleteBuffers(1, &iHandle));
        }
    public:
        std::size_t size() const
        {
            return iSize;
        }
        GLuint handle() const
        {
            return iHandle;
        }
        value_type* map()
        {
            if (iMemory == nullptr)
            {
                glCheck(iMemory = static_cast<value_type*>(glMapNamedBufferRange(handle(), 0, size() * sizeof(value_type), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_COHERENT_BIT)));
            }
            return iMemory;
        }
        void flush(std::size_t aElements)
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
        opengl_buffer_owner<T>& owner() const
        {
            return *iOwner;
        }
        void set_owner(opengl_buffer_owner<T>& aOwner)
        {
            iOwner = &aOwner;
        }
    private:
        opengl_buffer_owner<T>* iOwner;
        const std::size_t iSize;
        GLint iPreviousBindingHandle;
        GLuint iHandle;
        value_type* iMemory;
    };

    template <typename T, typename OriginalType = T>
    class opengl_buffer_allocator : public std::allocator<T>
    {
    public:
        using typename std::allocator<T>::pointer;
        using typename std::allocator<T>::size_type;
        template<class Other>
        struct rebind
        {    
            typedef opengl_buffer_allocator<Other, OriginalType> other;
        };
        typedef opengl_buffer<OriginalType> buffer;
        typedef std::vector<std::unique_ptr<buffer>> buffer_list;
    public:
        struct buffer_not_found : std::logic_error { buffer_not_found() : std::logic_error{ "neogfx::opengl_buffer_allocator::buffer_not_found" } {} };
    public:
        opengl_buffer_allocator()
        {
        }
        opengl_buffer_allocator(const opengl_buffer_allocator<T, OriginalType>& aOther)
        {
        }
        template <typename T2>
        opengl_buffer_allocator(const opengl_buffer_allocator<T2, OriginalType>& aOther)
        {
        }
    public:
        void deallocate(pointer aPointer, size_type aCount)
        {
            if constexpr (std::is_same_v<T, OriginalType>)
            {
                for (auto buffer = buffers().begin(); buffer != buffers().end(); ++buffer)
                {
                    if ((**buffer).map() == aPointer)
                    {
                        if (buffer != std::prev(buffers.end()))
                        {
                            buffers().back()->set_owner((**buffer).owner());
                            buffers().back()->owner().update(*buffers().back());
                        }
                        buffers().erase(buffer);
                        return;
                    }
                }
            }
            else
                return std::allocator<T>::deallocate(aPointer, aCount);
        }
        pointer allocate(size_type aCount)
        {    
            if constexpr (std::is_same_v<T, OriginalType>)
            {
                buffers().push_back(std::make_unique<opengl_buffer<OriginalType>>(aCount));
                auto nextOwner = next_owner();
                next_owner() = nullptr;
                if (nextOwner)
                {
                    buffers().back()->set_owner(*nextOwner);
                    buffers().back()->owner().update(*buffers().back());
                }
                return buffers().back()->map();
            }
            else
                return std::allocator<T>::allocate(aCount);
        }
        pointer allocate(size_type aCount, const void *)
        {
            return allocate(aCount);
        }
    public:
        static buffer_list& buffers()
        {
            static buffer_list sBuffers;
            return sBuffers;
        }
        static buffer& find_buffer(T& aFirstVertex)
        {
            for (auto& buffer : buffers())
                if (buffer.map() == &aFirstVertex)
                    return buffer;
            throw buffer_not_found();
        }
        static opengl_buffer_owner<OriginalType>*& next_owner()
        {
            static opengl_buffer_owner<OriginalType>* sNextOwner;
            return sNextOwner;
        }
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
        struct cannot_get_attrib_location : std::logic_error { cannot_get_attrib_location(const std::string& aName) : std::logic_error("neogfx::opengl_vertex_attrib_array::cannot_get_attrib_location: " + aName) {} };
    public:
        typedef Vertex vertex_type;
        typedef Attrib attribute_type;
        typedef typename attribute_type::value_type value_type;
        static constexpr std::size_t arity = sizeof(attribute_type) / sizeof(value_type);
    public:
        opengl_vertex_attrib_array(bool aNormalized, std::size_t aStride, std::size_t aOffset, const i_shader_program& aShaderProgram, const std::string& aVariableName) :
            iNormalized{ aNormalized }, iStride{ aStride }, iOffset{ aOffset }, iShaderProgram{ aShaderProgram }, iVariableName{ aVariableName }
        {
        }
        ~opengl_vertex_attrib_array()
        {
        }
    public:
        void update(opengl_buffer<Vertex>& aBuffer)
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

    inline vec4f color_to_vec4f(const std::array<uint8_t, 4>& aSource)
    {
        return vec4f{{ aSource[0] / 255.0f, aSource[1] / 255.0f, aSource[2] / 255.0f, aSource[3] / 255.0f }};
    }

    struct standard_vertex
    {
        vec3f xyz;
        vec4f rgba;
        vec2f st;
        standard_vertex(const vec3f& xyz = vec3f{}) :
            xyz{ xyz }
        {
        }
        standard_vertex(const vec3f& xyz, const vec4f& rgba, const vec2f& st = vec2f{}) :
            xyz{ xyz }, rgba{ rgba }, st{ st }
        {
        }
        struct offset
        {
            static constexpr std::size_t xyz = 0u;
            static constexpr std::size_t rgba = xyz + sizeof(decltype(standard_vertex::xyz));
            static constexpr std::size_t st = rgba + sizeof(decltype(standard_vertex::rgba));
        };
    };

    template <typename V = standard_vertex>
    class opengl_vertex_buffer : public vertex_buffer, private opengl_buffer_owner<V>
    {
    public:
        typedef V vertex;
    public:
        // Formally this is being far too clever for one's own good as formally this is UB (Undefined Behaviour)
        // as I am treating non-POD (vec3) as POD (by mapping it to OpenGL). May need to rethink this...
        typedef std::vector<vertex, opengl_buffer_allocator<vertex>> vertex_array;
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
                return iParent.iVertices;
            }
            vertex_array& vertices()
            {
                return iParent.iVertices;
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
            opengl_vertex_buffer<V>& iParent;
        };
    public:
        opengl_vertex_buffer(i_vertex_provider& aProvider, vertex_buffer_type aType) :
            vertex_buffer{ aProvider, aType }, iBuffer{ nullptr }
        {
            opengl_buffer_allocator<vertex>::next_owner() = this;
            iVertices.reserve(16384);
        }
    public:
        void attach_shader(i_rendering_context& aContext, i_shader_program& aShaderProgram) override
        {
            iVao.emplace();
            iVertexPositionAttribArray.emplace(
                false,
                sizeof(vertex),
                vertex::offset::xyz,
                aShaderProgram,
                standard_vertex_attribute_name(vertex_buffer_type::Vertices));
            iVertexColorAttribArray.emplace(
                false,
                sizeof(vertex),
                vertex::offset::rgba,
                aShaderProgram, 
                standard_vertex_attribute_name(vertex_buffer_type::Color));
            if (aShaderProgram.supports(vertex_buffer_type::UV))
                iVertexTextureCoordAttribArray.emplace(
                    false, 
                    sizeof(vertex), 
                    vertex::offset::st, 
                    aShaderProgram,
                    standard_vertex_attribute_name(vertex_buffer_type::UV));
            if (aShaderProgram.vertex_shader().has_standard_vertex_matrices())
            {
                auto& standardMatrices = aShaderProgram.vertex_shader().standard_vertex_matrices();
                standardMatrices.set_transformation_matrix(iTransformation);
            }
            vertex_buffer::attach_shader(aContext, aShaderProgram);
            update(*iBuffer);
        }
        void detach_shader() override
        {
            vertex_buffer::detach_shader();
        }
        void execute()
        {
            GLsync sync;
            glCheck(sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
            glCheck(glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, ~0ull));
            glCheck(glDeleteSync(sync));
        }
        void flush(std::size_t aElements)
        {
            if (iBuffer)
                iBuffer->flush(aElements);
        }
        vertex_array& vertices()
        {
            return iVertices;
        }
        std::size_t capacity() const
        {
            return iVertices.capacity();
        }
    private:
        void update(opengl_buffer<vertex>& aBuffer) override
        {
            iBuffer = &aBuffer;
            if (iVao)
                iVao->bind();
            if (iVertexPositionAttribArray)
                iVertexPositionAttribArray->update(aBuffer);
            if (iVertexColorAttribArray)
                iVertexColorAttribArray->update(aBuffer);
            if (iVertexTextureCoordAttribArray)
                iVertexTextureCoordAttribArray->update(aBuffer);
        }
    private:
        vertex_array iVertices;
        opengl_buffer<vertex>* iBuffer;
        optional_mat44 iTransformation;
        std::optional<opengl_vertex_array> iVao;
        std::optional<opengl_vertex_attrib_array<vertex, decltype(vertex::xyz)>> iVertexPositionAttribArray;
        std::optional<opengl_vertex_attrib_array<vertex, decltype(vertex::rgba)>> iVertexColorAttribArray;
        std::optional<opengl_vertex_attrib_array<vertex, decltype(vertex::st)>> iVertexTextureCoordAttribArray;
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
