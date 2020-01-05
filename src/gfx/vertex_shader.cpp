// vertex_shader.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/vertex_shader.hpp>

namespace neogfx
{
    standard_vertex_shader::standard_vertex_shader(const std::string& aName) :
        vertex_shader{ aName }
    {
        auto& coord = add_attribute<vec3f>("VertexPosition"_s, 0u);
        auto& color = add_attribute<vec4f>("VertexColor"_s, 1u);
        add_out_variable<vec3f>("Coord"_s, 0u).link(coord);
        add_out_variable<vec4f>("Color"_s, 1u).link(color);
    }

    bool standard_vertex_shader::has_standard_vertex_matrices() const
    {
        return true;
    }

    const i_standard_vertex_matrices& standard_vertex_shader::standard_vertex_matrices() const
    {
        return *this;
    }

    i_standard_vertex_matrices& standard_vertex_shader::standard_vertex_matrices()
    {
        return *this;
    }

    mat44 standard_vertex_shader::projection_matrix(const i_rendering_context& aContext) const
    {
        if (iProjectionMatrix != std::nullopt)
            return *iProjectionMatrix;
        auto const& logicalCoordinates = aContext.logical_coordinates();
        double left = logicalCoordinates.bottomLeft.x;
        double right = logicalCoordinates.topRight.x;
        double bottom = logicalCoordinates.bottomLeft.y;
        double top = logicalCoordinates.topRight.y;
        double zFar = 1.0;
        double zNear = -1.0;
        mat44 orthoMatrix = mat44{
            { 2.0 / (right - left), 0.0, 0.0, -(right + left) / (right - left) },
            { 0.0, 2.0 / (top - bottom), 0.0, -(top + bottom) / (top - bottom) },
            { 0.0, 0.0, -2.0 / (zFar - zNear), -(zFar + zNear) / (zFar - zNear) },
            { 0.0, 0.0, 0.0, 1.0 } };
        return orthoMatrix;
    }

    void standard_vertex_shader::set_projection_matrix(const optional_mat44& aProjectionMatrix)
    {
        iProjectionMatrix = aProjectionMatrix;
    }

    mat44 standard_vertex_shader::transformation_matrix(const i_rendering_context& aContext) const
    {
        auto transform = mat44::identity();
        if (iTransformationMatrix != std::nullopt)
            transform = *iTransformationMatrix;
        transform[3][0] += aContext.offset().x;
        transform[3][1] += aContext.offset().y;
        return transform;
    }

    void standard_vertex_shader::set_transformation_matrix(const optional_mat44& aTransformationMatrix)
    {
        iTransformationMatrix = aTransformationMatrix;
    }

    void standard_vertex_shader::prepare_uniforms(const i_rendering_context& aContext, i_shader_program&)
    {
        uProjectionMatrix = projection_matrix(aContext).transposed().as<float>();
        uTransformationMatrix = transformation_matrix(aContext).as<float>();
    }
    
    void standard_vertex_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        vertex_shader::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
        {
            static const string code =
            {
                "void standard_vertex_shader(inout vec3 coord, inout vec4 color)\n"
                "{\n"
                "    gl_Position = vec4((uProjectionMatrix * (uTransformationMatrix * vec4(coord, 1.0))).xyz, 1.0);\n"
                "}\n"_s
            };
            aOutput += code;
        }
        else
            throw unsupported_shader_language();
    }

    standard_texture_vertex_shader::standard_texture_vertex_shader(const std::string& aName) :
        standard_vertex_shader{ aName }
    {
        auto& texCoord = add_attribute("VertexTextureCoord"_s, 2u, shader_data_type::Vec2);
        add_out_variable<vec2f>("TexCoord"_s, 2u).link(texCoord);
    }

    void standard_texture_vertex_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_vertex_shader::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
        {
            static const string code =
            {
                "void standard_texture_vertex_shader(inout vec3 coord, inout vec4 color, inout vec2 texCoord)\n"
                "{\n"
                "    standard_vertex_shader(coord, color);\n"
                "}\n"_s
            };
            aOutput += code;
        }
        else
            throw unsupported_shader_language();
    }
}