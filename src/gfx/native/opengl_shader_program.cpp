// opengl_shader_program.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2019, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/set.hpp>
#include "opengl_shader_program.hpp"

namespace neogfx
{
    opengl_shader_program::opengl_shader_program(const std::string& aName) : 
        standard_shader_program{ aName }
    {
    }

    void opengl_shader_program::compile()
    {
        if (!dirty())
            return;

        GLint attachedShaderCount = 0;
        glCheck(glGetProgramiv(gl_handle(), GL_ATTACHED_SHADERS, &attachedShaderCount));
        std::vector<GLuint> attachedShaderHandles;
        attachedShaderHandles.resize(attachedShaderCount);
        if (attachedShaderCount != 0)
            glCheck(glGetAttachedShaders(gl_handle(), attachedShaderCount, NULL, &attachedShaderHandles[0]));

        for (auto const& stage : stages())
        {
            if (stage_clean(stage.first()))
                continue;
            auto const& shaders = stage.second();
            if (shaders.empty())
                continue;
            string code;
            string invokeDeclarations;
            string invokes;
            string invokeResults;
            neolib::set<shader_variable> ins;
            neolib::set<shader_variable> outs;
            for (auto const& shader : shaders)
            {
                if (shader->disabled())
                    continue;
                code += "\n"_s;
                for (auto const& in : shader->in_variables())
                    ins.insert(in);
                for (auto const& out : shader->out_variables())
                    outs.insert(out);
                shader->generate_code(*this, shader_language::Glsl, code);
                shader->generate_invoke(*this, shader_language::Glsl, invokes);
            }
            for (auto const& out : outs)
            {
                invokeDeclarations += "    "_s + enum_to_string<shader_data_type>(out.type()) + " arg"_s + out.name() + " = "_s + out.link().name() + ";\n"_s;
                invokeResults += "    "_s + out.name() + " = arg"_s + out.name() + ";\n"_s;
            }
            static const string mainFunction =
            {
                "\n"
                "void main()\n"
                "{\n"
                "%INVOKES%"
                "}\n"_s
            };
            code += mainFunction;
            code.replace_all("%INVOKES%"_s, invokeDeclarations + invokes + invokeResults);
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
            if (std::find(attachedShaderHandles.begin(), attachedShaderHandles.end(), 
                shaderHandle) == attachedShaderHandles.end())
                glCheck(glAttachShader(gl_handle(), shaderHandle));
        }
    }

    void opengl_shader_program::link()
    {
        if (!dirty())
            return;

        glCheck(glLinkProgram(gl_handle()));
        GLint result;
        glCheck(glGetProgramiv(gl_handle(), GL_LINK_STATUS, &result));
        if (GL_FALSE == result)
        {
            GLint buflen;
            glCheck(glGetProgramiv(gl_handle(), GL_INFO_LOG_LENGTH, &buflen));
            std::vector<GLchar> buf(buflen);
            glCheck(glGetProgramInfoLog(gl_handle(), static_cast<GLsizei>(buf.size()), NULL, &buf[0]));
            std::string error(&buf[0]);
            throw failed_to_create_shader_program(error);
        }
    }

    void opengl_shader_program::use()
    {
        if (!active())
        {
            glCheck(glUseProgram(gl_handle()));
            set_active();
        }
    }

    void opengl_shader_program::update_uniform_locations()
    {
        for (auto& stage : stages())
            for (auto& shader : stage.second())
                for (auto& uniform : shader->uniforms())
                {
                    shader->update_uniform_location(uniform.id(), glGetUniformLocation(gl_handle(), uniform.name().c_str()));
                    GLenum errorCode = glGetError();
                    if (errorCode != GL_NO_ERROR)
                        throw shader_program_error(glErrorString(errorCode));
                }
    }

    void opengl_shader_program::update_uniforms(const i_rendering_context& aContext)
    {
        prepare_uniforms(aContext);
        bool const updateAllUniforms = need_full_uniform_update();
        for (auto& stage : stages())
            for (auto& shader : stage.second())
                if (shader->enabled())
                    for (auto& uniform : shader->uniforms())
                    {
                        if (!uniform.is_dirty() && !updateAllUniforms)
                            continue;
                        if (uniform.value().empty())
                            continue;
                        uniform.clean();
                        auto const location = uniform.location();
                        std::visit([this, location](auto&& v)
                        {
                            typedef std::decay_t<decltype(v)> data_type;
                            if constexpr (std::is_same_v<bool, data_type>)
                                glCheck(glUniform1i(location, v))
                            else if constexpr (std::is_same_v<float, data_type>)
                                glCheck(glUniform1f(location, v))
                            else if constexpr (std::is_same_v<double, data_type>)
                                glCheck(glUniform1d(location, v))
                            else if constexpr (std::is_same_v<int32_t, data_type>)
                                glCheck(glUniform1i(location, v))
                            else if constexpr (std::is_same_v<uint32_t, data_type>)
                                glCheck(glUniform1ui(location, v))
                            else if constexpr (std::is_same_v<vec2f, data_type>)
                                glCheck(glUniform2f(location, v[0], v[1]))
                            else if constexpr (std::is_same_v<vec2, data_type>)
                                glCheck(glUniform2d(location, v[0], v[1]))
                            else if constexpr (std::is_same_v<vec2i32, data_type>)
                                glCheck(glUniform2i(location, v[0], v[1]))
                            else if constexpr (std::is_same_v<vec2u32, data_type>)
                                glCheck(glUniform2ui(location, v[0], v[1]))
                            else if constexpr (std::is_same_v<vec3f, data_type>)
                                glCheck(glUniform3f(location, v[0], v[1], v[2]))
                            else if constexpr (std::is_same_v<vec3, data_type>)
                                glCheck(glUniform3d(location, v[0], v[1], v[2]))
                            else if constexpr (std::is_same_v<vec3i32, data_type>)
                                glCheck(glUniform3i(location, v[0], v[1], v[2]))
                            else if constexpr (std::is_same_v<vec3u32, data_type>)
                                glCheck(glUniform3ui(location, v[0], v[1], v[2]))
                            else if constexpr (std::is_same_v<vec4f, data_type>)
                                glCheck(glUniform4f(location, v[0], v[1], v[2], v[3]))
                            else if constexpr (std::is_same_v<vec4, data_type>)
                                glCheck(glUniform4d(location, v[0], v[1], v[2], v[3]))
                            else if constexpr (std::is_same_v<vec4i32, data_type>)
                                glCheck(glUniform4i(location, v[0], v[1], v[2], v[3]))
                            else if constexpr (std::is_same_v<vec4u32, data_type>)
                                glCheck(glUniform4ui(location, v[0], v[1], v[2], v[3]))
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
                            else if constexpr (std::is_same_v<sampler2DRect, data_type>)
                                glCheck(glUniform1i(location, v.handle))
                        }, uniform.value());
                    }
    }

    void opengl_shader_program::deactivate()
    {
        if (active())
        {
            glCheck(glUseProgram(0));
            set_inactive();
        }
    }

    GLuint opengl_shader_program::gl_handle() const
    {
        return to_gl_handle<GLuint>(handle());
    }
}