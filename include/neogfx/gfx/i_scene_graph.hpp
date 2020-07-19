// i_scene_graph.hpp
/*
  neolib C++ App/Game Engine
  Copyright (c)  2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/core/i_optional.hpp>
#include <neolib/core/i_vector.hpp>
#include <neolib/plugin/i_plugin_variant.hpp>
#include <neogfx/core/numerical.hpp>

namespace neogfx
{
    namespace scene_graph
    {
        enum class accessor_component_type : uint32_t
        {
            BYTE            = 5120,
            UNSIGNED_BYTE   = 5121,
            SHORT           = 5122,
            UNSIGNED_SHORT  = 5123,
            UNSIGNED_INT    = 5125,
            FLOAT           = 5126
        };

        enum class accessor_type : uint32_t
        {
            SCALAR  = 0,
            VEC2    = 1,
            VEC3    = 2,
            VEC4    = 3,
            MAT2    = 4,
            MAT3    = 5,
            MAT4    = 6
        };

        enum class buffer_view_target : uint32_t
        {
            ARRAY_BUFFER            = 34962,
            ELEMENT_ARRAY_BUFFER    = 34963
        };

        enum class rendering_mode : uint32_t
        {
            POINTS          = 0,
            LINES           = 1,
            LINE_LOOP       = 2,
            LINE_STRIP      = 3,
            TRIANGLES       = 4,
            TRIANGLE_STRIP  = 5,
            TRIANGLE_FAN    = 6
        };

        enum class vertex_attribute : uint32_t
        {
            POSITION    = 0,
            NORMAL      = 1,
            TANGENT     = 2,
            TEXCOORD_0  = 3,
            TEXCOORD_1  = 4,
            COLOR_0     = 5,
            JOINTS_0    = 6,
            WEIGHTS_0   = 7
        };

        enum class camera_type : uint32_t
        {
            Perspective     = 0,
            Orthographic    = 1
        };

        enum class mag_filter : uint32_t
        {
            NEAREST = 9728,
            LINEAR  = 9729
        };

        enum class min_filter : uint32_t
        {
            NEAREST                 = 9728,
            LINEAR                  = 9729,
            NEAREST_MIPMAP_NEAREST  = 9984,
            LINEAR_MIPMAP_NEAREST   = 9985,
            NEAREST_MIPMAP_LINEAR   = 9986,
            LINEAR_MIPMAP_LINEAR    = 9987
        };

        enum class wrapping_mode : uint32_t
        {
            CLAMP_TO_EDGE   = 33071,
            MIRRORED_REPEAT = 33648,
            REPEAT          = 10497
        };

        enum class alpha_mode : uint32_t
        {
            Opaque  = 0,
            Mask    = 1,
            Blend   = 2
        };

        enum class tex_coord : uint32_t
        {
            TEXCOORD_0  = 0,
            TEXCOORD_1  = 1,
            TEXCOORD_2  = 2,
            TEXCOORD_3  = 3,
            TEXCOORD_4  = 4,
            TEXCOORD_5  = 5,
            TEXCOORD_6  = 6,
            TEXCOORD_7  = 7,
            TEXCOORD_8  = 8,
            TEXCOORD_9  = 9,
            TEXCOORD_10 = 10
        };

        class i_buffer
        {
        public:
            typedef i_buffer abstract_type;
        public:
            virtual ~i_buffer() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual std::size_t byte_length() const = 0;
            virtual void const* data() const = 0;
            virtual void* data() = 0;
        public:
            template <typename T>
            T const* data() const
            {
                return static_cast<T const*>(data());
            }
            template <typename T>
            T* data()
            {
                return static_cast<T*>(data());
            }
        };

        class i_buffer_view
        {
        public:
            typedef i_buffer_view abstract_type;
        public:
            virtual ~i_buffer_view() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual i_buffer const& buffer() const = 0;
            virtual i_buffer& buffer() = 0;
            virtual std::size_t byte_offset() const = 0;
            virtual std::size_t byte_length() const = 0;
            virtual std::size_t byte_stride() const = 0;
            virtual buffer_view_target target() const = 0;
        };

        class i_sparse_array
        {
        public:
            typedef i_sparse_array abstract_type;
        public:
            virtual ~i_sparse_array() = default;
        public:
            virtual i_buffer_view const& indices_buffer_view() const = 0;
            virtual i_buffer_view& indices_buffer_view() = 0;
            virtual std::size_t indices_byte_offset() const = 0;
            virtual accessor_component_type indices_component_type() const = 0;
            virtual i_buffer_view const& values_buffer_view() const = 0;
            virtual i_buffer_view& values_buffer_view() = 0;
            virtual std::size_t values_byte_offset() const = 0;
        };

        class i_accessor
        {
        public:
            typedef i_accessor abstract_type;
        public:
            virtual ~i_accessor() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual i_buffer_view const& buffer_view() const = 0;
            virtual i_buffer_view& buffer_view() = 0;
            virtual std::size_t byte_offset() const = 0;
            virtual accessor_component_type component_type() const = 0;
            virtual bool normalized() const = 0;
            virtual std::size_t count() const = 0;
            virtual accessor_type type() const = 0;
            virtual void const* max() const = 0;
            virtual void const* min() const = 0;
            virtual bool has_sparse() const = 0;
            virtual i_sparse_array const& sparse() const = 0;
            virtual i_sparse_array& sparse() = 0;
        };

        class i_attributes
        {
        public:
            typedef i_attributes abstract_type;
        public:
            virtual ~i_attributes() = default;
        public:
            virtual bool has_attribute(vertex_attribute aAttribute) const = 0;
            virtual i_accessor const& attribute(vertex_attribute aAttribute) const = 0;
            virtual i_accessor& attribute(vertex_attribute aAttribute) = 0;
        };

        class i_morph_target : public i_attributes
        {
        public:
            typedef i_morph_target abstract_type;
        };

        class i_image
        {
        public:
            typedef i_image abstract_type;
        public:
            virtual ~i_image() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual neolib::i_optional<neolib::i_string> const& uri() const = 0;
            virtual neolib::i_optional<neolib::i_string> const& mime_type() const = 0;
            virtual neolib::i_optional<i_buffer_view> const& buffer_view() const = 0;
        };

        class i_sampler
        {
        public:
            typedef i_sampler abstract_type;
        public:
            virtual ~i_sampler() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual scene_graph::mag_filter mag_filter() const = 0;
            virtual scene_graph::min_filter min_filter() const = 0;
            virtual wrapping_mode wrap_S() const = 0;
            virtual wrapping_mode wrap_T() const = 0;
        };

        class i_texture
        {
        public:
            typedef i_texture abstract_type;
        public:
            virtual ~i_texture() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual i_sampler const& sampler() const = 0;
            virtual i_sampler& sampler() = 0;
            virtual i_image const& source() const = 0;
            virtual i_image& source() = 0;
        };

        class i_texture_reference
        {
        public:
            typedef i_texture_reference abstract_type;
        public:
            virtual ~i_texture_reference() = default;
        public:
            virtual i_texture const& texture() const = 0;
            virtual i_texture& texture() = 0;
            virtual scene_graph::tex_coord tex_coord() const = 0;
        };

        class i_normal_texture : public i_texture_reference
        {
        public:
            typedef i_normal_texture abstract_type;
        public:
            virtual scalar scale() const = 0;
        };

        class i_occlusion_texture : public i_texture_reference
        {
        public:
            typedef i_occlusion_texture abstract_type;
        public:
            virtual scalar strength_cutoff() const = 0;
        };

        class i_emissive_texture : public i_texture_reference
        {
        public:
            typedef i_occlusion_texture abstract_type;
        };

        class i_pbr_metallic_roughness
        {
        public:
            typedef i_pbr_metallic_roughness abstract_type;
        public:
            virtual ~i_pbr_metallic_roughness() = default;
        public:
            virtual vec4 const& base_color_factor() const = 0;
            virtual i_texture_reference const& base_color_texture() const = 0;
            virtual i_texture_reference& base_color_texture() = 0;
            virtual scalar metallic_factor() const = 0;
            virtual scalar roughness_factor() const = 0;
            virtual i_texture_reference const& metallic_roughness_texture() const = 0;
            virtual i_texture_reference& metallic_roughness_texture() = 0;
        };

        class i_material
        {
        public:
            typedef i_material abstract_type;
        public:
            virtual ~i_material() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual i_pbr_metallic_roughness const& pbr_metallic_roughness() const = 0;
            virtual i_pbr_metallic_roughness& pbr_metallic_roughness() = 0;
            virtual i_normal_texture const& normal_texture() const = 0;
            virtual i_normal_texture& normal_texture() = 0;
            virtual i_occlusion_texture const& occlusion_texture() const = 0;
            virtual i_occlusion_texture& occlusion_texture() = 0;
            virtual i_emissive_texture const& emissive_texture() const = 0;
            virtual i_emissive_texture& emissive_texture() = 0;
            virtual vec3 const& emissive_factor() const = 0;
            virtual scalar alpha_cutoff() const = 0;
            virtual bool double_sided() const = 0;
        };
            
        class i_mesh_primitive
        {
        public:
            typedef i_mesh_primitive abstract_type;
        public:
            virtual ~i_mesh_primitive() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual rendering_mode mode() const = 0;
            virtual i_accessor const& indices() const = 0;
            virtual i_accessor& indices() = 0;
            virtual i_attributes const& attributes() const = 0;
            virtual i_attributes& attributes() = 0;
            virtual i_morph_target const& morph_targets() const = 0;
            virtual i_morph_target& morph_targets() = 0;
            virtual i_material const& material() const = 0;
            virtual i_material& material() = 0;
        };

        class i_mesh
        {
        public:
            typedef i_mesh abstract_type;
        public:
            virtual ~i_mesh() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual neolib::i_vector<scalar> const& weights() const = 0;
            virtual neolib::i_vector<scalar>& weights() = 0;
        };

        class i_camera
        {
        public:
            typedef i_camera abstract_type;
        public:
            virtual ~i_camera() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual camera_type type() = 0;
        };

        class i_orthographic_camera : public i_camera
        {
        public:
            typedef i_orthographic_camera abstract_type;
        public:
            virtual scalar xmag() const = 0;
            virtual scalar xmag() const = 0;
            virtual scalar zfar() const = 0;
            virtual scalar znear() const = 0;
        };

        class i_perspective_camera : public i_camera
        {
        public:
            typedef i_perspective_camera abstract_type;
        public:
            virtual neolib::i_optional<scalar> const& aspectRatio() const = 0;
            virtual scalar yfov() const = 0;
            virtual neolib::i_optional<scalar> const& zfar() const = 0;
            virtual scalar znear() const = 0;
        };

        class i_skin
        {
        public:
            typedef i_skin abstract_type;
        public:
            virtual ~i_skin() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        };

        class i_node
        {
        public:
            typedef i_node abstract_type;
        public:
            typedef mat44 matrix_transform;
            typedef struct { vec3 translation; vec4 rotation; vec3 scale; } trs_transform;
            enum class local_transform_flavour : uint32_t { Matrix, TRS };
            typedef neolib::i_plugin_variant<local_transform_flavour, matrix_transform, trs_transform> local_transform_type;
        public:
            virtual ~i_node() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual neolib::i_vector<i_node*> const& children() const = 0;
            virtual neolib::i_vector<i_node*>& children() = 0;
        public:
            virtual local_transform_type const& local_transform() const = 0;
            virtual local_transform_type& local_transform() = 0;
            virtual bool has_mesh() const = 0;
            virtual i_mesh const& mesh() const = 0;
            virtual i_mesh& mesh() = 0;
            virtual bool has_camera() const = 0;
            virtual i_camera const& camera() const = 0;
            virtual i_camera& camera() = 0;
            virtual bool has_skin() const = 0;
            virtual i_skin const& skin() const = 0;
            virtual i_skin& skin() = 0;
            virtual neolib::i_vector<scalar> const& weights() const = 0;
            virtual neolib::i_vector<scalar>& weights() = 0;
        };

        class i_scene
        {
        public:
            typedef i_scene abstract_type;
        public:
            virtual ~i_scene() = default;
        public:
            virtual i_node const& root() const = 0;
            virtual i_node& root() = 0;
        };
    }
}