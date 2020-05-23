// i_shader.hpp
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
#include <neolib/core/i_pair.hpp>
#include <neogfx/core/numerical.hpp>
#include <neogfx/gfx/i_shader.hpp>

namespace neogfx
{
    class i_rendering_context;

    class i_standard_vertex_matrices
    {
    public:
        virtual void set_projection_matrix(const optional_mat44& aProjectionMatrix) = 0;
        virtual void set_transformation_matrix(const optional_mat44& aProjectionMatrix) = 0;
    };

    struct no_standard_vertex_matrices : std::logic_error { no_standard_vertex_matrices() : std::logic_error{ "neogfx::no_standard_vertex_matrices" } {} };

    class i_vertex_shader : public i_shader
    {
    public:
        typedef i_vertex_shader abstract_type;
    public:
        typedef neolib::i_map<i_string, abstract_t<shader_variable>*> attribute_map;
    public:
        virtual const attribute_map& attributes() const = 0;
        virtual void clear_attribute(const i_string& aName) = 0;
        virtual i_shader_variable& add_attribute(const i_string& aName, uint32_t aLocation, shader_data_type aType) = 0;
        template <typename T>
        i_shader_variable& add_attribute(const i_string& aName, shader_variable_location aLocation)
        {
            return add_attribute(aName, aLocation, static_cast<shader_data_type>(neolib::variant_index_of<T, shader_value_type>()));
        }    
    public:
        virtual bool has_standard_vertex_matrices() const = 0;
        virtual const i_standard_vertex_matrices& standard_vertex_matrices() const = 0;
        virtual i_standard_vertex_matrices& standard_vertex_matrices() = 0;
    };
}