// i_scene_graph.hpp
/*
  neolib C++ App/Game Engine
  Copyright (c) 2020, 2026 Leigh Johnston.  All Rights Reserved.

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
#include <neolib/core/i_string.hpp>

#include <neogfx/core/numerical.hpp>

// The scene graph model mirrors the glTF 2.0 document structure: top-level arrays of
// objects that refer to each other by index. A 2D scene graph is an ordinary glTF
// document flagged with the NEOGFX_scene_2d extension; its geometry lies in the z = 0
// plane, its rotations are about the z axis and node z translations are drawing order.

namespace neogfx
{
    namespace scene_graph
    {
        using index = std::uint32_t;
        constexpr index invalid_index = ~index{};

        enum class dimension : std::uint32_t
        {
            Two     = 2,
            Three   = 3
        };

        enum class accessor_component_type : std::uint32_t
        {
            BYTE            = 5120,
            UNSIGNED_BYTE   = 5121,
            SHORT           = 5122,
            UNSIGNED_SHORT  = 5123,
            UNSIGNED_INT    = 5125,
            FLOAT           = 5126
        };

        enum class accessor_type : std::uint32_t
        {
            SCALAR  = 0,
            VEC2    = 1,
            VEC3    = 2,
            VEC4    = 3,
            MAT2    = 4,
            MAT3    = 5,
            MAT4    = 6
        };

        enum class buffer_view_target : std::uint32_t
        {
            None                    = 0,
            ARRAY_BUFFER            = 34962,
            ELEMENT_ARRAY_BUFFER    = 34963
        };

        enum class rendering_mode : std::uint32_t
        {
            POINTS          = 0,
            LINES           = 1,
            LINE_LOOP       = 2,
            LINE_STRIP      = 3,
            TRIANGLES       = 4,
            TRIANGLE_STRIP  = 5,
            TRIANGLE_FAN    = 6
        };

        enum class vertex_attribute : std::uint32_t
        {
            POSITION    = 0,
            NORMAL      = 1,
            TANGENT     = 2,
            TEXCOORD_0  = 3,
            TEXCOORD_1  = 4,
            COLOR_0     = 5,
            JOINTS_0    = 6,
            WEIGHTS_0   = 7,
            COUNT
        };

        enum class camera_type : std::uint32_t
        {
            Perspective     = 0,
            Orthographic    = 1
        };

        enum class mag_filter : std::uint32_t
        {
            None    = 0,
            NEAREST = 9728,
            LINEAR  = 9729
        };

        enum class min_filter : std::uint32_t
        {
            None                    = 0,
            NEAREST                 = 9728,
            LINEAR                  = 9729,
            NEAREST_MIPMAP_NEAREST  = 9984,
            LINEAR_MIPMAP_NEAREST   = 9985,
            NEAREST_MIPMAP_LINEAR   = 9986,
            LINEAR_MIPMAP_LINEAR    = 9987
        };

        enum class wrapping_mode : std::uint32_t
        {
            CLAMP_TO_EDGE   = 33071,
            MIRRORED_REPEAT = 33648,
            REPEAT          = 10497
        };

        enum class alpha_mode : std::uint32_t
        {
            Opaque  = 0,
            Mask    = 1,
            Blend   = 2
        };

        enum class tex_coord : std::uint32_t
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

        inline std::uint32_t component_count(accessor_type aType)
        {
            switch (aType)
            {
            case accessor_type::SCALAR:
                return 1u;
            case accessor_type::VEC2:
                return 2u;
            case accessor_type::VEC3:
                return 3u;
            case accessor_type::VEC4:
            case accessor_type::MAT2:
                return 4u;
            case accessor_type::MAT3:
                return 9u;
            case accessor_type::MAT4:
                return 16u;
            default:
                return 0u;
            }
        }

        inline std::uint32_t component_size(accessor_component_type aComponentType)
        {
            switch (aComponentType)
            {
            case accessor_component_type::BYTE:
            case accessor_component_type::UNSIGNED_BYTE:
                return 1u;
            case accessor_component_type::SHORT:
            case accessor_component_type::UNSIGNED_SHORT:
                return 2u;
            case accessor_component_type::UNSIGNED_INT:
            case accessor_component_type::FLOAT:
                return 4u;
            default:
                return 0u;
            }
        }

        class i_asset
        {
        public:
            typedef i_asset abstract_type;
        public:
            virtual ~i_asset() = default;
        public:
            virtual neolib::i_string const& version() const = 0;
            virtual neolib::i_optional<neolib::i_string> const& min_version() const = 0;
            virtual neolib::i_optional<neolib::i_string> const& generator() const = 0;
            virtual neolib::i_optional<neolib::i_string> const& copyright() const = 0;
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
            // no uri: the data is in the GLB binary chunk (or, when writing a .gltf, embedded as a data URI)
            virtual neolib::i_optional<neolib::i_string> const& uri() const = 0;
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
            virtual index buffer() const = 0;
            virtual std::size_t byte_offset() const = 0;
            virtual std::size_t byte_length() const = 0;
            // 0: tightly packed
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
            virtual std::size_t count() const = 0;
            virtual index indices_buffer_view() const = 0;
            virtual std::size_t indices_byte_offset() const = 0;
            virtual accessor_component_type indices_component_type() const = 0;
            virtual index values_buffer_view() const = 0;
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
            // no buffer view: all elements are zero (unless overridden by sparse values)
            virtual bool has_buffer_view() const = 0;
            virtual index buffer_view() const = 0;
            virtual std::size_t byte_offset() const = 0;
            virtual accessor_component_type component_type() const = 0;
            virtual bool normalized() const = 0;
            virtual std::size_t count() const = 0;
            virtual accessor_type type() const = 0;
            virtual neolib::i_vector<scalar> const& max() const = 0;
            virtual neolib::i_vector<scalar> const& min() const = 0;
            virtual bool has_sparse() const = 0;
            virtual i_sparse_array const& sparse() const = 0;
        };

        class i_attributes
        {
        public:
            typedef i_attributes abstract_type;
        public:
            virtual ~i_attributes() = default;
        public:
            virtual bool has_attribute(vertex_attribute aAttribute) const = 0;
            // the accessor holding the attribute's data
            virtual index attribute(vertex_attribute aAttribute) const = 0;
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
            virtual bool has_buffer_view() const = 0;
            virtual index buffer_view() const = 0;
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
            virtual bool has_sampler() const = 0;
            virtual index sampler() const = 0;
            virtual bool has_source() const = 0;
            virtual index source() const = 0;
        };

        class i_texture_reference
        {
        public:
            typedef i_texture_reference abstract_type;
        public:
            virtual ~i_texture_reference() = default;
        public:
            virtual bool has_texture() const = 0;
            virtual index texture() const = 0;
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
            virtual scalar strength() const = 0;
        };

        class i_emissive_texture : public i_texture_reference
        {
        public:
            typedef i_emissive_texture abstract_type;
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
            virtual scalar metallic_factor() const = 0;
            virtual scalar roughness_factor() const = 0;
            virtual i_texture_reference const& metallic_roughness_texture() const = 0;
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
            virtual i_normal_texture const& normal_texture() const = 0;
            virtual i_occlusion_texture const& occlusion_texture() const = 0;
            virtual i_emissive_texture const& emissive_texture() const = 0;
            virtual vec3 const& emissive_factor() const = 0;
            virtual scene_graph::alpha_mode alpha_mode() const = 0;
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
            virtual rendering_mode mode() const = 0;
            virtual bool has_indices() const = 0;
            virtual index indices() const = 0;
            virtual i_attributes const& attributes() const = 0;
            virtual std::uint32_t morph_target_count() const = 0;
            virtual i_morph_target const& morph_target(std::uint32_t aIndex) const = 0;
            virtual bool has_material() const = 0;
            virtual index material() const = 0;
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
            virtual std::uint32_t primitive_count() const = 0;
            virtual i_mesh_primitive const& primitive(std::uint32_t aIndex) const = 0;
            virtual neolib::i_vector<scalar> const& weights() const = 0;
        };

        class i_orthographic_camera
        {
        public:
            typedef i_orthographic_camera abstract_type;
        public:
            virtual ~i_orthographic_camera() = default;
        public:
            virtual scalar xmag() const = 0;
            virtual scalar ymag() const = 0;
            virtual scalar zfar() const = 0;
            virtual scalar znear() const = 0;
        };

        class i_perspective_camera
        {
        public:
            typedef i_perspective_camera abstract_type;
        public:
            virtual ~i_perspective_camera() = default;
        public:
            virtual neolib::i_optional<scalar> const& aspect_ratio() const = 0;
            virtual scalar yfov() const = 0;
            // no zfar: infinite projection
            virtual neolib::i_optional<scalar> const& zfar() const = 0;
            virtual scalar znear() const = 0;
        };

        class i_camera
        {
        public:
            typedef i_camera abstract_type;
        public:
            struct wrong_camera_type : std::logic_error { wrong_camera_type() : std::logic_error{ "neogfx::scene_graph::i_camera::wrong_camera_type" } {} };
        public:
            virtual ~i_camera() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual camera_type type() const = 0;
            virtual i_orthographic_camera const& orthographic() const = 0;
            virtual i_perspective_camera const& perspective() const = 0;
        };

        class i_skin
        {
        public:
            typedef i_skin abstract_type;
        public:
            virtual ~i_skin() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual bool has_inverse_bind_matrices() const = 0;
            virtual index inverse_bind_matrices() const = 0;
            virtual bool has_skeleton() const = 0;
            virtual index skeleton() const = 0;
            virtual neolib::i_vector<index> const& joints() const = 0;
        };

        enum class animation_path : std::uint32_t
        {
            Translation = 0,
            Rotation    = 1,
            Scale       = 2,
            Weights     = 3
        };

        enum class animation_interpolation : std::uint32_t
        {
            Linear      = 0,
            Step        = 1,
            CubicSpline = 2
        };

        class i_animation_sampler
        {
        public:
            typedef i_animation_sampler abstract_type;
        public:
            virtual ~i_animation_sampler() = default;
        public:
            // accessor of keyframe times (seconds)
            virtual index input() const = 0;
            virtual animation_interpolation interpolation() const = 0;
            // accessor of keyframe values (for CubicSpline: in-tangent, value, out-tangent per keyframe)
            virtual index output() const = 0;
        };

        class i_animation_channel
        {
        public:
            typedef i_animation_channel abstract_type;
        public:
            virtual ~i_animation_channel() = default;
        public:
            // the index of the sampler in the channel's animation
            virtual index sampler() const = 0;
            virtual bool has_target_node() const = 0;
            virtual index target_node() const = 0;
            virtual animation_path target_path() const = 0;
        };

        class i_animation
        {
        public:
            typedef i_animation abstract_type;
        public:
            virtual ~i_animation() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual std::uint32_t channel_count() const = 0;
            virtual i_animation_channel const& channel(std::uint32_t aIndex) const = 0;
            virtual std::uint32_t sampler_count() const = 0;
            virtual i_animation_sampler const& sampler(std::uint32_t aIndex) const = 0;
        };

        class i_node
        {
        public:
            typedef i_node abstract_type;
        public:
            typedef mat44 matrix_transform;
            // rotation is a unit quaternion (x, y, z, w)
            struct trs_transform
            {
                vec3 translation = vec3{ 0.0, 0.0, 0.0 };
                vec4 rotation = vec4{ 0.0, 0.0, 0.0, 1.0 };
                vec3 scale = vec3{ 1.0, 1.0, 1.0 };
                bool operator==(trs_transform const&) const = default;
            };
            enum class local_transform_flavour : std::uint32_t { Matrix, TRS };
        public:
            virtual ~i_node() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            virtual neolib::i_vector<index> const& children() const = 0;
            virtual neolib::i_vector<index>& children() = 0;
        public:
            virtual local_transform_flavour transform_flavour() const = 0;
            virtual matrix_transform const& matrix() const = 0;
            virtual trs_transform const& trs() const = 0;
            virtual void set_matrix(matrix_transform const& aMatrix) = 0;
            virtual void set_trs(trs_transform const& aTrs) = 0;
            virtual bool has_mesh() const = 0;
            virtual index mesh() const = 0;
            virtual bool has_camera() const = 0;
            virtual index camera() const = 0;
            virtual bool has_skin() const = 0;
            virtual index skin() const = 0;
            virtual neolib::i_vector<scalar> const& weights() const = 0;
            virtual neolib::i_vector<scalar>& weights() = 0;
        public:
            // the local transformation as a matrix whichever flavour is stored
            mat44 local_matrix() const
            {
                if (transform_flavour() == local_transform_flavour::Matrix)
                    return matrix();
                return to_matrix(trs());
            }
            static mat44 to_matrix(trs_transform const& aTrs)
            {
                auto const& q = aTrs.rotation;
                scalar const x = q[0], y = q[1], z = q[2], w = q[3];
                auto const& s = aTrs.scale;
                auto const& t = aTrs.translation;
                // column-major, as glTF
                return mat44{
                    { (1.0 - 2.0 * (y * y + z * z)) * s.x, (2.0 * (x * y + z * w)) * s.x, (2.0 * (x * z - y * w)) * s.x, 0.0 },
                    { (2.0 * (x * y - z * w)) * s.y, (1.0 - 2.0 * (x * x + z * z)) * s.y, (2.0 * (y * z + x * w)) * s.y, 0.0 },
                    { (2.0 * (x * z + y * w)) * s.z, (2.0 * (y * z - x * w)) * s.z, (1.0 - 2.0 * (x * x + y * y)) * s.z, 0.0 },
                    { t.x, t.y, t.z, 1.0 } };
            }
        };

        class i_scene
        {
        public:
            typedef i_scene abstract_type;
        public:
            virtual ~i_scene() = default;
        public:
            virtual neolib::i_optional<neolib::i_string> const& name() const = 0;
        public:
            // the root nodes of the scene
            virtual neolib::i_vector<index> const& nodes() const = 0;
            virtual neolib::i_vector<index>& nodes() = 0;
        };

        class i_scene_graph
        {
        public:
            typedef i_scene_graph abstract_type;
        public:
            struct bad_index : std::out_of_range { bad_index() : std::out_of_range{ "neogfx::scene_graph::i_scene_graph::bad_index" } {} };
        public:
            virtual ~i_scene_graph() = default;
        public:
            virtual scene_graph::dimension dimension() const = 0;
            virtual i_asset const& asset() const = 0;
            // incremented whenever the graph's structure (anything other than a node transformation) changes
            virtual std::uint64_t revision() const = 0;
            // the directory against which relative buffer and image URIs are resolved (empty if none)
            virtual neolib::i_string const& base_directory() const = 0;
        public:
            virtual std::uint32_t buffer_count() const = 0;
            virtual i_buffer const& buffer(index aIndex) const = 0;
            virtual std::uint32_t buffer_view_count() const = 0;
            virtual i_buffer_view const& buffer_view(index aIndex) const = 0;
            virtual std::uint32_t accessor_count() const = 0;
            virtual i_accessor const& accessor(index aIndex) const = 0;
            virtual std::uint32_t image_count() const = 0;
            virtual i_image const& image(index aIndex) const = 0;
            virtual std::uint32_t sampler_count() const = 0;
            virtual i_sampler const& sampler(index aIndex) const = 0;
            virtual std::uint32_t texture_count() const = 0;
            virtual i_texture const& texture(index aIndex) const = 0;
            virtual std::uint32_t material_count() const = 0;
            virtual i_material const& material(index aIndex) const = 0;
            virtual std::uint32_t mesh_count() const = 0;
            virtual i_mesh const& mesh(index aIndex) const = 0;
            virtual std::uint32_t camera_count() const = 0;
            virtual i_camera const& camera(index aIndex) const = 0;
            virtual std::uint32_t skin_count() const = 0;
            virtual i_skin const& skin(index aIndex) const = 0;
            virtual std::uint32_t node_count() const = 0;
            virtual i_node const& node(index aIndex) const = 0;
            virtual i_node& node(index aIndex) = 0;
            virtual std::uint32_t animation_count() const = 0;
            virtual i_animation const& animation(index aIndex) const = 0;
            virtual std::uint32_t scene_count() const = 0;
            virtual i_scene const& scene(index aIndex) const = 0;
            virtual bool has_default_scene() const = 0;
            virtual index default_scene() const = 0;
        public:
            // the scene to display: the default scene, else the first scene
            index active_scene() const
            {
                if (has_default_scene())
                    return default_scene();
                return scene_count() > 0u ? 0u : invalid_index;
            }
        };
    }
}
