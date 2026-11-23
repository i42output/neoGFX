// d3d12_shader_program.ipp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <cstring>
#include <sstream>

#include <neolib/core/set.hpp>

#include "d3d12_hlsl.hpp"
#include "d3d12_shader_program.hpp"

namespace neogfx
{
    // (the stage bits of vulkan_glsl's samplers, which Direct3D 12 does not use)
    inline VkShaderStageFlagBits to_d3d12_glsl_stage(shader_type aType)
    {
        switch (aType)
        {
        case shader_type::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        case shader_type::Vertex:
        default:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case shader_type::TessellationControl:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case shader_type::TessellationEvaluation:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case shader_type::Geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case shader_type::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }
    }

    template <typename Base>
    inline basic_d3d12_shader_program<Base>::basic_d3d12_shader_program(std::string const& aName) :
        base_type{ aName }, iScratch(4096u)
    {
        if constexpr (std::is_base_of_v<i_standard_shader_program, Base>)
            this->create_standard_shaders();
    }

    template <typename Base>
    basic_d3d12_shader_program<Base>::~basic_d3d12_shader_program()
    {
        if (this->created())
        {
            if (this->active() && d3d12_graphics_backend::instance() != nullptr)
                backend().use_program(nullptr);
        }
        this->stages().clear();
    }

    template <typename Base>
    inline void basic_d3d12_shader_program<Base>::compile()
    {
        if (!this->dirty())
            return;

        // the varyings: the vertex stage's outputs and the fragment stage's inputs must be the same, as Direct3D links them by
        // register (and SPIRV-Cross gives them registers in location order); each stage declares those of the other's shaders
        // (enabled or not) that its enabled shaders don't (the vertex stage writes zero to them)
        struct varying
        {
            string type;
            bool flat;
        };
        std::map<shader_variable_location, varying> varyings;
        bool vertexStage = false;
        bool fragmentStage = false;
        for (auto const& stage : this->stages())
        {
            if (stage->type() == shader_type::Vertex && !stage->shaders().empty())
            {
                vertexStage = true;
                for (auto const& shader : stage->shaders())
                    for (auto const& out : shader->out_variables())
                        varyings[out.location()] = varying{ enum_to_string<shader_data_type>(out.type()),
                            (out.qualifier().template value<shader_variable_qualifier>() & shader_variable_qualifier::Flat) == shader_variable_qualifier::Flat };
            }
            else if (stage->type() == shader_type::Fragment && !stage->shaders().empty())
            {
                fragmentStage = true;
                for (auto const& shader : stage->shaders())
                    for (auto const& in : shader->in_variables())
                        varyings[in.location()] = varying{ enum_to_string<shader_data_type>(in.type()),
                            (in.qualifier().template value<shader_variable_qualifier>() & shader_variable_qualifier::Flat) == shader_variable_qualifier::Flat };
            }
        }
        if (!vertexStage || !fragmentStage)
            varyings.clear();

        for (auto const& stage : this->stages())
        {
            if (this->stage_clean(stage->type()))
                continue;
            auto const& shaders = stage->shaders();
            if (shaders.empty())
                continue;
            // n.b. the GLSL generated as basic_opengl_shader_program generates it, then made Vulkan GLSL and Direct3D 12 GLSL
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
            if (stage->type() == shader_type::Vertex || stage->type() == shader_type::Fragment)
            {
                std::set<shader_variable_location> declared;
                for (auto const& v : (stage->type() == shader_type::Vertex ? outs : ins))
                    declared.insert(v.location());
                code += "\n"_s;
                for (auto const& v : varyings)
                {
                    if (declared.count(v.first) != 0u)
                        continue;
                    auto const name = "neogfx_varying"_s + to_string(v.first);
                    code += "layout (location = "_s + to_string(v.first) + ") "_s + (v.second.flat ? "flat "_s : ""_s) +
                        (stage->type() == shader_type::Vertex ? "out "_s : "in "_s) + v.second.type + " "_s + name + ";\n"_s;
                    if (stage->type() == shader_type::Vertex)
                        invokeResults += "    "_s + name + " = "_s + v.second.type + "(0);\n"_s;
                }
            }
            // OpenGL clip space to Direct3D's: depth from [-w, w] to [0, w], and y negated (see d3d12_graphics_backend)
            if (stage->type() == shader_type::Vertex)
            {
                invokeResults += "    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n"_s;
                invokeResults += "    gl_Position.y = -gl_Position.y;\n"_s;
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
            auto const dxCode = d3d12_glsl(vulkan_glsl(code.to_std_string(), to_d3d12_glsl_stage(stage->type()), iSamplers,
                d3d12_graphics_backend::StorageBindingBase, d3d12_graphics_backend::SamplerBindingBase));
            auto dump = [&](std::string const& why)
            {
                service<debug::logger>() << neolib::logger::severity::Debug << why << std::endl;
                std::int32_t lineNumber = 1;
                std::istringstream iss{ dxCode };
                std::string line;
                while (std::getline(iss, line))
                    service<debug::logger>() << neolib::logger::severity::Debug << lineNumber++ << ": " << line << std::endl;
            };
            auto& shaderObject = *static_cast<d3d12_shader_object*>(shaders[0]->handle(*this));
            shaderObject.name = this->name().to_std_string() + "." + enum_to_string<shader_type, std::string>(stage->type());
            shaderObject.glsl = dxCode;
            shaderObject.usesNumSamples = (dxCode.find("neogfx_NumSamples") != std::string::npos);
            shaderObject.bytecode.clear();
            try
            {
                shaderObject.spirv = backend().compile_spirv(stage->type(), dxCode, shaderObject.name);
                // n.b. the variant most draws use compiled now (so that errors are reported as the shader is compiled); the
                // others when first drawn with (see d3d12_graphics_backend::shader_bytecode)
                if (stage->type() == shader_type::Vertex || stage->type() == shader_type::Fragment)
                    (void)backend().shader_bytecode(shaderObject, backend().sample_count(4u), 0u);
            }
            catch (...)
            {
                shaderObject.spirv.clear();
                dump("Shader compilation error; shader code dump:-");
                throw;
            }
#ifndef NDEBUG
            dump("Shader code dump:-");
#endif
        }
    }

    template <typename Base>
    inline void basic_d3d12_shader_program<Base>::link()
    {
        if (!this->dirty())
            return;

        auto& program = dx_program();
        // the previous texture units of the samplers (they are set again when the uniforms are next updated)
        std::map<std::uint32_t, std::int32_t> previousTextureUnits;
        for (auto const& s : program.samplers)
            previousTextureUnits[s.binding] = s.textureUnit;

        backend().release_program(program);

        program.shaders = {};
        program.uniformBlocks.clear();
        program.samplers.clear();
        program.samplerSlots = 0u;
        program.storageBuffers.clear();
        program.vertexInputs.clear();

        for (auto& stage : this->stages())
        {
            auto const stageIndex = static_cast<std::size_t>(stage->type());
            if (stage->shaders().empty() || stage->type() == shader_type::Compute)
                continue;
            auto* const shaderObject = static_cast<d3d12_shader_object*>(stage->shaders()[0]->handle(*this));
            if (shaderObject->spirv.empty())
                continue;
            program.shaders[stageIndex] = shaderObject;
            // the stage's uniform block (see shader.glsl: its binding is its stage)
            auto const layout = spirv_uniform_block(shaderObject->spirv, enum_to_string<shader_type, std::string>(stage->type()) + "Uniforms");
            auto& block = iUniformBlocks[stageIndex];
            if (layout && layout->size != 0u)
            {
                iUniformBlockLayouts[stageIndex] = *layout;
                block.data.assign(layout->size, 0u);
                ++block.generation;
                program.uniformBlocks.push_back(d3d12_program::uniform_block_binding{ static_cast<std::uint32_t>(stageIndex), &block });
            }
            else
            {
                iUniformBlockLayouts[stageIndex] = spirv_block{};
                block.data.clear();
            }
            // the vertex attributes (cf. glGetAttribLocation)
            if (stage->type() == shader_type::Vertex)
                for (auto const& shader : stage->shaders())
                    if (shader->enabled())
                        for (auto const& in : shader->in_variables())
                            program.vertexInputs[in.name().to_std_string()] = d3d12_program::vertex_input{ in.location(), in.type().template value<shader_data_type>() };
        }
        for (auto const& s : iSamplers)
        {
            auto const previous = previousTextureUnits.find(s.second.binding);
            program.samplers.push_back(d3d12_program::sampler_binding{ s.second.binding, s.second.multisample,
                previous != previousTextureUnits.end() ? previous->second : -1 });
            program.samplerSlots = std::max(program.samplerSlots, s.second.binding - d3d12_graphics_backend::SamplerBindingBase + 1u);
        }
        for (auto const& sb : iSsboBuffers)
        {
            auto const id = static_cast<std::uint32_t>(sb.first);
            if (id >= d3d12_graphics_backend::SamplerBindingBase - d3d12_graphics_backend::StorageBindingBase)
                throw failed_to_create_shader_program("too many SSBOs");
            program.storageBuffers.push_back(d3d12_program::storage_binding{ d3d12_graphics_backend::StorageBindingBase + id, sb.second });
        }

        backend().create_root_signature(program);
        program.linked = true;
    }

    template <typename Base>
    inline void basic_d3d12_shader_program<Base>::use()
    {
        // n.b. always (another program may have been drawn with since)
        backend().use_program(&dx_program());
        if (!this->active())
            this->set_active();
    }

    template <typename Base>
    inline void basic_d3d12_shader_program<Base>::update_uniform_storage()
    {
        for (auto& stage : this->stages())
        {
            if (stage->shaders().empty())
                continue;

            auto const stageIndex = static_cast<std::size_t>(stage->type());
            auto& block = iUniformBlocks[stageIndex];
            auto const& layout = iUniformBlockLayouts[stageIndex];

            for (auto& shader : stage->shaders())
                if (shader->enabled() || shader->has_shared_uniforms())
                    for (auto& uniform : shader->uniforms())
                        if (!uniform.singular() && (shader->enabled() || uniform.shared()))
                        {
                            auto const member = layout.members.find(uniform.name().to_std_string());
                            // n.b. a uniform not in the block (e.g. of a stage not compiled) is given somewhere harmless to be written to
                            if (member != layout.members.end() && member->second.offset + member->second.size <= block.data.size())
                                shader->update_uniform_storage(uniform.id(), block.data.data() + member->second.offset);
                            else
                                shader->update_uniform_storage(uniform.id(), iScratch.data());
                        }
        }
    }

    template <typename Base>
    inline void basic_d3d12_shader_program<Base>::update_uniform_locations()
    {
        // n.b. a sampler uniform's location is its binding (see vulkan_glsl: its registers, t and s)
        for (auto& stage : this->stages())
            for (auto& shader : stage->shaders())
                for (auto& uniform : shader->uniforms())
                    if (uniform.singular())
                    {
                        auto const sampler = iSamplers.find(uniform.name().to_std_string());
                        shader->update_uniform_location(uniform.id(), sampler != iSamplers.end() ? static_cast<shader_uniform_location>(sampler->second.binding) : -1);
                    }
    }

    template <typename Base>
    inline void basic_d3d12_shader_program<Base>::update_uniforms(const i_rendering_context& aContext)
    {
        this->prepare_uniforms(aContext);

        bool const updateAllUniforms = this->need_full_uniform_update();

        auto& program = dx_program();

        for (auto& stage : this->stages())
        {
            auto const stageIndex = static_cast<std::size_t>(stage->type());
            auto& block = iUniformBlocks[stageIndex];
            auto const& layout = iUniformBlockLayouts[stageIndex];
            bool blockChanged = false;

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
                            auto* const storage = static_cast<std::uint8_t*>(uniform.storage());
                            if (storage == iScratch.data())
                                continue;
                            auto const member = layout.members.find(uniform.name().to_std_string());
                            std::uint32_t const arrayStride = (member != layout.members.end() ? member->second.arrayStride : 0u);
                            std::visit([&](auto&& v)
                            {
                                typedef std::decay_t<decltype(v)> data_type;
                                if constexpr (std::is_same_v<bool, data_type>)
                                {
                                    std::int32_t const boolean = v;
                                    std::memcpy(storage, &boolean, sizeof(boolean));
                                }
                                else if constexpr (
                                    std::is_same_v<mat4f, data_type> ||
                                    std::is_same_v<mat4, data_type>)
                                {
                                    std::memcpy(storage, v.data(), sizeof(v[0][0]) * 4 * 4);
                                }
                                else if constexpr (
                                    std::is_same_v<maybe_abstract_t<shader_float_array>, data_type> ||
                                    std::is_same_v<maybe_abstract_t<shader_double_array>, data_type>)
                                {
                                    // n.b. std140 (as an HLSL constant buffer): the elements of an array are each aligned as a vec4
                                    auto const elementSize = sizeof(v[0]);
                                    auto const stride = (arrayStride != 0u ? arrayStride : elementSize);
                                    auto const capacity = (member != layout.members.end() ? member->second.size : static_cast<std::uint32_t>(stride * v.size()));
                                    for (std::size_t i = 0u; i < v.size() && (i * stride + elementSize) <= capacity; ++i)
                                        std::memcpy(storage + i * stride, &v[i], elementSize);
                                }
                                else if constexpr (
                                    std::is_same_v<sampler2D, data_type> ||
                                    std::is_same_v<sampler2DMS, data_type> ||
                                    std::is_same_v<sampler2DRect, data_type>)
                                {
                                    // not in a uniform block
                                }
                                else
                                    std::memcpy(storage, &v, sizeof(data_type));
                            }, uniform.value());
                            blockChanged = true;
                        }
                        else if (uniform.has_location())
                        {
                            // the texture unit the sampler is to sample (see d3d12_graphics_backend::prepare_draw)
                            auto const binding = static_cast<std::uint32_t>(uniform.location());
                            std::visit([&](auto&& v)
                            {
                                typedef std::decay_t<decltype(v)> data_type;
                                if constexpr (
                                    std::is_same_v<sampler2D, data_type> ||
                                    std::is_same_v<sampler2DMS, data_type> ||
                                    std::is_same_v<sampler2DRect, data_type>)
                                {
                                    for (auto& s : program.samplers)
                                        if (s.binding == binding)
                                            s.textureUnit = v.handle;
                                }
                            }, uniform.value());
                        }
                    }

            if (blockChanged)
                ++block.generation;
        }
    }

    template <typename Base>
    std::size_t basic_d3d12_shader_program<Base>::ssbo_count() const
    {
        return iSsbos.size();
    }

    template <typename Base>
    i_ssbo const& basic_d3d12_shader_program<Base>::ssbo(std::size_t aIndex) const
    {
        return *iSsbos.at_index(aIndex);
    }

    template <typename Base>
    i_ssbo& basic_d3d12_shader_program<Base>::ssbo(std::size_t aIndex)
    {
        return *iSsbos.at_index(aIndex);
    }

    template <typename Base>
    template <typename T>
    inline void basic_d3d12_shader_program<Base>::add_ssbo(i_string const& aName, ssbo_id aId, i_ref_ptr<i_ssbo>& aSsbo)
    {
        auto newSsbo = make_ref<d3d12_ssbo<T>>(aName, aId);
        auto* const ssboBuffer = &*newSsbo;
        iSsbos.add(aId, aSsbo = newSsbo);
        // n.b. a program's SSBOs live as long as it does (e.g. standard_shader_program's)
        iSsboBuffers.emplace_back(aId, [ssboBuffer]() { return ssboBuffer->handle(); });
        // n.b. the root signature includes the SSBOs
        for (auto& stage : this->stages())
            for (auto& shader : stage->shaders())
                shader->set_dirty();
    }

    template <typename Base>
    inline void basic_d3d12_shader_program<Base>::create_ssbo(i_string const& aName, shader_data_type aDataType, i_ref_ptr<i_ssbo>& aSsbo)
    {
        ssbo_id const ssboId = iSsbos.next_cookie();
        switch(aDataType)
        {
        case shader_data_type::Boolean:
            add_ssbo<bool>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::Float:
            add_ssbo<float>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::Double:
            add_ssbo<double>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::Int:
            add_ssbo<std::int32_t>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::Uint:
            add_ssbo<std::uint32_t>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::Vec2:
            add_ssbo<vec2f>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::DVec2:
            add_ssbo<vec2>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::IVec2:
            add_ssbo<vec2i32>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::UVec2:
            add_ssbo<vec2u32>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::Vec3:
            add_ssbo<vec3f>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::DVec3:
            add_ssbo<vec3>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::IVec3:
            add_ssbo<vec3i32>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::UVec3:
            add_ssbo<vec3u32>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::Vec4:
            add_ssbo<vec4f>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::DVec4:
            add_ssbo<vec4>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::IVec4:
            add_ssbo<vec4i32>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::UVec4:
            add_ssbo<vec4u32>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::Mat4:
            add_ssbo<mat4f>(aName, ssboId, aSsbo);
            break;
        case shader_data_type::DMat4:
            add_ssbo<mat4>(aName, ssboId, aSsbo);
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
    inline void basic_d3d12_shader_program<Base>::deactivate()
    {
        if (this->active())
        {
            if (d3d12_graphics_backend::instance() != nullptr)
                backend().use_program(nullptr);
            this->set_inactive();
        }
    }

    template <typename Base>
    inline d3d12_graphics_backend& basic_d3d12_shader_program<Base>::backend() const
    {
        return *d3d12_graphics_backend::instance();
    }

    template <typename Base>
    inline d3d12_program& basic_d3d12_shader_program<Base>::dx_program() const
    {
        return *static_cast<d3d12_program*>(this->handle());
    }
}
