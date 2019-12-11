// vertex_shader.hpp
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
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/shader.hpp>
#include <neogfx/gfx/i_vertex_shader.hpp>
#include <neogfx/gfx/i_shader_program.hpp>

namespace neogfx
{
    class vertex_shader : public shader<i_vertex_shader>
    {
        typedef shader<i_vertex_shader> base_type;
    public:
        typedef neolib::map<string, neolib::pair<uint32_t, std::size_t>> attribute_map;
    public:
        vertex_shader(const std::string& aName) :
            base_type{ shader_type::Vertex, aName }
        {
        }
    public:
        const attribute_map& attributes() const override
        {
            return iAttributes;
        }
        void clear_attribute(const i_string& aName) override
        {
            auto a = iAttributes.find(aName);
            if (a != iAttributes.end())
            {
                iAttributes.erase(a);
                invalidate();
            }
        }
        void add_attribute(const i_string& aName, uint32_t aLocation, std::size_t aTypeIndex) override
        {
            iAttributes.emplace(aName, neolib::make_pair(aLocation, aTypeIndex)); 
            invalidate();
        }
    public:
        bool has_standard_vertex_matrices() const override
        {
            return false;
        }
        const i_standard_vertex_matrices& standard_vertex_matrices() const override
        {
            throw no_standard_vertex_matrices();
        }
        i_standard_vertex_matrices& standard_vertex_matrices() override
        {
            throw no_standard_vertex_matrices();
        }
    private:
        attribute_map iAttributes;
    };

    class standard_vertex_shader : public vertex_shader, i_standard_vertex_matrices
    {
    public:
        standard_vertex_shader(const std::string& aName = "standard_vertex_shader") :
            vertex_shader{ aName }
        {
            add_attribute("VertexPosition"_s, 0u, neolib::index_of<vec3f, value_type>());
            add_attribute("VertexColor"_s, 1u, neolib::index_of<vec4f, value_type>());
            add_out_variable<vec2f>("OutputCoord"_s, 0u);
            add_out_variable<vec4f>("Color"_s, 1u);
        }
    public:
        bool has_standard_vertex_matrices() const override
        {
            return true;
        }
        const i_standard_vertex_matrices& standard_vertex_matrices() const override
        {
            return *this;
        }
        i_standard_vertex_matrices& standard_vertex_matrices() override
        {
            return *this;
        }
    public:
        mat44 projection_matrix(const i_rendering_context& aRenderingContext) const override
        {
            if (iProjectionMatrix != std::nullopt)
                return *iProjectionMatrix;
            auto const& logicalCoordinates = aRenderingContext.logical_coordinates();
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
        void set_projection_matrix(const optional_mat44& aProjectionMatrix) override
        {
            iProjectionMatrix = aProjectionMatrix;
        }
        mat44 transformation_matrix(const i_rendering_context&) const override
        {
            if (iTransformationMatrix != std::nullopt)
                return *iTransformationMatrix;
            return mat44::identity();
        }
        void set_transformation_matrix(const optional_mat44& aTransformationMatrix) override
        {
            iTransformationMatrix = aTransformationMatrix;
        }
    public:
        void prepare_uniforms(const i_rendering_context& aRenderingContext, i_shader_program&) override
        {
            set_uniform("uProjectionMatrix"_s, projection_matrix(aRenderingContext).transposed());
            set_uniform("uTransformationMatrix"_s, transformation_matrix(aRenderingContext));
        }
        const i_string& generate_code(i_shader_program& aProgram, shader_language aLanguage) const override
        {
            if (aLanguage == shader_language::Glsl)
            {
                static const string sSource =
                {
                    "void " + name().to_std_string() + "()\n"
                    "{\n"
                    "    gl_Position = uProjectionMatrix * (uTransformationMatrix * vec4(VertexPosition, 1.0));\n"
                    "    OutputCoord = VertexPosition.xy;\n"
                    "    Color = VertexColor;\n"
                    "}\n"
                };
                return sSource;
            }
            else
                throw unsupported_language();
        }
    private:
        attribute_map iAttributes;
        optional_mat44 iProjectionMatrix;
        optional_mat44 iTransformationMatrix;
    };

    class standard_texture_vertex_shader : public standard_vertex_shader
    {
    public:
        standard_texture_vertex_shader(const std::string& aName = "standard_texture_vertex_shader") :
            standard_vertex_shader{ aName }
        {
            add_attribute("VertexTextureCoord"_s, 2u, neolib::index_of<vec2f, value_type>());
            add_out_variable<vec2f>("TexCoord"_s, 2u);
        }
    public:
        const i_string& generate_code(i_shader_program& aProgram, shader_language aLanguage) const override
        {
            if (aLanguage == shader_language::Glsl)
            {
                static const string sSource =
                {
                    "void " + name().to_std_string() + "()\n"
                    "{\n"
                    "    gl_Position = uProjectionMatrix * (uTransformationMatrix * vec4(VertexPosition, 1.0));\n"
                    "    OutputCoord = VertexPosition.xy;\n"
                    "    Color = VertexColor;\n"
                    "    TexCoord = VertexTextureCoord;\n"
                    "}\n"
                };
                return sSource;
            }
            else
                throw unsupported_language();
        }
    private:
    };
}