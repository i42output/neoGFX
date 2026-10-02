// gltf.cpp
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

#include <fstream>
#include <sstream>
#include <charconv>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>

#include <neolib/file/json.hpp>

#include <neogfx/support/file/gfx/gltf.hpp>

namespace neogfx::file
{
    namespace
    {
        using namespace scene_graph;

        constexpr std::uint32_t GLB_MAGIC = 0x46546C67u;      // "glTF"
        constexpr std::uint32_t GLB_CHUNK_JSON = 0x4E4F534Au; // "JSON"
        constexpr std::uint32_t GLB_CHUNK_BIN = 0x004E4942u;  // "BIN\0"

        char const* const sAttributeNames[] =
        {
            "POSITION", "NORMAL", "TANGENT", "TEXCOORD_0", "TEXCOORD_1", "COLOR_0", "JOINTS_0", "WEIGHTS_0"
        };
        static_assert(std::size(sAttributeNames) == static_cast<std::size_t>(vertex_attribute::COUNT));

        char const* const sAnimationPathNames[] = { "translation", "rotation", "scale", "weights" };
        char const* const sInterpolationNames[] = { "LINEAR", "STEP", "CUBICSPLINE" };

        char const* const sAccessorTypeNames[] =
        {
            "SCALAR", "VEC2", "VEC3", "VEC4", "MAT2", "MAT3", "MAT4"
        };

        // base64

        char const sBase64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string base64_encode(std::byte const* aData, std::size_t aLength)
        {
            std::string result;
            result.reserve((aLength + 2u) / 3u * 4u);
            for (std::size_t i = 0u; i < aLength; i += 3u)
            {
                std::uint32_t triple = std::to_integer<std::uint32_t>(aData[i]) << 16u;
                if (i + 1u < aLength)
                    triple |= std::to_integer<std::uint32_t>(aData[i + 1u]) << 8u;
                if (i + 2u < aLength)
                    triple |= std::to_integer<std::uint32_t>(aData[i + 2u]);
                result.push_back(sBase64Alphabet[(triple >> 18u) & 0x3Fu]);
                result.push_back(sBase64Alphabet[(triple >> 12u) & 0x3Fu]);
                result.push_back(i + 1u < aLength ? sBase64Alphabet[(triple >> 6u) & 0x3Fu] : '=');
                result.push_back(i + 2u < aLength ? sBase64Alphabet[triple & 0x3Fu] : '=');
            }
            return result;
        }

        std::string percent_decode(std::string_view aUri)
        {
            std::string result;
            for (std::size_t i = 0u; i < aUri.size(); ++i)
            {
                if (aUri[i] == '%' && i + 2u < aUri.size())
                {
                    int value = 0;
                    auto const [ptr, ec] = std::from_chars(aUri.data() + i + 1u, aUri.data() + i + 3u, value, 16);
                    if (ec == std::errc{} && ptr == aUri.data() + i + 3u)
                    {
                        result.push_back(static_cast<char>(value));
                        i += 2u;
                        continue;
                    }
                }
                result.push_back(aUri[i]);
            }
            return result;
        }

        // JSON writing

        class json_writer
        {
        public:
            json_writer(std::ostream& aOutput) : iOutput{ aOutput } {}
        public:
            json_writer& begin_object(std::string_view aKey = {}) { key(aKey); iOutput << '{'; push(); return *this; }
            json_writer& end_object() { pop(); iOutput << '}'; return *this; }
            json_writer& begin_array(std::string_view aKey = {}) { key(aKey); iOutput << '['; push(); return *this; }
            json_writer& end_array() { pop(); iOutput << ']'; return *this; }
            json_writer& value(std::string_view aKey, std::string_view aValue) { key(aKey); string(aValue); return *this; }
            json_writer& value(std::string_view aKey, char const* aValue) { return value(aKey, std::string_view{ aValue }); }
            json_writer& value(std::string_view aKey, neolib::i_string const& aValue) { return value(aKey, aValue.to_std_string_view()); }
            json_writer& value(std::string_view aKey, bool aValue) { key(aKey); iOutput << (aValue ? "true" : "false"); return *this; }
            json_writer& value(std::string_view aKey, std::uint64_t aValue) { key(aKey); iOutput << aValue; return *this; }
            json_writer& value(std::string_view aKey, std::uint32_t aValue) { return value(aKey, static_cast<std::uint64_t>(aValue)); }
            json_writer& value(std::string_view aKey, double aValue)
            {
                key(aKey);
                if (!std::isfinite(aValue))
                    aValue = 0.0;
                char buffer[32];
                auto const [ptr, ec] = std::to_chars(std::begin(buffer), std::end(buffer), aValue);
                iOutput.write(buffer, ptr - buffer);
                return *this;
            }
            template <typename Container>
            json_writer& numbers(std::string_view aKey, Container const& aValues)
            {
                begin_array(aKey);
                for (auto const& v : aValues)
                    value({}, static_cast<double>(v));
                return end_array();
            }
            json_writer& indices(std::string_view aKey, neolib::i_vector<index> const& aValues)
            {
                begin_array(aKey);
                for (auto const& v : aValues)
                    value({}, v);
                return end_array();
            }
            json_writer& name(neolib::i_optional<neolib::i_string> const& aName)
            {
                if (aName.has_value())
                    value("name", aName.value());
                return *this;
            }
        private:
            void key(std::string_view aKey)
            {
                if (!iFirst.empty())
                {
                    if (!iFirst.back())
                        iOutput << ',';
                    iFirst.back() = false;
                }
                if (!aKey.empty())
                {
                    string(aKey);
                    iOutput << ':';
                }
            }
            void push() { iFirst.push_back(true); }
            void pop() { iFirst.pop_back(); }
            void string(std::string_view aValue)
            {
                iOutput << '"';
                for (auto ch : aValue)
                {
                    switch (ch)
                    {
                    case '"': iOutput << "\\\""; break;
                    case '\\': iOutput << "\\\\"; break;
                    case '\n': iOutput << "\\n"; break;
                    case '\r': iOutput << "\\r"; break;
                    case '\t': iOutput << "\\t"; break;
                    case '\b': iOutput << "\\b"; break;
                    case '\f': iOutput << "\\f"; break;
                    default:
                        if (static_cast<unsigned char>(ch) < 0x20u)
                        {
                            char buffer[8];
                            std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(ch)));
                            iOutput << buffer;
                        }
                        else
                            iOutput << ch;
                    }
                }
                iOutput << '"';
            }
        private:
            std::ostream& iOutput;
            std::vector<bool> iFirst;
        };

        void write_texture_reference(json_writer& aWriter, std::string_view aKey, i_texture_reference const& aReference,
            std::optional<std::pair<char const*, scalar>> const& aExtra = {})
        {
            if (!aReference.has_texture())
                return;
            aWriter.begin_object(aKey);
            aWriter.value("index", aReference.texture());
            if (aReference.tex_coord() != tex_coord::TEXCOORD_0)
                aWriter.value("texCoord", static_cast<std::uint32_t>(aReference.tex_coord()));
            if (aExtra && aExtra->second != 1.0)
                aWriter.value(aExtra->first, aExtra->second);
            aWriter.end_object();
        }


        void write_attributes(json_writer& aWriter, std::string_view aKey, i_attributes const& aAttributes)
        {
            aWriter.begin_object(aKey);
            for (std::uint32_t a = 0u; a < static_cast<std::uint32_t>(vertex_attribute::COUNT); ++a)
                if (aAttributes.has_attribute(static_cast<vertex_attribute>(a)))
                    aWriter.value(sAttributeNames[a], aAttributes.attribute(static_cast<vertex_attribute>(a)));
            aWriter.end_object();
        }

        // aBinaryBuffer: the buffer whose data goes in the GLB BIN chunk (written without a uri); all
        // other buffers are embedded as base64 data URIs so that the output is self-contained
        void write_json(std::ostream& aOutput, i_scene_graph const& aGraph, std::optional<index> const& aBinaryBuffer)
        {
            json_writer w{ aOutput };
            w.begin_object();

            w.begin_object("asset");
            w.value("version", aGraph.asset().version());
            if (aGraph.asset().min_version().has_value())
                w.value("minVersion", aGraph.asset().min_version().value());
            if (aGraph.asset().generator().has_value())
                w.value("generator", aGraph.asset().generator().value());
            if (aGraph.asset().copyright().has_value())
                w.value("copyright", aGraph.asset().copyright().value());
            w.end_object();

            if (aGraph.dimension() == scene_graph::dimension::Two)
            {
                w.begin_array("extensionsUsed").value({}, gltf::extension_2d).end_array();
                w.begin_object("extensions").begin_object(gltf::extension_2d).end_object().end_object();
            }

            if (aGraph.has_default_scene())
                w.value("scene", aGraph.default_scene());

            if (aGraph.scene_count() > 0u)
            {
                w.begin_array("scenes");
                for (index s = 0u; s < aGraph.scene_count(); ++s)
                {
                    auto const& scene = aGraph.scene(s);
                    w.begin_object().name(scene.name());
                    if (!scene.nodes().empty())
                        w.indices("nodes", scene.nodes());
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.node_count() > 0u)
            {
                w.begin_array("nodes");
                for (index n = 0u; n < aGraph.node_count(); ++n)
                {
                    auto const& node = aGraph.node(n);
                    w.begin_object().name(node.name());
                    if (!node.children().empty())
                        w.indices("children", node.children());
                    if (node.has_camera())
                        w.value("camera", node.camera());
                    if (node.has_mesh())
                        w.value("mesh", node.mesh());
                    if (node.has_skin())
                        w.value("skin", node.skin());
                    if (node.transform_flavour() == i_node::local_transform_flavour::Matrix)
                    {
                        if (node.matrix() != mat44::identity())
                        {
                            // column-major, as glTF
                            w.begin_array("matrix");
                            for (std::uint32_t c = 0u; c < 4u; ++c)
                                for (std::uint32_t r = 0u; r < 4u; ++r)
                                    w.value({}, node.matrix()[c][r]);
                            w.end_array();
                        }
                    }
                    else
                    {
                        auto const& trs = node.trs();
                        i_node::trs_transform const identity;
                        if (trs.translation != identity.translation)
                            w.numbers("translation", std::array<scalar, 3>{ trs.translation[0], trs.translation[1], trs.translation[2] });
                        if (trs.rotation != identity.rotation)
                            w.numbers("rotation", std::array<scalar, 4>{ trs.rotation[0], trs.rotation[1], trs.rotation[2], trs.rotation[3] });
                        if (trs.scale != identity.scale)
                            w.numbers("scale", std::array<scalar, 3>{ trs.scale[0], trs.scale[1], trs.scale[2] });
                    }
                    if (!node.weights().empty())
                        w.numbers("weights", node.weights());
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.mesh_count() > 0u)
            {
                w.begin_array("meshes");
                for (index m = 0u; m < aGraph.mesh_count(); ++m)
                {
                    auto const& mesh = aGraph.mesh(m);
                    w.begin_object().name(mesh.name());
                    w.begin_array("primitives");
                    for (std::uint32_t p = 0u; p < mesh.primitive_count(); ++p)
                    {
                        auto const& primitive = mesh.primitive(p);
                        w.begin_object();
                        write_attributes(w, "attributes", primitive.attributes());
                        if (primitive.has_indices())
                            w.value("indices", primitive.indices());
                        if (primitive.has_material())
                            w.value("material", primitive.material());
                        if (primitive.mode() != rendering_mode::TRIANGLES)
                            w.value("mode", static_cast<std::uint32_t>(primitive.mode()));
                        if (primitive.morph_target_count() > 0u)
                        {
                            w.begin_array("targets");
                            for (std::uint32_t t = 0u; t < primitive.morph_target_count(); ++t)
                                write_attributes(w, {}, primitive.morph_target(t));
                            w.end_array();
                        }
                        w.end_object();
                    }
                    w.end_array();
                    if (!mesh.weights().empty())
                        w.numbers("weights", mesh.weights());
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.material_count() > 0u)
            {
                w.begin_array("materials");
                for (index m = 0u; m < aGraph.material_count(); ++m)
                {
                    auto const& material = aGraph.material(m);
                    w.begin_object().name(material.name());
                    auto const& pbr = material.pbr_metallic_roughness();
                    w.begin_object("pbrMetallicRoughness");
                    if (pbr.base_color_factor() != vec4{ 1.0, 1.0, 1.0, 1.0 })
                        w.numbers("baseColorFactor", std::array<scalar, 4>{ pbr.base_color_factor()[0], pbr.base_color_factor()[1], pbr.base_color_factor()[2], pbr.base_color_factor()[3] });
                    write_texture_reference(w, "baseColorTexture", pbr.base_color_texture());
                    if (pbr.metallic_factor() != 1.0)
                        w.value("metallicFactor", pbr.metallic_factor());
                    if (pbr.roughness_factor() != 1.0)
                        w.value("roughnessFactor", pbr.roughness_factor());
                    write_texture_reference(w, "metallicRoughnessTexture", pbr.metallic_roughness_texture());
                    w.end_object();
                    write_texture_reference(w, "normalTexture", material.normal_texture(), std::make_pair("scale", material.normal_texture().scale()));
                    write_texture_reference(w, "occlusionTexture", material.occlusion_texture(), std::make_pair("strength", material.occlusion_texture().strength()));
                    write_texture_reference(w, "emissiveTexture", material.emissive_texture());
                    if (material.emissive_factor() != vec3{ 0.0, 0.0, 0.0 })
                        w.numbers("emissiveFactor", std::array<scalar, 3>{ material.emissive_factor()[0], material.emissive_factor()[1], material.emissive_factor()[2] });
                    switch (material.alpha_mode())
                    {
                    case alpha_mode::Mask:
                        w.value("alphaMode", "MASK");
                        if (material.alpha_cutoff() != 0.5)
                            w.value("alphaCutoff", material.alpha_cutoff());
                        break;
                    case alpha_mode::Blend:
                        w.value("alphaMode", "BLEND");
                        break;
                    default:
                        break;
                    }
                    if (material.double_sided())
                        w.value("doubleSided", true);
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.camera_count() > 0u)
            {
                w.begin_array("cameras");
                for (index c = 0u; c < aGraph.camera_count(); ++c)
                {
                    auto const& camera = aGraph.camera(c);
                    w.begin_object().name(camera.name());
                    if (camera.type() == camera_type::Perspective)
                    {
                        w.value("type", "perspective");
                        auto const& perspective = camera.perspective();
                        w.begin_object("perspective");
                        if (perspective.aspect_ratio().has_value())
                            w.value("aspectRatio", perspective.aspect_ratio().value());
                        w.value("yfov", perspective.yfov());
                        if (perspective.zfar().has_value())
                            w.value("zfar", perspective.zfar().value());
                        w.value("znear", perspective.znear());
                        w.end_object();
                    }
                    else
                    {
                        w.value("type", "orthographic");
                        auto const& orthographic = camera.orthographic();
                        w.begin_object("orthographic");
                        w.value("xmag", orthographic.xmag());
                        w.value("ymag", orthographic.ymag());
                        w.value("zfar", orthographic.zfar());
                        w.value("znear", orthographic.znear());
                        w.end_object();
                    }
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.skin_count() > 0u)
            {
                w.begin_array("skins");
                for (index s = 0u; s < aGraph.skin_count(); ++s)
                {
                    auto const& skin = aGraph.skin(s);
                    w.begin_object().name(skin.name());
                    if (skin.has_inverse_bind_matrices())
                        w.value("inverseBindMatrices", skin.inverse_bind_matrices());
                    if (skin.has_skeleton())
                        w.value("skeleton", skin.skeleton());
                    w.indices("joints", skin.joints());
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.animation_count() > 0u)
            {
                w.begin_array("animations");
                for (index a = 0u; a < aGraph.animation_count(); ++a)
                {
                    auto const& animation = aGraph.animation(a);
                    w.begin_object().name(animation.name());
                    w.begin_array("channels");
                    for (std::uint32_t c = 0u; c < animation.channel_count(); ++c)
                    {
                        auto const& channel = animation.channel(c);
                        w.begin_object();
                        w.value("sampler", channel.sampler());
                        w.begin_object("target");
                        if (channel.has_target_node())
                            w.value("node", channel.target_node());
                        w.value("path", sAnimationPathNames[static_cast<std::size_t>(channel.target_path())]);
                        w.end_object();
                        w.end_object();
                    }
                    w.end_array();
                    w.begin_array("samplers");
                    for (std::uint32_t s = 0u; s < animation.sampler_count(); ++s)
                    {
                        auto const& sampler = animation.sampler(s);
                        w.begin_object();
                        w.value("input", sampler.input());
                        if (sampler.interpolation() != animation_interpolation::Linear)
                            w.value("interpolation", sInterpolationNames[static_cast<std::size_t>(sampler.interpolation())]);
                        w.value("output", sampler.output());
                        w.end_object();
                    }
                    w.end_array();
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.texture_count() > 0u)
            {
                w.begin_array("textures");
                for (index t = 0u; t < aGraph.texture_count(); ++t)
                {
                    auto const& texture = aGraph.texture(t);
                    w.begin_object().name(texture.name());
                    if (texture.has_sampler())
                        w.value("sampler", texture.sampler());
                    if (texture.has_source())
                        w.value("source", texture.source());
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.image_count() > 0u)
            {
                w.begin_array("images");
                for (index i = 0u; i < aGraph.image_count(); ++i)
                {
                    auto const& image = aGraph.image(i);
                    w.begin_object().name(image.name());
                    if (image.uri().has_value())
                        w.value("uri", image.uri().value());
                    if (image.mime_type().has_value())
                        w.value("mimeType", image.mime_type().value());
                    if (image.has_buffer_view())
                        w.value("bufferView", image.buffer_view());
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.sampler_count() > 0u)
            {
                w.begin_array("samplers");
                for (index s = 0u; s < aGraph.sampler_count(); ++s)
                {
                    auto const& sampler = aGraph.sampler(s);
                    w.begin_object().name(sampler.name());
                    if (sampler.mag_filter() != mag_filter::None)
                        w.value("magFilter", static_cast<std::uint32_t>(sampler.mag_filter()));
                    if (sampler.min_filter() != min_filter::None)
                        w.value("minFilter", static_cast<std::uint32_t>(sampler.min_filter()));
                    if (sampler.wrap_S() != wrapping_mode::REPEAT)
                        w.value("wrapS", static_cast<std::uint32_t>(sampler.wrap_S()));
                    if (sampler.wrap_T() != wrapping_mode::REPEAT)
                        w.value("wrapT", static_cast<std::uint32_t>(sampler.wrap_T()));
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.accessor_count() > 0u)
            {
                w.begin_array("accessors");
                for (index a = 0u; a < aGraph.accessor_count(); ++a)
                {
                    auto const& accessor = aGraph.accessor(a);
                    w.begin_object().name(accessor.name());
                    if (accessor.has_buffer_view())
                    {
                        w.value("bufferView", accessor.buffer_view());
                        if (accessor.byte_offset() != 0u)
                            w.value("byteOffset", static_cast<std::uint64_t>(accessor.byte_offset()));
                    }
                    w.value("componentType", static_cast<std::uint32_t>(accessor.component_type()));
                    if (accessor.normalized())
                        w.value("normalized", true);
                    w.value("count", static_cast<std::uint64_t>(accessor.count()));
                    w.value("type", sAccessorTypeNames[static_cast<std::size_t>(accessor.type())]);
                    if (!accessor.max().empty())
                        w.numbers("max", accessor.max());
                    if (!accessor.min().empty())
                        w.numbers("min", accessor.min());
                    if (accessor.has_sparse())
                    {
                        auto const& sparse = accessor.sparse();
                        w.begin_object("sparse");
                        w.value("count", static_cast<std::uint64_t>(sparse.count()));
                        w.begin_object("indices");
                        w.value("bufferView", sparse.indices_buffer_view());
                        if (sparse.indices_byte_offset() != 0u)
                            w.value("byteOffset", static_cast<std::uint64_t>(sparse.indices_byte_offset()));
                        w.value("componentType", static_cast<std::uint32_t>(sparse.indices_component_type()));
                        w.end_object();
                        w.begin_object("values");
                        w.value("bufferView", sparse.values_buffer_view());
                        if (sparse.values_byte_offset() != 0u)
                            w.value("byteOffset", static_cast<std::uint64_t>(sparse.values_byte_offset()));
                        w.end_object();
                        w.end_object();
                    }
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.buffer_view_count() > 0u)
            {
                w.begin_array("bufferViews");
                for (index v = 0u; v < aGraph.buffer_view_count(); ++v)
                {
                    auto const& view = aGraph.buffer_view(v);
                    w.begin_object().name(view.name());
                    w.value("buffer", view.buffer());
                    if (view.byte_offset() != 0u)
                        w.value("byteOffset", static_cast<std::uint64_t>(view.byte_offset()));
                    w.value("byteLength", static_cast<std::uint64_t>(view.byte_length()));
                    if (view.byte_stride() != 0u)
                        w.value("byteStride", static_cast<std::uint64_t>(view.byte_stride()));
                    if (view.target() != buffer_view_target::None)
                        w.value("target", static_cast<std::uint32_t>(view.target()));
                    w.end_object();
                }
                w.end_array();
            }

            if (aGraph.buffer_count() > 0u)
            {
                w.begin_array("buffers");
                for (index b = 0u; b < aGraph.buffer_count(); ++b)
                {
                    auto const& buffer = aGraph.buffer(b);
                    w.begin_object().name(buffer.name());
                    w.value("byteLength", static_cast<std::uint64_t>(buffer.byte_length()));
                    if (!aBinaryBuffer || *aBinaryBuffer != b)
                        w.value("uri", "data:application/octet-stream;base64," + 
                            base64_encode(buffer.data<std::byte>(), buffer.byte_length()));
                    w.end_object();
                }
                w.end_array();
            }

            w.end_object();
        }

        // JSON reading

        using json_document = neolib::json;
        using json_value = json_document::json_value;
        using json_object = json_document::json_object;
        using json_array = json_document::json_array;

        json_value const* member(json_value const& aObject, char const* aKey)
        {
            if (aObject.type() != neolib::json_type::Object)
                return nullptr;
            auto const& object = aObject.as<json_object>();
            if (!object.has(aKey))
                return nullptr;
            return &object.at(aKey);
        }

        std::vector<json_value const*> elements(json_value const* aArray)
        {
            std::vector<json_value const*> result;
            if (aArray && aArray->type() == neolib::json_type::Array)
                for (auto e : aArray->as<json_array>())
                    result.push_back(e);
            return result;
        }

        bool is_number(json_value const& aValue)
        {
            switch (aValue.type())
            {
            case neolib::json_type::Double:
            case neolib::json_type::Int64:
            case neolib::json_type::Uint64:
            case neolib::json_type::Int:
            case neolib::json_type::Uint:
                return true;
            default:
                return false;
            }
        }

        scalar number(json_value const& aObject, char const* aKey, scalar aDefault)
        {
            auto const value = member(aObject, aKey);
            if (value && is_number(*value))
                return value->as<double>();
            return aDefault;
        }

        std::optional<scalar> optional_number(json_value const& aObject, char const* aKey)
        {
            auto const value = member(aObject, aKey);
            if (value && is_number(*value))
                return value->as<double>();
            return {};
        }

        index index_of(json_value const& aObject, char const* aKey)
        {
            auto const value = member(aObject, aKey);
            if (value && is_number(*value))
                return static_cast<index>(value->as<double>());
            return invalid_index;
        }

        bool boolean(json_value const& aObject, char const* aKey, bool aDefault)
        {
            auto const value = member(aObject, aKey);
            if (value && value->type() == neolib::json_type::Bool)
                return value->as<bool>();
            return aDefault;
        }

        std::optional<std::string> text(json_value const& aObject, char const* aKey)
        {
            auto const value = member(aObject, aKey);
            if (value && value->type() == neolib::json_type::String)
                return value->text().to_std_string();
            return {};
        }

        std::vector<scalar> numbers(json_value const& aObject, char const* aKey)
        {
            std::vector<scalar> result;
            for (auto e : elements(member(aObject, aKey)))
                if (is_number(*e))
                    result.push_back(e->as<double>());
            return result;
        }

        template <typename Vector>
        void read_indices(json_value const& aObject, char const* aKey, Vector& aResult)
        {
            for (auto e : elements(member(aObject, aKey)))
                if (is_number(*e))
                    aResult.push_back(static_cast<index>(e->as<double>()));
        }

        template <typename Target>
        void read_texture_reference(json_value const& aObject, char const* aKey, Target& aTarget)
        {
            auto const reference = member(aObject, aKey);
            if (!reference)
                return;
            aTarget.set_texture(index_of(*reference, "index"), static_cast<tex_coord>(static_cast<std::uint32_t>(number(*reference, "texCoord", 0.0))));
        }

        void read_attributes(json_value const* aObject, attributes& aAttributes, std::vector<std::string>& aWarnings)
        {
            if (!aObject || aObject->type() != neolib::json_type::Object)
                return;
            for (auto const& a : *aObject)
            {
                if (!a.has_name() || !is_number(a))
                    continue;
                auto const name = a.name().to_std_string();
                auto const found = std::find_if(std::begin(sAttributeNames), std::end(sAttributeNames), [&](char const* n) { return name == n; });
                if (found == std::end(sAttributeNames))
                {
                    aWarnings.push_back("vertex attribute '" + name + "' not supported (dropped)");
                    continue;
                }
                aAttributes.set_attribute(static_cast<vertex_attribute>(found - std::begin(sAttributeNames)), static_cast<index>(a.as<double>()));
            }
        }

        std::uint32_t read_u32(char const* aSource)
        {
            std::uint32_t result;
            std::memcpy(&result, aSource, sizeof(result));
            return result; // n.b. GLB is little-endian, as are all our targets
        }

        void write_u32(std::ostream& aOutput, std::uint32_t aValue)
        {
            aOutput.write(reinterpret_cast<char const*>(&aValue), sizeof(aValue));
        }
    }

    gltf::gltf(std::string const& aUri) :
        iModel{ std::make_shared<scene_graph::scene_graph_model>() }
    {
        std::ifstream input{ std::filesystem::path{ aUri }, std::ios::binary };
        if (!input)
            throw error{ "neogfx::file::gltf: cannot open '" + aUri + "'" };
        read(input, std::filesystem::path{ aUri }.parent_path().string());
    }

    gltf::gltf(std::istream& aInput, std::string const& aBaseDirectory) :
        iModel{ std::make_shared<scene_graph::scene_graph_model>() }
    {
        read(aInput, aBaseDirectory);
    }

    gltf::gltf(std::shared_ptr<scene_graph::scene_graph_model> aModel) :
        iModel{ aModel ? aModel : std::make_shared<scene_graph::scene_graph_model>() }
    {
        update_scenes();
    }

    gltf::asset gltf::asset_info() const
    {
        auto const& source = model().asset();
        return gltf::asset{
            source.version().to_std_string(),
            source.min_version().has_value() ? std::optional<std::string>{ source.min_version().value().to_std_string() } : std::nullopt,
            source.generator().has_value() ? source.generator().value().to_std_string() : std::string{},
            source.copyright().has_value() ? source.copyright().value().to_std_string() : std::string{} };
    }

    gltf::local_transform gltf::transform(node const& aNode)
    {
        if (aNode.transform_flavour() == i_node::local_transform_flavour::Matrix)
            return aNode.matrix();
        return aNode.trs();
    }

    scene_graph::scene_graph_model const& gltf::model() const
    {
        return *iModel;
    }

    scene_graph::scene_graph_model& gltf::model()
    {
        return *iModel;
    }

    std::shared_ptr<scene_graph::scene_graph_model> const& gltf::shared_model() const
    {
        return iModel;
    }

    std::vector<std::string> const& gltf::warnings() const
    {
        return iWarnings;
    }

    void gltf::save(std::string const& aUri) const
    {
        write(model(), aUri);
    }

    void gltf::save(std::ostream& aOutput, format aFormat) const
    {
        write(model(), aOutput, aFormat);
    }

    void gltf::write(scene_graph::i_scene_graph const& aGraph, std::string const& aUri)
    {
        auto extension = std::filesystem::path{ aUri }.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
        auto const fileFormat = (extension == ".glb" ? format::Binary : format::Json);
        std::ofstream output{ std::filesystem::path{ aUri }, fileFormat == format::Binary ? std::ios::binary | std::ios::out : std::ios::out };
        if (!output)
            throw error{ "neogfx::file::gltf: cannot create '" + aUri + "'" };
        write(aGraph, output, fileFormat);
        if (!output)
            throw error{ "neogfx::file::gltf: error writing '" + aUri + "'" };
    }

    void gltf::write(scene_graph::i_scene_graph const& aGraph, std::ostream& aOutput, format aFormat)
    {
        if (aFormat == format::Json)
        {
            write_json(aOutput, aGraph, std::nullopt);
            return;
        }
        bool const hasBinary = aGraph.buffer_count() > 0u;
        std::ostringstream jsonStream;
        write_json(jsonStream, aGraph, hasBinary ? std::optional<index>{ 0u } : std::nullopt);
        auto json = jsonStream.str();
        while (json.size() % 4u != 0u)
            json.push_back(' ');
        std::size_t binarySize = 0u;
        if (hasBinary)
            binarySize = (aGraph.buffer(0u).byte_length() + 3u) / 4u * 4u;
        std::size_t const totalSize = 12u + 8u + json.size() + (hasBinary ? 8u + binarySize : 0u);
        write_u32(aOutput, GLB_MAGIC);
        write_u32(aOutput, 2u);
        write_u32(aOutput, static_cast<std::uint32_t>(totalSize));
        write_u32(aOutput, static_cast<std::uint32_t>(json.size()));
        write_u32(aOutput, GLB_CHUNK_JSON);
        aOutput.write(json.data(), json.size());
        if (hasBinary)
        {
            auto const& buffer = aGraph.buffer(0u);
            write_u32(aOutput, static_cast<std::uint32_t>(binarySize));
            write_u32(aOutput, GLB_CHUNK_BIN);
            aOutput.write(buffer.data<char>(), buffer.byte_length());
            for (std::size_t pad = buffer.byte_length(); pad < binarySize; ++pad)
                aOutput.put('\0');
        }
    }

    std::string gltf::to_json(scene_graph::i_scene_graph const& aGraph)
    {
        std::ostringstream output;
        write_json(output, aGraph, std::nullopt);
        return output.str();
    }

    void gltf::read(std::istream& aInput, std::string const& aBaseDirectory)
    {
        std::string const content{ std::istreambuf_iterator<char>{ aInput }, std::istreambuf_iterator<char>{} };
        std::string jsonText;
        std::optional<std::vector<std::byte>> binaryChunk;
        if (content.size() >= 12u && read_u32(content.data()) == GLB_MAGIC)
        {
            if (read_u32(content.data() + 4u) != 2u)
                throw error{ "neogfx::file::gltf: unsupported GLB version" };
            auto const length = std::min<std::size_t>(read_u32(content.data() + 8u), content.size());
            std::size_t offset = 12u;
            while (offset + 8u <= length)
            {
                auto const chunkLength = read_u32(content.data() + offset);
                auto const chunkType = read_u32(content.data() + offset + 4u);
                offset += 8u;
                if (offset + chunkLength > length)
                    throw error{ "neogfx::file::gltf: truncated GLB chunk" };
                if (chunkType == GLB_CHUNK_JSON && jsonText.empty())
                    jsonText.assign(content.data() + offset, chunkLength);
                else if (chunkType == GLB_CHUNK_BIN && !binaryChunk)
                {
                    auto const begin = reinterpret_cast<std::byte const*>(content.data() + offset);
                    binaryChunk.emplace(begin, begin + chunkLength);
                }
                offset += (chunkLength + 3u) / 4u * 4u;
            }
            if (jsonText.empty())
                throw error{ "neogfx::file::gltf: GLB has no JSON chunk" };
        }
        else
            jsonText = content;

        json_document document;
        std::istringstream jsonStream{ jsonText };
        if (!document.read(jsonStream) || !document.has_root())
            throw error{ "neogfx::file::gltf: invalid JSON: " + document.error_text() };
        auto const& root = document.root();
        if (root.type() != neolib::json_type::Object)
            throw error{ "neogfx::file::gltf: invalid glTF" };

        auto& m = model();
        m.clear();
        m.set_base_directory(aBaseDirectory);

        if (auto const assetValue = member(root, "asset"))
        {
            auto const version = text(*assetValue, "version");
            if (!version || version->empty() || (*version)[0] != '2')
                throw error{ "neogfx::file::gltf: unsupported glTF version '" + version.value_or("") + "'" };
            m.asset().set_version(*version);
            m.asset().set_min_version(text(*assetValue, "minVersion"));
            m.asset().set_generator(text(*assetValue, "generator"));
            m.asset().set_copyright(text(*assetValue, "copyright"));
        }
        else
            throw error{ "neogfx::file::gltf: missing asset" };

        m.set_dimension(scene_graph::dimension::Three);
        for (auto e : elements(member(root, "extensionsUsed")))
        {
            if (e->type() != neolib::json_type::String)
                continue;
            auto const extension = e->text().to_std_string();
            if (extension == extension_2d)
                m.set_dimension(scene_graph::dimension::Two);
            else
                iWarnings.push_back("extension '" + extension + "' not supported (ignored)");
        }

        auto const buffers = elements(member(root, "buffers"));
        for (std::size_t b = 0u; b < buffers.size(); ++b)
        {
            auto const& source = *buffers[b];
            scene_graph::buffer newBuffer;
            newBuffer.set_name(text(source, "name"));
            auto const byteLength = static_cast<std::size_t>(number(source, "byteLength", 0.0));
            auto const uri = text(source, "uri");
            if (!uri)
            {
                if (b != 0u || !binaryChunk)
                    throw error{ "neogfx::file::gltf: buffer " + std::to_string(b) + " has no data" };
                newBuffer.bytes() = *binaryChunk;
            }
            else if (uri->rfind("data:", 0u) == 0u)
            {
                auto const comma = uri->find(',');
                if (comma == std::string::npos || uri->substr(0u, comma).find(";base64") == std::string::npos)
                    throw error{ "neogfx::file::gltf: unsupported data URI in buffer " + std::to_string(b) };
                newBuffer.bytes() = decode_base64(std::string_view{ *uri }.substr(comma + 1u));
            }
            else
            {
                auto const path = std::filesystem::path{ aBaseDirectory } / std::filesystem::path{ percent_decode(*uri) };
                std::ifstream file{ path, std::ios::binary };
                if (!file)
                    throw error{ "neogfx::file::gltf: cannot open buffer '" + path.string() + "'" };
                std::string const data{ std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
                auto const begin = reinterpret_cast<std::byte const*>(data.data());
                newBuffer.bytes().assign(begin, begin + data.size());
                // keep the reference; data is embedded when written
                newBuffer.set_uri(uri);
            }
            if (newBuffer.bytes().size() < byteLength)
                throw error{ "neogfx::file::gltf: buffer " + std::to_string(b) + " is shorter than its byteLength" };
            newBuffer.bytes().resize(byteLength);
            m.add(newBuffer);
        }

        for (auto e : elements(member(root, "bufferViews")))
        {
            scene_graph::buffer_view newView{
                index_of(*e, "buffer"),
                static_cast<std::size_t>(number(*e, "byteOffset", 0.0)),
                static_cast<std::size_t>(number(*e, "byteLength", 0.0)),
                static_cast<std::size_t>(number(*e, "byteStride", 0.0)),
                static_cast<buffer_view_target>(static_cast<std::uint32_t>(number(*e, "target", 0.0))) };
            newView.set_name(text(*e, "name"));
            if (newView.buffer() >= m.buffer_count() || newView.byte_offset() + newView.byte_length() > m.buffer(newView.buffer()).byte_length())
                throw error{ "neogfx::file::gltf: buffer view out of range" };
            m.add(newView);
        }

        for (auto e : elements(member(root, "accessors")))
        {
            scene_graph::accessor newAccessor;
            newAccessor.set_name(text(*e, "name"));
            newAccessor.set_buffer_view(index_of(*e, "bufferView"));
            newAccessor.set_byte_offset(static_cast<std::size_t>(number(*e, "byteOffset", 0.0)));
            newAccessor.set_component_type(static_cast<accessor_component_type>(static_cast<std::uint32_t>(number(*e, "componentType", 5126.0))));
            newAccessor.set_normalized(boolean(*e, "normalized", false));
            newAccessor.set_count(static_cast<std::size_t>(number(*e, "count", 0.0)));
            auto const type = text(*e, "type").value_or("SCALAR");
            auto const foundType = std::find_if(std::begin(sAccessorTypeNames), std::end(sAccessorTypeNames), [&](char const* n) { return type == n; });
            if (foundType == std::end(sAccessorTypeNames))
                throw error{ "neogfx::file::gltf: unknown accessor type '" + type + "'" };
            newAccessor.set_type(static_cast<accessor_type>(foundType - std::begin(sAccessorTypeNames)));
            for (auto v : numbers(*e, "max"))
                newAccessor.max().push_back(v);
            for (auto v : numbers(*e, "min"))
                newAccessor.min().push_back(v);
            if (auto const sparseValue = member(*e, "sparse"))
            {
                scene_graph::sparse_array sparse;
                sparse.set_count(static_cast<std::size_t>(number(*sparseValue, "count", 0.0)));
                if (auto const indices = member(*sparseValue, "indices"))
                    sparse.set_indices(index_of(*indices, "bufferView"), static_cast<std::size_t>(number(*indices, "byteOffset", 0.0)),
                        static_cast<accessor_component_type>(static_cast<std::uint32_t>(number(*indices, "componentType", 5125.0))));
                if (auto const values = member(*sparseValue, "values"))
                    sparse.set_values(index_of(*values, "bufferView"), static_cast<std::size_t>(number(*values, "byteOffset", 0.0)));
                newAccessor.set_sparse(sparse);
            }
            m.add(newAccessor);
        }

        for (auto e : elements(member(root, "images")))
        {
            scene_graph::image newImage;
            newImage.set_name(text(*e, "name"));
            newImage.set_uri(text(*e, "uri"));
            newImage.set_mime_type(text(*e, "mimeType"));
            newImage.set_buffer_view(index_of(*e, "bufferView"));
            m.add(newImage);
        }

        for (auto e : elements(member(root, "samplers")))
        {
            scene_graph::sampler newSampler;
            newSampler.set_name(text(*e, "name"));
            newSampler.set_mag_filter(static_cast<mag_filter>(static_cast<std::uint32_t>(number(*e, "magFilter", 0.0))));
            newSampler.set_min_filter(static_cast<min_filter>(static_cast<std::uint32_t>(number(*e, "minFilter", 0.0))));
            newSampler.set_wrap_S(static_cast<wrapping_mode>(static_cast<std::uint32_t>(number(*e, "wrapS", 10497.0))));
            newSampler.set_wrap_T(static_cast<wrapping_mode>(static_cast<std::uint32_t>(number(*e, "wrapT", 10497.0))));
            m.add(newSampler);
        }

        for (auto e : elements(member(root, "textures")))
        {
            scene_graph::texture newTexture;
            newTexture.set_name(text(*e, "name"));
            newTexture.set_sampler(index_of(*e, "sampler"));
            newTexture.set_source(index_of(*e, "source"));
            m.add(newTexture);
        }

        for (auto e : elements(member(root, "materials")))
        {
            scene_graph::material newMaterial;
            newMaterial.set_name(text(*e, "name"));
            if (auto const pbr = member(*e, "pbrMetallicRoughness"))
            {
                auto const factor = numbers(*pbr, "baseColorFactor");
                if (factor.size() == 4u)
                    newMaterial.pbr_metallic_roughness().set_base_color_factor(vec4{ factor[0], factor[1], factor[2], factor[3] });
                read_texture_reference(*pbr, "baseColorTexture", newMaterial.pbr_metallic_roughness().base_color_texture());
                newMaterial.pbr_metallic_roughness().set_metallic_factor(number(*pbr, "metallicFactor", 1.0));
                newMaterial.pbr_metallic_roughness().set_roughness_factor(number(*pbr, "roughnessFactor", 1.0));
                read_texture_reference(*pbr, "metallicRoughnessTexture", newMaterial.pbr_metallic_roughness().metallic_roughness_texture());
            }
            read_texture_reference(*e, "normalTexture", newMaterial.normal_texture());
            if (auto const normalTexture = member(*e, "normalTexture"))
                newMaterial.normal_texture().set_scale(number(*normalTexture, "scale", 1.0));
            read_texture_reference(*e, "occlusionTexture", newMaterial.occlusion_texture());
            if (auto const occlusionTexture = member(*e, "occlusionTexture"))
                newMaterial.occlusion_texture().set_strength(number(*occlusionTexture, "strength", 1.0));
            read_texture_reference(*e, "emissiveTexture", newMaterial.emissive_texture());
            auto const emissive = numbers(*e, "emissiveFactor");
            if (emissive.size() == 3u)
                newMaterial.set_emissive_factor(vec3{ emissive[0], emissive[1], emissive[2] });
            auto const alphaMode = text(*e, "alphaMode").value_or("OPAQUE");
            newMaterial.set_alpha_mode(alphaMode == "MASK" ? alpha_mode::Mask : alphaMode == "BLEND" ? alpha_mode::Blend : alpha_mode::Opaque);
            newMaterial.set_alpha_cutoff(number(*e, "alphaCutoff", 0.5));
            newMaterial.set_double_sided(boolean(*e, "doubleSided", false));
            m.add(newMaterial);
        }

        for (auto e : elements(member(root, "meshes")))
        {
            scene_graph::mesh newMesh;
            newMesh.set_name(text(*e, "name"));
            for (auto p : elements(member(*e, "primitives")))
            {
                scene_graph::mesh_primitive primitive;
                read_attributes(member(*p, "attributes"), primitive.attributes(), iWarnings);
                primitive.set_indices(index_of(*p, "indices"));
                primitive.set_material(index_of(*p, "material"));
                primitive.set_mode(static_cast<rendering_mode>(static_cast<std::uint32_t>(number(*p, "mode", 4.0))));
                for (auto t : elements(member(*p, "targets")))
                {
                    primitive.morph_targets().emplace_back();
                    read_attributes(t, primitive.morph_targets().back(), iWarnings);
                }
                newMesh.primitives().push_back(primitive);
            }
            for (auto w : numbers(*e, "weights"))
                newMesh.weights().push_back(w);
            m.add(newMesh);
        }

        for (auto e : elements(member(root, "cameras")))
        {
            auto const type = text(*e, "type").value_or("perspective");
            std::optional<scene_graph::camera> newCamera;
            if (type == "orthographic")
            {
                auto const o = member(*e, "orthographic");
                if (!o)
                    throw error{ "neogfx::file::gltf: orthographic camera has no parameters" };
                newCamera.emplace(orthographic_camera{ number(*o, "xmag", 1.0), number(*o, "ymag", 1.0), number(*o, "zfar", 100.0), number(*o, "znear", 0.0) });
            }
            else
            {
                auto const p = member(*e, "perspective");
                if (!p)
                    throw error{ "neogfx::file::gltf: perspective camera has no parameters" };
                newCamera.emplace(perspective_camera{ number(*p, "yfov", to_rad(45.0)), number(*p, "znear", 0.1), optional_number(*p, "zfar"), optional_number(*p, "aspectRatio") });
            }
            newCamera->set_name(text(*e, "name"));
            m.add(*newCamera);
        }

        for (auto e : elements(member(root, "skins")))
        {
            scene_graph::skin newSkin;
            newSkin.set_name(text(*e, "name"));
            newSkin.set_inverse_bind_matrices(index_of(*e, "inverseBindMatrices"));
            newSkin.set_skeleton(index_of(*e, "skeleton"));
            read_indices(*e, "joints", newSkin.joints());
            m.add(newSkin);
        }

        for (auto e : elements(member(root, "nodes")))
        {
            scene_graph::node newNode;
            newNode.set_name(text(*e, "name"));
            read_indices(*e, "children", newNode.children());
            newNode.set_camera(index_of(*e, "camera"));
            newNode.set_mesh(index_of(*e, "mesh"));
            newNode.set_skin(index_of(*e, "skin"));
            auto const matrix = numbers(*e, "matrix");
            if (matrix.size() == 16u)
            {
                mat44 nodeMatrix;
                for (std::uint32_t c = 0u; c < 4u; ++c)
                    for (std::uint32_t r = 0u; r < 4u; ++r)
                        nodeMatrix[c][r] = matrix[c * 4u + r];
                newNode.set_matrix(nodeMatrix);
            }
            else
            {
                i_node::trs_transform trs;
                auto const translation = numbers(*e, "translation");
                if (translation.size() == 3u)
                    trs.translation = vec3{ translation[0], translation[1], translation[2] };
                auto const rotation = numbers(*e, "rotation");
                if (rotation.size() == 4u)
                    trs.rotation = vec4{ rotation[0], rotation[1], rotation[2], rotation[3] };
                auto const scale = numbers(*e, "scale");
                if (scale.size() == 3u)
                    trs.scale = vec3{ scale[0], scale[1], scale[2] };
                newNode.set_trs(trs);
            }
            for (auto w : numbers(*e, "weights"))
                newNode.weights().push_back(w);
            m.add(newNode);
        }

        for (auto e : elements(member(root, "animations")))
        {
            scene_graph::animation newAnimation;
            newAnimation.set_name(text(*e, "name"));
            for (auto c : elements(member(*e, "channels")))
            {
                auto const target = member(*c, "target");
                auto const path = target ? text(*target, "path").value_or("") : std::string{};
                auto const foundPath = std::find_if(std::begin(sAnimationPathNames), std::end(sAnimationPathNames), [&](char const* n) { return path == n; });
                if (foundPath == std::end(sAnimationPathNames))
                {
                    iWarnings.push_back("animation channel path '" + path + "' not supported (dropped)");
                    continue;
                }
                newAnimation.channels().emplace_back(index_of(*c, "sampler"), target ? index_of(*target, "node") : invalid_index,
                    static_cast<animation_path>(foundPath - std::begin(sAnimationPathNames)));
            }
            for (auto s : elements(member(*e, "samplers")))
            {
                auto const interpolation = text(*s, "interpolation").value_or("LINEAR");
                auto const foundInterpolation = std::find_if(std::begin(sInterpolationNames), std::end(sInterpolationNames), [&](char const* n) { return interpolation == n; });
                newAnimation.samplers().emplace_back(index_of(*s, "input"), index_of(*s, "output"),
                    foundInterpolation != std::end(sInterpolationNames) ? static_cast<animation_interpolation>(foundInterpolation - std::begin(sInterpolationNames)) : animation_interpolation::Linear);
            }
            m.add(newAnimation);
        }

        for (auto e : elements(member(root, "scenes")))
        {
            scene_graph::scene newScene;
            newScene.set_name(text(*e, "name"));
            read_indices(*e, "nodes", newScene.nodes());
            m.add(newScene);
        }

        auto const defaultScene = index_of(root, "scene");
        if (defaultScene != invalid_index && defaultScene < m.scene_count())
            m.set_default_scene(defaultScene);

        update_scenes();
    }

    void gltf::update_scenes()
    {
        scenes.clear();
        displayScene = nullptr;
        auto& m = model();
        for (index s = 0u; s < m.scene_count(); ++s)
            scenes.push_back(&m.scene(s));
        auto const active = m.active_scene();
        if (active != invalid_index)
            displayScene = std::shared_ptr<scene>{ iModel, &m.scene(active) };
    }
}
