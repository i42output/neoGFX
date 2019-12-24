// i_shader_program.hpp
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
#include <neolib/i_vector.hpp>
#include <neolib/i_string.hpp>
#include <neolib/i_reference_counted.hpp>
#include <neogfx/gfx/i_shader.hpp>
#include <neogfx/gfx/i_vertex_shader.hpp>
#include <neogfx/gfx/i_fragment_shader.hpp>

namespace neogfx
{
    struct shader_not_found : std::logic_error { shader_not_found() : std::logic_error{ "neogfx::shader_not_found" } {} };
    struct shader_name_exists : std::logic_error { shader_name_exists() : std::logic_error{ "neogfx::shader_name_exists" } {} };
    struct no_vertex_shader : std::logic_error { no_vertex_shader() : std::logic_error{ "neogfx::no_vertex_shader" } {} };
    struct no_gradient_shader : std::logic_error { no_gradient_shader() : std::logic_error{ "neogfx::no_gradient_shader" } {} };
    struct shader_last_in_stage : std::logic_error { shader_last_in_stage() : std::logic_error{ "neogfx::shader_last_in_stage" } {} };
    struct failed_to_create_shader : std::runtime_error { failed_to_create_shader() : std::runtime_error("neogfx::failed_to_create_shader") {} };
    struct failed_to_create_shader_program : std::runtime_error { failed_to_create_shader_program(const std::string& aReason) : std::runtime_error("neogfx::failed_to_create_shader_program: " + aReason) {} };
    struct shader_program_error : std::runtime_error { shader_program_error(const std::string& aError) : std::runtime_error("neogfx::shader_program_error: " + aError) {} };

    class i_shader_program : public neolib::i_reference_counted
    {
        typedef i_shader_program self_type;
        // types
    public:
        typedef self_type abstract_type;
    public:
        typedef neolib::i_vector<neolib::i_ref_ptr<i_shader>> shaders_t;
        typedef neolib::i_map<shader_type, shaders_t> stages_t;
        // construction
    public:
        virtual ~i_shader_program() {}
        // operations
    public:
        virtual const i_string& name() const = 0;
        virtual bool created() const = 0;
        virtual void* handle() const = 0;
        virtual const stages_t& stages() const = 0;
        virtual const i_shader& shader(const neolib::i_string& aName) const = 0;
        virtual i_shader& shader(const neolib::i_string& aName) = 0;
        virtual const i_vertex_shader& vertex_shader() const = 0;
        virtual i_vertex_shader& vertex_shader() = 0;
        virtual const i_gradient_shader& gradient_shader() const = 0;
        virtual i_gradient_shader& gradient_shader() = 0;
        virtual const i_texture_shader& texture_shader() const = 0;
        virtual i_texture_shader& texture_shader() = 0;
        virtual bool is_first_in_stage(const i_shader& aShader) const = 0;
        virtual bool is_last_in_stage(const i_shader& aShader) const = 0;
        virtual const i_shader& first_in_stage(shader_type aStage) const = 0;
        virtual const i_shader& next_in_stage(const i_shader& aPreviousShader) const = 0;
        virtual const i_shader& last_in_stage(shader_type aStage) const = 0;
        virtual i_shader& add_shader(const neolib::i_ref_ptr<i_shader>& aShader) = 0;
        virtual bool dirty() const = 0;
        virtual void set_clean() = 0;
        virtual void prepare(const i_rendering_context& aRenderingContext) = 0;
        virtual void compile() = 0;
        virtual void link() = 0;
        virtual void use() = 0;
        virtual void update_uniforms() = 0;
        virtual bool active() const = 0;
        virtual void activate(const i_rendering_context& aRenderingContext) = 0;
        virtual void deactivate() = 0;
    public:
        bool have_stage(shader_type aStage) const
        {
            return stages().find(aStage) != stages().end() && !stages().at(aStage).empty();
        }
        bool stage_clean(shader_type aStage) const
        {
            const i_shader* shader = &first_in_stage(aStage);
            for(;;)
            {
                if (shader->dirty())
                    return false;
                if (is_last_in_stage(*shader))
                    return true;
                shader = &next_in_stage(*shader);
            }
        }
        bool stage_dirty(shader_type aStage) const
        {
            return !stage_clean(aStage);
        }
    };
}