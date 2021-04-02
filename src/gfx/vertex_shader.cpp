// vertex_shader.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/vertex_shader.hpp>

namespace neogfx
{
    standard_vertex_shader::standard_vertex_shader(std::string const& aName) :
        vertex_shader{ aName }
    {
        auto& coord = add_attribute<vec3f>("VertexPosition"_s, 0u);
        auto& color = add_attribute<vec4f>("VertexColor"_s, 1u);
        auto& function = add_attribute<vec4f>("VertexFunction"_s, 3u);
        add_out_variable<vec3f>("Coord"_s, 0u).link(coord);
        add_out_variable<vec4f>("Color"_s, 1u).link(color);
        add_out_variable<vec4f>("Function"_s, 3u).link(function);
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

    void standard_vertex_shader::set_projection_matrix(const optional_mat44& aProjectionMatrix)
    {
        if (iProjectionMatrix != aProjectionMatrix)
        {
            iProjectionMatrix = aProjectionMatrix;
            uProjectionMatrix.uniform().mutable_value();
        }
    }

    void standard_vertex_shader::set_transformation_matrix(const optional_mat44& aTransformationMatrix)
    {
        if (iTransformationMatrix != aTransformationMatrix)
        {
            iTransformationMatrix = aTransformationMatrix;
            uTransformationMatrix.uniform().mutable_value();
        }
    }

    void standard_vertex_shader::prepare_uniforms(const i_rendering_context& aContext, i_shader_program&)
    {
        if (iProjectionMatrix == std::nullopt)
        {
            auto const& logicalCoordinates = aContext.logical_coordinates();
            if (uProjectionMatrix.uniform().is_dirty() || iLogicalCoordinates == std::nullopt || *iLogicalCoordinates != logicalCoordinates)
            {
                iLogicalCoordinates = logicalCoordinates;
                auto const bottomLeft = logicalCoordinates.bottomLeft.as<float>();
                auto const topRight = logicalCoordinates.topRight.as<float>();
                float const left = bottomLeft.x;
                float const right = topRight.x;
                float const bottom = bottomLeft.y;
                float const top = topRight.y;
                float const zFar = 1.0f;
                float const zNear = -1.0f;
                uProjectionMatrix = mat44f{
                    { 2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left) },
                    { 0.0f, 2.0f / (top - bottom), 0.0f, -(top + bottom) / (top - bottom) },
                    { 0.0f, 0.0f, -2.0f / (zFar - zNear), -(zFar + zNear) / (zFar - zNear) },
                    { 0.0f, 0.0f, 0.0f, 1.0f } }.transposed();
            }
        }
        else if (uProjectionMatrix.uniform().is_dirty())
            uProjectionMatrix = iProjectionMatrix->as<float>().transposed();

        auto const& offset = aContext.offset();
        if (uTransformationMatrix.uniform().is_dirty() || iOffset == std::nullopt || *iOffset != offset)
        {
            iOffset = offset;
            if (iTransformationMatrix == std::nullopt)
                uTransformationMatrix = mat44f::identity();
            else
                uTransformationMatrix = iTransformationMatrix->as<float>();
            uTransformationMatrix.uniform().mutable_value().get<mat44f>()[3][0] += static_cast<float>(offset.x);
            uTransformationMatrix.uniform().mutable_value().get<mat44f>()[3][1] += static_cast<float>(offset.y);
        }
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

    standard_texture_vertex_shader::standard_texture_vertex_shader(std::string const& aName) :
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
                "void standard_texture_vertex_shader(inout vec3 coord, inout vec4 color, inout vec2 texCoord, inout vec4 function)\n"
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