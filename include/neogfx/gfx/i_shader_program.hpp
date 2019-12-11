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
    class i_shader_program
    {
        // exceptions
    public:
        struct shader_not_found : std::logic_error { shader_not_found() : std::logic_error{ "neogfx::i_shader_program::shader_not_found" } {} };
        struct no_vertex_shader : std::logic_error { no_vertex_shader() : std::logic_error{ "neogfx::i_shader_program::no_vertex_shader" } {} };
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
        virtual i_shader_program& add_shader(neolib::i_ref_ptr<i_shader>& aShader) = 0;
        virtual void compile() = 0;
        virtual void link() = 0;
        virtual void use() = 0;
    };
}