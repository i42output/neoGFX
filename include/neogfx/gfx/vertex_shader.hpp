// vertex_shader.hpp
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

#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/shader.hpp>
#include <neogfx/gfx/i_vertex_shader.hpp>
#include <neogfx/gfx/i_shader_program.hpp>

namespace neogfx
{
    template <typename Base = i_vertex_shader>
    class vertex_shader : public shader<Base>
    {
        typedef shader<Base> base_type;
    public:
        typedef neolib::map<string, abstract_t<shader_variable>*> attribute_map;
    public:
        vertex_shader(std::string const& aName) :
            base_type{ shader_type::Vertex, aName }
        {
        }
    public:
        const attribute_map& attributes() const final
        {
            return iAttributes;
        }
        void clear_attribute(const i_string& aName) final
        {
            auto a = iAttributes.find(aName);
            if (a != iAttributes.end())
            {
                iAttributes.erase(a);
                auto v = base_type::in_variables().find(*a->second());
                if (v != base_type::in_variables().end())
                    base_type::in_variables().erase(v);
                base_type::set_dirty();
            }
        }
        using base_type::add_attribute;
        i_shader_variable& add_attribute(const i_string& aName, std::uint32_t aLocation, bool aFlat, shader_data_type aType) final
        {
            auto& in = base_type::add_variable(
                shader_variable
                {
                    aName,
                    aLocation,
                    aFlat ? shader_variable_qualifier::In | shader_variable_qualifier::Flat : shader_variable_qualifier::In,
                    aType
                });
            iAttributes.emplace(aName, &in);
            base_type::set_dirty();
            return in;
        }
    private:
        attribute_map iAttributes;
    };

    class standard_vertex_shader : public vertex_shader<i_standard_vertex_shader>
    {
    public:
        standard_vertex_shader(std::string const& aName = "standard_vertex_shader");
    public:
        void set_projection_matrix(const optional_mat44& aProjectionMatrix) final;
        void set_transformation_matrix(const optional_mat44& aTransformationMatrix) final;
        void set_opacity(scalar aOpacity) final;
    public:
        void prepare_uniforms(const i_rendering_context& aContext, i_shader_program& aProgram) override;
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override;
    private:
        optional_mat44 iProjectionMatrix;
        optional_mat44 iTransformationMatrix;
        scalar iOpacity;
    private:
        cache_uniform(uProjectionMatrix)
        cache_uniform(uTransformationMatrix)
        cache_uniform(uOpacity)
        optional_logical_coordinates iLogicalCoordinates;
        optional_vec2 iOffset;
    };

    class standard_texture_vertex_shader : public standard_vertex_shader
    {
    public:
        standard_texture_vertex_shader(std::string const& aName = "standard_texture_vertex_shader");
    public:
        void generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const override;
    };
}