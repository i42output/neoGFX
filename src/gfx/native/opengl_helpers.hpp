// opengl_helpers.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include "opengl.hpp"
#include "i_native_graphics_context.hpp"

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
			glCheck(glBufferStorage(GL_ARRAY_BUFFER, size() * sizeof(value_type), nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT));
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
				glCheck(iMemory = static_cast<value_type*>(glMapNamedBufferRange(handle(), 0, size() * sizeof(value_type), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT)));
			}
			return iMemory;
		}
		void unmap()
		{
			if (iMemory != nullptr)
			{
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

	template <typename T>
	class opengl_buffer_allocator_arena
	{
	public:
		typedef opengl_buffer<T> buffer_type;
		typedef std::unique_ptr<buffer_type> buffer_pointer;
		typedef std::map<void*, buffer_pointer> arena_map_type;
	public:
		static arena_map_type& arena_map()
		{
			thread_local arena_map_type tArenaMap;
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
		typedef opengl_buffer_allocator_arena<OriginalType>::buffer_type buffer_type;
		typedef opengl_buffer_allocator_arena<OriginalType>::buffer_pointer buffer_pointer;
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
		typedef Vertex vertex_type;
		typedef Attrib attribute_type;
		typedef typename attribute_type::value_type value_type;
		static constexpr std::size_t arity = sizeof(attribute_type) / sizeof(value_type);
	public:
		template <typename Buffer>
		opengl_vertex_attrib_array(Buffer& aBuffer, bool aNormalized, std::size_t aOffset, const i_rendering_engine::i_shader_program& aShaderProgram, const std::string& aVariableName)
		{
			glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &iPreviousBindingHandle));
			GLuint index = reinterpret_cast<GLuint>(aShaderProgram.variable(aVariableName));
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, aBuffer.handle()));
			glCheck(glVertexAttribPointer(
				index, 
				arity, 
				opengl_attrib_data_type<value_type>::type,
				aNormalized ? GL_TRUE : GL_FALSE,
				sizeof(vertex_type),
				reinterpret_cast<const GLvoid*>(aOffset)));
			glCheck(glEnableVertexAttribArray(index));
		}
		~opengl_vertex_attrib_array()
		{
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, iPreviousBindingHandle));
		}
	private:
		GLint iPreviousBindingHandle;
	};

	class opengl_standard_vertex_arrays
	{
	public:
		// Formally this is being far too clever for one's own good as formally this is UB (Undefined Behaviour)
		// as I am treating non-POD (vec3) as POD (by mapping it to OpenGL). May need to rethink this...
		struct vertex
		{
			vec3 xyz;
			std::array<uint8_t, 4> rgba;
			vec2f st;
			vertex(const vec3& xyz = vec3{}, const std::array<uint8_t, 4>& rgba = std::array<uint8_t, 4>{}, const vec2f& st = vec2f{}) :
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
	private:
		class instance
		{
		public:
			instance(const i_rendering_engine::i_shader_program& aShaderProgram, 
				const opengl_buffer<vertex_array::value_type>& aVertexBuffer) :
				iCapacity{ aVertexBuffer.size() },
				iVertexPositionAttribArray{ aVertexBuffer, false, vertex::offset::xyz, aShaderProgram, "VertexPosition" },
				iVertexColorAttribArray{ aVertexBuffer, false, vertex::offset::rgba, aShaderProgram, "VertexColor" },
				iVertexTextureCoordAttribArray{ aVertexBuffer, false, vertex::offset::st, aShaderProgram, "VertexTextureCoord" }
			{
			}
		public:
			std::size_t capacity() const
			{
				return iCapacity;
			}
		private:
			std::size_t iCapacity;
			opengl_vertex_array iVao;
			opengl_vertex_attrib_array<vertex, decltype(vertex::xyz)> iVertexPositionAttribArray;
			opengl_vertex_attrib_array<vertex, decltype(vertex::rgba)> iVertexColorAttribArray;
			opengl_vertex_attrib_array<vertex, decltype(vertex::st)> iVertexTextureCoordAttribArray;
		};
	public:
		opengl_standard_vertex_arrays() :
			iShaderProgram{ nullptr }
		{
			vertices().reserve(4096);
		}
	public:
		vertex_array& vertices()
		{
			return iVertices;
		}
		void instantiate(i_native_graphics_context& aGraphicsContext, i_rendering_engine::i_shader_program& aShaderProgram)
		{
			if (iInstance.get() == nullptr || iInstance->capacity() != iVertices.capacity() || iShaderProgram != &aShaderProgram)
			{
				iShaderProgram = &aShaderProgram;
				iInstance.reset();
				iInstance = std::make_unique<instance>(aShaderProgram, vertex_array::allocator_type::buffer(&iVertices[0]));
			}
			if (iShaderProgram->has_projection_matrix())
				iShaderProgram->set_projection_matrix(aGraphicsContext);
		}
	private:
		i_rendering_engine::i_shader_program* iShaderProgram;
		std::unique_ptr<instance> iInstance;
		vertex_array iVertices;
	};

	class use_shader_program
	{
	public:
		use_shader_program(i_native_graphics_context& aGraphicsContext, i_rendering_engine& aRenderingEngine, i_rendering_engine::i_shader_program& aShaderProgram) :
			iGraphicsContext{ aGraphicsContext },
			iRenderingEngine{ aRenderingEngine },
			iPreviousProgram{ aRenderingEngine.shader_program_active() ? &aRenderingEngine.active_shader_program() : nullptr }
		{
			iRenderingEngine.activate_shader_program(iGraphicsContext, aShaderProgram);
		}
		~use_shader_program()
		{
			if (iPreviousProgram != nullptr)
				iRenderingEngine.activate_shader_program(iGraphicsContext, *iPreviousProgram);
		}
	private:
		i_native_graphics_context& iGraphicsContext;
		i_rendering_engine& iRenderingEngine;
		i_rendering_engine::i_shader_program* iPreviousProgram;
	};
}
