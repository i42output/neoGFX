// scene_graph.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <vector>
#include <array>
#include <optional>
#include <cstddef>

#include <neolib/core/optional.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/string.hpp>

#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/i_scene_graph.hpp>
#include <neogfx/game/mesh.hpp>

namespace neogfx
{
    namespace scene_graph
    {
        using optional_string = neolib::optional<neolib::string>;

        inline optional_string to_optional_string(std::optional<std::string> const& aString)
        {
            if (aString)
                return neolib::string{ *aString };
            return std::nullopt;
        }

        class asset : public i_asset
        {
        public:
            neolib::string const& version() const final { return iVersion; }
            optional_string const& min_version() const final { return iMinVersion; }
            optional_string const& generator() const final { return iGenerator; }
            optional_string const& copyright() const final { return iCopyright; }
        public:
            void set_version(std::string const& aVersion) { iVersion = aVersion; }
            void set_min_version(std::optional<std::string> const& aMinVersion) { iMinVersion = to_optional_string(aMinVersion); }
            void set_generator(std::optional<std::string> const& aGenerator) { iGenerator = to_optional_string(aGenerator); }
            void set_copyright(std::optional<std::string> const& aCopyright) { iCopyright = to_optional_string(aCopyright); }
        private:
            neolib::string iVersion = "2.0";
            optional_string iMinVersion;
            optional_string iGenerator = neolib::string{ "neoGFX" };
            optional_string iCopyright;
        };

        template <typename Interface>
        class named : public Interface
        {
        public:
            optional_string const& name() const final { return iName; }
            void set_name(std::optional<std::string> const& aName) { iName = to_optional_string(aName); }
        private:
            optional_string iName;
        };

        class buffer : public named<i_buffer>
        {
        public:
            using i_buffer::data;
        public:
            optional_string const& uri() const final { return iUri; }
            std::size_t byte_length() const final { return iData.size(); }
            void const* data() const final { return iData.data(); }
            void* data() final { return iData.data(); }
        public:
            void set_uri(std::optional<std::string> const& aUri) { iUri = to_optional_string(aUri); }
            std::vector<std::byte> const& bytes() const { return iData; }
            std::vector<std::byte>& bytes() { return iData; }
        private:
            optional_string iUri;
            std::vector<std::byte> iData;
        };

        class buffer_view : public named<i_buffer_view>
        {
        public:
            buffer_view() = default;
            buffer_view(index aBuffer, std::size_t aByteOffset, std::size_t aByteLength, std::size_t aByteStride = 0u, buffer_view_target aTarget = buffer_view_target::None) :
                iBuffer{ aBuffer }, iByteOffset{ aByteOffset }, iByteLength{ aByteLength }, iByteStride{ aByteStride }, iTarget{ aTarget } {}
        public:
            index buffer() const final { return iBuffer; }
            std::size_t byte_offset() const final { return iByteOffset; }
            std::size_t byte_length() const final { return iByteLength; }
            std::size_t byte_stride() const final { return iByteStride; }
            buffer_view_target target() const final { return iTarget; }
        public:
            void set_buffer(index aBuffer) { iBuffer = aBuffer; }
            void set_byte_offset(std::size_t aByteOffset) { iByteOffset = aByteOffset; }
            void set_byte_length(std::size_t aByteLength) { iByteLength = aByteLength; }
            void set_byte_stride(std::size_t aByteStride) { iByteStride = aByteStride; }
            void set_target(buffer_view_target aTarget) { iTarget = aTarget; }
        private:
            index iBuffer = invalid_index;
            std::size_t iByteOffset = 0u;
            std::size_t iByteLength = 0u;
            std::size_t iByteStride = 0u;
            buffer_view_target iTarget = buffer_view_target::None;
        };

        class sparse_array : public i_sparse_array
        {
        public:
            std::size_t count() const final { return iCount; }
            index indices_buffer_view() const final { return iIndicesBufferView; }
            std::size_t indices_byte_offset() const final { return iIndicesByteOffset; }
            accessor_component_type indices_component_type() const final { return iIndicesComponentType; }
            index values_buffer_view() const final { return iValuesBufferView; }
            std::size_t values_byte_offset() const final { return iValuesByteOffset; }
        public:
            void set_count(std::size_t aCount) { iCount = aCount; }
            void set_indices(index aBufferView, std::size_t aByteOffset, accessor_component_type aComponentType)
            {
                iIndicesBufferView = aBufferView; iIndicesByteOffset = aByteOffset; iIndicesComponentType = aComponentType;
            }
            void set_values(index aBufferView, std::size_t aByteOffset)
            {
                iValuesBufferView = aBufferView; iValuesByteOffset = aByteOffset;
            }
        private:
            std::size_t iCount = 0u;
            index iIndicesBufferView = invalid_index;
            std::size_t iIndicesByteOffset = 0u;
            accessor_component_type iIndicesComponentType = accessor_component_type::UNSIGNED_INT;
            index iValuesBufferView = invalid_index;
            std::size_t iValuesByteOffset = 0u;
        };

        class accessor : public named<i_accessor>
        {
        public:
            struct no_sparse : std::logic_error { no_sparse() : std::logic_error{ "neogfx::scene_graph::accessor::no_sparse" } {} };
        public:
            bool has_buffer_view() const final { return iBufferView != invalid_index; }
            index buffer_view() const final { return iBufferView; }
            std::size_t byte_offset() const final { return iByteOffset; }
            accessor_component_type component_type() const final { return iComponentType; }
            bool normalized() const final { return iNormalized; }
            std::size_t count() const final { return iCount; }
            accessor_type type() const final { return iType; }
            neolib::vector<scalar> const& max() const final { return iMax; }
            neolib::vector<scalar> const& min() const final { return iMin; }
            bool has_sparse() const final { return iSparse.has_value(); }
            sparse_array const& sparse() const final { if (iSparse) return *iSparse; throw no_sparse(); }
        public:
            void set_buffer_view(index aBufferView) { iBufferView = aBufferView; }
            void set_byte_offset(std::size_t aByteOffset) { iByteOffset = aByteOffset; }
            void set_component_type(accessor_component_type aComponentType) { iComponentType = aComponentType; }
            void set_normalized(bool aNormalized) { iNormalized = aNormalized; }
            void set_count(std::size_t aCount) { iCount = aCount; }
            void set_type(accessor_type aType) { iType = aType; }
            neolib::vector<scalar>& max() { return iMax; }
            neolib::vector<scalar>& min() { return iMin; }
            void set_sparse(std::optional<sparse_array> const& aSparse) { iSparse = aSparse; }
        private:
            index iBufferView = invalid_index;
            std::size_t iByteOffset = 0u;
            accessor_component_type iComponentType = accessor_component_type::FLOAT;
            bool iNormalized = false;
            std::size_t iCount = 0u;
            accessor_type iType = accessor_type::SCALAR;
            neolib::vector<scalar> iMax;
            neolib::vector<scalar> iMin;
            std::optional<sparse_array> iSparse;
        };

        class attributes : public i_morph_target
        {
        public:
            attributes() { iAttributes.fill(invalid_index); }
        public:
            bool has_attribute(vertex_attribute aAttribute) const final
            {
                return static_cast<std::size_t>(aAttribute) < iAttributes.size() && iAttributes[static_cast<std::size_t>(aAttribute)] != invalid_index;
            }
            index attribute(vertex_attribute aAttribute) const final
            {
                return has_attribute(aAttribute) ? iAttributes[static_cast<std::size_t>(aAttribute)] : invalid_index;
            }
        public:
            void set_attribute(vertex_attribute aAttribute, index aAccessor)
            {
                iAttributes.at(static_cast<std::size_t>(aAttribute)) = aAccessor;
            }
        private:
            std::array<index, static_cast<std::size_t>(vertex_attribute::COUNT)> iAttributes;
        };

        class image : public named<i_image>
        {
        public:
            optional_string const& uri() const final { return iUri; }
            optional_string const& mime_type() const final { return iMimeType; }
            bool has_buffer_view() const final { return iBufferView != invalid_index; }
            index buffer_view() const final { return iBufferView; }
        public:
            void set_uri(std::optional<std::string> const& aUri) { iUri = to_optional_string(aUri); }
            void set_mime_type(std::optional<std::string> const& aMimeType) { iMimeType = to_optional_string(aMimeType); }
            void set_buffer_view(index aBufferView) { iBufferView = aBufferView; }
        private:
            optional_string iUri;
            optional_string iMimeType;
            index iBufferView = invalid_index;
        };

        class sampler : public named<i_sampler>
        {
        public:
            scene_graph::mag_filter mag_filter() const final { return iMagFilter; }
            scene_graph::min_filter min_filter() const final { return iMinFilter; }
            wrapping_mode wrap_S() const final { return iWrapS; }
            wrapping_mode wrap_T() const final { return iWrapT; }
        public:
            void set_mag_filter(scene_graph::mag_filter aFilter) { iMagFilter = aFilter; }
            void set_min_filter(scene_graph::min_filter aFilter) { iMinFilter = aFilter; }
            void set_wrap_S(wrapping_mode aMode) { iWrapS = aMode; }
            void set_wrap_T(wrapping_mode aMode) { iWrapT = aMode; }
        private:
            scene_graph::mag_filter iMagFilter = scene_graph::mag_filter::None;
            scene_graph::min_filter iMinFilter = scene_graph::min_filter::None;
            wrapping_mode iWrapS = wrapping_mode::REPEAT;
            wrapping_mode iWrapT = wrapping_mode::REPEAT;
        };

        class texture : public named<i_texture>
        {
        public:
            bool has_sampler() const final { return iSampler != invalid_index; }
            index sampler() const final { return iSampler; }
            bool has_source() const final { return iSource != invalid_index; }
            index source() const final { return iSource; }
        public:
            void set_sampler(index aSampler) { iSampler = aSampler; }
            void set_source(index aSource) { iSource = aSource; }
        private:
            index iSampler = invalid_index;
            index iSource = invalid_index;
        };

        template <typename Interface>
        class basic_texture_reference : public Interface
        {
        public:
            bool has_texture() const final { return iTexture != invalid_index; }
            index texture() const final { return iTexture; }
            scene_graph::tex_coord tex_coord() const final { return iTexCoord; }
        public:
            void set_texture(index aTexture, scene_graph::tex_coord aTexCoord = scene_graph::tex_coord::TEXCOORD_0) { iTexture = aTexture; iTexCoord = aTexCoord; }
        private:
            index iTexture = invalid_index;
            scene_graph::tex_coord iTexCoord = scene_graph::tex_coord::TEXCOORD_0;
        };

        using texture_reference = basic_texture_reference<i_texture_reference>;
        using emissive_texture = basic_texture_reference<i_emissive_texture>;

        class normal_texture : public basic_texture_reference<i_normal_texture>
        {
        public:
            scalar scale() const final { return iScale; }
            void set_scale(scalar aScale) { iScale = aScale; }
        private:
            scalar iScale = 1.0;
        };

        class occlusion_texture : public basic_texture_reference<i_occlusion_texture>
        {
        public:
            scalar strength() const final { return iStrength; }
            void set_strength(scalar aStrength) { iStrength = aStrength; }
        private:
            scalar iStrength = 1.0;
        };

        class pbr_metallic_roughness : public i_pbr_metallic_roughness
        {
        public:
            vec4 const& base_color_factor() const final { return iBaseColorFactor; }
            texture_reference const& base_color_texture() const final { return iBaseColorTexture; }
            scalar metallic_factor() const final { return iMetallicFactor; }
            scalar roughness_factor() const final { return iRoughnessFactor; }
            texture_reference const& metallic_roughness_texture() const final { return iMetallicRoughnessTexture; }
        public:
            void set_base_color_factor(vec4 const& aFactor) { iBaseColorFactor = aFactor; }
            texture_reference& base_color_texture() { return iBaseColorTexture; }
            void set_metallic_factor(scalar aFactor) { iMetallicFactor = aFactor; }
            void set_roughness_factor(scalar aFactor) { iRoughnessFactor = aFactor; }
            texture_reference& metallic_roughness_texture() { return iMetallicRoughnessTexture; }
        private:
            vec4 iBaseColorFactor = vec4{ 1.0, 1.0, 1.0, 1.0 };
            texture_reference iBaseColorTexture;
            scalar iMetallicFactor = 1.0;
            scalar iRoughnessFactor = 1.0;
            texture_reference iMetallicRoughnessTexture;
        };

        class material : public named<i_material>
        {
        public:
            scene_graph::pbr_metallic_roughness const& pbr_metallic_roughness() const final { return iPbrMetallicRoughness; }
            scene_graph::normal_texture const& normal_texture() const final { return iNormalTexture; }
            scene_graph::occlusion_texture const& occlusion_texture() const final { return iOcclusionTexture; }
            scene_graph::emissive_texture const& emissive_texture() const final { return iEmissiveTexture; }
            vec3 const& emissive_factor() const final { return iEmissiveFactor; }
            scene_graph::alpha_mode alpha_mode() const final { return iAlphaMode; }
            scalar alpha_cutoff() const final { return iAlphaCutoff; }
            bool double_sided() const final { return iDoubleSided; }
        public:
            scene_graph::pbr_metallic_roughness& pbr_metallic_roughness() { return iPbrMetallicRoughness; }
            scene_graph::normal_texture& normal_texture() { return iNormalTexture; }
            scene_graph::occlusion_texture& occlusion_texture() { return iOcclusionTexture; }
            scene_graph::emissive_texture& emissive_texture() { return iEmissiveTexture; }
            void set_emissive_factor(vec3 const& aFactor) { iEmissiveFactor = aFactor; }
            void set_alpha_mode(scene_graph::alpha_mode aMode) { iAlphaMode = aMode; }
            void set_alpha_cutoff(scalar aCutoff) { iAlphaCutoff = aCutoff; }
            void set_double_sided(bool aDoubleSided) { iDoubleSided = aDoubleSided; }
        private:
            scene_graph::pbr_metallic_roughness iPbrMetallicRoughness;
            scene_graph::normal_texture iNormalTexture;
            scene_graph::occlusion_texture iOcclusionTexture;
            scene_graph::emissive_texture iEmissiveTexture;
            vec3 iEmissiveFactor = vec3{ 0.0, 0.0, 0.0 };
            scene_graph::alpha_mode iAlphaMode = scene_graph::alpha_mode::Opaque;
            scalar iAlphaCutoff = 0.5;
            bool iDoubleSided = false;
        };

        class mesh_primitive : public i_mesh_primitive
        {
        public:
            rendering_mode mode() const final { return iMode; }
            bool has_indices() const final { return iIndices != invalid_index; }
            index indices() const final { return iIndices; }
            scene_graph::attributes const& attributes() const final { return iAttributes; }
            std::uint32_t morph_target_count() const final { return static_cast<std::uint32_t>(iTargets.size()); }
            scene_graph::attributes const& morph_target(std::uint32_t aIndex) const final { return iTargets.at(aIndex); }
            bool has_material() const final { return iMaterial != invalid_index; }
            index material() const final { return iMaterial; }
        public:
            void set_mode(rendering_mode aMode) { iMode = aMode; }
            void set_indices(index aAccessor) { iIndices = aAccessor; }
            scene_graph::attributes& attributes() { return iAttributes; }
            std::vector<scene_graph::attributes>& morph_targets() { return iTargets; }
            void set_material(index aMaterial) { iMaterial = aMaterial; }
        private:
            rendering_mode iMode = rendering_mode::TRIANGLES;
            index iIndices = invalid_index;
            scene_graph::attributes iAttributes;
            std::vector<scene_graph::attributes> iTargets;
            index iMaterial = invalid_index;
        };

        class mesh : public named<i_mesh>
        {
        public:
            std::uint32_t primitive_count() const final { return static_cast<std::uint32_t>(iPrimitives.size()); }
            mesh_primitive const& primitive(std::uint32_t aIndex) const final { return iPrimitives.at(aIndex); }
            neolib::vector<scalar> const& weights() const final { return iWeights; }
        public:
            std::vector<mesh_primitive>& primitives() { return iPrimitives; }
            neolib::vector<scalar>& weights() { return iWeights; }
        private:
            std::vector<mesh_primitive> iPrimitives;
            neolib::vector<scalar> iWeights;
        };

        class orthographic_camera : public i_orthographic_camera
        {
        public:
            orthographic_camera(scalar aXmag = 1.0, scalar aYmag = 1.0, scalar aZfar = 100.0, scalar aZnear = 0.0) :
                iXmag{ aXmag }, iYmag{ aYmag }, iZfar{ aZfar }, iZnear{ aZnear } {}
        public:
            scalar xmag() const final { return iXmag; }
            scalar ymag() const final { return iYmag; }
            scalar zfar() const final { return iZfar; }
            scalar znear() const final { return iZnear; }
        private:
            scalar iXmag;
            scalar iYmag;
            scalar iZfar;
            scalar iZnear;
        };

        class perspective_camera : public i_perspective_camera
        {
        public:
            perspective_camera(scalar aYfov = to_rad(45.0), scalar aZnear = 0.1, std::optional<scalar> const& aZfar = {}, std::optional<scalar> const& aAspectRatio = {}) :
                iAspectRatio{ aAspectRatio ? neolib::optional<scalar>{ *aAspectRatio } : neolib::optional<scalar>{} },
                iYfov{ aYfov },
                iZfar{ aZfar ? neolib::optional<scalar>{ *aZfar } : neolib::optional<scalar>{} },
                iZnear{ aZnear } {}
        public:
            neolib::optional<scalar> const& aspect_ratio() const final { return iAspectRatio; }
            scalar yfov() const final { return iYfov; }
            neolib::optional<scalar> const& zfar() const final { return iZfar; }
            scalar znear() const final { return iZnear; }
        private:
            neolib::optional<scalar> iAspectRatio;
            scalar iYfov;
            neolib::optional<scalar> iZfar;
            scalar iZnear;
        };

        class camera : public named<i_camera>
        {
        public:
            camera(scene_graph::orthographic_camera const& aCamera) : iType{ camera_type::Orthographic }, iOrthographic{ aCamera } {}
            camera(scene_graph::perspective_camera const& aCamera = {}) : iType{ camera_type::Perspective }, iPerspective{ aCamera } {}
        public:
            camera_type type() const final { return iType; }
            scene_graph::orthographic_camera const& orthographic() const final
            {
                if (iType != camera_type::Orthographic)
                    throw wrong_camera_type();
                return iOrthographic;
            }
            scene_graph::perspective_camera const& perspective() const final
            {
                if (iType != camera_type::Perspective)
                    throw wrong_camera_type();
                return iPerspective;
            }
        private:
            camera_type iType;
            scene_graph::orthographic_camera iOrthographic;
            scene_graph::perspective_camera iPerspective;
        };

        class skin : public named<i_skin>
        {
        public:
            bool has_inverse_bind_matrices() const final { return iInverseBindMatrices != invalid_index; }
            index inverse_bind_matrices() const final { return iInverseBindMatrices; }
            bool has_skeleton() const final { return iSkeleton != invalid_index; }
            index skeleton() const final { return iSkeleton; }
            neolib::vector<index> const& joints() const final { return iJoints; }
        public:
            void set_inverse_bind_matrices(index aAccessor) { iInverseBindMatrices = aAccessor; }
            void set_skeleton(index aNode) { iSkeleton = aNode; }
            neolib::vector<index>& joints() { return iJoints; }
        private:
            index iInverseBindMatrices = invalid_index;
            index iSkeleton = invalid_index;
            neolib::vector<index> iJoints;
        };

        class animation_sampler : public i_animation_sampler
        {
        public:
            animation_sampler(index aInput = invalid_index, index aOutput = invalid_index, animation_interpolation aInterpolation = animation_interpolation::Linear) :
                iInput{ aInput }, iOutput{ aOutput }, iInterpolation{ aInterpolation } {}
        public:
            index input() const final { return iInput; }
            animation_interpolation interpolation() const final { return iInterpolation; }
            index output() const final { return iOutput; }
        private:
            index iInput;
            index iOutput;
            animation_interpolation iInterpolation;
        };

        class animation_channel : public i_animation_channel
        {
        public:
            animation_channel(index aSampler = invalid_index, index aTargetNode = invalid_index, animation_path aTargetPath = animation_path::Translation) :
                iSampler{ aSampler }, iTargetNode{ aTargetNode }, iTargetPath{ aTargetPath } {}
        public:
            index sampler() const final { return iSampler; }
            bool has_target_node() const final { return iTargetNode != invalid_index; }
            index target_node() const final { return iTargetNode; }
            animation_path target_path() const final { return iTargetPath; }
        private:
            index iSampler;
            index iTargetNode;
            animation_path iTargetPath;
        };

        class animation : public named<i_animation>
        {
        public:
            std::uint32_t channel_count() const final { return static_cast<std::uint32_t>(iChannels.size()); }
            animation_channel const& channel(std::uint32_t aIndex) const final { return iChannels.at(aIndex); }
            std::uint32_t sampler_count() const final { return static_cast<std::uint32_t>(iSamplers.size()); }
            animation_sampler const& sampler(std::uint32_t aIndex) const final { return iSamplers.at(aIndex); }
        public:
            std::vector<animation_channel>& channels() { return iChannels; }
            std::vector<animation_sampler>& samplers() { return iSamplers; }
        private:
            std::vector<animation_channel> iChannels;
            std::vector<animation_sampler> iSamplers;
        };

        class node : public named<i_node>
        {
        public:
            neolib::vector<index> const& children() const final { return iChildren; }
            neolib::vector<index>& children() final { return iChildren; }
        public:
            local_transform_flavour transform_flavour() const final { return iFlavour; }
            matrix_transform const& matrix() const final { return iMatrix; }
            trs_transform const& trs() const final { return iTrs; }
            void set_matrix(matrix_transform const& aMatrix) final { iFlavour = local_transform_flavour::Matrix; iMatrix = aMatrix; }
            void set_trs(trs_transform const& aTrs) final { iFlavour = local_transform_flavour::TRS; iTrs = aTrs; }
            bool has_mesh() const final { return iMesh != invalid_index; }
            index mesh() const final { return iMesh; }
            bool has_camera() const final { return iCamera != invalid_index; }
            index camera() const final { return iCamera; }
            bool has_skin() const final { return iSkin != invalid_index; }
            index skin() const final { return iSkin; }
            neolib::vector<scalar> const& weights() const final { return iWeights; }
        public:
            void set_mesh(index aMesh) { iMesh = aMesh; }
            void set_camera(index aCamera) { iCamera = aCamera; }
            void set_skin(index aSkin) { iSkin = aSkin; }
            neolib::vector<scalar>& weights() final { return iWeights; }
        private:
            neolib::vector<index> iChildren;
            local_transform_flavour iFlavour = local_transform_flavour::TRS;
            matrix_transform iMatrix = mat44::identity();
            trs_transform iTrs;
            index iMesh = invalid_index;
            index iCamera = invalid_index;
            index iSkin = invalid_index;
            neolib::vector<scalar> iWeights;
        };

        class scene : public named<i_scene>
        {
        public:
            neolib::vector<index> const& nodes() const final { return iNodes; }
            neolib::vector<index>& nodes() final { return iNodes; }
        private:
            neolib::vector<index> iNodes;
        };

        // The concrete glTF compatible scene graph. Objects are stored by value in arrays and refer to
        // each other by index; references returned by the accessors are invalidated by additions.
        class scene_graph_model : public i_scene_graph
        {
        public:
            explicit scene_graph_model(scene_graph::dimension aDimension = scene_graph::dimension::Three);
            scene_graph_model(scene_graph_model const&) = default;
            scene_graph_model(scene_graph_model&&) = default;
            ~scene_graph_model() override = default;
        public:
            scene_graph_model& operator=(scene_graph_model const&) = default;
            scene_graph_model& operator=(scene_graph_model&&) = default;
        public:
            scene_graph::dimension dimension() const final;
            scene_graph::asset const& asset() const final;
            std::uint64_t revision() const final;
            neolib::string const& base_directory() const final;
        public:
            std::uint32_t buffer_count() const final;
            scene_graph::buffer const& buffer(index aIndex) const final;
            std::uint32_t buffer_view_count() const final;
            scene_graph::buffer_view const& buffer_view(index aIndex) const final;
            std::uint32_t accessor_count() const final;
            scene_graph::accessor const& accessor(index aIndex) const final;
            std::uint32_t image_count() const final;
            scene_graph::image const& image(index aIndex) const final;
            std::uint32_t sampler_count() const final;
            scene_graph::sampler const& sampler(index aIndex) const final;
            std::uint32_t texture_count() const final;
            scene_graph::texture const& texture(index aIndex) const final;
            std::uint32_t material_count() const final;
            scene_graph::material const& material(index aIndex) const final;
            std::uint32_t mesh_count() const final;
            scene_graph::mesh const& mesh(index aIndex) const final;
            std::uint32_t camera_count() const final;
            scene_graph::camera const& camera(index aIndex) const final;
            std::uint32_t skin_count() const final;
            scene_graph::skin const& skin(index aIndex) const final;
            std::uint32_t node_count() const final;
            scene_graph::node const& node(index aIndex) const final;
            scene_graph::node& node(index aIndex) final;
            std::uint32_t animation_count() const final;
            scene_graph::animation const& animation(index aIndex) const final;
            std::uint32_t scene_count() const final;
            scene_graph::scene const& scene(index aIndex) const final;
            bool has_default_scene() const final;
            index default_scene() const final;
        public:
            void set_dimension(scene_graph::dimension aDimension);
            scene_graph::asset& asset();
            scene_graph::buffer& buffer(index aIndex);
            scene_graph::buffer_view& buffer_view(index aIndex);
            scene_graph::accessor& accessor(index aIndex);
            scene_graph::image& image(index aIndex);
            scene_graph::sampler& sampler(index aIndex);
            scene_graph::texture& texture(index aIndex);
            scene_graph::material& material(index aIndex);
            scene_graph::mesh& mesh(index aIndex);
            scene_graph::camera& camera(index aIndex);
            scene_graph::skin& skin(index aIndex);
            scene_graph::animation& animation(index aIndex);
            scene_graph::scene& scene(index aIndex);
            void set_default_scene(index aScene);
            void clear_default_scene();
            // call after structural changes made through the object accessors (e.g. reparenting a node)
            void touch();
            void set_base_directory(std::string const& aBaseDirectory);
            void clear();
        public:
            index add(scene_graph::buffer const& aBuffer);
            index add(scene_graph::buffer_view const& aBufferView);
            index add(scene_graph::accessor const& aAccessor);
            index add(scene_graph::image const& aImage);
            index add(scene_graph::sampler const& aSampler);
            index add(scene_graph::texture const& aTexture);
            index add(scene_graph::material const& aMaterial);
            index add(scene_graph::mesh const& aMesh);
            index add(scene_graph::camera const& aCamera);
            index add(scene_graph::skin const& aSkin);
            index add(scene_graph::node const& aNode);
            index add(scene_graph::animation const& aAnimation);
            index add(scene_graph::scene const& aScene);
        public:
            // appends data to the shared binary buffer (buffer 0, created on demand), returning its byte offset
            std::size_t append_data(void const* aData, std::size_t aSize, std::size_t aAlignment = 4u);
            index add_buffer_view(void const* aData, std::size_t aSize, buffer_view_target aTarget = buffer_view_target::None, std::size_t aByteStride = 0u);
            index add_positions(std::vector<vec3f> const& aPositions);
            index add_normals(std::vector<vec3f> const& aNormals);
            index add_tex_coords(std::vector<vec2f> const& aTexCoords);
            index add_colors(std::vector<vec4f> const& aColors);
            index add_indices(std::vector<std::uint32_t> const& aIndices);
            index add_material(neogfx::color const& aBaseColor, bool aDoubleSided = false, std::optional<std::string> const& aName = {});
            // an encoded image (e.g. PNG file contents) embedded in the shared binary buffer
            index add_image(void const* aEncodedData, std::size_t aSize, std::string const& aMimeType = "image/png", std::optional<std::string> const& aName = {});
            index add_texture(index aImage, std::optional<scene_graph::mag_filter> const& aMagFilter = {}, std::optional<std::string> const& aName = {});
            // sets the base colour texture of the materials of all of a mesh's primitives
            void set_base_color_texture(index aMesh, index aTexture);
            // a single triangles primitive mesh from an ECS mesh (vertices, optional uv (neoGFX convention: v up), faces)
            index add_mesh(game::mesh const& aMesh, index aMaterial = invalid_index, std::optional<std::string> const& aName = {});
            // adds a primitive to an existing mesh
            void add_primitive(index aMesh, game::mesh const& aMesh2, index aMaterial = invalid_index);
            index add_node(std::optional<std::string> const& aName, i_node::trs_transform const& aTransform = {}, index aParent = invalid_index, index aMesh = invalid_index);
            index add_scene(std::optional<std::string> const& aName, std::vector<index> const& aRootNodes = {});
            void add_child(index aParent, index aChild);
            // Copies another graph into this one: its buffers (merged into the shared binary buffer), buffer views,
            // accessors, images (external image files are embedded), samplers, textures, materials, meshes, cameras,
            // skins, nodes and animations, with the indices they refer to each other by offset. The root nodes of
            // aOther's active scene are made children of aParent or, if aParent is invalid, root nodes of this graph's
            // active scene (created if there is none). aOther's scenes are not copied and this graph's dimension and
            // default scene are unchanged. Returns the copied root nodes (e.g. to position them with set_trs()).
            std::vector<index> append(scene_graph_model const& aOther, index aParent = invalid_index);
        private:
            void changed();
            template <typename T>
            static T& checked(std::vector<T>& aVector, index aIndex);
            template <typename T>
            static T const& checked(std::vector<T> const& aVector, index aIndex);
        private:
            scene_graph::dimension iDimension;
            scene_graph::asset iAsset;
            std::uint64_t iRevision = 0ull;
            neolib::string iBaseDirectory;
            std::vector<scene_graph::buffer> iBuffers;
            std::vector<scene_graph::buffer_view> iBufferViews;
            std::vector<scene_graph::accessor> iAccessors;
            std::vector<scene_graph::image> iImages;
            std::vector<scene_graph::sampler> iSamplers;
            std::vector<scene_graph::texture> iTextures;
            std::vector<scene_graph::material> iMaterials;
            std::vector<scene_graph::mesh> iMeshes;
            std::vector<scene_graph::camera> iCameras;
            std::vector<scene_graph::skin> iSkins;
            std::vector<scene_graph::node> iNodes;
            std::vector<scene_graph::animation> iAnimations;
            std::vector<scene_graph::scene> iScenes;
            index iDefaultScene = invalid_index;
        };

        // 2D: geometry in the z = 0 plane, rotation about z (radians), z translation is drawing order
        class scene_graph_2d : public scene_graph_model
        {
        public:
            scene_graph_2d();
            explicit scene_graph_2d(scene_graph_model&& aModel);
        public:
            static i_node::trs_transform to_trs(vec2 const& aTranslation, scalar aRotation = 0.0, vec2 const& aScale = vec2{ 1.0, 1.0 }, scalar aZ = 0.0);
        public:
            using scene_graph_model::add_node;
            index add_node(std::optional<std::string> const& aName, vec2 const& aTranslation, scalar aRotation = 0.0, vec2 const& aScale = vec2{ 1.0, 1.0 }, index aParent = invalid_index, index aMesh = invalid_index, scalar aZ = 0.0);
            void set_transform(index aNode, vec2 const& aTranslation, scalar aRotation = 0.0, vec2 const& aScale = vec2{ 1.0, 1.0 });
            // convex outline, counter-clockwise
            index add_polygon(std::vector<vec2> const& aOutline, neogfx::color const& aColor, std::optional<std::string> const& aName = {});
            index add_rectangle(size const& aExtents, neogfx::color const& aColor, std::optional<std::string> const& aName = {});
            index add_regular_polygon(scalar aRadius, std::uint32_t aSides, neogfx::color const& aColor, std::optional<std::string> const& aName = {});
        };

        // 3D: right-handed, +y up, cameras look down -z, as glTF
        class scene_graph_3d : public scene_graph_model
        {
        public:
            scene_graph_3d();
            explicit scene_graph_3d(scene_graph_model&& aModel);
        public:
            static vec4 axis_angle(vec3 const& aAxis, scalar aAngle);
            static vec4 multiply(vec4 const& aLhs, vec4 const& aRhs);
            // rotation of a node at aEye so that its -z axis points at aTarget
            static vec4 look_at(vec3 const& aEye, vec3 const& aTarget, vec3 const& aUp = vec3{ 0.0, 1.0, 0.0 });
        public:
            using scene_graph_model::add_node;
            index add_node(std::optional<std::string> const& aName, vec3 const& aTranslation, vec4 const& aRotation = vec4{ 0.0, 0.0, 0.0, 1.0 }, vec3 const& aScale = vec3{ 1.0, 1.0, 1.0 }, index aParent = invalid_index, index aMesh = invalid_index);
            void set_transform(index aNode, vec3 const& aTranslation, vec4 const& aRotation = vec4{ 0.0, 0.0, 0.0, 1.0 }, vec3 const& aScale = vec3{ 1.0, 1.0, 1.0 });
            // a box centred on the origin with one primitive (and material) per face: +x, -x, +y, -y, +z, -z
            index add_box(vec3 const& aExtents, std::array<neogfx::color, 6> const& aFaceColors, std::optional<std::string> const& aName = {});
            index add_box(vec3 const& aExtents, neogfx::color const& aColor, std::optional<std::string> const& aName = {});
            index add_perspective_camera(scalar aYfov = to_rad(45.0), scalar aZnear = 0.1, std::optional<scalar> const& aZfar = {}, std::optional<std::string> const& aName = {});
            index add_orthographic_camera(scalar aXmag, scalar aYmag, scalar aZnear, scalar aZfar, std::optional<std::string> const& aName = {});
        };

        // accessor data as scalars, aComponents per element (normalization, byte stride and sparse substitution applied)
        std::vector<scalar> read_accessor(i_scene_graph const& aGraph, index aAccessor, std::uint32_t& aComponents);
        std::vector<std::uint32_t> read_indices(i_scene_graph const& aGraph, index aAccessor);
        // world transformations of the nodes of a scene; nodes not in the scene are std::nullopt
        std::vector<std::optional<mat44>> world_transformations(i_scene_graph const& aGraph, index aScene);
        // triangle geometry of a primitive (TRIANGLES, TRIANGLE_STRIP and TRIANGLE_FAN); false for other modes
        // n.b. texture coordinates are converted from glTF's convention (v down, origin top left) to neoGFX's (v up);
        // aTexCoord selects the TEXCOORD_n set (0 or 1)
        bool to_ecs_mesh(i_scene_graph const& aGraph, i_mesh_primitive const& aPrimitive, game::mesh& aResult, std::uint32_t aTexCoord = 0u);
        // the encoded image file (e.g. PNG) of an image from its buffer view, data URI or file URI (relative to the base directory)
        bool image_data(i_scene_graph const& aGraph, index aImage, std::vector<std::byte>& aResult);
        std::vector<std::byte> decode_base64(std::string_view aText);
        neogfx::color base_color(i_scene_graph const& aGraph, i_mesh_primitive const& aPrimitive);
        // per vertex skin joint indices and weights of a primitive (JOINTS_0, WEIGHTS_0; weights normalized); false if not skinned
        bool skin_weights(i_scene_graph const& aGraph, i_mesh_primitive const& aPrimitive, std::vector<vec4f>& aJoints, std::vector<vec4f>& aWeights);
        // a skin's joint matrices (joint world transformation * inverse bind matrix) for the given world transformations
        void joint_matrices(i_scene_graph const& aGraph, index aSkin, std::vector<std::optional<mat44>> const& aWorld, std::vector<mat44f>& aResult);

        // Plays an animation by setting the TRS (and morph weights) of its target nodes. The keyframe data
        // is read when constructed so later changes to the graph's accessors are not seen.
        class animation_player
        {
        public:
            animation_player(i_scene_graph& aGraph, index aAnimation);
        public:
            index animation() const;
            scalar duration() const;
            // apply the animation at aTime (seconds; clamped to the keyframes)
            void apply(scalar aTime);
        private:
            struct channel
            {
                index node;
                animation_path path;
                animation_interpolation interpolation;
                std::vector<scalar> times;
                std::vector<scalar> values;
                std::uint32_t components;
            };
        private:
            i_scene_graph& iGraph;
            index iAnimation;
            std::vector<channel> iChannels;
            scalar iDuration = 0.0;
        };
    }

    using scene_graph::i_scene_graph;
    using scene_graph::scene_graph_model;
    using scene_graph::scene_graph_2d;
    using scene_graph::scene_graph_3d;
}
