// scene_graph.cpp
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

#include <neogfx/neogfx.hpp>

#include <cstring>
#include <algorithm>
#include <limits>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <cctype>
#include <string>

#include <neogfx/gfx/scene_graph.hpp>

namespace neogfx::scene_graph
{
    struct accessor_out_of_bounds : std::runtime_error { accessor_out_of_bounds() : std::runtime_error{ "neogfx::scene_graph::accessor_out_of_bounds" } {} };

    scene_graph_model::scene_graph_model(scene_graph::dimension aDimension) :
        iDimension{ aDimension }
    {
    }

    scene_graph::dimension scene_graph_model::dimension() const
    {
        return iDimension;
    }

    scene_graph::asset const& scene_graph_model::asset() const
    {
        return iAsset;
    }

    std::uint64_t scene_graph_model::revision() const
    {
        return iRevision;
    }

    neolib::string const& scene_graph_model::base_directory() const
    {
        return iBaseDirectory;
    }

    void scene_graph_model::set_base_directory(std::string const& aBaseDirectory)
    {
        iBaseDirectory = aBaseDirectory;
        changed();
    }

    template <typename T>
    T& scene_graph_model::checked(std::vector<T>& aVector, index aIndex)
    {
        if (aIndex >= aVector.size())
            throw bad_index();
        return aVector[aIndex];
    }

    template <typename T>
    T const& scene_graph_model::checked(std::vector<T> const& aVector, index aIndex)
    {
        if (aIndex >= aVector.size())
            throw bad_index();
        return aVector[aIndex];
    }

    std::uint32_t scene_graph_model::buffer_count() const
    {
        return static_cast<std::uint32_t>(iBuffers.size());
    }

    scene_graph::buffer const& scene_graph_model::buffer(index aIndex) const
    {
        return checked(iBuffers, aIndex);
    }

    std::uint32_t scene_graph_model::buffer_view_count() const
    {
        return static_cast<std::uint32_t>(iBufferViews.size());
    }

    scene_graph::buffer_view const& scene_graph_model::buffer_view(index aIndex) const
    {
        return checked(iBufferViews, aIndex);
    }

    std::uint32_t scene_graph_model::accessor_count() const
    {
        return static_cast<std::uint32_t>(iAccessors.size());
    }

    scene_graph::accessor const& scene_graph_model::accessor(index aIndex) const
    {
        return checked(iAccessors, aIndex);
    }

    std::uint32_t scene_graph_model::image_count() const
    {
        return static_cast<std::uint32_t>(iImages.size());
    }

    scene_graph::image const& scene_graph_model::image(index aIndex) const
    {
        return checked(iImages, aIndex);
    }

    std::uint32_t scene_graph_model::sampler_count() const
    {
        return static_cast<std::uint32_t>(iSamplers.size());
    }

    scene_graph::sampler const& scene_graph_model::sampler(index aIndex) const
    {
        return checked(iSamplers, aIndex);
    }

    std::uint32_t scene_graph_model::texture_count() const
    {
        return static_cast<std::uint32_t>(iTextures.size());
    }

    scene_graph::texture const& scene_graph_model::texture(index aIndex) const
    {
        return checked(iTextures, aIndex);
    }

    std::uint32_t scene_graph_model::material_count() const
    {
        return static_cast<std::uint32_t>(iMaterials.size());
    }

    scene_graph::material const& scene_graph_model::material(index aIndex) const
    {
        return checked(iMaterials, aIndex);
    }

    std::uint32_t scene_graph_model::mesh_count() const
    {
        return static_cast<std::uint32_t>(iMeshes.size());
    }

    scene_graph::mesh const& scene_graph_model::mesh(index aIndex) const
    {
        return checked(iMeshes, aIndex);
    }

    std::uint32_t scene_graph_model::camera_count() const
    {
        return static_cast<std::uint32_t>(iCameras.size());
    }

    scene_graph::camera const& scene_graph_model::camera(index aIndex) const
    {
        return checked(iCameras, aIndex);
    }

    std::uint32_t scene_graph_model::skin_count() const
    {
        return static_cast<std::uint32_t>(iSkins.size());
    }

    scene_graph::skin const& scene_graph_model::skin(index aIndex) const
    {
        return checked(iSkins, aIndex);
    }

    std::uint32_t scene_graph_model::node_count() const
    {
        return static_cast<std::uint32_t>(iNodes.size());
    }

    scene_graph::node const& scene_graph_model::node(index aIndex) const
    {
        return checked(iNodes, aIndex);
    }

    scene_graph::node& scene_graph_model::node(index aIndex)
    {
        return checked(iNodes, aIndex);
    }

    std::uint32_t scene_graph_model::animation_count() const
    {
        return static_cast<std::uint32_t>(iAnimations.size());
    }

    scene_graph::animation const& scene_graph_model::animation(index aIndex) const
    {
        return checked(iAnimations, aIndex);
    }

    scene_graph::animation& scene_graph_model::animation(index aIndex)
    {
        return checked(iAnimations, aIndex);
    }

    std::uint32_t scene_graph_model::scene_count() const
    {
        return static_cast<std::uint32_t>(iScenes.size());
    }

    scene_graph::scene const& scene_graph_model::scene(index aIndex) const
    {
        return checked(iScenes, aIndex);
    }

    bool scene_graph_model::has_default_scene() const
    {
        return iDefaultScene != invalid_index;
    }

    index scene_graph_model::default_scene() const
    {
        return iDefaultScene;
    }

    void scene_graph_model::set_dimension(scene_graph::dimension aDimension)
    {
        if (iDimension != aDimension)
        {
            iDimension = aDimension;
            changed();
        }
    }

    scene_graph::asset& scene_graph_model::asset()
    {
        return iAsset;
    }

    scene_graph::buffer& scene_graph_model::buffer(index aIndex)
    {
        return checked(iBuffers, aIndex);
    }

    scene_graph::buffer_view& scene_graph_model::buffer_view(index aIndex)
    {
        return checked(iBufferViews, aIndex);
    }

    scene_graph::accessor& scene_graph_model::accessor(index aIndex)
    {
        return checked(iAccessors, aIndex);
    }

    scene_graph::image& scene_graph_model::image(index aIndex)
    {
        return checked(iImages, aIndex);
    }

    scene_graph::sampler& scene_graph_model::sampler(index aIndex)
    {
        return checked(iSamplers, aIndex);
    }

    scene_graph::texture& scene_graph_model::texture(index aIndex)
    {
        return checked(iTextures, aIndex);
    }

    scene_graph::material& scene_graph_model::material(index aIndex)
    {
        return checked(iMaterials, aIndex);
    }

    scene_graph::mesh& scene_graph_model::mesh(index aIndex)
    {
        return checked(iMeshes, aIndex);
    }

    scene_graph::camera& scene_graph_model::camera(index aIndex)
    {
        return checked(iCameras, aIndex);
    }

    scene_graph::skin& scene_graph_model::skin(index aIndex)
    {
        return checked(iSkins, aIndex);
    }

    scene_graph::scene& scene_graph_model::scene(index aIndex)
    {
        return checked(iScenes, aIndex);
    }

    void scene_graph_model::set_default_scene(index aScene)
    {
        if (aScene >= iScenes.size())
            throw bad_index();
        iDefaultScene = aScene;
        changed();
    }

    void scene_graph_model::clear_default_scene()
    {
        iDefaultScene = invalid_index;
        changed();
    }

    void scene_graph_model::touch()
    {
        changed();
    }

    void scene_graph_model::clear()
    {
        iAsset = {};
        iBuffers.clear();
        iBufferViews.clear();
        iAccessors.clear();
        iImages.clear();
        iSamplers.clear();
        iTextures.clear();
        iMaterials.clear();
        iMeshes.clear();
        iCameras.clear();
        iSkins.clear();
        iNodes.clear();
        iAnimations.clear();
        iScenes.clear();
        iDefaultScene = invalid_index;
        changed();
    }

    namespace
    {
        template <typename T>
        index add_to(std::vector<T>& aVector, T const& aValue)
        {
            aVector.push_back(aValue);
            return static_cast<index>(aVector.size() - 1u);
        }
    }

    index scene_graph_model::add(scene_graph::buffer const& aBuffer)
    {
        changed();
        return add_to(iBuffers, aBuffer);
    }

    index scene_graph_model::add(scene_graph::buffer_view const& aBufferView)
    {
        changed();
        return add_to(iBufferViews, aBufferView);
    }

    index scene_graph_model::add(scene_graph::accessor const& aAccessor)
    {
        changed();
        return add_to(iAccessors, aAccessor);
    }

    index scene_graph_model::add(scene_graph::image const& aImage)
    {
        changed();
        return add_to(iImages, aImage);
    }

    index scene_graph_model::add(scene_graph::sampler const& aSampler)
    {
        changed();
        return add_to(iSamplers, aSampler);
    }

    index scene_graph_model::add(scene_graph::texture const& aTexture)
    {
        changed();
        return add_to(iTextures, aTexture);
    }

    index scene_graph_model::add(scene_graph::material const& aMaterial)
    {
        changed();
        return add_to(iMaterials, aMaterial);
    }

    index scene_graph_model::add(scene_graph::mesh const& aMesh)
    {
        changed();
        return add_to(iMeshes, aMesh);
    }

    index scene_graph_model::add(scene_graph::camera const& aCamera)
    {
        changed();
        return add_to(iCameras, aCamera);
    }

    index scene_graph_model::add(scene_graph::skin const& aSkin)
    {
        changed();
        return add_to(iSkins, aSkin);
    }

    index scene_graph_model::add(scene_graph::node const& aNode)
    {
        changed();
        return add_to(iNodes, aNode);
    }

    index scene_graph_model::add(scene_graph::animation const& aAnimation)
    {
        changed();
        return add_to(iAnimations, aAnimation);
    }

    index scene_graph_model::add(scene_graph::scene const& aScene)
    {
        changed();
        return add_to(iScenes, aScene);
    }

    std::size_t scene_graph_model::append_data(void const* aData, std::size_t aSize, std::size_t aAlignment)
    {
        if (iBuffers.empty())
            add(scene_graph::buffer{});
        auto& bytes = iBuffers[0].bytes();
        if (aAlignment > 1u && bytes.size() % aAlignment != 0u)
            bytes.resize(bytes.size() + aAlignment - bytes.size() % aAlignment, std::byte{});
        auto const offset = bytes.size();
        bytes.resize(offset + aSize);
        if (aSize != 0u)
            std::memcpy(bytes.data() + offset, aData, aSize);
        changed();
        return offset;
    }

    index scene_graph_model::add_buffer_view(void const* aData, std::size_t aSize, buffer_view_target aTarget, std::size_t aByteStride)
    {
        auto const offset = append_data(aData, aSize);
        return add(scene_graph::buffer_view{ 0u, offset, aSize, aByteStride, aTarget });
    }

    namespace
    {
        template <typename Vector>
        index add_float_accessor(scene_graph_model& aModel, std::vector<Vector> const& aData, accessor_type aType, bool aMinMax)
        {
            constexpr std::size_t components = sizeof(Vector) / sizeof(float);
            std::vector<float> flat;
            flat.reserve(aData.size() * components);
            for (auto const& v : aData)
                for (std::size_t c = 0u; c < components; ++c)
                    flat.push_back(v[static_cast<std::uint32_t>(c)]);
            scene_graph::accessor newAccessor;
            newAccessor.set_buffer_view(aModel.add_buffer_view(flat.data(), flat.size() * sizeof(float), buffer_view_target::ARRAY_BUFFER));
            newAccessor.set_component_type(accessor_component_type::FLOAT);
            newAccessor.set_type(aType);
            newAccessor.set_count(aData.size());
            if (aMinMax && !aData.empty())
            {
                for (std::size_t c = 0u; c < components; ++c)
                {
                    scalar minValue = std::numeric_limits<scalar>::max();
                    scalar maxValue = std::numeric_limits<scalar>::lowest();
                    for (auto const& v : aData)
                    {
                        minValue = std::min<scalar>(minValue, v[static_cast<std::uint32_t>(c)]);
                        maxValue = std::max<scalar>(maxValue, v[static_cast<std::uint32_t>(c)]);
                    }
                    newAccessor.min().push_back(minValue);
                    newAccessor.max().push_back(maxValue);
                }
            }
            return aModel.add(newAccessor);
        }
    }

    index scene_graph_model::add_positions(std::vector<vec3f> const& aPositions)
    {
        // POSITION accessors must have min and max
        return add_float_accessor(*this, aPositions, accessor_type::VEC3, true);
    }

    index scene_graph_model::add_normals(std::vector<vec3f> const& aNormals)
    {
        return add_float_accessor(*this, aNormals, accessor_type::VEC3, false);
    }

    index scene_graph_model::add_tex_coords(std::vector<vec2f> const& aTexCoords)
    {
        return add_float_accessor(*this, aTexCoords, accessor_type::VEC2, false);
    }

    index scene_graph_model::add_colors(std::vector<vec4f> const& aColors)
    {
        return add_float_accessor(*this, aColors, accessor_type::VEC4, false);
    }

    index scene_graph_model::add_indices(std::vector<std::uint32_t> const& aIndices)
    {
        scene_graph::accessor newAccessor;
        auto const maxIndex = aIndices.empty() ? 0u : *std::max_element(aIndices.begin(), aIndices.end());
        if (maxIndex < 0xFFFFu)
        {
            std::vector<std::uint16_t> shortIndices;
            shortIndices.reserve(aIndices.size());
            for (auto i : aIndices)
                shortIndices.push_back(static_cast<std::uint16_t>(i));
            newAccessor.set_buffer_view(add_buffer_view(shortIndices.data(), shortIndices.size() * sizeof(std::uint16_t), buffer_view_target::ELEMENT_ARRAY_BUFFER));
            newAccessor.set_component_type(accessor_component_type::UNSIGNED_SHORT);
        }
        else
        {
            newAccessor.set_buffer_view(add_buffer_view(aIndices.data(), aIndices.size() * sizeof(std::uint32_t), buffer_view_target::ELEMENT_ARRAY_BUFFER));
            newAccessor.set_component_type(accessor_component_type::UNSIGNED_INT);
        }
        newAccessor.set_type(accessor_type::SCALAR);
        newAccessor.set_count(aIndices.size());
        return add(newAccessor);
    }

    index scene_graph_model::add_material(neogfx::color const& aBaseColor, bool aDoubleSided, std::optional<std::string> const& aName)
    {
        // glTF colour factors are linear
        auto const linear = aBaseColor.to_linear();
        scene_graph::material newMaterial;
        newMaterial.set_name(aName);
        newMaterial.pbr_metallic_roughness().set_base_color_factor(
            vec4{ linear.red<scalar>(), linear.green<scalar>(), linear.blue<scalar>(), aBaseColor.alpha<scalar>() });
        newMaterial.pbr_metallic_roughness().set_metallic_factor(0.0);
        newMaterial.pbr_metallic_roughness().set_roughness_factor(1.0);
        if (aBaseColor.alpha() != 0xFF)
            newMaterial.set_alpha_mode(alpha_mode::Blend);
        newMaterial.set_double_sided(aDoubleSided);
        return add(newMaterial);
    }

    index scene_graph_model::add_image(void const* aEncodedData, std::size_t aSize, std::string const& aMimeType, std::optional<std::string> const& aName)
    {
        scene_graph::image newImage;
        newImage.set_name(aName);
        newImage.set_mime_type(aMimeType);
        newImage.set_buffer_view(add_buffer_view(aEncodedData, aSize));
        return add(newImage);
    }

    index scene_graph_model::add_texture(index aImage, std::optional<scene_graph::mag_filter> const& aMagFilter, std::optional<std::string> const& aName)
    {
        scene_graph::texture newTexture;
        newTexture.set_name(aName);
        newTexture.set_source(aImage);
        if (aMagFilter)
        {
            scene_graph::sampler newSampler;
            newSampler.set_mag_filter(*aMagFilter);
            newTexture.set_sampler(add(newSampler));
        }
        return add(newTexture);
    }

    void scene_graph_model::set_base_color_texture(index aMesh, index aTexture)
    {
        for (auto& primitive : mesh(aMesh).primitives())
            if (primitive.has_material())
                material(primitive.material()).pbr_metallic_roughness().base_color_texture().set_texture(aTexture);
        changed();
    }

    void scene_graph_model::add_primitive(index aMesh, game::mesh const& aSource, index aMaterial)
    {
        std::vector<vec3f> positions{ aSource.vertices.begin(), aSource.vertices.end() };
        std::vector<std::uint32_t> indices;
        indices.reserve(aSource.faces.size() * 3u);
        for (auto const& f : aSource.faces)
            for (std::uint32_t fv = 0u; fv < 3u; ++fv)
                indices.push_back(static_cast<std::uint32_t>(f[fv]));
        scene_graph::mesh_primitive primitive;
        primitive.set_mode(rendering_mode::TRIANGLES);
        primitive.attributes().set_attribute(vertex_attribute::POSITION, add_positions(positions));
        if (!aSource.uv.empty() && aSource.uv.size() == aSource.vertices.size())
        {
            // neoGFX texture space (v up) to glTF's (v down)
            std::vector<vec2f> texCoords;
            for (auto const& uv : aSource.uv)
                texCoords.push_back(vec2f{ uv.x, 1.0f - uv.y });
            primitive.attributes().set_attribute(vertex_attribute::TEXCOORD_0, add_tex_coords(texCoords));
        }
        if (!indices.empty())
            primitive.set_indices(add_indices(indices));
        primitive.set_material(aMaterial);
        mesh(aMesh).primitives().push_back(primitive);
        changed();
    }

    index scene_graph_model::add_mesh(game::mesh const& aSource, index aMaterial, std::optional<std::string> const& aName)
    {
        scene_graph::mesh newMesh;
        newMesh.set_name(aName);
        auto const newMeshIndex = add(newMesh);
        add_primitive(newMeshIndex, aSource, aMaterial);
        return newMeshIndex;
    }

    index scene_graph_model::add_node(std::optional<std::string> const& aName, i_node::trs_transform const& aTransform, index aParent, index aMesh)
    {
        scene_graph::node newNode;
        newNode.set_name(aName);
        newNode.set_trs(aTransform);
        newNode.set_mesh(aMesh);
        auto const newNodeIndex = add(newNode);
        if (aParent != invalid_index)
            add_child(aParent, newNodeIndex);
        return newNodeIndex;
    }

    index scene_graph_model::add_scene(std::optional<std::string> const& aName, std::vector<index> const& aRootNodes)
    {
        scene_graph::scene newScene;
        newScene.set_name(aName);
        for (auto n : aRootNodes)
            newScene.nodes().push_back(n);
        auto const newSceneIndex = add(newScene);
        if (!has_default_scene())
            set_default_scene(newSceneIndex);
        return newSceneIndex;
    }

    void scene_graph_model::add_child(index aParent, index aChild)
    {
        if (aChild >= iNodes.size())
            throw bad_index();
        node(aParent).children().push_back(aChild);
        changed();
    }

    void scene_graph_model::changed()
    {
        ++iRevision;
    }

    scene_graph_2d::scene_graph_2d() :
        scene_graph_model{ scene_graph::dimension::Two }
    {
    }

    scene_graph_2d::scene_graph_2d(scene_graph_model&& aModel) :
        scene_graph_model{ std::move(aModel) }
    {
        set_dimension(scene_graph::dimension::Two);
    }

    i_node::trs_transform scene_graph_2d::to_trs(vec2 const& aTranslation, scalar aRotation, vec2 const& aScale, scalar aZ)
    {
        return i_node::trs_transform{
            vec3{ aTranslation.x, aTranslation.y, aZ },
            vec4{ 0.0, 0.0, std::sin(aRotation / 2.0), std::cos(aRotation / 2.0) },
            vec3{ aScale.x, aScale.y, 1.0 } };
    }

    index scene_graph_2d::add_node(std::optional<std::string> const& aName, vec2 const& aTranslation, scalar aRotation, vec2 const& aScale, index aParent, index aMesh, scalar aZ)
    {
        return scene_graph_model::add_node(aName, to_trs(aTranslation, aRotation, aScale, aZ), aParent, aMesh);
    }

    void scene_graph_2d::set_transform(index aNode, vec2 const& aTranslation, scalar aRotation, vec2 const& aScale)
    {
        auto& n = node(aNode);
        // preserve drawing order
        auto const z = n.transform_flavour() == i_node::local_transform_flavour::TRS ? n.trs().translation.z : n.matrix()[3][2];
        n.set_trs(to_trs(aTranslation, aRotation, aScale, z));
    }

    index scene_graph_2d::add_polygon(std::vector<vec2> const& aOutline, neogfx::color const& aColor, std::optional<std::string> const& aName)
    {
        game::mesh polygon;
        if (aOutline.size() >= 3u)
        {
            scalar minX = aOutline[0].x, minY = aOutline[0].y, maxX = minX, maxY = minY;
            for (auto const& p : aOutline)
            {
                minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
                minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
            }
            auto const w = std::max(maxX - minX, 1e-9);
            auto const h = std::max(maxY - minY, 1e-9);
            for (auto const& p : aOutline)
            {
                polygon.vertices.push_back(vec3f{ static_cast<float>(p.x), static_cast<float>(p.y), 0.0f });
                // neoGFX texture space (v up); add_mesh converts to glTF's
                polygon.uv.push_back(vec2f{ static_cast<float>((p.x - minX) / w), static_cast<float>((p.y - minY) / h) });
            }
            for (std::uint32_t i = 1u; i + 1u < static_cast<std::uint32_t>(aOutline.size()); ++i)
                polygon.faces.push_back(game::face{ 0u, i, i + 1u });
        }
        return add_mesh(polygon, add_material(aColor, true, aName), aName);
    }

    index scene_graph_2d::add_rectangle(size const& aExtents, neogfx::color const& aColor, std::optional<std::string> const& aName)
    {
        auto const hw = aExtents.cx / 2.0;
        auto const hh = aExtents.cy / 2.0;
        return add_polygon({ vec2{ -hw, -hh }, vec2{ hw, -hh }, vec2{ hw, hh }, vec2{ -hw, hh } }, aColor, aName);
    }

    index scene_graph_2d::add_regular_polygon(scalar aRadius, std::uint32_t aSides, neogfx::color const& aColor, std::optional<std::string> const& aName)
    {
        std::vector<vec2> outline;
        aSides = std::max(aSides, 3u);
        for (std::uint32_t s = 0u; s < aSides; ++s)
        {
            auto const angle = math::pi<scalar>() / 2.0 + 2.0 * math::pi<scalar>() * s / aSides;
            outline.push_back(vec2{ aRadius * std::cos(angle), aRadius * std::sin(angle) });
        }
        return add_polygon(outline, aColor, aName);
    }

    scene_graph_3d::scene_graph_3d() :
        scene_graph_model{ scene_graph::dimension::Three }
    {
    }

    scene_graph_3d::scene_graph_3d(scene_graph_model&& aModel) :
        scene_graph_model{ std::move(aModel) }
    {
        set_dimension(scene_graph::dimension::Three);
    }

    vec4 scene_graph_3d::axis_angle(vec3 const& aAxis, scalar aAngle)
    {
        auto const axis = aAxis.magnitude() != 0.0 ? aAxis.normalized() : vec3{ 0.0, 0.0, 1.0 };
        auto const s = std::sin(aAngle / 2.0);
        return vec4{ axis.x * s, axis.y * s, axis.z * s, std::cos(aAngle / 2.0) };
    }

    vec4 scene_graph_3d::multiply(vec4 const& a, vec4 const& b)
    {
        return vec4{
            a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1],
            a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0],
            a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3],
            a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2] };
    }

    vec4 scene_graph_3d::look_at(vec3 const& aEye, vec3 const& aTarget, vec3 const& aUp)
    {
        auto forward = aTarget - aEye;
        if (forward.magnitude() == 0.0)
            return vec4{ 0.0, 0.0, 0.0, 1.0 };
        auto const zAxis = (-forward).normalized();
        auto xAxis = aUp.cross(zAxis);
        if (xAxis.magnitude() < 1e-9)
            xAxis = vec3{ 1.0, 0.0, 0.0 }.cross(zAxis);
        xAxis = xAxis.normalized();
        auto const yAxis = zAxis.cross(xAxis);
        // rotation matrix (columns x, y, z) to quaternion
        scalar const m00 = xAxis.x, m10 = xAxis.y, m20 = xAxis.z;
        scalar const m01 = yAxis.x, m11 = yAxis.y, m21 = yAxis.z;
        scalar const m02 = zAxis.x, m12 = zAxis.y, m22 = zAxis.z;
        scalar const trace = m00 + m11 + m22;
        vec4 q;
        if (trace > 0.0)
        {
            scalar const s = std::sqrt(trace + 1.0) * 2.0;
            q = vec4{ (m21 - m12) / s, (m02 - m20) / s, (m10 - m01) / s, 0.25 * s };
        }
        else if (m00 > m11 && m00 > m22)
        {
            scalar const s = std::sqrt(1.0 + m00 - m11 - m22) * 2.0;
            q = vec4{ 0.25 * s, (m01 + m10) / s, (m02 + m20) / s, (m21 - m12) / s };
        }
        else if (m11 > m22)
        {
            scalar const s = std::sqrt(1.0 + m11 - m00 - m22) * 2.0;
            q = vec4{ (m01 + m10) / s, 0.25 * s, (m12 + m21) / s, (m02 - m20) / s };
        }
        else
        {
            scalar const s = std::sqrt(1.0 + m22 - m00 - m11) * 2.0;
            q = vec4{ (m02 + m20) / s, (m12 + m21) / s, 0.25 * s, (m10 - m01) / s };
        }
        return q.normalized();
    }

    index scene_graph_3d::add_node(std::optional<std::string> const& aName, vec3 const& aTranslation, vec4 const& aRotation, vec3 const& aScale, index aParent, index aMesh)
    {
        return scene_graph_model::add_node(aName, i_node::trs_transform{ aTranslation, aRotation, aScale }, aParent, aMesh);
    }

    void scene_graph_3d::set_transform(index aNode, vec3 const& aTranslation, vec4 const& aRotation, vec3 const& aScale)
    {
        node(aNode).set_trs(i_node::trs_transform{ aTranslation, aRotation, aScale });
    }

    index scene_graph_3d::add_box(vec3 const& aExtents, std::array<neogfx::color, 6> const& aFaceColors, std::optional<std::string> const& aName)
    {
        struct face_basis { vec3 n; vec3 u; vec3 v; };
        // u x v == n, so the quad (-u-v, +u-v, +u+v, -u+v) is counter-clockwise seen from outside
        static face_basis const sFaces[] =
        {
            { vec3{  1.0,  0.0,  0.0 }, vec3{ 0.0, 1.0, 0.0 }, vec3{ 0.0, 0.0, 1.0 } },
            { vec3{ -1.0,  0.0,  0.0 }, vec3{ 0.0, 0.0, 1.0 }, vec3{ 0.0, 1.0, 0.0 } },
            { vec3{  0.0,  1.0,  0.0 }, vec3{ 0.0, 0.0, 1.0 }, vec3{ 1.0, 0.0, 0.0 } },
            { vec3{  0.0, -1.0,  0.0 }, vec3{ 1.0, 0.0, 0.0 }, vec3{ 0.0, 0.0, 1.0 } },
            { vec3{  0.0,  0.0,  1.0 }, vec3{ 1.0, 0.0, 0.0 }, vec3{ 0.0, 1.0, 0.0 } },
            { vec3{  0.0,  0.0, -1.0 }, vec3{ 0.0, 1.0, 0.0 }, vec3{ 1.0, 0.0, 0.0 } }
        };
        auto const half = vec3{ aExtents.x / 2.0, aExtents.y / 2.0, aExtents.z / 2.0 };
        auto scaled = [&](vec3 const& aAxis) { return vec3{ aAxis.x * half.x, aAxis.y * half.y, aAxis.z * half.z }; };
        scene_graph::mesh newMesh;
        newMesh.set_name(aName);
        auto const newMeshIndex = add(newMesh);
        for (std::size_t f = 0u; f < 6u; ++f)
        {
            auto const& basis = sFaces[f];
            auto const c = scaled(basis.n);
            auto const u = scaled(basis.u);
            auto const v = scaled(basis.v);
            std::vector<vec3f> positions{
                (c - u - v).as<float>(), (c + u - v).as<float>(), (c + u + v).as<float>(), (c - u + v).as<float>() };
            std::vector<vec3f> normals( 4u, basis.n.as<float>() );
            std::vector<vec2f> texCoords{ vec2f{ 0.0f, 1.0f }, vec2f{ 1.0f, 1.0f }, vec2f{ 1.0f, 0.0f }, vec2f{ 0.0f, 0.0f } };
            scene_graph::mesh_primitive primitive;
            primitive.attributes().set_attribute(vertex_attribute::POSITION, add_positions(positions));
            primitive.attributes().set_attribute(vertex_attribute::NORMAL, add_normals(normals));
            primitive.attributes().set_attribute(vertex_attribute::TEXCOORD_0, add_tex_coords(texCoords));
            primitive.set_indices(add_indices({ 0u, 1u, 2u, 0u, 2u, 3u }));
            primitive.set_material(add_material(aFaceColors[f]));
            mesh(newMeshIndex).primitives().push_back(primitive);
        }
        touch();
        return newMeshIndex;
    }

    index scene_graph_3d::add_box(vec3 const& aExtents, neogfx::color const& aColor, std::optional<std::string> const& aName)
    {
        return add_box(aExtents, { aColor, aColor, aColor, aColor, aColor, aColor }, aName);
    }

    index scene_graph_3d::add_perspective_camera(scalar aYfov, scalar aZnear, std::optional<scalar> const& aZfar, std::optional<std::string> const& aName)
    {
        scene_graph::camera newCamera{ perspective_camera{ aYfov, aZnear, aZfar } };
        newCamera.set_name(aName);
        return add(newCamera);
    }

    index scene_graph_3d::add_orthographic_camera(scalar aXmag, scalar aYmag, scalar aZnear, scalar aZfar, std::optional<std::string> const& aName)
    {
        scene_graph::camera newCamera{ orthographic_camera{ aXmag, aYmag, aZfar, aZnear } };
        newCamera.set_name(aName);
        return add(newCamera);
    }

    namespace
    {
        scalar read_component(std::byte const* aSource, accessor_component_type aType, bool aNormalized)
        {
            switch (aType)
            {
            case accessor_component_type::BYTE:
                {
                    std::int8_t v;
                    std::memcpy(&v, aSource, sizeof(v));
                    return aNormalized ? std::max(v / 127.0, -1.0) : static_cast<scalar>(v);
                }
            case accessor_component_type::UNSIGNED_BYTE:
                {
                    std::uint8_t v;
                    std::memcpy(&v, aSource, sizeof(v));
                    return aNormalized ? v / 255.0 : static_cast<scalar>(v);
                }
            case accessor_component_type::SHORT:
                {
                    std::int16_t v;
                    std::memcpy(&v, aSource, sizeof(v));
                    return aNormalized ? std::max(v / 32767.0, -1.0) : static_cast<scalar>(v);
                }
            case accessor_component_type::UNSIGNED_SHORT:
                {
                    std::uint16_t v;
                    std::memcpy(&v, aSource, sizeof(v));
                    return aNormalized ? v / 65535.0 : static_cast<scalar>(v);
                }
            case accessor_component_type::UNSIGNED_INT:
                {
                    std::uint32_t v;
                    std::memcpy(&v, aSource, sizeof(v));
                    return static_cast<scalar>(v);
                }
            case accessor_component_type::FLOAT:
                {
                    float v;
                    std::memcpy(&v, aSource, sizeof(v));
                    return static_cast<scalar>(v);
                }
            default:
                throw std::runtime_error{ "neogfx::scene_graph::read_accessor: unknown component type" };
            }
        }

        std::byte const* view_data(i_scene_graph const& aGraph, index aBufferView, std::size_t aOffset, std::size_t aLength)
        {
            auto const& view = aGraph.buffer_view(aBufferView);
            auto const& buffer = aGraph.buffer(view.buffer());
            if (aOffset + aLength > view.byte_length() || view.byte_offset() + view.byte_length() > buffer.byte_length())
                throw accessor_out_of_bounds();
            return buffer.data<std::byte>() + view.byte_offset() + aOffset;
        }
    }

    std::vector<scalar> read_accessor(i_scene_graph const& aGraph, index aAccessor, std::uint32_t& aComponents)
    {
        auto const& accessor = aGraph.accessor(aAccessor);
        aComponents = component_count(accessor.type());
        auto const componentSize = component_size(accessor.component_type());
        auto const elementSize = aComponents * componentSize;
        std::vector<scalar> result(accessor.count() * aComponents, 0.0);
        if (accessor.has_buffer_view() && accessor.count() > 0u)
        {
            auto const& view = aGraph.buffer_view(accessor.buffer_view());
            auto const stride = view.byte_stride() != 0u ? view.byte_stride() : elementSize;
            auto const span = accessor.byte_offset() + stride * (accessor.count() - 1u) + elementSize;
            auto const source = view_data(aGraph, accessor.buffer_view(), 0u, span) + accessor.byte_offset();
            for (std::size_t e = 0u; e < accessor.count(); ++e)
                for (std::uint32_t c = 0u; c < aComponents; ++c)
                    result[e * aComponents + c] = read_component(source + e * stride + c * componentSize, accessor.component_type(), accessor.normalized());
        }
        if (accessor.has_sparse())
        {
            auto const& sparse = accessor.sparse();
            auto const indexSize = component_size(sparse.indices_component_type());
            auto const indices = view_data(aGraph, sparse.indices_buffer_view(), sparse.indices_byte_offset(), sparse.count() * indexSize);
            auto const values = view_data(aGraph, sparse.values_buffer_view(), sparse.values_byte_offset(), sparse.count() * elementSize);
            for (std::size_t s = 0u; s < sparse.count(); ++s)
            {
                auto const target = static_cast<std::size_t>(read_component(indices + s * indexSize, sparse.indices_component_type(), false));
                if (target >= accessor.count())
                    throw accessor_out_of_bounds();
                for (std::uint32_t c = 0u; c < aComponents; ++c)
                    result[target * aComponents + c] = read_component(values + s * elementSize + c * componentSize, accessor.component_type(), accessor.normalized());
            }
        }
        return result;
    }

    std::vector<std::uint32_t> read_indices(i_scene_graph const& aGraph, index aAccessor)
    {
        std::uint32_t components = 0u;
        auto const values = read_accessor(aGraph, aAccessor, components);
        std::vector<std::uint32_t> result;
        result.reserve(values.size());
        for (auto v : values)
            result.push_back(static_cast<std::uint32_t>(v));
        return result;
    }

    std::vector<std::optional<mat44>> world_transformations(i_scene_graph const& aGraph, index aScene)
    {
        std::vector<std::optional<mat44>> result(aGraph.node_count());
        if (aScene == invalid_index || aScene >= aGraph.scene_count())
            return result;
        std::vector<std::pair<index, mat44>> stack;
        for (auto root : aGraph.scene(aScene).nodes())
            if (root < aGraph.node_count())
                stack.emplace_back(root, mat44::identity());
        while (!stack.empty())
        {
            auto const [n, parentTransformation] = stack.back();
            stack.pop_back();
            if (result[n])
                continue; // not a tree: ignore
            auto const& node = aGraph.node(n);
            result[n] = parentTransformation * node.local_matrix();
            for (auto child : node.children())
                if (child < aGraph.node_count())
                    stack.emplace_back(child, *result[n]);
        }
        return result;
    }

    bool to_ecs_mesh(i_scene_graph const& aGraph, i_mesh_primitive const& aPrimitive, game::mesh& aResult, std::uint32_t aTexCoord)
    {
        aResult.vertices.clear();
        aResult.uv.clear();
        aResult.faces.clear();
        if (aPrimitive.mode() != rendering_mode::TRIANGLES &&
            aPrimitive.mode() != rendering_mode::TRIANGLE_STRIP &&
            aPrimitive.mode() != rendering_mode::TRIANGLE_FAN)
            return false;
        if (!aPrimitive.attributes().has_attribute(vertex_attribute::POSITION))
            return false;
        std::uint32_t components = 0u;
        auto const positions = read_accessor(aGraph, aPrimitive.attributes().attribute(vertex_attribute::POSITION), components);
        if (components != 3u)
            return false;
        auto const vertexCount = static_cast<std::uint32_t>(positions.size() / 3u);
        for (std::uint32_t v = 0u; v < vertexCount; ++v)
            aResult.vertices.push_back(vec3f{ static_cast<float>(positions[v * 3u]), static_cast<float>(positions[v * 3u + 1u]), static_cast<float>(positions[v * 3u + 2u]) });
        auto const texCoordAttribute = (aTexCoord == 1u ? vertex_attribute::TEXCOORD_1 : vertex_attribute::TEXCOORD_0);
        if (aPrimitive.attributes().has_attribute(texCoordAttribute))
        {
            auto const texCoords = read_accessor(aGraph, aPrimitive.attributes().attribute(texCoordAttribute), components);
            if (components == 2u && texCoords.size() / 2u == vertexCount)
                for (std::uint32_t v = 0u; v < vertexCount; ++v)
                    aResult.uv.push_back(vec2f{ static_cast<float>(texCoords[v * 2u]), static_cast<float>(1.0 - texCoords[v * 2u + 1u]) });
        }
        std::vector<std::uint32_t> indices;
        if (aPrimitive.has_indices())
            indices = read_indices(aGraph, aPrimitive.indices());
        else
        {
            indices.resize(vertexCount);
            for (std::uint32_t v = 0u; v < vertexCount; ++v)
                indices[v] = v;
        }
        auto add_face = [&](std::uint32_t a, std::uint32_t b, std::uint32_t c)
        {
            if (a < vertexCount && b < vertexCount && c < vertexCount && a != b && b != c && a != c)
                aResult.faces.push_back(game::face{ a, b, c });
        };
        auto const indexCount = static_cast<std::uint32_t>(indices.size());
        switch (aPrimitive.mode())
        {
        case rendering_mode::TRIANGLES:
            for (std::uint32_t i = 0u; i + 2u < indexCount; i += 3u)
                add_face(indices[i], indices[i + 1u], indices[i + 2u]);
            break;
        case rendering_mode::TRIANGLE_STRIP:
            for (std::uint32_t i = 0u; i + 2u < indexCount; ++i)
                if (i % 2u == 0u)
                    add_face(indices[i], indices[i + 1u], indices[i + 2u]);
                else
                    add_face(indices[i], indices[i + 2u], indices[i + 1u]);
            break;
        case rendering_mode::TRIANGLE_FAN:
            for (std::uint32_t i = 1u; i + 1u < indexCount; ++i)
                add_face(indices[i], indices[i + 1u], indices[0u]);
            break;
        default:
            break;
        }
        return true;
    }

    neogfx::color base_color(i_scene_graph const& aGraph, i_mesh_primitive const& aPrimitive)
    {
        if (!aPrimitive.has_material() || aPrimitive.material() >= aGraph.material_count())
            return neogfx::color::White;
        auto const& factor = aGraph.material(aPrimitive.material()).pbr_metallic_roughness().base_color_factor();
        auto const result = neogfx::color::from_linear(linear_color{ vec4{ factor[0], factor[1], factor[2], 1.0 } });
        return result.with_alpha(std::clamp(factor[3], 0.0, 1.0));
    }

    std::vector<std::byte> decode_base64(std::string_view aText)
    {
        auto decode = [](char c) -> int
        {
            if (c >= 'A' && c <= 'Z') return c - 'A';
            if (c >= 'a' && c <= 'z') return c - 'a' + 26;
            if (c >= '0' && c <= '9') return c - '0' + 52;
            if (c == '+' || c == '-') return 62;
            if (c == '/' || c == '_') return 63;
            return -1;
        };
        std::vector<std::byte> result;
        result.reserve(aText.size() / 4u * 3u);
        std::uint32_t accumulator = 0u;
        int bits = 0;
        for (auto c : aText)
        {
            if (c == '=')
                break;
            auto const value = decode(c);
            if (value < 0)
                continue; // whitespace etc.
            accumulator = (accumulator << 6u) | static_cast<std::uint32_t>(value);
            bits += 6;
            if (bits >= 8)
            {
                bits -= 8;
                result.push_back(static_cast<std::byte>((accumulator >> bits) & 0xFFu));
            }
        }
        return result;
    }

    bool image_data(i_scene_graph const& aGraph, index aImage, std::vector<std::byte>& aResult)
    {
        aResult.clear();
        if (aImage >= aGraph.image_count())
            return false;
        auto const& image = aGraph.image(aImage);
        if (image.has_buffer_view())
        {
            if (image.buffer_view() >= aGraph.buffer_view_count())
                return false;
            auto const& view = aGraph.buffer_view(image.buffer_view());
            if (view.buffer() >= aGraph.buffer_count() || view.byte_offset() + view.byte_length() > aGraph.buffer(view.buffer()).byte_length())
                return false;
            auto const begin = aGraph.buffer(view.buffer()).data<std::byte>() + view.byte_offset();
            aResult.assign(begin, begin + view.byte_length());
            return true;
        }
        if (!image.uri().has_value())
            return false;
        auto const uri = image.uri().value().to_std_string();
        if (uri.rfind("data:", 0u) == 0u)
        {
            auto const comma = uri.find(',');
            if (comma == std::string::npos || uri.substr(0u, comma).find(";base64") == std::string::npos)
                return false;
            aResult = decode_base64(std::string_view{ uri }.substr(comma + 1u));
            return !aResult.empty();
        }
        // relative (percent-encoded) file reference
        std::string path;
        for (std::size_t i = 0u; i < uri.size(); ++i)
        {
            if (uri[i] == '%' && i + 2u < uri.size() && std::isxdigit(static_cast<unsigned char>(uri[i + 1u])) && std::isxdigit(static_cast<unsigned char>(uri[i + 2u])))
            {
                path.push_back(static_cast<char>(std::stoi(uri.substr(i + 1u, 2u), nullptr, 16)));
                i += 2u;
            }
            else
                path.push_back(uri[i]);
        }
        std::ifstream file{ std::filesystem::path{ aGraph.base_directory().to_std_string() } / std::filesystem::path{ path }, std::ios::binary };
        if (!file)
            return false;
        std::string const data{ std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
        auto const begin = reinterpret_cast<std::byte const*>(data.data());
        aResult.assign(begin, begin + data.size());
        return !aResult.empty();
    }

    bool skin_weights(i_scene_graph const& aGraph, i_mesh_primitive const& aPrimitive, std::vector<vec4f>& aJoints, std::vector<vec4f>& aWeights)
    {
        aJoints.clear();
        aWeights.clear();
        if (!aPrimitive.attributes().has_attribute(vertex_attribute::JOINTS_0) || !aPrimitive.attributes().has_attribute(vertex_attribute::WEIGHTS_0) ||
            !aPrimitive.attributes().has_attribute(vertex_attribute::POSITION))
            return false;
        auto const vertexCount = aGraph.accessor(aPrimitive.attributes().attribute(vertex_attribute::POSITION)).count();
        std::uint32_t jointComponents = 0u;
        std::uint32_t weightComponents = 0u;
        auto const joints = read_accessor(aGraph, aPrimitive.attributes().attribute(vertex_attribute::JOINTS_0), jointComponents);
        auto const weights = read_accessor(aGraph, aPrimitive.attributes().attribute(vertex_attribute::WEIGHTS_0), weightComponents);
        if (jointComponents != 4u || weightComponents != 4u || joints.size() != vertexCount * 4u || weights.size() != vertexCount * 4u)
            return false;
        aJoints.reserve(vertexCount);
        aWeights.reserve(vertexCount);
        for (std::size_t v = 0u; v < vertexCount; ++v)
        {
            aJoints.push_back(vec4f{ static_cast<float>(joints[v * 4u]), static_cast<float>(joints[v * 4u + 1u]), static_cast<float>(joints[v * 4u + 2u]), static_cast<float>(joints[v * 4u + 3u]) });
            vec4f weight{ static_cast<float>(weights[v * 4u]), static_cast<float>(weights[v * 4u + 1u]), static_cast<float>(weights[v * 4u + 2u]), static_cast<float>(weights[v * 4u + 3u]) };
            auto const sum = weight[0] + weight[1] + weight[2] + weight[3];
            if (sum > 0.0f)
                for (std::uint32_t c = 0u; c < 4u; ++c)
                    weight[c] /= sum;
            aWeights.push_back(weight);
        }
        return true;
    }

    void joint_matrices(i_scene_graph const& aGraph, index aSkin, std::vector<std::optional<mat44>> const& aWorld, std::vector<mat44f>& aResult)
    {
        aResult.clear();
        if (aSkin >= aGraph.skin_count())
            return;
        auto const& skin = aGraph.skin(aSkin);
        std::vector<scalar> inverseBindMatrices;
        if (skin.has_inverse_bind_matrices())
        {
            std::uint32_t components = 0u;
            inverseBindMatrices = read_accessor(aGraph, skin.inverse_bind_matrices(), components);
            if (components != 16u)
                inverseBindMatrices.clear();
        }
        std::size_t jointIndex = 0u;
        for (auto joint : skin.joints())
        {
            mat44 inverseBind = mat44::identity();
            if (inverseBindMatrices.size() >= (jointIndex + 1u) * 16u)
                for (std::uint32_t c = 0u; c < 4u; ++c)
                    for (std::uint32_t r = 0u; r < 4u; ++r)
                        inverseBind[c][r] = inverseBindMatrices[jointIndex * 16u + c * 4u + r];
            auto const& jointWorld = (joint < aWorld.size() && aWorld[joint]) ? *aWorld[joint] : mat44::identity();
            aResult.push_back((jointWorld * inverseBind).as<float>());
            ++jointIndex;
        }
    }

    namespace
    {
        vec4 slerp(vec4 const& aFrom, vec4 aTo, scalar aT)
        {
            auto dot = aFrom[0] * aTo[0] + aFrom[1] * aTo[1] + aFrom[2] * aTo[2] + aFrom[3] * aTo[3];
            if (dot < 0.0)
            {
                aTo = -aTo;
                dot = -dot;
            }
            vec4 result;
            if (dot > 0.9995)
            {
                for (std::uint32_t c = 0u; c < 4u; ++c)
                    result[c] = aFrom[c] + (aTo[c] - aFrom[c]) * aT;
            }
            else
            {
                auto const theta = std::acos(std::clamp(dot, -1.0, 1.0));
                auto const sinTheta = std::sin(theta);
                auto const a = std::sin((1.0 - aT) * theta) / sinTheta;
                auto const b = std::sin(aT * theta) / sinTheta;
                for (std::uint32_t c = 0u; c < 4u; ++c)
                    result[c] = aFrom[c] * a + aTo[c] * b;
            }
            auto const magnitude = result.magnitude();
            return magnitude > 0.0 ? result / magnitude : vec4{ 0.0, 0.0, 0.0, 1.0 };
        }
    }

    animation_player::animation_player(i_scene_graph& aGraph, index aAnimation) :
        iGraph{ aGraph }, iAnimation{ aAnimation }
    {
        auto const& animation = aGraph.animation(aAnimation);
        for (std::uint32_t c = 0u; c < animation.channel_count(); ++c)
        {
            auto const& source = animation.channel(c);
            if (!source.has_target_node() || source.target_node() >= aGraph.node_count() || source.sampler() >= animation.sampler_count())
                continue;
            auto const& sampler = animation.sampler(source.sampler());
            channel newChannel{ source.target_node(), source.target_path(), sampler.interpolation() };
            std::uint32_t inputComponents = 0u;
            try
            {
                newChannel.times = read_accessor(aGraph, sampler.input(), inputComponents);
                newChannel.values = read_accessor(aGraph, sampler.output(), newChannel.components);
            }
            catch (std::exception const&)
            {
                continue; // malformed: skip
            }
            if (inputComponents != 1u || newChannel.times.empty() || newChannel.components == 0u)
                continue;
            auto const keyCount = newChannel.times.size();
            auto const valuesPerKey = (newChannel.interpolation == animation_interpolation::CubicSpline ? 3u : 1u);
            if (newChannel.path == animation_path::Weights)
            {
                // output is (weight count * keys) scalars
                if (newChannel.values.size() % (keyCount * valuesPerKey) != 0u)
                    continue;
                newChannel.components = static_cast<std::uint32_t>(newChannel.values.size() / (keyCount * valuesPerKey));
            }
            else if (newChannel.values.size() != keyCount * valuesPerKey * newChannel.components)
                continue;
            iDuration = std::max(iDuration, newChannel.times.back());
            iChannels.push_back(std::move(newChannel));
        }
    }

    index animation_player::animation() const
    {
        return iAnimation;
    }

    scalar animation_player::duration() const
    {
        return iDuration;
    }

    void animation_player::apply(scalar aTime)
    {
        thread_local std::vector<scalar> value;
        for (auto const& c : iChannels)
        {
            auto const n = c.components;
            auto const cubic = (c.interpolation == animation_interpolation::CubicSpline);
            auto const stride = (cubic ? 3u : 1u) * n;
            // the value (not tangent) of keyframe k
            auto key_value = [&](std::size_t k, std::uint32_t i) { return c.values[k * stride + (cubic ? n : 0u) + i]; };
            value.assign(n, 0.0);
            auto const& times = c.times;
            if (aTime <= times.front() || times.size() == 1u)
            {
                for (std::uint32_t i = 0u; i < n; ++i)
                    value[i] = key_value(0u, i);
            }
            else if (aTime >= times.back())
            {
                for (std::uint32_t i = 0u; i < n; ++i)
                    value[i] = key_value(times.size() - 1u, i);
            }
            else
            {
                auto const next = static_cast<std::size_t>(std::upper_bound(times.begin(), times.end(), aTime) - times.begin());
                auto const k = next - 1u;
                auto const td = times[next] - times[k];
                auto const s = td > 0.0 ? (aTime - times[k]) / td : 0.0;
                switch (c.interpolation)
                {
                case animation_interpolation::Step:
                    for (std::uint32_t i = 0u; i < n; ++i)
                        value[i] = key_value(k, i);
                    break;
                case animation_interpolation::CubicSpline:
                    {
                        auto const s2 = s * s;
                        auto const s3 = s2 * s;
                        for (std::uint32_t i = 0u; i < n; ++i)
                        {
                            auto const outTangent = c.values[k * stride + 2u * n + i];
                            auto const inTangent = c.values[next * stride + i];
                            value[i] = (2.0 * s3 - 3.0 * s2 + 1.0) * key_value(k, i) + (s3 - 2.0 * s2 + s) * td * outTangent +
                                (-2.0 * s3 + 3.0 * s2) * key_value(next, i) + (s3 - s2) * td * inTangent;
                        }
                    }
                    break;
                case animation_interpolation::Linear:
                default:
                    if (c.path == animation_path::Rotation && n == 4u)
                    {
                        auto const q = slerp(
                            vec4{ key_value(k, 0u), key_value(k, 1u), key_value(k, 2u), key_value(k, 3u) },
                            vec4{ key_value(next, 0u), key_value(next, 1u), key_value(next, 2u), key_value(next, 3u) }, s);
                        for (std::uint32_t i = 0u; i < 4u; ++i)
                            value[i] = q[i];
                    }
                    else
                        for (std::uint32_t i = 0u; i < n; ++i)
                            value[i] = key_value(k, i) + (key_value(next, i) - key_value(k, i)) * s;
                    break;
                }
            }
            auto& node = iGraph.node(c.node);
            auto trs = (node.transform_flavour() == i_node::local_transform_flavour::TRS ? node.trs() : i_node::trs_transform{});
            switch (c.path)
            {
            case animation_path::Translation:
                if (n == 3u)
                    trs.translation = vec3{ value[0], value[1], value[2] };
                break;
            case animation_path::Rotation:
                if (n == 4u)
                {
                    vec4 q{ value[0], value[1], value[2], value[3] };
                    auto const magnitude = q.magnitude();
                    trs.rotation = magnitude > 0.0 ? q / magnitude : vec4{ 0.0, 0.0, 0.0, 1.0 };
                }
                break;
            case animation_path::Scale:
                if (n == 3u)
                    trs.scale = vec3{ value[0], value[1], value[2] };
                break;
            case animation_path::Weights:
                // n.b. morph targets are not rendered: the weights are only stored
                if (node.has_mesh())
                {
                    auto& weights = node.weights();
                    weights.clear();
                    for (auto w : value)
                        weights.push_back(w);
                }
                continue;
            }
            node.set_trs(trs);
        }
    }
}
