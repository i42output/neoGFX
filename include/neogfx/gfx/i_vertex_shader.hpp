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

    // how model transformed vertices (see game::model_transformation) are lit
    enum class scene_lighting : std::uint32_t
    {
        None            = 0,
        PerVertex       = 1,    // ambient plus diffuse, per vertex
        PhysicallyBased = 2     // per pixel, glTF metallic-roughness (see i_pbr_shader)
    };

    // a point light lighting model transformed vertices (see i_standard_vertex_shader::set_scene_point_lights): radiance (colour
    // times intensity; the directional light's is 2.8) falls off with the square of the distance and, if a range is given, smoothly
    // to zero at that range; shadows (by shadow mapping) are cast with physically based shading
    struct scene_point_light
    {
        vec3 position;      // world space
        vec3 radiance;
        scalar range = 0.0;
        bool castsShadows = false;
        scalar size = 0.0;  // the radius of what emits it (e.g. a lamp's mesh), which casts no shadow from it

        bool operator==(scene_point_light const&) const = default;
    };

    class i_vertex_shader : public i_shader
    {
    public:
        typedef i_vertex_shader abstract_type;
    public:
        typedef neolib::i_map<i_string, maybe_abstract_t<shader_variable>*> attribute_map;
    public:
        virtual const attribute_map& attributes() const = 0;
        virtual void clear_attribute(const i_string& aName) = 0;
        virtual i_shader_variable& add_attribute(const i_string& aName, std::uint32_t aLocation, bool aFlat, shader_data_type aType) = 0;
        template <typename T>
        i_shader_variable& add_attribute(const i_string& aName, shader_variable_location aLocation, bool aFlat = false)
        {
            return add_attribute(aName, aLocation, aFlat, static_cast<shader_data_type>(neolib::variant_index_of<T, shader_value_type::variant_type>()));
        }    
    };

    class i_standard_vertex_shader : public i_vertex_shader
    {
    public:
        virtual void set_projection_matrix(const optional_mat44& aProjectionMatrix) = 0;
        virtual void set_transformation_matrix(const optional_mat44& aProjectionMatrix) = 0;
        virtual void set_opacity(scalar aOpacity) = 0;
        // the start of the current frame's model table (see game::model_transformation)
        virtual void set_model_table_base(std::uint32_t aBase) = 0;
        // a directional light (world space direction towards the light) lighting model transformed vertices
        // (see game::model_transformation) by their normals; std::nullopt (or scene_lighting::None) for none (unlit).
        // n.b. for scene_lighting::PhysicallyBased the shading is done by the PBR fragment shader (i_pbr_shader)
        virtual void set_scene_light(std::optional<vec3> const& aDirection, scene_lighting aLighting = scene_lighting::PerVertex) = 0;
        // point lights also lighting model transformed vertices (by both scene_lighting::PerVertex and PhysicallyBased); the
        // renderer puts them (in the space of the transformed vertices) in i_standard_shader_program::scene_lights() when drawing
        virtual std::vector<scene_point_light> const& scene_point_lights() const = 0;
        virtual void set_scene_point_lights(std::vector<scene_point_light> const& aLights) = 0;
        // where the renderer put them: the first element (two per light) and count
        virtual void set_scene_light_buffer(std::uint32_t aBase, std::uint32_t aCount) = 0;
    };
}