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

	class opengl_buffer
	{
	public:
		template <typename ContainerT>
		opengl_buffer(const ContainerT& aData) :
			iElementSize{ aData[0].size() }
		{
			glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &iPreviousBindingHandle));
			glCheck(glGenBuffers(1, &iHandle));
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, iHandle));
			glCheck(glBufferData(GL_ARRAY_BUFFER, aData.size() * sizeof(aData[0]), &aData[0], GL_STATIC_DRAW));
		}
		~opengl_buffer()
		{
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, iPreviousBindingHandle));
			glCheck(glDeleteBuffers(1, &iHandle));
		}
	public:
		std::size_t element_size() const
		{
			return iElementSize;
		}
		GLuint handle() const
		{
			return iHandle;
		}
	private:
		const std::size_t iElementSize;
		GLint iPreviousBindingHandle;
		GLuint iHandle;
	};

	class opengl_vertex_attrib_array
	{
	public:
		opengl_vertex_attrib_array(opengl_buffer& aBuffer, const i_rendering_engine::i_shader_program& aShaderProgram, const std::string& aVariableName)
		{
			glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &iPreviousBindingHandle));
			GLuint index = reinterpret_cast<GLuint>(aShaderProgram.variable(aVariableName));
			glCheck(glEnableVertexAttribArray(index));
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, aBuffer.handle()));
			glCheck(glVertexAttribPointer(index, aBuffer.element_size(), GL_DOUBLE, GL_FALSE, 0, 0));
		}
		~opengl_vertex_attrib_array()
		{
			glCheck(glBindBuffer(GL_ARRAY_BUFFER, iPreviousBindingHandle));
		}
	private:
		GLint iPreviousBindingHandle;
	};
}
