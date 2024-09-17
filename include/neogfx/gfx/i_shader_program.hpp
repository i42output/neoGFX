// i_shader_program.hpp
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

#include <neolib/core/i_vector.hpp>
#include <neolib/core/i_string.hpp>
#include <neolib/core/i_reference_counted.hpp>

#include <neogfx/gfx/i_vertex_buffer.hpp>
#include <neogfx/gfx/i_shader.hpp>
#include <neogfx/gfx/i_vertex_shader.hpp>
#include <neogfx/gfx/i_fragment_shader.hpp>
#include <neogfx/gfx/i_ssbo.hpp>

namespace neogfx
{
    struct shader_stage_not_found : std::logic_error { shader_stage_not_found() : std::logic_error{ "neogfx::shader_stage_not_found" } {} };
    struct shader_not_found : std::logic_error { shader_not_found() : std::logic_error{ "neogfx::shader_not_found" } {} };
    struct shader_name_exists : std::logic_error { shader_name_exists() : std::logic_error{ "neogfx::shader_name_exists" } {} };
    struct no_vertex_shader : std::logic_error { no_vertex_shader() : std::logic_error{ "neogfx::no_vertex_shader" } {} };
    struct no_fragment_shader : std::logic_error { no_fragment_shader() : std::logic_error{ "neogfx::no_fragment_shader" } {} };
    struct shader_last_in_stage : std::logic_error { shader_last_in_stage() : std::logic_error{ "neogfx::shader_last_in_stage" } {} };
    struct shader_program_dirty : std::logic_error { shader_program_dirty() : std::logic_error{ "neogfx::shader_program_dirty" } {} };
    struct failed_to_create_shader : std::runtime_error { failed_to_create_shader() : std::runtime_error{ "neogfx::failed_to_create_shader" } {} };
    struct failed_to_create_shader_program : std::runtime_error { failed_to_create_shader_program(std::string const& aReason) : std::runtime_error{ "neogfx::failed_to_create_shader_program: " + aReason } {} };
    struct shader_program_error : std::runtime_error { shader_program_error(std::string const& aError) : std::runtime_error{ "neogfx::shader_program_error: " + aError } {} };

    enum class shader_program_type : std::uint32_t
    {
        Standard,
        User
    };

    class i_shader_stage : public i_reference_counted
    {
        // types
    public:
        typedef i_shader_stage abstract_type;
    public:
        typedef neolib::i_ref_ptr<i_shader> i_shader_t;
        typedef neolib::i_vector<i_shader_t> i_shaders_t;
        // construction
    public:
        virtual ~i_shader_stage() = default;
        // operations
    public:
        virtual shader_type type() const = 0;
        virtual i_shaders_t const& shaders() const = 0;
        virtual i_shaders_t& shaders() = 0;
    };

    class i_shader_program : public i_reference_counted
    {
        // types
    public:
        typedef i_shader_program abstract_type;
    public:
        typedef neolib::i_ref_ptr<i_shader_stage> i_stage_t;
        typedef neolib::i_vector<i_stage_t> i_stages_t;
        // construction
    public:
        virtual ~i_shader_program() = default;
        // operations
    public:
        virtual shader_program_type type() const = 0;
        virtual const i_string& name() const = 0;
        virtual bool supports(vertex_buffer_type aBufferType) const = 0;
        virtual bool created() const = 0;
        virtual void* handle() const = 0;
        virtual const i_stages_t& stages() const = 0;
        virtual i_stages_t& stages() = 0;
        virtual const i_stage_t& stage(shader_type aStage) const = 0;
        virtual i_stage_t& stage(shader_type aStage) = 0;
        virtual const i_shader& shader(const neolib::i_string& aName) const = 0;
        virtual i_shader& shader(const neolib::i_string& aName) = 0;
        virtual const i_vertex_shader& vertex_shader() const = 0;
        virtual i_vertex_shader& vertex_shader() = 0;
        virtual const i_fragment_shader& fragment_shader() const = 0;
        virtual i_fragment_shader& fragment_shader() = 0;
        virtual bool is_first_in_stage(const i_shader& aShader) const = 0;
        virtual bool is_last_in_stage(const i_shader& aShader) const = 0;
        virtual const i_shader& first_in_stage(shader_type aStage) const = 0;
        virtual const i_shader& next_in_stage(const i_shader& aPreviousShader) const = 0;
        virtual const i_shader& last_in_stage(shader_type aStage) const = 0;
        virtual i_shader& add_shader(const neolib::i_ref_ptr<i_shader>& aShader) = 0;
        virtual bool dirty() const = 0;
        virtual void set_clean() = 0;
        virtual void prepare_uniforms(const i_rendering_context& aContext) = 0;
        virtual void make() = 0;
        virtual void compile() = 0;
        virtual void link() = 0;
        virtual void use() = 0;
        virtual void update_uniform_storage() = 0;
        virtual void update_uniform_locations() = 0;
        virtual bool uniforms_changed() const = 0;
        virtual void update_uniforms(const i_rendering_context& aContext) = 0;
        virtual std::size_t ssbo_count() const = 0;
        virtual i_ssbo const& ssbo(std::size_t aIndex) const = 0;
        virtual void create_ssbo(i_string const& aName, shader_data_type aDataType, i_ref_ptr<i_ssbo>& aSsbo) = 0;
        virtual bool active() const = 0;
        virtual void activate(const i_rendering_context& aContext) = 0;
        virtual void deactivate() = 0;
        virtual void instantiate(const i_rendering_context& aContext) = 0;
    public:
        template <typename T>
        const T& as() const
        {
            return static_cast<const T&>(*this);
        }
        template <typename T>
        T& as()
        {
            return static_cast<T&>(*this);
        }
        bool have_stage(shader_type aStage) const
        {
            return !stages().at(static_cast<std::size_t>(aStage))->shaders().empty();
        }
        bool stage_clean(shader_type aStage) const
        {
            if (!have_stage(aStage))
                return true;
            const i_shader* shader = &first_in_stage(aStage);
            for (;;)
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
    public:
        template <typename T>
        ref_ptr<i_ssbo> create_ssbo(i_string const& aName)
        {
            ref_ptr<i_ssbo> result;
            create_ssbo(aName, shader_data_type_v<T>, result);
            return result;
        }
    };
}