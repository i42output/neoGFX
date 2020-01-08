// shader_program.inl
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/shader_program.hpp>

namespace neogfx
{
    template <typename Base>
    inline shader_program<Base>::shader_program(const std::string aName) :
        iName{ aName }
    {
        iStages.push_back(stage_t{ shader_type::Compute, shaders_t{} });
        iStages.push_back(stage_t{ shader_type::Vertex, shaders_t{} });
        iStages.push_back(stage_t{ shader_type::TessellationControl, shaders_t{} });
        iStages.push_back(stage_t{ shader_type::TessellationEvaluation, shaders_t{} });
        iStages.push_back(stage_t{ shader_type::Geometry, shaders_t{} });
        iStages.push_back(stage_t{ shader_type::Fragment, shaders_t{} });
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
    const typename shader_program<Base>::shaders_t& shader_program<Base>::stage(shader_type aStage) const
    {
        return static_cast<const shaders_t&>(stages().at(static_cast<std::size_t>(aStage)).second());
    }

    template <typename Base>
    typename shader_program<Base>::shaders_t& shader_program<Base>::stage(shader_type aStage)
    {
        return static_cast<shaders_t&>(stages().at(static_cast<std::size_t>(aStage)).second());
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
            return static_cast<const i_vertex_shader&>(*stage(shader_type::Vertex)[0]);
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
            return static_cast<const i_fragment_shader&>(*stage(shader_type::Fragment)[0]);
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
            for (auto const& shader : stage(aStage).container())
                if (shader->enabled())
                    return *shader;
        throw shader_not_found();
    }

    template <typename Base>
    inline const i_shader& shader_program<Base>::last_in_stage(shader_type aStage) const
    {
        if (have_stage(aStage))
            for (auto shader = stage(aStage).rbegin(); shader != stage(aStage).rend(); ++shader)
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
            auto shader = std::find_if(currentStage.begin(), currentStage.end(), [&aPreviousShader](auto& s) { return &*s == &aPreviousShader; });
            while(shader != currentStage.end())
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
            stage(aShader->type()).push_back(aShader);
            iShaderIndex.emplace(string{ aShader->name() }, aShader);
            return *aShader;
        }
        throw shader_name_exists();
    }

    template <typename Base>
    inline bool shader_program<Base>::dirty() const
    {
        for (auto const& stage : stages().container())
            for (auto const& shader : stage.second().container())
                if (shader->dirty())
                    return true;
        return false;
    }

    template <typename Base>
    inline void shader_program<Base>::set_clean()
    {
        for (auto& stage : stages().container())
            for (auto& shader : stage.second().container())
                shader->set_clean();
    }

    template <typename Base>
    inline void shader_program<Base>::prepare_uniforms(const i_rendering_context& aContext)
    {
        for (auto& stage : stages().container())
            for (auto& shader : stage.second().container())
                shader->prepare_uniforms(aContext, *this);
    }

    template <typename Base>
    inline void shader_program<Base>::activate(const i_rendering_context& aContext)
    {
        if (dirty())
        {
            prepare_uniforms(aContext);
            compile();
            link();
            use();
            update_uniform_locations(aContext);
            update_uniforms(aContext);
            set_clean();
        }
        else
        {
            use();
            update_uniforms(aContext);
        }
    }
}