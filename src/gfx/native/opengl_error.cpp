// opengl_error.cpp
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

#include <neogfx/neogfx.hpp>
#include <iostream>
#include <neolib/string_utils.hpp>
#include "opengl_error.hpp"

namespace
{
#define MAKE_GL_ERROR_STRING(x) {x, #x}
	const std::map<GLenum, std::string> GL_ERRORS = 
	{
		MAKE_GL_ERROR_STRING(GL_INVALID_ENUM),
		MAKE_GL_ERROR_STRING(GL_INVALID_VALUE),
		MAKE_GL_ERROR_STRING(GL_INVALID_OPERATION),
		MAKE_GL_ERROR_STRING(GL_STACK_OVERFLOW),
		MAKE_GL_ERROR_STRING(GL_STACK_UNDERFLOW),
		MAKE_GL_ERROR_STRING(GL_OUT_OF_MEMORY),
		MAKE_GL_ERROR_STRING(GL_INVALID_FRAMEBUFFER_OPERATION),
		MAKE_GL_ERROR_STRING(GL_FRAMEBUFFER_UNSUPPORTED),
		MAKE_GL_ERROR_STRING(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT),
		MAKE_GL_ERROR_STRING(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT),
/*		MAKE_GL_ERROR_STRING(GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT),
		MAKE_GL_ERROR_STRING(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS),
		MAKE_GL_ERROR_STRING(GL_FRAMEBUFFER_INCOMPLETE_FORMATS), */
		MAKE_GL_ERROR_STRING(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER),
		MAKE_GL_ERROR_STRING(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER),
		MAKE_GL_ERROR_STRING(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
	};
}

std::string glErrorString(GLenum aErrorCode)
{
	if (GL_ERRORS.find(aErrorCode) != GL_ERRORS.end())
		return GL_ERRORS.find(aErrorCode)->second;
	else
		return "Unknwon error, code: 0x" + neolib::integer_to_string<char>(aErrorCode, 16);
}

GLenum glCheckError(const char* file, unsigned int line)
{
	// Get the last error
	GLenum errorCode = glGetError();

	if (errorCode != GL_NO_ERROR)
	{
		std::string fileString(file);
		std::string error = glErrorString(errorCode);
		std::string errorMessage = "An internal OpenGL call failed in " +
			fileString.substr(fileString.find_last_of("\\/") + 1) + " (" + neolib::unsigned_integer_to_string<char>(line) + ") : " +
			error;			
		std::cerr << "neogfx (OpenGL): " << errorMessage << std::endl;
		throw neogfx::opengl_error(errorMessage);
	}

	return errorCode;
}
