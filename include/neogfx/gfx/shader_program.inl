// shader_program.inl
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/shader_program.hpp>

namespace neogfx
{
    template <typename Base>
    inline shader_stage<Base>::shader_stage(shader_type aType) : 
        iType{ aType }
    {
    }

    template <typename Base>
    inline shader_type shader_stage<Base>::type() const
    {
        return iType;
    }

    template <typename Base>
    inline shader_stage<Base>::shaders_t const& shader_stage<Base>::shaders() const
    {
        return iShaders;
    }

    template <typename Base>
    inline shader_stage<Base>::shaders_t& shader_stage<Base>::shaders()
    {
        return iShaders;
    }

    template <typename Base>
    inline shader_program<Base>::shader_program(const std::string aName) :
        iName{ aName }, iNeedFullUniformUpdate{ false }, iActive{ false }
    {
        iStages.push_back(neolib::make_ref<shader_stage<>>(shader_type::Compute));
        iStages.push_back(neolib::make_ref<shader_stage<>>(shader_type::Vertex));
        iStages.push_back(neolib::make_ref<shader_stage<>>(shader_type::TessellationControl));
        iStages.push_back(neolib::make_ref<shader_stage<>>(shader_type::TessellationEvaluation));
        iStages.push_back(neolib::make_ref<shader_stage<>>(shader_type::Geometry));
        iStages.push_back(neolib::make_ref<shader_stage<>>(shader_type::Fragment));
    }

    template <typename Base>
    inline shader_program<Base>::~shader_program()
    {
        if (iHandle != std::nullopt)
            service<i_rendering_engine>().destroy_shader_program_object(*iHandle);
    }

    template <typename Base>
    inline const i_string& shader_program<Base>::name() const
    {
        return iName;
    }

    template <typename Base>
    inline bool shader_program<Base>::supports(vertex_buffer_type aBufferType) const
    {
        for (auto& stage : stages())
            for (auto& shader : stage->shaders())
                if (shader->supports(aBufferType))
                    return true;
        return false;
    }

    template <typename Base>
    inline bool shader_program<Base>::created() const
    {
        return iHandle != std::nullopt;
    }

    template <typename Base>
    inline void* shader_program<Base>::handle() const
    {
        if (!created())
        {
            auto newHandle = service<i_rendering_engine>().create_shader_program_object();
            if (newHandle == nullptr)
                throw failed_to_create_shader_program("Failed to create shader program object");
            iHandle = newHandle;
        }
        return *iHandle;
    }

    template <typename Base>
    inline const typename shader_program<Base>::stages_t& shader_program<Base>::stages() const
    {
        return iStages;
    }

    template <typename Base>
    inline typename shader_program<Base>::stages_t& shader_program<Base>::stages()
    {
        return iStages;
    }

    template <typename Base>
    const typename shader_program<Base>::stage_t& shader_program<Base>::stage(shader_type aStage) const
    {
        return stages().at(static_cast<std::size_t>(aStage));
    }

    template <typename Base>
    typename shader_program<Base>::stage_t& shader_program<Base>::stage(shader_type aStage)
    {
        return stages().at(static_cast<std::size_t>(aStage));
    }

    template <typename Base>
    inline const i_shader& shader_program<Base>::shader(const neolib::i_string& aName) const
    {
        auto s = iShaderIndex.find(aName);
        if (s != iShaderIndex.end())
            return *s->second;
        throw shader_not_found();
    }

    template <typename Base>
    inline i_shader& shader_program<Base>::shader(const neolib::i_string& aName)
    {
        return const_cast<i_shader&>(to_const(*this).shader(aName));
    }

    template <typename Base>
    inline const i_vertex_shader& shader_program<Base>::vertex_shader() const
    {
        if (have_stage(shader_type::Vertex))
            return static_cast<const i_vertex_shader&>(*stage(shader_type::Vertex)->shaders()[0]);
        throw no_vertex_shader();
    }

    template <typename Base>
    inline i_vertex_shader& shader_program<Base>::vertex_shader()
    {
        return const_cast<i_vertex_shader&>(to_const(*this).vertex_shader());
    }

    template <typename Base>
    inline const i_fragment_shader& shader_program<Base>::fragment_shader() const
    {
        if (have_stage(shader_type::Fragment))
            return static_cast<const i_fragment_shader&>(*stage(shader_type::Fragment)->shaders()[0]);
        throw no_fragment_shader();
    }

    template <typename Base>
    inline i_fragment_shader& shader_program<Base>::fragment_shader()
    {
        return const_cast<i_fragment_shader&>(to_const(*this).fragment_shader());
    }

    template <typename Base>
    inline bool shader_program<Base>::is_first_in_stage(const i_shader& aShader) const
    {
        return &first_in_stage(aShader.type()) == &aShader;
    }

    template <typename Base>
    inline bool shader_program<Base>::is_last_in_stage(const i_shader& aShader) const
    {
        return &last_in_stage(aShader.type()) == &aShader;
    }

    template <typename Base>
    inline const i_shader& shader_program<Base>::first_in_stage(shader_type aStage) const
    {
        if (have_stage(aStage))
            for (auto const& shader : stage(aStage)->shaders())
                if (shader->enabled())
                    return *shader;
        throw shader_not_found();
    }

    template <typename Base>
    inline const i_shader& shader_program<Base>::last_in_stage(shader_type aStage) const
    {
        if (have_stage(aStage))
            for (auto shader = stage(aStage)->shaders().rbegin(); shader != stage(aStage)->shaders().rend(); ++shader)
                if ((**shader).enabled())
                    return **shader;
        throw shader_not_found();
    }

    template <typename Base>
    inline const i_shader& shader_program<Base>::next_in_stage(const i_shader& aPreviousShader) const
    {
        if (have_stage(aPreviousShader.type()))
        {
            auto const& currentStage = stage(aPreviousShader.type());
            auto shader = std::find_if(currentStage->shaders().begin(), currentStage->shaders().end(), [&aPreviousShader](auto& s) { return &*s == &aPreviousShader; });
            while(shader != currentStage->shaders().end())
            {
                std::advance(shader, 1);
                if ((**shader).enabled())
                    return **shader;
            }
        }
        throw shader_not_found();
    }

    template <typename Base>
    inline i_shader& shader_program<Base>::add_shader(const neolib::i_ref_ptr<i_shader>& aShader)
    {
        if (iShaderIndex.find(aShader->name()) == iShaderIndex.end())
        {
            stage(aShader->type())->shaders().push_back(aShader);
            iShaderIndex.emplace(string{ aShader->name() }, aShader);
            return *aShader;
        }
        throw shader_name_exists();
    }

    template <typename Base>
    inline bool shader_program<Base>::dirty() const
    {
        for (auto const& stage : stages())
            for (auto const& shader : stage->shaders())
                if (shader->dirty())
                    return (iNeedFullUniformUpdate = true);
        return false;
    }

    template <typename Base>
    inline void shader_program<Base>::set_clean()
    {
        for (auto& stage : stages())
            for (auto& shader : stage->shaders())
                shader->set_clean();
    }

    template <typename Base>
    inline void shader_program<Base>::prepare_uniforms(const i_rendering_context& aContext)
    {
        for (auto& stage : stages())
            for (auto& shader : stage->shaders())
                shader->prepare_uniforms(aContext, *this);
    }

    template <typename Base>
    inline void shader_program<Base>::make()
    {
        if (dirty())
        {
            compile();
            link();
            use();
            update_uniform_storage();
            update_uniform_locations();
            set_clean();
        }
    }

    template <typename Base>
    bool shader_program<Base>::uniforms_changed() const
    {
        for (auto& stage : stages())
            for (auto& shader : stage->shaders())
                if (shader->uniforms_changed())
                    return true;
        return false;
    }
    
    template <typename Base>
    inline bool shader_program<Base>::active() const
    {
        return iActive;
    }

    template <typename Base>
    inline void shader_program<Base>::activate(const i_rendering_context& aContext)
    {
        prepare_uniforms(aContext);
        make();
        use();
    }

    template <typename Base>
    inline void shader_program<Base>::instantiate(const i_rendering_context& aContext)
    {
        activate(aContext);
        update_uniforms(aContext);
    }

    template <typename Base>
    bool shader_program<Base>::need_full_uniform_update() const
    {
        bool const needIt = iNeedFullUniformUpdate;
        iNeedFullUniformUpdate = false;
        return needIt;
    }

    template <typename Base>
    void shader_program<Base>::set_active()
    {
        iActive = true;
    }

    template <typename Base>
    void shader_program<Base>::set_inactive()
    {
        iActive = false;
    }
}