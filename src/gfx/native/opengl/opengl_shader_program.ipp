// opengl_shader_program.ipp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2019, 2020, 2024 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/set.hpp>

#include "opengl_shader_program.hpp"

namespace neogfx
{
    template <typename Base>
    inline basic_opengl_shader_program<Base>::basic_opengl_shader_program(std::string const& aName) :
        base_type{ aName }
    {
        if constexpr (std::is_base_of_v<i_standard_shader_program, Base>)
            this->create_standard_shaders();
    }

    template <typename Base>
    basic_opengl_shader_program<Base>::~basic_opengl_shader_program()
    {
        this->stages().clear();
    }

    template <typename Base>
    inline void basic_opengl_shader_program<Base>::compile()
    {
        if (!this->dirty())
            return;

        GLint attachedShaderCount = 0;
        glCheck(glGetProgramiv(gl_handle(), GL_ATTACHED_SHADERS, &attachedShaderCount));
        std::vector<GLuint> attachedShaderHandles;
        attachedShaderHandles.resize(attachedShaderCount);
        if (attachedShaderCount != 0)
            glCheck(glGetAttachedShaders(gl_handle(), attachedShaderCount, NULL, &attachedShaderHandles[0]));

        for (auto const& stage : this->stages())
        {
            if (this->stage_clean(stage->type()))
                continue;
            auto const& shaders = stage->shaders();
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
            auto dump = [&](std::string const& why)
            {
                service<debug::logger>() << neolib::logger::severity::Debug << why << std::endl;
                std::int32_t lineNumber = 1;
                std::istringstream iss{ code.to_std_string() };
                std::string line;
                while (std::getline(iss, line))
                    service<debug::logger>() << neolib::logger::severity::Debug << lineNumber++ << ": " << line << std::endl;
            };
            if (GL_FALSE == result)
            {
                dump("Shader compilation error; shader code dump:-");
                GLint buflen;
                glCheck(glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &buflen));
                std::vector<GLchar> buf(buflen);
                glCheck(glGetShaderInfoLog(shaderHandle, static_cast<GLsizei>(buf.size()), NULL, &buf[0]));
                std::string error(&buf[0]);
                throw failed_to_create_shader_program(error);
            }
            else
            {
#ifndef NDEBUG
                dump("Shader code dump:-");
#endif
            }
            if (std::find(attachedShaderHandles.begin(), attachedShaderHandles.end(), 
                shaderHandle) == attachedShaderHandles.end())
                glCheck(glAttachShader(gl_handle(), shaderHandle));
        }
    }

    template <typename Base>
    inline void basic_opengl_shader_program<Base>::link()
    {
        if (!this->dirty())
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

    template <typename Base>
    inline void basic_opengl_shader_program<Base>::use()
    {
        if (!this->active())
        {
            glCheck(glUseProgram(gl_handle()));
            this->set_active();
        }
    }

    template <typename Base>
    inline void basic_opengl_shader_program<Base>::update_uniform_storage()
    {
        for (auto& stage : this->stages())
        {
            if (stage->shaders().empty())
                continue;

            GLuint const uniformBlockIndex = glGetUniformBlockIndex(gl_handle(), (enum_to_string(stage->type()) + "Uniforms").c_str());
            GLint uniformBlockSize;
            glCheck(glGetActiveUniformBlockiv(gl_handle(), uniformBlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockSize))
            ubo_block_buffer(stage->type()).resize(uniformBlockSize);

            thread_local std::vector<GLchar const*> uniformNames;
            thread_local std::vector<std::pair<i_shader*, i_shader_uniform const*>> uniformRefs;

            uniformNames.clear();
            uniformRefs.clear();

            for (auto& shader : stage->shaders())
                if (shader->enabled() || shader->has_shared_uniforms())
                    for (auto& uniform : shader->uniforms())
                        if (!uniform.singular() && (shader->enabled() || uniform.shared()))
                        {
                            uniformNames.push_back(uniform.name().c_str());
                            uniformRefs.emplace_back(&*shader, &uniform);
                        }

            thread_local std::vector<GLuint> uniformIndices;
            uniformIndices.resize(uniformNames.size());
            glCheck(glGetUniformIndices(gl_handle(), static_cast<GLsizei>(uniformNames.size()), uniformNames.data(), uniformIndices.data()))

            thread_local std::vector<GLint> uniformOffsets;
            uniformOffsets.resize(uniformIndices.size());
            glCheck(glGetActiveUniformsiv(gl_handle(), static_cast<GLsizei>(uniformIndices.size()), uniformIndices.data(), GL_UNIFORM_OFFSET, uniformOffsets.data()))

            for (auto uniformRef = uniformRefs.begin(); uniformRef != uniformRefs.end(); ++uniformRef)
                (*uniformRef).first->update_uniform_storage((*uniformRef).second->id(), std::next(ubo_block_buffer(stage->type()).data(), uniformOffsets[std::distance(uniformRefs.begin(), uniformRef)]));
        }
    }

    template <typename Base>
    inline void basic_opengl_shader_program<Base>::update_uniform_locations()
    {
        for (auto& stage : this->stages())
            for (auto& shader : stage->shaders())
                for (auto& uniform : shader->uniforms())
                    if (uniform.singular())
                    {
                        shader->update_uniform_location(uniform.id(), glGetUniformLocation(gl_handle(), uniform.name().c_str()));
                        GLenum errorCode = glGetError();
                        if (errorCode != GL_NO_ERROR)
                            throw shader_program_error(glErrorString(errorCode));
                    }
    }

    template <typename Base>
    inline void basic_opengl_shader_program<Base>::update_uniforms(const i_rendering_context& aContext)
    {
        this->prepare_uniforms(aContext);

        bool const updateAllUniforms = this->need_full_uniform_update();

        for (auto& stage : this->stages())
        {
            glCheck(glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle(stage->type())))
            
            auto& blockBuffer = ubo_block_buffer(stage->type());
            
            for (auto& shader : stage->shaders())
                if (shader->enabled() || shader->has_shared_uniforms())
                    for (auto& uniform : shader->uniforms())
                    {
                        if (!shader->enabled() && !uniform.shared())
                            continue;
                        if (!uniform.is_dirty() && !updateAllUniforms)
                            continue;
                        if (uniform.value().empty())
                            continue;
                        uniform.clean();
                        if (!uniform.singular())
                        {
                            auto const storage = uniform.storage();
                            std::visit([this, &blockBuffer, storage, updateAllUniforms](auto&& v)
                            {
                                typedef std::decay_t<decltype(v)> data_type;
                                void const* src = &v;
                                auto dataOffset = static_cast<GLubyte const*>(storage) - blockBuffer.data();
                                auto dataSize = sizeof(data_type);
                                if constexpr (
                                    std::is_same_v<bool, data_type>)
                                {
                                    thread_local std::int32_t boolean;
                                    boolean = v;
                                    src = &boolean;
                                    dataSize = sizeof(boolean);
                                }
                                else if constexpr (
                                    std::is_same_v<mat4f, data_type> ||
                                    std::is_same_v<mat4, data_type>)
                                {
                                    src = v.data();
                                    dataSize = sizeof(v[0][0]) * 4 * 4;
                                }
                                else if constexpr (
                                    std::is_same_v<abstract_t<shader_float_array>, data_type> || 
                                    std::is_same_v<abstract_t<shader_double_array>, data_type>)
                                {
                                    src = v.data();
                                    dataSize = sizeof(v[0]) * v.size();
                                }
                                std::memcpy(storage, src, dataSize);
                                if (!updateAllUniforms)
                                    glCheck(glBufferSubData(GL_UNIFORM_BUFFER, dataOffset, dataSize, storage));
                            }, uniform.value());
                        }
                        else
                        {
                            auto const location = uniform.location();
                            std::visit([this, location](auto&& v)
                            {
                                typedef std::decay_t<decltype(v)> data_type;
                                if constexpr (std::is_same_v<sampler2D, data_type>)
                                    glCheck(glUniform1i(location, v.handle))
                                else if constexpr (std::is_same_v<sampler2DMS, data_type>)
                                    glCheck(glUniform1i(location, v.handle))
                                else if constexpr (std::is_same_v<sampler2DRect, data_type>)
                                    glCheck(glUniform1i(location, v.handle))
                            }, uniform.value());
                        }
                    }

            if (updateAllUniforms)
                glCheck(glBufferData(GL_UNIFORM_BUFFER, blockBuffer.size(), blockBuffer.data(), GL_DYNAMIC_DRAW));

            glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(stage->type()), ubo_handle(stage->type())))
        }
    }

    template <typename Base>
    std::size_t basic_opengl_shader_program<Base>::ssbo_count() const
    {
        return iSsbos.size();
    }

    template <typename Base>
    i_ssbo const& basic_opengl_shader_program<Base>::ssbo(std::size_t aIndex) const
    {
        return *iSsbos.at_index(aIndex);
    }

    template <typename Base>
    inline void basic_opengl_shader_program<Base>::create_ssbo(i_string const& aName, shader_data_type aDataType, i_ref_ptr<i_ssbo>& aSsbo)
    {
        ssbo_id const ssboId = iSsbos.next_cookie();
        switch(aDataType)
        {
        case shader_data_type::Boolean:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<bool>>(aName, ssboId));
            break;
        case shader_data_type::Float:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<float>>(aName, ssboId));
            break;
        case shader_data_type::Double:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<double>>(aName, ssboId));
            break;
        case shader_data_type::Int:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<std::int32_t>>(aName, ssboId));
            break;
        case shader_data_type::Uint:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<std::uint32_t>>(aName, ssboId));
            break;
        case shader_data_type::Vec2:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec2f>>(aName, ssboId));
            break;
        case shader_data_type::DVec2:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec2>>(aName, ssboId));
            break;
        case shader_data_type::IVec2:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec2i32>>(aName, ssboId));
            break;
        case shader_data_type::UVec2:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec2u32>>(aName, ssboId));
            break;
        case shader_data_type::Vec3:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec3f>>(aName, ssboId));
            break;
        case shader_data_type::DVec3:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec3>>(aName, ssboId));
            break;
        case shader_data_type::IVec3:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec3i32>>(aName, ssboId));
            break;
        case shader_data_type::UVec3:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec3u32>>(aName, ssboId));
            break;
        case shader_data_type::Vec4:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec4f>>(aName, ssboId));
            break;
        case shader_data_type::DVec4:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec4>>(aName, ssboId));
            break;
        case shader_data_type::IVec4:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec4i32>>(aName, ssboId));
            break;
        case shader_data_type::UVec4:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<vec4u32>>(aName, ssboId));
            break;
        case shader_data_type::Mat4:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<mat4f>>(aName, ssboId));
            break;
        case shader_data_type::DMat4:
            iSsbos.add(ssboId, aSsbo = make_ref<opengl_ssbo<mat4>>(aName, ssboId));
            break;
        case shader_data_type::FloatArray:
        case shader_data_type::DoubleArray:
        case shader_data_type::Sampler2D:
        case shader_data_type::Sampler2DMS:
        case shader_data_type::Sampler2DRect:
        default:
            throw std::logic_error("not supported");
        }
    }

    template <typename Base>
    inline void basic_opengl_shader_program<Base>::deactivate()
    {
        if (this->active())
        {
            glCheck(glUseProgram(0));
            this->set_inactive();
        }
    }

    template <typename Base>
    inline GLuint basic_opengl_shader_program<Base>::gl_handle() const
    {
        return to_gl_handle<GLuint>(this->handle());
    }

    template <typename Base>
    inline basic_opengl_shader_program<Base>::ubo_block_buffer_t& basic_opengl_shader_program<Base>::ubo_block_buffer(shader_type aShaderType)
    {
        return iUbos[static_cast<std::size_t>(aShaderType)].uniformBlockBuffer;
    }

    template <typename Base>
    inline GLuint basic_opengl_shader_program<Base>::ubo_handle(shader_type aShaderType) const
    {
        auto& ubo = iUbos[static_cast<std::size_t>(aShaderType)];
        if (ubo.uboHandle == std::nullopt)
        {
            GLuint uboHandle;
            glCheck(glGenBuffers(1, &uboHandle))
            ubo.uboHandle = uboHandle;
        }
        return ubo.uboHandle.value();
    }
}