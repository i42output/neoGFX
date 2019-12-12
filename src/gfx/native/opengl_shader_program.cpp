// opengl_shader_program.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2019 Leigh Johnston.  All Rights Reserved.
  
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

#include "opengl_shader_program.hpp"

namespace neogfx
{
    opengl_shader_program::opengl_shader_program()
    {
        glCheck(iHandle = glCreateProgram());
        if (0 == *iHandle)
            throw failed_to_create_shader_program("Failed to create shader program object");
    }

    bool opengl_shader_program::dirty() const
    {
        return iCode == std::nullopt || shader_program::dirty();
    }

    void opengl_shader_program::set_dirty()
    {
        iCode = std::nullopt;
    }

    void opengl_shader_program::compile()
    {
            neolib::replace_all(aSource, "%FIRST_SHADER_NAME%", aProgram.first_in_stage(*this).name().to_std_string());
            neolib::replace_all(aSource, "%SHADER_NAME%", name().to_std_string());
            neolib::replace_all(aSource, "%NEXT_SHADER_NAME%", aProgram.is_last_in_stage(*this) ? "" : aProgram.next_in_stage(*this).name().to_std_string());

        set_dirty();
        for (auto& s : shaders())
        {
            GLuint shader;
            glCheck(shader = glCreateShader(s.second));
            if (0 == shader)
                throw failed_to_create_shader_program("Failed to create shader object");
            std::string source = s.first;
            if (source.find("uProjectionMatrix") != std::string::npos)
                hasProjectionMatrix = true;
            if (source.find("uTransformationMatrix") != std::string::npos)
                hasTransformationMatrix = true;
            if (renderer() == neogfx::renderer::DirectX)
            {
                std::size_t v;
                const std::size_t VERSION_STRING_LENGTH = 12;
                if ((v = source.find("#version 400")) != std::string::npos)
                    source.replace(v, VERSION_STRING_LENGTH, "#version 110");
                else if ((v = source.find("#version 400")) != std::string::npos)
                    source.replace(v, VERSION_STRING_LENGTH, "#version 110");
            }
            const char* codeArray[] = { source.c_str() };
            glCheck(glShaderSource(shader, 1, codeArray, NULL));
            glCheck(glCompileShader(shader));
            GLint result;
            glCheck(glGetShaderiv(shader, GL_COMPILE_STATUS, &result));
            if (GL_FALSE == result)
            {
                GLint buflen;
                glCheck(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &buflen));
                std::vector<GLchar> buf(buflen);
                glCheck(glGetShaderInfoLog(shader, static_cast<GLsizei>(buf.size()), NULL, &buf[0]));
                std::string error(&buf[0]);
                throw failed_to_create_shader_program(error);
            }
            glCheck(glAttachShader(programHandle, shader));
        }
        shader_program program(programHandle, hasProjectionMatrix, hasTransformationMatrix);
        for (auto& v : aVariables)
            glCheck(glBindAttribLocation(programHandle, program.register_variable(v), v.c_str()));
        auto s = iShaderPrograms.insert(iShaderPrograms.end(), program);
        glCheck(glLinkProgram(programHandle));
        GLint result;
        glCheck(glGetProgramiv(programHandle, GL_LINK_STATUS, &result));
        if (GL_FALSE == result)
            throw failed_to_create_shader_program("Failed to link");
        return s;
    }

    void opengl_shader_program::link()
    {
    }

    void opengl_shader_program::use()
    {
        shader_program::use();
        if (!dirty())
            glCheck(glUseProgram(*iHandle));
    }
}