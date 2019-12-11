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
#include <neogfx/gfx/i_shader.hpp>
#include <neogfx/gfx/i_vertex_shader.hpp>

namespace neogfx
{
    struct shader_not_found : std::logic_error { shader_not_found() : std::logic_error{ "neogfx:shader_not_found" } {} };
    struct no_vertex_shader : std::logic_error { no_vertex_shader() : std::logic_error{ "neogfx:no_vertex_shader" } {} };
    struct shader_last_in_stage : std::logic_error { shader_last_in_stage() : std::logic_error{ "neogfx:shader_last_in_stage" } {} };
    struct failed_to_create_shader_program : std::runtime_error { failed_to_create_shader_program(const std::string& aReason) : std::runtime_error("neogfx::failed_to_create_shader_program: " + aReason) {} };
    struct shader_program_error : std::runtime_error { shader_program_error(const std::string& aError) : std::runtime_error("neogfx::shader_program_error: " + aError) {} };

    class i_shader_program
    {
        // types
    public:
        typedef neolib::i_vector<neolib::i_ref_ptr<i_shader>> shader_list;
        // construction
    public:
        virtual ~i_shader_program() {}
        // operations
    public:
        virtual const shader_list& shaders() const = 0;
        virtual const i_shader& shader(const neolib::i_string& aName) const = 0;
        virtual i_shader& shader(const neolib::i_string& aName) = 0;
        virtual const i_vertex_shader& vertex_shader() const = 0;
        virtual i_vertex_shader& vertex_shader() = 0;
        virtual bool is_first_in_stage(const i_shader& aShader) const = 0;
        virtual bool is_last_in_stage(const i_shader& aShader) const = 0;
        virtual const i_shader& first_in_stage(const i_shader& aCurrentShader) const = 0;
        virtual const i_shader& next_in_stage(const i_shader& aPreviousShader) const = 0;
        virtual i_shader_program& add_shader(neolib::i_ref_ptr<i_shader>& aShader) = 0;
        virtual bool dirty() const = 0;
        virtual void set_dirty() = 0;
        virtual void compile() = 0;
        virtual void link() = 0;
        virtual void use() = 0;
    };
}