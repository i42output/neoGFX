// shader_program.cpp
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

#include <neogfx/gfx/shader_program.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>

namespace neogfx
{
    shader_program::shader_program(const std::string aName) :
        iName{ aName }
    {
    }

    shader_program::~shader_program()
    {
        if (iHandle != std::nullopt)
            service<i_rendering_engine>().destroy_shader_program_object(*iHandle);
    }

    const i_string& shader_program::name() const
    {
        return iName;
    }

    bool shader_program::created() const
    {
        return iHandle != std::nullopt;
    }

    void* shader_program::handle() const
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

    const shader_program::stages_t& shader_program::stages() const
    {
        return iStages;
    }

    const i_shader& shader_program::shader(const neolib::i_string& aName) const
    {
        auto s = iShaderIndex.find(aName);
        if (s != iShaderIndex.end())
            return *s->second();
        throw shader_not_found();
    }

    i_shader& shader_program::shader(const neolib::i_string& aName)
    {
        return const_cast<i_shader&>(to_const(*this).shader(aName));
    }

    const i_vertex_shader& shader_program::vertex_shader() const
    {
        if (have_stage(shader_type::Vertex))
            return static_cast<const i_vertex_shader&>(*stages().at(shader_type::Vertex)[0]);
        throw no_vertex_shader();
    }

    i_vertex_shader& shader_program::vertex_shader()
    {
        return const_cast<i_vertex_shader&>(to_const(*this).vertex_shader());
    }

    bool shader_program::is_first_in_stage(const i_shader& aShader) const
    {
        return &first_in_stage(aShader.type()) == &aShader;
    }

    bool shader_program::is_last_in_stage(const i_shader& aShader) const
    {
        return &last_in_stage(aShader.type()) == &aShader;
    }

    const i_shader& shader_program::first_in_stage(shader_type aStage) const
    {
        if (have_stage(aStage))
            for (auto const& shader : stages().at(aStage))
                if (shader->enabled())
                    return *shader;
        throw shader_not_found();
    }

    const i_shader& shader_program::last_in_stage(shader_type aStage) const
    {
        if (have_stage(aStage))
            for (auto shader = stages().at(aStage).rbegin(); shader != stages().at(aStage).rend(); ++shader)
                if ((**shader).enabled())
                    return **shader;
        throw shader_not_found();
    }

    const i_shader& shader_program::next_in_stage(const i_shader& aPreviousShader) const
    {
        auto stage = stages().find(aPreviousShader.type());
        if (stage != stages().end())
        {
            auto shader = std::find_if(stage->second().begin(), stage->second().end(), [&aPreviousShader](auto& s) { return &*s == &aPreviousShader; });
            while(shader != stage->second().end())
            {
                std::advance(shader, 1);
                if ((**shader).enabled())
                    return **shader;
            }
        }
        throw shader_not_found();
    }

    i_shader& shader_program::add_shader(const neolib::i_ref_ptr<i_shader>& aShader)
    {
        if (iShaderIndex.find(aShader->name()) == iShaderIndex.end())
        {
            iStages[aShader->type()].push_back(aShader);
            iShaderIndex.insert(aShader->name(), aShader);
            return *aShader;
        }
        throw shader_name_exists();
    }

    bool shader_program::dirty() const
    {
        for (auto const& stage : stages())
            for (auto const& shader : stage.second())
                if (shader->dirty())
                    return true;
        return false;
    }

    void shader_program::set_clean()
    {
        for (auto& stage : stages())
            for (auto& shader : stage.second())
                shader->set_clean();
    }

    void shader_program::prepare_uniforms(const i_rendering_context& aRenderingContext)
    {
        for (auto& stage : stages())
            for (auto& shader : stage.second())
                shader->prepare_uniforms(aRenderingContext, *this);
    }

    void shader_program::activate(const i_rendering_context& aContext)
    {
        if (dirty())
        {
            prepare_uniforms(aContext);
            compile();
            link();
        }
        use();
        update_uniforms(aContext);
        set_clean();
    }
}