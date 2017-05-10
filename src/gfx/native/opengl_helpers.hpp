// opengl_helpers.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
		opengl_buffer(std::size_t aSize, std::size_t aElementSize) :
			iSize{ aSize }, iElementSize { aElementSize	}
		{
			glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &iPreviousBindingHandle));
			glCheck(glGenBuffers(1, &iHandle));
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, iHandle));
			glCheck(glBufferData(GL_ARRAY_BUFFER, iSize * iElementSize * sizeof(value_type), nullptr, GL_DYNAMIC_DRAW));
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
		std::size_t element_size() const
		{
			return iElementSize;
		}
		GLuint handle() const
		{
			return iHandle;
		}
	private:
		const std::size_t iSize;
		const std::size_t iElementSize;
		GLint iPreviousBindingHandle;
		GLuint iHandle;
	};

	template <typename T>
	class opengl_vertex_attrib_array
	{
	public:
		typedef T value_type;
	public:
		opengl_vertex_attrib_array(opengl_buffer<value_type>& aBuffer, const i_rendering_engine::i_shader_program& aShaderProgram, const std::string& aVariableName)
		{
			glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &iPreviousBindingHandle));
			GLuint index = reinterpret_cast<GLuint>(aShaderProgram.variable(aVariableName));
			glCheck(glEnableVertexAttribArray(index));
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, aBuffer.handle()));
			glCheck(glVertexAttribPointer(index, aBuffer.element_size(), std::is_same<value_type, double>() ? GL_DOUBLE : GL_UNSIGNED_BYTE, GL_FALSE, 0, 0));
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
		typedef std::vector<std::array<double, 3>> vertex_array;
		typedef std::vector<std::array<uint8_t, 4>> colour_array;
		typedef std::vector<std::array<double, 2>> texture_coord_array;
	private:
		class buffer_instance
		{
		public:
			buffer_instance(std::size_t aSize) :
				iSize{ aSize },
				iPositionBuffer{ aSize, 3 },
				iColourBuffer{ aSize, 4 },
				iTextureCoordBuffer{ aSize, 2 }
			{
			}
		public:
			std::size_t size() const
			{
				return iSize;
			}
			opengl_buffer<double>& position_buffer()
			{ 
				return iPositionBuffer; 
			}
			opengl_buffer<uint8_t>& colour_buffer()
			{
				return iColourBuffer;
			}
			opengl_buffer<double>& texture_coord_buffer()
			{
				return iTextureCoordBuffer;
			}
		private:
			std::size_t iSize;
			opengl_buffer<double> iPositionBuffer;
			opengl_buffer<uint8_t> iColourBuffer;
			opengl_buffer<double> iTextureCoordBuffer;
		};
		class instance
		{
		public:
			instance(const i_rendering_engine::i_shader_program& aShaderProgram, opengl_buffer<double>& aPositionBuffer, opengl_buffer<uint8_t>& aColourBuffer, opengl_buffer<double>& aTextureCoordBuffer) :
				iVertexPositionAttribArray{ aPositionBuffer, aShaderProgram, "VertexPosition" },
				iVertexColorAttribArray{ aColourBuffer, aShaderProgram, "VertexColor" },
				iVertexTextureCoordAttribArray{ aTextureCoordBuffer, aShaderProgram, "VertexTextureCoord" }
			{
			}
		private:
			opengl_vertex_array iVao;
			opengl_vertex_attrib_array<double> iVertexPositionAttribArray;
			opengl_vertex_attrib_array<uint8_t> iVertexColorAttribArray;
			opengl_vertex_attrib_array<double> iVertexTextureCoordAttribArray;
		};
	public:
		opengl_standard_vertex_arrays() :
			iShaderProgram{ nullptr },
			iBufferInstance{ std::make_unique<buffer_instance>(32) }
		{
		}
	public:
		std::vector<std::array<double, 3>>& vertices()
		{
			return iVertices;
		}
		std::vector<std::array<uint8_t, 4>>& colours()
		{
			return iColours;
		}
		std::vector<std::array<double, 2>>& texture_coords()
		{
			return iTextureCoords;
		}
		void instantiate(const i_rendering_engine::i_shader_program& aShaderProgram)
		{
			if (buffers().size() < vertices().size())
			{
				iInstance.reset();
				iBufferInstance.reset();
				iBufferInstance = std::make_unique<buffer_instance>(vertices().size() * 2);
			}
			void* data;
			glCheck(data = glMapNamedBuffer(buffers().position_buffer().handle(), GL_WRITE_ONLY));
			std::memcpy(data, &vertices()[0][0], vertices().size() * sizeof(vertices()[0]));
			glCheck(glUnmapNamedBuffer(buffers().position_buffer().handle()));
			glCheck(data = glMapNamedBuffer(buffers().colour_buffer().handle(), GL_WRITE_ONLY));
			std::memcpy(data, &colours()[0][0], colours().size() * sizeof(colours()[0]));
			glCheck(glUnmapNamedBuffer(buffers().colour_buffer().handle()));
			glCheck(data = glMapNamedBuffer(buffers().texture_coord_buffer().handle(), GL_WRITE_ONLY));
			std::memcpy(data, &texture_coords()[0][0], texture_coords().size() * sizeof(texture_coords()[0]));
			glCheck(glUnmapNamedBuffer(buffers().texture_coord_buffer().handle()));
			if (iInstance.get() == nullptr || iShaderProgram != &aShaderProgram)
			{
				iShaderProgram = &aShaderProgram;
				iInstance.reset();
				iInstance = std::make_unique<instance>(aShaderProgram, buffers().position_buffer(), buffers().colour_buffer(), buffers().texture_coord_buffer());
			}
		}
	private:
		buffer_instance& buffers()
		{
			return *iBufferInstance;
		}
	private:
		const i_rendering_engine::i_shader_program* iShaderProgram;
		std::unique_ptr<buffer_instance> iBufferInstance;
		std::unique_ptr<instance> iInstance;
		std::vector<std::array<double, 3>> iVertices;
		std::vector<std::array<uint8_t, 4>> iColours;
		std::vector<std::array<double, 2>> iTextureCoords;
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
