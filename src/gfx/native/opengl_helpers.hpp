// opengl_helpers.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
    private:
        GLint iPreviousVertexArrayBindingHandle;
        GLuint iHandle;
    };

    template <typename T>
    class opengl_buffer
    {
    public:
        typedef T value_type;
    public:
        opengl_buffer(std::size_t aSize) :
            iSize{ aSize }, iMemory{ nullptr }
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
    private:
        const std::size_t iSize;
        GLint iPreviousBindingHandle;
        GLuint iHandle;
        value_type* iMemory;
    };

    typedef std::vector<std::function<void()>> opengl_buffer_cleanup_functions_type;

    inline opengl_buffer_cleanup_functions_type& opengl_buffer_cleanup_functions()
    {
        static opengl_buffer_cleanup_functions_type sCleanupFunctions;
        return sCleanupFunctions;
    }

    inline void opengl_buffer_cleanup()
    {
        for (auto const& cleanupFunction : opengl_buffer_cleanup_functions())
            cleanupFunction();
    }

    template <typename T>
    class opengl_buffer_allocator_arena
    {
    public:
        typedef opengl_buffer<T> buffer_type;
        typedef std::unique_ptr<buffer_type> buffer_pointer;
        typedef std::map<void*, buffer_pointer> arena_map_type;
        typedef std::vector<std::function<void()>> cleanup_functions_type;
    public:
        static arena_map_type& arena_map()
        {
            thread_local arena_map_type tArenaMap;
            arena_map_type* const capturedMap = &tArenaMap;
            opengl_buffer_cleanup_functions().push_back([capturedMap]() { capturedMap->clear(); });
            return tArenaMap;
        }
    };

    template <typename T, typename OriginalType = T>
    class opengl_buffer_allocator : public std::allocator<T>, private opengl_buffer_allocator_arena<OriginalType>
    {
        template <typename, typename>
        friend class opengl_buffer_allocator;
    public:
        template<class Other>
        struct rebind
        {    
            typedef opengl_buffer_allocator<Other, OriginalType> other;
        };
    private:
        typedef typename opengl_buffer_allocator_arena<OriginalType>::buffer_type buffer_type;
        typedef typename opengl_buffer_allocator_arena<OriginalType>::buffer_pointer buffer_pointer;
    public:
        struct no_opengl_buffer : std::logic_error { no_opengl_buffer() : std::logic_error{ "neogfx::opengl_buffer_allocator::no_opengl_buffer" } {} };
        struct arena_not_found : std::logic_error { arena_not_found() : std::logic_error{ "neogfx::opengl_buffer_allocator::arena_not_found" } {} };
    private:
        using opengl_buffer_allocator_arena<OriginalType>::arena_map;
    public:
        opengl_buffer_allocator()
        {
        }
        opengl_buffer_allocator(const opengl_buffer_allocator<T, OriginalType>&)
        {
        }
        template <typename T2>
        opengl_buffer_allocator(const opengl_buffer_allocator<T2, OriginalType>&)
        {
        }
    public:
        void deallocate(pointer aPointer, size_type)
        {
            auto existingArena = arena_map().find(aPointer);
            if (existingArena == arena_map().end())
                throw arena_not_found();
            arena_map().erase(existingArena);
        }
        pointer allocate(size_type aCount)
        {    
            buffer_pointer newBuffer = std::make_unique<buffer_type>(aCount);
            auto iterNewBuffer = arena_map().emplace(newBuffer->map(), std::move(newBuffer)).first;
            return static_cast<pointer>(iterNewBuffer->first);
        }
        pointer allocate(size_type aCount, const void *)
        {
            return allocate(aCount);
        }
    public:
        static buffer_type& buffer(void* aMemory)
        {
            return *arena_map().find(aMemory)->second;
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
        template <typename Buffer>
        opengl_vertex_attrib_array(Buffer& aBuffer, bool aNormalized, std::size_t aStride, std::size_t aOffset, const i_shader_program& aShaderProgram, const std::string& aVariableName)
        {
            GLint previousBindingHandle;
            glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBindingHandle));
            if (previousBindingHandle != gl_handle_cast<GLint>(aBuffer.handle()))
                glCheck(glBindBuffer(GL_ARRAY_BUFFER, aBuffer.handle()));
            GLuint index;
            glCheck(index = glGetAttribLocation(to_gl_handle<GLuint>(aShaderProgram.handle()), aVariableName.c_str()));
            if (index != -1)
            {
                glCheck(glVertexAttribPointer(
                    index,
                    static_cast<GLint>(arity),
                    opengl_attrib_data_type<value_type>::type,
                    aNormalized ? GL_TRUE : GL_FALSE,
                    static_cast<GLsizei>(aStride),
                    reinterpret_cast<const GLvoid*>(aOffset)));
                glCheck(glEnableVertexAttribArray(index));
            }
            if (previousBindingHandle != gl_handle_cast<GLint>(aBuffer.handle()))
                glCheck(glBindBuffer(GL_ARRAY_BUFFER, previousBindingHandle));
        }
        ~opengl_vertex_attrib_array()
        {
        }
    };

    inline vec4f colour_to_vec4f(const std::array<uint8_t, 4>& aSource)
    {
        return vec4f{{ aSource[0] / 255.0f, aSource[1] / 255.0f, aSource[2] / 255.0f, aSource[3] / 255.0f }};
    }

    class opengl_standard_vertex_arrays
    {
    public:
        // Formally this is being far too clever for one's own good as formally this is UB (Undefined Behaviour)
        // as I am treating non-POD (vec3) as POD (by mapping it to OpenGL). May need to rethink this...
        struct vertex
        {
            vec3f xyz;
            vec4f rgba;
            vec2f st;
            vertex(const vec3f& xyz = vec3f{}) :
                xyz{ xyz }
            {
            }
            vertex(const vec3f& xyz, const vec4f& rgba, const vec2f& st = vec2f{}) :
                xyz{ xyz }, rgba{ rgba }, st{ st }
            {
            }
            struct offset
            {
                static constexpr std::size_t xyz = 0u;
                static constexpr std::size_t rgba = xyz + sizeof(decltype(vertex::xyz));
                static constexpr std::size_t st = rgba + sizeof(decltype(vertex::rgba));
            };
        };
        typedef std::vector<vertex, opengl_buffer_allocator<vertex>> vertex_array;
        class use
        {
        public:
            use(opengl_standard_vertex_arrays& aParent) : iParent{ aParent }
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
            opengl_standard_vertex_arrays& iParent;
        };
    private:
        class instance
        {
        public:
            instance(const i_shader_program& aShaderProgram, 
                opengl_buffer<vertex_array::value_type>& aVertexBuffer, bool aWithTextureCoords) :
                iVertexBuffer{ aVertexBuffer },
                iCapacity{ aVertexBuffer.size() },
                iVertexPositionAttribArray{ aVertexBuffer, false, sizeof(vertex), vertex::offset::xyz, aShaderProgram, "VertexPosition" },
                iVertexColorAttribArray{ aVertexBuffer, false, sizeof(vertex), vertex::offset::rgba, aShaderProgram, "VertexColor" }
            {
                if (aWithTextureCoords)
                    iVertexTextureCoordAttribArray.emplace(aVertexBuffer, false, sizeof(vertex), vertex::offset::st, aShaderProgram, "VertexTextureCoord");
            }
        public:
            std::size_t capacity() const
            {
                return iCapacity;
            }
            bool has_texture_coords() const
            {
                return iVertexTextureCoordAttribArray != std::nullopt;
            }
            void flush_buffer(std::size_t aElements)
            {
                iVertexBuffer.flush(aElements);
            }
        private:
            opengl_buffer<vertex_array::value_type>& iVertexBuffer;
            std::size_t iCapacity;
            opengl_vertex_array iVao;
            opengl_vertex_attrib_array<vertex, decltype(vertex::xyz)> iVertexPositionAttribArray;
            opengl_vertex_attrib_array<vertex, decltype(vertex::rgba)> iVertexColorAttribArray;
            std::optional<opengl_vertex_attrib_array<vertex, decltype(vertex::st)>> iVertexTextureCoordAttribArray;
        };
    public:
        opengl_standard_vertex_arrays() :
            iShaderProgram{ nullptr }
        {
            iVertices.reserve(16384);
        }
    public:
        void instantiate(i_rendering_context& aContext, i_shader_program& aShaderProgram)
        {
            do_instantiate(aContext, aShaderProgram, false);
        }
        void instantiate_with_texture_coords(i_rendering_context& aContext, i_shader_program& aShaderProgram)
        {
            do_instantiate(aContext, aShaderProgram, true);
        }
        void execute()
        {
            GLsync sync;
            glCheck(sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
            glCheck(glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, ~0ull));
            glCheck(glDeleteSync(sync));
        }
        std::size_t capacity() const
        {
            return iVertices.capacity();
        }
    private:
        void do_instantiate(i_rendering_context& aContext, i_shader_program& aShaderProgram, bool aWithTextureCoords)
        {
            if (iInstance.get() == nullptr || iInstance->capacity() < iVertices.capacity() || iShaderProgram != &aShaderProgram || iInstance->has_texture_coords() != aWithTextureCoords)
            {
                iShaderProgram = &aShaderProgram;
                iInstance.reset();
                iInstance = std::make_unique<instance>(aShaderProgram, vertex_array::allocator_type::buffer(&iVertices[0]), aWithTextureCoords);
            }
            //iInstance->flush_buffer(iVertices.size());
            if (aShaderProgram.vertex_shader().has_standard_vertex_matrices())
            {
                auto& standardMatrices = aShaderProgram.vertex_shader().standard_vertex_matrices();
                standardMatrices.set_transformation_matrix(iTransformation);
            }
        }
    private:
        i_shader_program* iShaderProgram;
        std::unique_ptr<instance> iInstance;
        vertex_array iVertices;
        optional_mat44 iTransformation;
    };

    class use_shader_program
    {
    public:
        use_shader_program(i_rendering_context& aContext, i_shader_program& aShaderProgram, const optional_mat44& aProjectionMatrix = optional_mat44{}, const optional_mat44& aTransformationMatrix = optional_mat44{}) :
            iRenderingContext{ aContext },
            iCurrentProgram{ aShaderProgram },
            iPreviousProgram{ service<i_rendering_engine>().is_shader_program_active() ? &service<i_rendering_engine>().active_shader_program() : nullptr }
        {
            iCurrentProgram.activate(aContext);
        }
        ~use_shader_program()
        {
            if (&iCurrentProgram != iPreviousProgram)
            {
                iCurrentProgram.deactivate();
                if (iPreviousProgram != nullptr)
                    iPreviousProgram->activate(iRenderingContext);
            }
            if (iCurrentProgram.type() == shader_program_type::Standard)
            {
                iCurrentProgram.as<i_standard_shader_program>().gradient_shader().clear_gradient();
                iCurrentProgram.as<i_standard_shader_program>().texture_shader().clear_texture();
                iCurrentProgram.as<i_standard_shader_program>().glyph_shader().clear_glyph();
                iCurrentProgram.as<i_standard_shader_program>().stipple_shader().clear_stipple();
            }
        }
    private:
        i_rendering_context& iRenderingContext;
        i_shader_program& iCurrentProgram;
        i_shader_program* iPreviousProgram;
    };
}
