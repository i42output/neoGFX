// opengl_error.hpp
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
#include <GL/glew.h>
#include <GL/GL.h>

namespace neogfx
{
    std::string glErrorString(GLenum aErrorCode);
    GLenum glCheckError(const char* file, unsigned int line);

    class scoped_gl_check
    {
    public:
        scoped_gl_check(const char* file, unsigned int line) : iFile{ file }, iLine{ line }
        {
        }
        ~scoped_gl_check()
        {
            glCheckError(iFile, iLine);
        }
    private:
        const char* const iFile;
        unsigned int const iLine;
    };

    #ifdef glCheck
    #undef glCheck 
    #endif
    #define glCheck(x) { scoped_gl_check sgc{__FILE__, __LINE__}; x; }

    struct opengl_error : std::runtime_error
    {
        opengl_error(std::string const& aMessage) : std::runtime_error("neogfx::opengl_error: " + aMessage) {};
    };
}