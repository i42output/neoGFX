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
#include "standard.vert.hpp"
#include "standard-texture.vert.hpp"

namespace neogfx
{
    standard_vertex_shader::standard_vertex_shader(std::string const& aName) :
        vertex_shader{ aName }, iOpacity{ 1.0 }, iModelTableBase{ 0u }, iSceneLighting{ scene_lighting::None }
    {
        auto& coord = add_attribute<vec3f>("VertexPosition"_s, 0u);
        auto& color = add_attribute<vec4f>("VertexColor"_s, 1u);
        auto& function0 = add_attribute<vec4f>("VertexFunction0"_s, 3u);
        auto& function1 = add_attribute<vec4f>("VertexFunction1"_s, 4u);
        auto& function2 = add_attribute<vec4f>("VertexFunction2"_s, 5u);
        auto& function3 = add_attribute<vec4f>("VertexFunction3"_s, 6u);
        auto& function4 = add_attribute<vec4f>("VertexFunction4"_s, 7u);
        auto& function5 = add_attribute<vec4f>("VertexFunction5"_s, 8u);
        auto& function6 = add_attribute<vec4f>("VertexFunction6"_s, 9u);
        add_attribute<vec3f>("VertexDebug"_s, 10u);
        add_attribute<float>("VertexModel"_s, 11u);
        add_attribute<vec4f>("VertexJoints"_s, 12u);
        add_attribute<vec4f>("VertexWeights"_s, 13u);
        auto& normal = add_attribute<vec3f>("VertexNormal"_s, 14u);
        uModelTableBase = iModelTableBase;
        uSceneLight = vec4f{};
        uScenePointLight = vec4f{};
        uScenePointLightRadiance = vec4f{};
        add_out_variable<vec3f>("Coord"_s, 0u).link(coord);
        add_out_variable<vec4f>("Color"_s, 1u).link(color);
        add_out_variable<vec4f>("Function0"_s, 3u, true).link(function0);
        add_out_variable<vec4f>("Function1"_s, 4u, true).link(function1);
        add_out_variable<vec4f>("Function2"_s, 5u, true).link(function2);
        add_out_variable<vec4f>("Function3"_s, 6u, true).link(function3);
        add_out_variable<vec4f>("Function4"_s, 7u, true).link(function4);
        add_out_variable<vec4f>("Function5"_s, 8u, true).link(function5);
        add_out_variable<vec4f>("Function6"_s, 9u, true).link(function6);
        // world space normal of model transformed vertices (for per pixel lighting; see i_pbr_shader)
        add_out_variable<vec3f>("WorldNormal"_s, 10u).link(normal);
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

    void standard_vertex_shader::set_model_table_base(std::uint32_t aBase)
    {
        if (iModelTableBase != aBase)
        {
            iModelTableBase = aBase;
            uModelTableBase.uniform().mutable_value();
        }
    }

    void standard_vertex_shader::set_scene_light(std::optional<vec3> const& aDirection, scene_lighting aLighting)
    {
        if (iSceneLight != aDirection || iSceneLighting != aLighting)
        {
            iSceneLight = aDirection;
            iSceneLighting = aLighting;
            uSceneLight.uniform().mutable_value();
        }
    }

    void standard_vertex_shader::set_scene_point_light(std::optional<vec3> const& aPosition, vec3 const& aRadiance, scalar aRange)
    {
        vec4 const radiance{ aRadiance.x, aRadiance.y, aRadiance.z, aRange };
        if (iScenePointLight != aPosition || iScenePointLightRadiance != radiance)
        {
            iScenePointLight = aPosition;
            iScenePointLightRadiance = radiance;
            uScenePointLight.uniform().mutable_value();
        }
    }

    bool standard_vertex_shader::supports(vertex_buffer_type aBufferType) const
    {
        return (aBufferType & (vertex_buffer_type::Model | vertex_buffer_type::Joints | vertex_buffer_type::Weights | vertex_buffer_type::Normal)) != vertex_buffer_type::Invalid;
    }

    void standard_vertex_shader::set_opacity(scalar aOpacity)
    {
        if (iOpacity != aOpacity)
        {
            iOpacity = aOpacity;
            uOpacity.uniform().mutable_value();
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
                float const zFar = static_cast<float>(logicalCoordinates.z_far());
                float const zNear = static_cast<float>(logicalCoordinates.z_near());
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
            // the offset is a translation applied after the transformation; n.b. this is the same as adding it to the
            // translation column when the transformation is affine (bottom row 0, 0, 0, 1) but also correct if it is projective
            auto& transformation = uTransformationMatrix.uniform().mutable_value().get<mat44f>();
            for (std::uint32_t column = 0u; column < 4u; ++column)
            {
                transformation[column][0] += static_cast<float>(offset.x) * transformation[column][3];
                transformation[column][1] += static_cast<float>(offset.y) * transformation[column][3];
            }
        }

        if (uOpacity.uniform().is_dirty())
            uOpacity = static_cast<float>(iOpacity);

        if (uModelTableBase.uniform().is_dirty())
            uModelTableBase = iModelTableBase;

        if (uSceneLight.uniform().is_dirty())
        {
            // xyz: direction towards the light (normalized), w: the lighting (scene_lighting; 0 if unlit)
            if (iSceneLight && iSceneLight->magnitude() > 0.0 && iSceneLighting != scene_lighting::None)
            {
                auto const direction = iSceneLight->normalized().as<float>();
                uSceneLight = vec4f{ direction.x, direction.y, direction.z, static_cast<float>(iSceneLighting) };
            }
            else
                uSceneLight = vec4f{};
        }

        // the point light is in the same space as the model transformed vertices, which includes the context origin (see draw_entities)
        if (uScenePointLight.uniform().is_dirty() || iScenePointLightOrigin == std::nullopt || *iScenePointLightOrigin != aContext.origin())
        {
            iScenePointLightOrigin = aContext.origin();
            // xyz: position, w: 1 if lit; radiance: rgb and range (0 for none)
            if (iScenePointLight)
            {
                auto const position = (*iScenePointLight + iScenePointLightOrigin->to_vec3()).as<float>();
                uScenePointLight = vec4f{ position.x, position.y, position.z, 1.0f };
                uScenePointLightRadiance = iScenePointLightRadiance.as<float>();
            }
            else
                uScenePointLight = vec4f{};
        }
    }

    void standard_vertex_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        vertex_shader::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardVertexShader };
        else
            throw unsupported_shader_language();
    }

    standard_texture_vertex_shader::standard_texture_vertex_shader(std::string const& aName) :
        standard_vertex_shader{ aName }
    {
        auto& texCoord = add_attribute("VertexTextureCoord"_s, 2u, false, shader_data_type::Vec2);
        add_out_variable<vec2f>("TexCoord"_s, 2u).link(texCoord);
    }

    void standard_texture_vertex_shader::generate_code(const i_shader_program& aProgram, shader_language aLanguage, i_string& aOutput) const
    {
        standard_vertex_shader::generate_code(aProgram, aLanguage, aOutput);
        if (aLanguage == shader_language::Glsl)
            aOutput += string{ glsl::StandardTextureVertexShader };
        else
            throw unsupported_shader_language();
    }
}