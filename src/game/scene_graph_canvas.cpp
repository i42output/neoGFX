// scene_graph_canvas.cpp
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

#include <functional>

#include <neogfx/app/i_resource_manager.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_standard_shader_program.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/gfx/scene_graph.hpp>
#include <neogfx/game/scene_graph_canvas.hpp>
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/ecs_helpers.hpp>
#include <neogfx/game/renderable_entity_archetype.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/mesh_render_cache.hpp>
#include <neogfx/game/model_transformation.hpp>

namespace neogfx::game
{
    namespace sg = neogfx::scene_graph;

    namespace
    {
        renderable_entity_archetype const& scene_graph_primitive_archetype(i_ecs& aEcs)
        {
            static const renderable_entity_archetype sArchetype
            {
                { 0xd61bac8c, 0xed15, 0x4019, 0xbd0a, { 0x02, 0x57, 0x80, 0x7c, 0x40, 0x1b } },
                "SceneGraphPrimitive",
                { mesh_renderer::meta::id(), mesh_filter::meta::id(), model_transformation::meta::id() }
            };
            if (!aEcs.archetype_registered(sArchetype))
                aEcs.register_archetype(sArchetype);
            return sArchetype;
        }

        // the fixed directional light: world space, towards the light
        vec3 scene_light()
        {
            return vec3{ -0.4, 0.8, 0.45 }.normalized();
        }

        mat44 translation_matrix(vec3 const& aTranslation)
        {
            auto result = mat44::identity();
            result[3][0] = aTranslation.x;
            result[3][1] = aTranslation.y;
            result[3][2] = aTranslation.z;
            return result;
        }

        mat44 scaling_matrix(vec3 const& aScale)
        {
            auto result = mat44::identity();
            result[0][0] = aScale.x;
            result[1][1] = aScale.y;
            result[2][2] = aScale.z;
            return result;
        }

        // general 4x4 inverse (column-major m[column][row]); identity if singular
        mat44 inverse(mat44 const& aMatrix)
        {
            scalar a[16];
            for (std::uint32_t c = 0u; c < 4u; ++c)
                for (std::uint32_t r = 0u; r < 4u; ++r)
                    a[c * 4u + r] = aMatrix[c][r];
            scalar inv[16];
            inv[0] = a[5] * a[10] * a[15] - a[5] * a[11] * a[14] - a[9] * a[6] * a[15] + a[9] * a[7] * a[14] + a[13] * a[6] * a[11] - a[13] * a[7] * a[10];
            inv[4] = -a[4] * a[10] * a[15] + a[4] * a[11] * a[14] + a[8] * a[6] * a[15] - a[8] * a[7] * a[14] - a[12] * a[6] * a[11] + a[12] * a[7] * a[10];
            inv[8] = a[4] * a[9] * a[15] - a[4] * a[11] * a[13] - a[8] * a[5] * a[15] + a[8] * a[7] * a[13] + a[12] * a[5] * a[11] - a[12] * a[7] * a[9];
            inv[12] = -a[4] * a[9] * a[14] + a[4] * a[10] * a[13] + a[8] * a[5] * a[14] - a[8] * a[6] * a[13] - a[12] * a[5] * a[10] + a[12] * a[6] * a[9];
            inv[1] = -a[1] * a[10] * a[15] + a[1] * a[11] * a[14] + a[9] * a[2] * a[15] - a[9] * a[3] * a[14] - a[13] * a[2] * a[11] + a[13] * a[3] * a[10];
            inv[5] = a[0] * a[10] * a[15] - a[0] * a[11] * a[14] - a[8] * a[2] * a[15] + a[8] * a[3] * a[14] + a[12] * a[2] * a[11] - a[12] * a[3] * a[10];
            inv[9] = -a[0] * a[9] * a[15] + a[0] * a[11] * a[13] + a[8] * a[1] * a[15] - a[8] * a[3] * a[13] - a[12] * a[1] * a[11] + a[12] * a[3] * a[9];
            inv[13] = a[0] * a[9] * a[14] - a[0] * a[10] * a[13] - a[8] * a[1] * a[14] + a[8] * a[2] * a[13] + a[12] * a[1] * a[10] - a[12] * a[2] * a[9];
            inv[2] = a[1] * a[6] * a[15] - a[1] * a[7] * a[14] - a[5] * a[2] * a[15] + a[5] * a[3] * a[14] + a[13] * a[2] * a[7] - a[13] * a[3] * a[6];
            inv[6] = -a[0] * a[6] * a[15] + a[0] * a[7] * a[14] + a[4] * a[2] * a[15] - a[4] * a[3] * a[14] - a[12] * a[2] * a[7] + a[12] * a[3] * a[6];
            inv[10] = a[0] * a[5] * a[15] - a[0] * a[7] * a[13] - a[4] * a[1] * a[15] + a[4] * a[3] * a[13] + a[12] * a[1] * a[7] - a[12] * a[3] * a[5];
            inv[14] = -a[0] * a[5] * a[14] + a[0] * a[6] * a[13] + a[4] * a[1] * a[14] - a[4] * a[2] * a[13] - a[12] * a[1] * a[6] + a[12] * a[2] * a[5];
            inv[3] = -a[1] * a[6] * a[11] + a[1] * a[7] * a[10] + a[5] * a[2] * a[11] - a[5] * a[3] * a[10] - a[9] * a[2] * a[7] + a[9] * a[3] * a[6];
            inv[7] = a[0] * a[6] * a[11] - a[0] * a[7] * a[10] - a[4] * a[2] * a[11] + a[4] * a[3] * a[10] + a[8] * a[2] * a[7] - a[8] * a[3] * a[6];
            inv[11] = -a[0] * a[5] * a[11] + a[0] * a[7] * a[9] + a[4] * a[1] * a[11] - a[4] * a[3] * a[9] - a[8] * a[1] * a[7] + a[8] * a[3] * a[5];
            inv[15] = a[0] * a[5] * a[10] - a[0] * a[6] * a[9] - a[4] * a[1] * a[10] + a[4] * a[2] * a[9] + a[8] * a[1] * a[6] - a[8] * a[2] * a[5];
            scalar const det = a[0] * inv[0] + a[1] * inv[4] + a[2] * inv[8] + a[3] * inv[12];
            if (std::abs(det) < 1e-12)
                return mat44::identity();
            mat44 result;
            for (std::uint32_t c = 0u; c < 4u; ++c)
                for (std::uint32_t r = 0u; r < 4u; ++r)
                    result[c][r] = inv[c * 4u + r] / det;
            return result;
        }

        // glTF projection matrices (column-major)
        mat44 perspective_matrix(scalar aAspectRatio, scalar aYfov, scalar aZnear, std::optional<scalar> const& aZfar)
        {
            scalar const f = 1.0 / std::tan(aYfov / 2.0);
            mat44 result;
            result[0][0] = f / aAspectRatio;
            result[1][1] = f;
            result[2][3] = -1.0;
            if (aZfar && *aZfar > aZnear)
            {
                result[2][2] = (*aZfar + aZnear) / (aZnear - *aZfar);
                result[3][2] = 2.0 * *aZfar * aZnear / (aZnear - *aZfar);
            }
            else
            {
                result[2][2] = -1.0;
                result[3][2] = -2.0 * aZnear;
            }
            return result;
        }

        mat44 orthographic_matrix(scalar aXmag, scalar aYmag, scalar aZnear, scalar aZfar)
        {
            mat44 result = mat44::identity();
            result[0][0] = 1.0 / aXmag;
            result[1][1] = 1.0 / aYmag;
            result[2][2] = 2.0 / (aZnear - aZfar);
            result[3][2] = (aZfar + aZnear) / (aZnear - aZfar);
            return result;
        }
    }

    scene_graph_canvas::scene_graph_canvas() :
        canvas{ std::make_shared<game::ecs>(ecs_flags::Default | ecs_flags::NoThreads) },
        iScene{ sg::invalid_index },
        iCameraNode{ sg::invalid_index },
        iLighting{ true },
        iLightingModel{ scene_lighting::PerVertex },
        iMouseCameraControl{ true },
        iDragPan{ false },
        iLastCameraWorld{ mat44::identity() },
        iLastYfov{ to_rad(45.0) },
        iZoom2D{ 1.0 },
        iLastScale2D{ 1.0 }
    {
        init();
    }

    scene_graph_canvas::scene_graph_canvas(i_widget& aParent) :
        canvas{ aParent, std::make_shared<game::ecs>(ecs_flags::Default | ecs_flags::NoThreads) },
        iScene{ sg::invalid_index },
        iCameraNode{ sg::invalid_index },
        iLighting{ true },
        iLightingModel{ scene_lighting::PerVertex },
        iMouseCameraControl{ true },
        iDragPan{ false },
        iLastCameraWorld{ mat44::identity() },
        iLastYfov{ to_rad(45.0) },
        iZoom2D{ 1.0 },
        iLastScale2D{ 1.0 }
    {
        init();
    }

    scene_graph_canvas::scene_graph_canvas(i_layout& aLayout) :
        canvas{ aLayout, std::make_shared<game::ecs>(ecs_flags::Default | ecs_flags::NoThreads) },
        iScene{ sg::invalid_index },
        iCameraNode{ sg::invalid_index },
        iLighting{ true },
        iLightingModel{ scene_lighting::PerVertex },
        iMouseCameraControl{ true },
        iDragPan{ false },
        iLastCameraWorld{ mat44::identity() },
        iLastYfov{ to_rad(45.0) },
        iZoom2D{ 1.0 },
        iLastScale2D{ 1.0 }
    {
        init();
    }

    scene_graph_canvas::~scene_graph_canvas()
    {
        stop_animation();
        iSink.clear();
    }

    void scene_graph_canvas::play_animation(sg::index aAnimation)
    {
        stop_animation();
        if (!has_graph() || aAnimation >= graph().animation_count())
            return;
        iAnimationPlayer.emplace(graph(), aAnimation);
        iAnimationTime = 0.0;
        iAnimationClock = std::chrono::steady_clock::now();
        // the timer only requests frames: the animation is advanced when they are rendered
        iAnimationTimer.emplace(*this, [this](widget_timer& aTimer)
            {
                aTimer.again();
                if (!iAnimationPaused)
                    update();
            }, std::chrono::milliseconds{ 8 });
        update();
    }

    void scene_graph_canvas::stop_animation()
    {
        iAnimationTimer = std::nullopt;
        iAnimationPlayer = std::nullopt;
    }

    std::optional<sg::index> scene_graph_canvas::playing_animation() const
    {
        if (iAnimationPlayer)
            return iAnimationPlayer->animation();
        return std::nullopt;
    }

    bool scene_graph_canvas::animation_paused() const
    {
        return iAnimationPaused;
    }

    void scene_graph_canvas::set_animation_paused(bool aPaused)
    {
        iAnimationPaused = aPaused;
    }

    void scene_graph_canvas::advance_animation()
    {
        if (!iAnimationPlayer)
            return;
        auto const now = std::chrono::steady_clock::now();
        auto const elapsed = std::chrono::duration<scalar>(now - iAnimationClock).count();
        iAnimationClock = now;
        if (iAnimationPaused)
            return;
        iAnimationTime += elapsed;
        auto const duration = iAnimationPlayer->duration();
        iAnimationPlayer->apply(duration > 0.0 ? std::fmod(iAnimationTime, duration) : 0.0);
    }

    bool scene_graph_canvas::has_graph() const
    {
        return iGraph != nullptr;
    }

    sg::i_scene_graph const& scene_graph_canvas::graph() const
    {
        if (!has_graph())
            throw std::logic_error{ "neogfx::game::scene_graph_canvas::no_graph" };
        return *iGraph;
    }

    sg::i_scene_graph& scene_graph_canvas::graph()
    {
        return const_cast<sg::i_scene_graph&>(static_cast<scene_graph_canvas const&>(*this).graph());
    }

    void scene_graph_canvas::set_graph(sg::i_scene_graph& aGraph)
    {
        set_graph(std::shared_ptr<sg::i_scene_graph>{ std::shared_ptr<sg::i_scene_graph>{}, &aGraph });
    }

    void scene_graph_canvas::set_graph(std::shared_ptr<sg::i_scene_graph> aGraph)
    {
        stop_animation();
        iGraph = aGraph;
        reset_view();
        rebuild();
    }

    void scene_graph_canvas::clear_graph()
    {
        set_graph(std::shared_ptr<sg::i_scene_graph>{});
    }

    void scene_graph_canvas::rebuild()
    {
        iBuiltRevision = std::nullopt;
        iTextures.clear();
        update();
    }

    sg::index scene_graph_canvas::displayed_scene() const
    {
        return iScene;
    }

    void scene_graph_canvas::set_displayed_scene(sg::index aScene)
    {
        if (iScene != aScene)
        {
            iScene = aScene;
            rebuild();
        }
    }

    sg::index scene_graph_canvas::camera_node() const
    {
        return iCameraNode;
    }

    void scene_graph_canvas::set_camera_node(sg::index aNode)
    {
        iCameraNode = aNode;
        update();
    }

    std::optional<scalar> const& scene_graph_canvas::view_scale() const
    {
        return iViewScale;
    }

    void scene_graph_canvas::set_view_scale(std::optional<scalar> const& aScale)
    {
        iViewScale = aScale;
        update();
    }

    bool scene_graph_canvas::lighting() const
    {
        return iLighting;
    }

    void scene_graph_canvas::set_lighting(bool aLighting)
    {
        iLighting = aLighting;
        update();
    }

    scene_lighting scene_graph_canvas::lighting_model() const
    {
        return iLightingModel;
    }

    void scene_graph_canvas::set_lighting_model(scene_lighting aLightingModel)
    {
        bool const texturesChanged = (iLightingModel == scene_lighting::PhysicallyBased) != (aLightingModel == scene_lighting::PhysicallyBased);
        iLightingModel = aLightingModel;
        if (texturesChanged)
            rebuild(); // physically based shading's textures are only loaded if it is used
        else
            update();
    }

    void scene_graph_canvas::add_point_light(sg::index aNode, vec3 const& aRadiance, scalar aRange, bool aCastsShadows, scalar aSize)
    {
        iPointLights.push_back(point_light{ aNode, aRadiance, aRange, aCastsShadows, aSize });
        update();
    }

    void scene_graph_canvas::clear_point_lights()
    {
        iPointLights.clear();
        update();
    }

    bool scene_graph_canvas::shadows() const
    {
        return iShadows;
    }

    void scene_graph_canvas::set_shadows(bool aShadows)
    {
        iShadows = aShadows;
        update();
    }

    void scene_graph_canvas::init()
    {
        ecs().component<mesh_filter>();
        ecs().component<mesh_renderer>();
        ecs().component<mesh_render_cache>();
        iSink += RenderingEntities([this](i_graphics_context&, std::int32_t aLayer)
        {
            if (aLayer == 0)
            {
                // animation is sampled now, as the frame is rendered, rather than when a timer happened to fire
                Animating();
                advance_animation();
                update_entities();
            }
        });
    }

    void scene_graph_canvas::destroy_entities()
    {
        for (auto const& e : iEntities)
            ecs().destroy_entity(e.entity);
        iEntities.clear();
        iBounds = std::nullopt;
    }

    sg::index scene_graph_canvas::scene_index() const
    {
        if (!has_graph())
            return sg::invalid_index;
        if (iScene != sg::invalid_index)
            return iScene < graph().scene_count() ? iScene : sg::invalid_index;
        return graph().active_scene();
    }

    sg::index scene_graph_canvas::find_camera_node(std::vector<std::optional<mat44>> const& aWorld) const
    {
        if (iCameraNode != sg::invalid_index)
            return iCameraNode < aWorld.size() && aWorld[iCameraNode] && graph().node(iCameraNode).has_camera() &&
                graph().node(iCameraNode).camera() < graph().camera_count() ? iCameraNode : sg::invalid_index;
        for (sg::index n = 0u; n < aWorld.size(); ++n)
            if (aWorld[n] && graph().node(n).has_camera() && graph().node(n).camera() < graph().camera_count())
                return n;
        return sg::invalid_index;
    }

    void scene_graph_canvas::build()
    {
        destroy_entities();
        iBuiltRevision = has_graph() ? std::optional<std::uint64_t>{ graph().revision() } : std::nullopt;
        auto const scene = scene_index();
        if (scene == sg::invalid_index)
            return;
        auto const& g = graph();
        bool const twoD = (g.dimension() == sg::dimension::Two);
        auto const world = sg::world_transformations(g, scene);
        for (sg::index n = 0u; n < g.node_count(); ++n)
        {
            if (!world[n] || !g.node(n).has_mesh() || g.node(n).mesh() >= g.mesh_count())
                continue;
            auto const& mesh = g.mesh(g.node(n).mesh());
            for (std::uint32_t p = 0u; p < mesh.primitive_count(); ++p)
            {
                auto const& primitive = mesh.primitive(p);
                primitive_entity newEntity{};
                game::mesh localMesh;
                // base colour texture
                sg::index textureImage = sg::invalid_index;
                std::uint32_t texCoord = 0u;
                auto sampling = texture_sampling::NormalMipmap;
                if (primitive.has_material() && primitive.material() < g.material_count())
                {
                    auto const& reference = g.material(primitive.material()).pbr_metallic_roughness().base_color_texture();
                    if (reference.has_texture() && reference.texture() < g.texture_count() && g.texture(reference.texture()).has_source())
                    {
                        auto const& texture = g.texture(reference.texture());
                        textureImage = texture.source();
                        texCoord = static_cast<std::uint32_t>(reference.tex_coord());
                        if (texture.has_sampler() && texture.sampler() < g.sampler_count() &&
                            g.sampler(texture.sampler()).mag_filter() == sg::mag_filter::NEAREST)
                            sampling = texture_sampling::Nearest;
                    }
                }
                try
                {
                    if (!sg::to_ecs_mesh(g, primitive, localMesh, texCoord) || localMesh.faces.empty())
                        continue;
                }
                catch (std::exception const&)
                {
                    continue; // malformed primitive: skip it
                }
                // n.b. textures aren't repeated (glTF's default wrapping) so texture coordinates confined to one whole tile other
                // than the first (e.g. 1 to 2, as some exporters write) are moved into the first
                if (!localMesh.uv.empty())
                {
                    vec2f minimum = localMesh.uv[0];
                    vec2f maximum = localMesh.uv[0];
                    for (auto const& uv : localMesh.uv)
                    {
                        minimum = minimum.min(uv);
                        maximum = maximum.max(uv);
                    }
                    constexpr float tolerance = 1e-4f;
                    vec2f offset;
                    for (std::uint32_t axis = 0u; axis < 2u; ++axis)
                    {
                        float const tile = std::floor(minimum[axis] + tolerance);
                        if (tile != 0.0f && maximum[axis] - tile <= 1.0f + tolerance)
                            offset[axis] = -tile;
                    }
                    if (offset != vec2f{})
                        for (auto& uv : localMesh.uv)
                            uv += offset;
                }
                std::optional<game::texture> texture;
                if (textureImage != sg::invalid_index && localMesh.uv.size() == localMesh.vertices.size())
                    texture = load_texture(textureImage, sampling);
                // the rest of the material, for physically based shading (its textures, which use the base colour texture's
                // coordinates, are only loaded if it is used); n.b. no material: the glTF default material
                game::pbr_material pbr;
                if (primitive.has_material() && primitive.material() < g.material_count())
                {
                    auto const& material = g.material(primitive.material());
                    pbr.metallic = material.pbr_metallic_roughness().metallic_factor();
                    pbr.roughness = material.pbr_metallic_roughness().roughness_factor();
                    pbr.normalScale = material.normal_texture().scale();
                    pbr.occlusionStrength = material.occlusion_texture().strength();
                    pbr.emissive = material.emissive_factor();
                    if (material.alpha_mode() == sg::alpha_mode::Mask)
                        pbr.alphaCutoff = material.alpha_cutoff();
                    pbr.doubleSided = material.double_sided();
                    auto const pbr_texture = [&](sg::i_texture_reference const& aReference) -> std::optional<game::texture>
                    {
                        if (iLightingModel != scene_lighting::PhysicallyBased || !aReference.has_texture() || aReference.texture() >= g.texture_count() ||
                            !g.texture(aReference.texture()).has_source() || localMesh.uv.size() != localMesh.vertices.size())
                            return std::nullopt;
                        auto const& textureInfo = g.texture(aReference.texture());
                        auto textureSampling = texture_sampling::NormalMipmap;
                        if (textureInfo.has_sampler() && textureInfo.sampler() < g.sampler_count() &&
                            g.sampler(textureInfo.sampler()).mag_filter() == sg::mag_filter::NEAREST)
                            textureSampling = texture_sampling::Nearest;
                        return load_texture(textureInfo.source(), textureSampling);
                    };
                    pbr.metallicRoughnessTexture = pbr_texture(material.pbr_metallic_roughness().metallic_roughness_texture());
                    pbr.normalTexture = pbr_texture(material.normal_texture());
                    pbr.occlusionTexture = pbr_texture(material.occlusion_texture());
                    pbr.emissiveTexture = pbr_texture(material.emissive_texture());
                }
                newEntity.node = n;
                newEntity.color = sg::base_color(g, primitive);
                newEntity.doubleSided = twoD || (primitive.has_material() && primitive.material() < g.material_count() &&
                    g.material(primitive.material()).double_sided());
                for (auto const& v : localMesh.vertices)
                {
                    auto const wv = *world[n] * v.as<scalar>();
                    if (!iBounds)
                        iBounds.emplace(wv, wv);
                    else
                        iBounds = std::make_pair(iBounds->first.min(wv), iBounds->second.max(wv));
                }
                // vertices are cached in model space and transformed (and skinned) on the GPU
                model_transformation modelTransformation{ world[n]->as<float>() };
                // the primitive's own normals for lighting (if it has none they are calculated from its faces when drawn)
                if (primitive.attributes().has_attribute(sg::vertex_attribute::NORMAL))
                {
                    try
                    {
                        std::uint32_t components = 0u;
                        auto const normals = sg::read_accessor(g, primitive.attributes().attribute(sg::vertex_attribute::NORMAL), components);
                        if (components == 3u && normals.size() == localMesh.vertices.size() * 3u)
                        {
                            modelTransformation.vertexNormals.reserve(localMesh.vertices.size());
                            for (std::size_t v = 0u; v < localMesh.vertices.size(); ++v)
                                modelTransformation.vertexNormals.push_back(vec3f{ 
                                    static_cast<float>(normals[v * 3u]), static_cast<float>(normals[v * 3u + 1u]), static_cast<float>(normals[v * 3u + 2u]) });
                        }
                    }
                    catch (std::exception const&)
                    {
                        modelTransformation.vertexNormals.clear();
                    }
                }
                // emissive primitives lit per vertex: unlit (zero normals) with the emission added to the base colour (physically
                // based shading adds the emission itself)
                if (iLightingModel != scene_lighting::PhysicallyBased && pbr.emissive != vec3{})
                {
                    auto const base = newEntity.color.to_linear();
                    newEntity.color = neogfx::color::from_linear(linear_color{ vec4{
                        std::min(base.red<scalar>() + pbr.emissive.x, 1.0),
                        std::min(base.green<scalar>() + pbr.emissive.y, 1.0),
                        std::min(base.blue<scalar>() + pbr.emissive.z, 1.0), 1.0 } }).with_alpha(newEntity.color.alpha());
                    modelTransformation.vertexNormals.assign(localMesh.vertices.size(), vec3f{});
                }
                if (g.node(n).has_skin() && g.node(n).skin() < g.skin_count())
                {
                    try
                    {
                        if (sg::skin_weights(g, primitive, modelTransformation.vertexJoints, modelTransformation.vertexWeights) &&
                            modelTransformation.vertexJoints.size() == localMesh.vertices.size())
                        {
                            newEntity.skin = g.node(n).skin();
                        }
                        else
                        {
                            modelTransformation.vertexJoints.clear();
                            modelTransformation.vertexWeights.clear();
                        }
                    }
                    catch (std::exception const&)
                    {
                        modelTransformation.vertexJoints.clear();
                        modelTransformation.vertexWeights.clear();
                    }
                }
                newEntity.entity = ecs().create_entity(scene_graph_primitive_archetype(ecs()),
                    mesh_filter{ {}, std::move(localMesh), {} },
                    mesh_renderer{ material{ to_ecs_component(newEntity.color), {}, {}, texture, {}, {}, false, pbr } },
                    std::move(modelTransformation));
                iEntities.push_back(std::move(newEntity));
            }
        }
    }

    void scene_graph_canvas::update_entities()
    {
        if (!has_graph())
        {
            if (!iEntities.empty())
                destroy_entities();
            set_entity_transformation({});
            return;
        }
        if (iBuiltRevision != graph().revision())
            build();
        auto const scene = scene_index();
        if (scene == sg::invalid_index || iEntities.empty())
        {
            set_entity_transformation({});
            return;
        }

        auto const& g = graph();
        bool const twoD = (g.dimension() == sg::dimension::Two);
        auto const world = sg::world_transformations(g, scene);
        auto const extents = client_rect().extents();
        scalar const width = std::max(extents.cx, 1.0);
        scalar const height = std::max(extents.cy, 1.0);
        auto const cameraNode = find_camera_node(world);

        // entity vertices are cached in model space: the model (or skin joint) matrices, view, projection and
        // lighting are applied on the GPU so the vertices don't change as the scene animates or the camera moves
        {
            scoped_component_data_lock<mesh_renderer, mesh_render_cache, model_transformation> lock{ ecs() };
            auto& renderers = ecs().component<mesh_renderer>();
            auto& cache = ecs().component<mesh_render_cache>();
            auto& models = ecs().component<model_transformation>();
            vec3 const light = scene_light();
            bool const lit = iLighting && !twoD && iLightingModel != scene_lighting::None;
            service<i_rendering_engine>().default_shader_program().standard_vertex_shader().set_scene_light(
                lit ? std::optional<vec3>{ light } : std::nullopt, lit ? iLightingModel : scene_lighting::None);
            // n.b. the camera position is set when the camera is known (below)
            if (!lit || iLightingModel != scene_lighting::PhysicallyBased)
                service<i_rendering_engine>().default_shader_program().pbr_shader().set_pbr_light(std::nullopt, vec3{});
            // the point lights: at their nodes' (world) positions
            thread_local std::vector<scene_point_light> tPointLights;
            tPointLights.clear();
            if (lit)
                for (auto const& pointLight : iPointLights)
                    if (pointLight.node < world.size() && world[pointLight.node])
                    {
                        auto const& nodeWorld = *world[pointLight.node];
                        tPointLights.push_back(scene_point_light{ vec3{ nodeWorld[3][0], nodeWorld[3][1], nodeWorld[3][2] },
                            pointLight.radiance, pointLight.range, pointLight.castsShadows, pointLight.size });
                    }
            service<i_rendering_engine>().default_shader_program().standard_vertex_shader().set_scene_point_lights(tPointLights);
            // shadows: the directional light's covers the scene's bounds (as built, with room for animation)
            std::optional<std::pair<vec3, scalar>> shadowBounds;
            if (lit && iShadows && iLightingModel == scene_lighting::PhysicallyBased && iBounds)
                shadowBounds.emplace((iBounds->first + iBounds->second) / 2.0, (iBounds->second - iBounds->first).magnitude() / 2.0 * 1.1);
            service<i_rendering_engine>().default_shader_program().pbr_shader().set_pbr_shadows(shadowBounds);
            thread_local std::map<sg::index, std::vector<mat44f>> tJointMatrices;
            tJointMatrices.clear();
            for (auto& e : iEntities)
            {
                auto& renderer = renderers.entity_record_no_lock(e.entity);
                bool const visible = world[e.node].has_value();
                if (renderer.render != visible)
                {
                    renderer.render = visible;
                    set_render_cache_dirty_no_lock(cache, e.entity);
                }
                if (!visible)
                    continue;
                auto& model = models.entity_record_no_lock(e.entity);
                auto const transformation = world[e.node]->as<float>();
                if (e.skin != sg::invalid_index)
                {
                    // glTF: a skinned mesh's own node transformation is ignored
                    auto existing = tJointMatrices.find(e.skin);
                    if (existing == tJointMatrices.end())
                    {
                        existing = tJointMatrices.emplace(e.skin, std::vector<mat44f>{}).first;
                        sg::joint_matrices(g, e.skin, world, existing->second);
                    }
                    model.matrix = mat44f::identity();
                    model.joints = existing->second;
                }
                else
                    model.matrix = transformation;
            }
        }

        if (twoD)
        {
            // view: scene origin (or the fitted bounds centre) at the canvas centre, y up
            mat44 view = mat44::identity();
            scalar scale = iViewScale.value_or(1.0);
            if (cameraNode != sg::invalid_index && g.camera(g.node(cameraNode).camera()).type() == sg::camera_type::Orthographic)
            {
                auto const& camera = g.camera(g.node(cameraNode).camera()).orthographic();
                if (!iViewScale && camera.ymag() > 0.0)
                    scale = (height / 2.0) / camera.ymag();
                view = inverse(*world[cameraNode]);
            }
            else if (!iViewScale && iBounds)
            {
                auto const sceneExtents = iBounds->second - iBounds->first;
                scale = 0.9 * std::min(width / std::max(sceneExtents.x, 1e-9), height / std::max(sceneExtents.y, 1e-9));
                view = translation_matrix(-vec3{ (iBounds->first.x + iBounds->second.x) / 2.0, (iBounds->first.y + iBounds->second.y) / 2.0, 0.0 });
            }
            iLastScale2D = scale * iZoom2D;
            view = translation_matrix(vec3{ width / 2.0, height / 2.0, 0.0 }) * scaling_matrix(vec3{ iLastScale2D, iLastScale2D, 1.0 }) * 
                translation_matrix(vec3{ iPan2D.x, iPan2D.y, 0.0 }) * view;
            // n.b. only x and y are viewed: z (drawing order) passes through unchanged
            view[0][2] = 0.0;
            view[1][2] = 0.0;
            view[2][2] = 1.0;
            view[3][2] = 0.0;
            cull_faces(face_culling::None);
            set_entity_transformation(view);
            return;
        }

        // 3D
        mat44 cameraWorld;
        mat44 projection;
        scalar const aspectRatio = width / height;
        vec3 centre;
        scalar radius = 1.0;
        if (iBounds)
        {
            centre = (iBounds->first + iBounds->second) / 2.0;
            radius = std::max((iBounds->second - iBounds->first).magnitude() / 2.0, 1e-3);
        }
        if (cameraNode != sg::invalid_index)
        {
            cameraWorld = *world[cameraNode];
            auto const& camera = g.camera(g.node(cameraNode).camera());
            if (camera.type() == sg::camera_type::Perspective)
            {
                iLastYfov = camera.perspective().yfov();
                projection = perspective_matrix(aspectRatio, camera.perspective().yfov(), camera.perspective().znear(), 
                    camera.perspective().zfar().has_value() ? std::optional<scalar>{ camera.perspective().zfar().value() } : std::nullopt);
            }
            else
            {
                iLastYfov = to_rad(45.0);
                projection = orthographic_matrix(camera.orthographic().ymag() * aspectRatio, camera.orthographic().ymag(), 
                    camera.orthographic().znear(), camera.orthographic().zfar());
            }
        }
        else
        {
            // default camera framing the scene's bounds (as built)
            iLastYfov = to_rad(45.0);
            auto const eye = centre + vec3{ 0.5, 0.6, 1.0 }.normalized() * (radius / std::sin(iLastYfov / 2.0));
            cameraWorld = sg::i_node::to_matrix(sg::i_node::trs_transform{ eye, sg::scene_graph_3d::look_at(eye, centre) });
            projection = perspective_matrix(aspectRatio, iLastYfov, radius * 0.05, radius * 10.0);
        }
        if (iOrbit)
        {
            // mouse controlled camera: the projection is kept (but for the default camera's clipping planes)
            cameraWorld = orbit_camera_world(*iOrbit);
            if (cameraNode == sg::invalid_index)
                projection = perspective_matrix(aspectRatio, iLastYfov, std::max(iOrbit->distance * 0.01, 1e-6), 
                    iOrbit->distance + (iOrbit->target - centre).magnitude() + radius * 4.0);
        }
        iLastCameraWorld = cameraWorld;
        if (iLighting && iLightingModel == scene_lighting::PhysicallyBased)
            service<i_rendering_engine>().default_shader_program().pbr_shader().set_pbr_light(
                scene_light(), vec3{ cameraWorld[3][0], cameraWorld[3][1], cameraWorld[3][2] });
        // NDC to canvas coordinates (y up, nearer is greater z): affine, so it composes with the projection
        // ahead of the GPU's perspective divide
        scalar const depthScale = 0.45 * std::max(width, height);
        mat44 viewport = mat44::identity();
        viewport[0][0] = width / 2.0;
        viewport[1][1] = height / 2.0;
        viewport[2][2] = -depthScale;
        viewport[3][0] = width / 2.0;
        viewport[3][1] = height / 2.0;
        bool doubleSided = false;
        for (auto const& e : iEntities)
            doubleSided = doubleSided || e.doubleSided;
        cull_faces(doubleSided ? face_culling::None : face_culling::Back);
        set_entity_transformation(viewport * projection * inverse(cameraWorld));
    }

    bool scene_graph_canvas::mouse_camera_control() const
    {
        return iMouseCameraControl;
    }

    void scene_graph_canvas::set_mouse_camera_control(bool aEnable)
    {
        iMouseCameraControl = aEnable;
        iDragButton = std::nullopt;
    }

    void scene_graph_canvas::reset_view()
    {
        iOrbit = std::nullopt;
        iPan2D = vec2{};
        iZoom2D = 1.0;
        update();
    }

    std::optional<game::texture> scene_graph_canvas::load_texture(sg::index aImage, texture_sampling aSampling)
    {
        auto const key = std::make_pair(aImage, aSampling);
        auto existing = iTextures.find(key);
        if (existing != iTextures.end())
            return existing->second;
        auto& result = iTextures[key];
        try
        {
            std::vector<std::byte> data;
            if (!sg::image_data(graph(), aImage, data))
                return result;
            // the encoded image is registered as a resource keyed by its content so that neogfx::image can decode it
            auto const hash = std::hash<std::string_view>{}(std::string_view{ reinterpret_cast<char const*>(data.data()), data.size() });
            std::string const uri = ":/neogfx/scene_graph/image/" + std::to_string(hash) + "_" + std::to_string(data.size());
            service<i_resource_manager>().add_resource(uri, data.data(), data.size());
            neogfx::image image{ string{ uri }, 1.0, aSampling };
            if (!image.error() && !image.is_empty() && image.extents().cx > 0.0 && image.extents().cy > 0.0)
                result = to_ecs_component(image);
        }
        catch (std::exception const&)
        {
            // unsupported image format (only PNG and JPEG are decoded): untextured
        }
        return result;
    }

    mat44 scene_graph_canvas::orbit_camera_world(orbit_camera const& aOrbit)
    {
        auto const eye = aOrbit.target + vec3{ 
            std::cos(aOrbit.pitch) * std::sin(aOrbit.yaw), 
            std::sin(aOrbit.pitch), 
            std::cos(aOrbit.pitch) * std::cos(aOrbit.yaw) } * aOrbit.distance;
        return sg::i_node::to_matrix(sg::i_node::trs_transform{ eye, sg::scene_graph_3d::look_at(eye, aOrbit.target) });
    }

    void scene_graph_canvas::begin_orbit()
    {
        if (iOrbit)
            return;
        // seed from the camera last drawn with: orbit about the point it looks at nearest the scene's centre
        vec3 const eye{ iLastCameraWorld[3][0], iLastCameraWorld[3][1], iLastCameraWorld[3][2] };
        vec3 forward{ -iLastCameraWorld[2][0], -iLastCameraWorld[2][1], -iLastCameraWorld[2][2] };
        forward = forward.magnitude() > 0.0 ? forward.normalized() : vec3{ 0.0, 0.0, -1.0 };
        vec3 centre;
        scalar radius = 1.0;
        if (iBounds)
        {
            centre = (iBounds->first + iBounds->second) / 2.0;
            radius = std::max((iBounds->second - iBounds->first).magnitude() / 2.0, 1e-3);
        }
        auto distance = (centre - eye).dot(forward);
        if (distance <= radius * 0.1)
            distance = radius * 2.0;
        orbit_camera orbit;
        orbit.distance = distance;
        orbit.target = eye + forward * distance;
        auto const offset = (eye - orbit.target) / distance;
        orbit.pitch = std::asin(std::clamp(offset.y, -1.0, 1.0));
        orbit.yaw = std::atan2(offset.x, offset.z);
        iOrbit = orbit;
    }

    bool scene_graph_canvas::mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifier aKeyModifier)
    {
        if (!iMouseCameraControl || !has_graph() || aWheel != mouse_wheel::Vertical || aDelta.dy == 0.0)
            return canvas::mouse_wheel_scrolled(aWheel, aPosition, aDelta, aKeyModifier);
        if (graph().dimension() == sg::dimension::Two)
            iZoom2D = std::clamp(iZoom2D * std::pow(1.15, aDelta.dy), 1e-4, 1e4);
        else
        {
            begin_orbit();
            iOrbit->distance = std::clamp(iOrbit->distance * std::pow(0.85, aDelta.dy), 1e-6, 1e9);
        }
        update();
        return true;
    }

    void scene_graph_canvas::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        if (aButton != mouse_button::Middle)
            canvas::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        if (!iMouseCameraControl || !has_graph() || iDragButton)
            return;
        if (aButton == mouse_button::Left || aButton == mouse_button::Right || aButton == mouse_button::Middle)
        {
            iDragButton = aButton;
            iDragPan = aButton != mouse_button::Left || (aKeyModifier & key_modifier::SHIFT) != key_modifier::None;
            iLastMousePosition = aPosition;
            if (!capturing())
                set_capture(capture_reason::MouseEvent, aPosition);
        }
    }

    void scene_graph_canvas::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        canvas::mouse_button_double_clicked(aButton, aPosition, aKeyModifier);
        if (iMouseCameraControl && aButton == mouse_button::Left)
            reset_view();
    }

    void scene_graph_canvas::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        if (iDragButton == aButton)
        {
            iDragButton = std::nullopt;
            if (capturing())
                release_capture(capture_reason::MouseEvent);
        }
        if (aButton != mouse_button::Middle)
            canvas::mouse_button_released(aButton, aPosition);
    }

    void scene_graph_canvas::mouse_moved(const point& aPosition, key_modifier aKeyModifier)
    {
        canvas::mouse_moved(aPosition, aKeyModifier);
        if (!iDragButton || !iMouseCameraControl || !has_graph())
            return;
        // n.b. mouse positions are y down
        auto const dx = aPosition.x - iLastMousePosition.x;
        auto const dy = aPosition.y - iLastMousePosition.y;
        iLastMousePosition = aPosition;
        if (dx == 0.0 && dy == 0.0)
            return;
        if (graph().dimension() == sg::dimension::Two)
        {
            iPan2D.x += dx / iLastScale2D;
            iPan2D.y -= dy / iLastScale2D;
        }
        else
        {
            begin_orbit();
            if (iDragPan)
            {
                auto const world = orbit_camera_world(*iOrbit);
                vec3 const right{ world[0][0], world[0][1], world[0][2] };
                vec3 const up{ world[1][0], world[1][1], world[1][2] };
                auto const unitsPerPixel = 2.0 * iOrbit->distance * std::tan(iLastYfov / 2.0) / std::max(client_rect().extents().cy, 1.0);
                iOrbit->target = iOrbit->target - right * (dx * unitsPerPixel) + up * (dy * unitsPerPixel);
            }
            else
            {
                iOrbit->yaw -= dx * 0.01;
                iOrbit->pitch = std::clamp(iOrbit->pitch + dy * 0.01, -1.55, 1.55);
            }
        }
        update();
    }
}
