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
#include <neogfx/gfx/vertex_shader.hpp>
#include <neogfx/gfx/fragment_shader.hpp>

namespace neogfx
{
    opengl_shader_program::opengl_shader_program(const std::string& aName) : 
        shader_program{ aName }
    {
        if (aName == "default_shader_program")
        {
            add_shader(neolib::make_ref<standard_vertex_shader>().as<i_shader>());
            iGradientShader = static_cast<i_gradient_shader&>(add_shader(neolib::make_ref<standard_gradient_shader>().as<i_shader>()));
            iTextureShader = static_cast<i_texture_shader&>(add_shader(neolib::make_ref<standard_texture_shader>().as<i_shader>()));
        }
    }

    const i_gradient_shader& opengl_shader_program::gradient_shader() const
    {
        return *iGradientShader;
    }

    i_gradient_shader& opengl_shader_program::gradient_shader()
    {
        return *iGradientShader;
    }

    const i_texture_shader& opengl_shader_program::texture_shader() const
    {
        return *iTextureShader;
    }

    i_texture_shader& opengl_shader_program::texture_shader()
    {
        return *iTextureShader;
    }

    void opengl_shader_program::compile()
    {
        if (!dirty())
            return;

        for (auto const& stage : stages())
        {
            if (stage_clean(stage.first()))
                continue;
            auto const& shaders = stage.second();
            if (shaders.empty())
                continue;
            string code;
            for (auto const& shader : shaders)
            {
                if (shader->disabled())
                    continue;
                shader->generate_code(*this, shader_language::Glsl, code);
                code.replace_all("%FUNCTIONS%"_s, ""_s);
                code.replace_all("%CODE%"_s, ""_s);
            }
            code.replace_all("%INVOKE_NEXT%"_s, ""_s);
            auto shaderHandle = to_gl_handle<GLuint>(shaders[0]->handle(*this));
            const char* codeArray[] = { code.c_str() };
            glCheck(glShaderSource(shaderHandle, 1, codeArray, NULL));
            glCheck(glCompileShader(shaderHandle));
            GLint result;
            glCheck(glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result));
            if (GL_FALSE == result)
            {
                GLint buflen;
                glCheck(glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &buflen));
                std::vector<GLchar> buf(buflen);
                glCheck(glGetShaderInfoLog(shaderHandle, static_cast<GLsizei>(buf.size()), NULL, &buf[0]));
                std::string error(&buf[0]);
                throw failed_to_create_shader_program(error);
            }
            glCheck(glAttachShader(gl_handle(), shaderHandle));
        }
        
        if (have_stage(shader_type::Vertex))
            for (auto const& vertexShader : stages().at(shader_type::Vertex))
                for (auto const& attribute : static_cast<const i_vertex_shader&>(*vertexShader).attributes())
                    glCheck(glBindAttribLocation(gl_handle(), attribute.second().first(), attribute.first().c_str()));
    }

    void opengl_shader_program::link()
    {
        if (!dirty())
            return;

        glCheck(glLinkProgram(gl_handle()));
        GLint result;
        glCheck(glGetProgramiv(gl_handle(), GL_LINK_STATUS, &result));
        if (GL_FALSE == result)
            throw failed_to_create_shader_program("Failed to link");
    }

    void opengl_shader_program::use()
    {
        glCheck(glUseProgram(gl_handle()));
    }

    void opengl_shader_program::update_uniforms()
    {
        bool const updateAllUniforms = dirty();
        for (auto& stage : stages())
            for (auto& shader : stage.second())
                if (shader->enabled())
                    for (auto& uniform : shader->uniforms())
                    {
                        if (!uniform.second().second() && !updateAllUniforms)
                            continue;
                        GLint location = glGetUniformLocation(gl_handle(), uniform.first().c_str());
                        GLenum errorCode = glGetError();
                        if (errorCode != GL_NO_ERROR)
                            throw shader_program_error(glErrorString(errorCode));
                        std::visit([this, location](auto&& v)
                        {
                            typedef std::decay_t<decltype(v)> data_type;
                            if constexpr (std::is_same_v<bool, data_type>)
                                glCheck(glUniform1i(location, v))
                            else if constexpr (std::is_same_v<float, data_type>)
                                glCheck(glUniform1f(location, v))
                            else if constexpr (std::is_same_v<double, data_type>)
                                glCheck(glUniform1d(location, v))
                            else if constexpr (std::is_same_v<int, data_type>)
                                glCheck(glUniform1i(location, v))
                            else if constexpr (std::is_same_v<vec2f, data_type>)
                                glCheck(glUniform2f(location, v[0], v[1]))
                            else if constexpr (std::is_same_v<vec2, data_type>)
                                glCheck(glUniform2d(location, v[0], v[1]))
                            else if constexpr (std::is_same_v<vec3f, data_type>)
                                glCheck(glUniform3f(location, v[0], v[1], v[2]))
                            else if constexpr (std::is_same_v<vec3, data_type>)
                                glCheck(glUniform3d(location, v[0], v[1], v[2]))
                            else if constexpr (std::is_same_v<vec4f, data_type>)
                                glCheck(glUniform4f(location, v[0], v[1], v[2], v[3]))
                            else if constexpr (std::is_same_v<vec4, data_type>)
                                glCheck(glUniform4d(location, v[0], v[1], v[2], v[3]))
                            else if constexpr (std::is_same_v<mat4f, data_type>)
                                glCheck(glUniformMatrix4fv(location, 1, false, v.data()))
                            else if constexpr (std::is_same_v<mat4, data_type>)
                                glCheck(glUniformMatrix4dv(location, 1, false, v.data()))
                            else if constexpr (std::is_same_v<shader_float_array, data_type>)
                                glCheck(glUniform1fv(location, v.size(), v.data()))
                            else if constexpr (std::is_same_v<shader_double_array, data_type>)
                                glCheck(glUniform1dv(location, v.size(), v.data()))
                            else if constexpr (std::is_same_v<sampler2D, data_type>)
                                glCheck(glUniform1i(location, v.handle))
                            else if constexpr (std::is_same_v<sampler2DMS, data_type>)
                                glCheck(glUniform1i(location, v.handle))
                        }, uniform.second().first());
                    }
    }

    bool opengl_shader_program::active() const
    {
        if (!created())
            return false;
        GLint activeProgram = 0;
        glCheck(glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram));
        return static_cast<GLuint>(activeProgram) == gl_handle();
    }

    void opengl_shader_program::deactivate()
    {
        if (active())
            glCheck(glUseProgram(0));
    }

    GLuint opengl_shader_program::gl_handle() const
    {
        return to_gl_handle<GLuint>(handle());
    }
}