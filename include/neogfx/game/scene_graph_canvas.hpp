// scene_graph_canvas.hpp
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

#include <chrono>

#include <neogfx/gfx/i_scene_graph.hpp>
#include <neogfx/gfx/scene_graph.hpp>
#include <neogfx/gfx/i_vertex_shader.hpp>
#include <neogfx/gfx/i_fragment_shader.hpp>
#include <neogfx/game/canvas.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/texture.hpp>

namespace neogfx::game
{
    // Renders a glTF compatible scene graph. Each triangle primitive of each mesh node in the
    // displayed scene becomes an ECS entity on the canvas's own ECS; node transformations are
    // re-evaluated whenever the canvas is painted, so animating a scene graph is a matter of
    // changing node transformations and calling update().
    // 2D: drawn with an affine view transformation (scene origin at the centre of the canvas,
    //     y up); z translation is drawing order. Entities are pickable (box colliders): see
    //     canvas::EntityClicked and entity_node. Emissive primitives glow (see set_glow).
    // 3D: drawn through the scene's camera (or a default camera framing the scene), depth tested
    //     and back face culled (unless any material is double sided); lit (on the GPU, by their normals) by a
    //     fixed directional light: per vertex (the default) or physically based (per pixel, glTF metallic-roughness
    //     with metallic-roughness, normal, occlusion and emissive textures, plus image based lighting from the
    //     environment of the PBR shader, i_pbr_shader::set_environment; see set_lighting_model).
    // The view (and 3D projection) is applied on the GPU (canvas::set_entity_transformation) so moving
    // the camera does not change entity vertices: they only change when a node's world transformation does.
    // Base colour textures (PNG or JPEG; from buffer views, data URIs or files) are drawn using TEXCOORD_0/1.
    // Skinned meshes (glTF skins) are skinned on the GPU and glTF animations can be played.
    // Mouse camera control (enabled by default):
    //     3D: left drag orbits, right drag (or shift + left drag) pans, wheel dollies
    //     2D: drag pans, wheel zooms
    //     double click resets to the scene's own camera/view
    // Animation: change node transformations in an Animating handler (it is triggered as each frame is rendered, so
    // animation sampled from a clock there matches the frame shown) and call update() regularly to request frames.
    class scene_graph_canvas : public canvas
    {
    public:
        define_event(Animating, animating)
    public:
        scene_graph_canvas();
        scene_graph_canvas(i_widget& aParent);
        scene_graph_canvas(i_layout& aLayout);
        ~scene_graph_canvas();
    public:
        bool has_graph() const;
        neogfx::scene_graph::i_scene_graph const& graph() const;
        neogfx::scene_graph::i_scene_graph& graph();
        void set_graph(neogfx::scene_graph::i_scene_graph& aGraph);
        void set_graph(std::shared_ptr<neogfx::scene_graph::i_scene_graph> aGraph);
        void clear_graph();
        // re-create the entities (done automatically when the graph's revision changes; when a node's mesh changes, which doesn't
        // change the revision, just that node's entities are updated: they are stable, their components replaced)
        void rebuild();
        // the node of one of the canvas's entities (e.g. from EntityClicked), or invalid_index
        neogfx::scene_graph::index entity_node(entity_id aEntity) const;
        // 2D: the glow of emissive primitives (drawn again, in their emissive colour, blurred and lightening what is drawn): its
        // extent (scene units; 0 for none) and intensity (its opacity: at 1.0 the blurred emission's own, so a diffuse glow)
        scalar glow_extent() const;
        scalar glow_intensity() const;
        void set_glow(scalar aExtent, scalar aIntensity = 1.0);
    public:
        // the scene displayed; invalid_index (the default) means the graph's active scene
        neogfx::scene_graph::index displayed_scene() const;
        void set_displayed_scene(neogfx::scene_graph::index aScene);
        // the camera node used for 3D (and optionally 2D); invalid_index (the default) means the first camera node in the scene
        neogfx::scene_graph::index camera_node() const;
        void set_camera_node(neogfx::scene_graph::index aNode);
        // 2D view scale (logical units per scene unit); std::nullopt (the default) fits the scene to the canvas
        std::optional<scalar> const& view_scale() const;
        void set_view_scale(std::optional<scalar> const& aScale);
        bool lighting() const;
        void set_lighting(bool aLighting);
        // scene_lighting::PerVertex (the default) or scene_lighting::PhysicallyBased (scene_lighting::None is the same as set_lighting(false))
        scene_lighting lighting_model() const;
        void set_lighting_model(scene_lighting aLightingModel);
        // point lights (3D, lit) at nodes' positions, e.g. ones with emissive meshes: radiance (colour times intensity; the
        // fixed directional light's is 2.8) falls off with the square of the distance and, if a range is given, smoothly to
        // zero at that range. Shadows are cast (if enabled, physically based shading only) by up to eight of them; aSize is the
        // radius of what emits the light (e.g. the node's own emissive mesh), which casts no shadow from it.
        void add_point_light(neogfx::scene_graph::index aNode, vec3 const& aRadiance, scalar aRange = 0.0, bool aCastsShadows = true, scalar aSize = 0.0);
        void clear_point_lights();
        // shadows (shadow mapping; physically based shading only) cast by the directional light and the point lights
        bool shadows() const;
        void set_shadows(bool aShadows);
        // the environment (see i_pbr_shader::set_environment) drawn as the background (3D, in place of the background colour):
        // from the prefiltered environment texture or the background texture (see i_pbr_shader::set_background_texture), blurred
        // from 0 (sharp) to 1; std::nullopt (the default) for none
        std::optional<pbr_background_source> const& environment_background() const;
        scalar environment_background_blur() const;
        void set_environment_background(std::optional<pbr_background_source> const& aSource, scalar aBlur = 0.0);
        bool mouse_camera_control() const;
        void set_mouse_camera_control(bool aEnable);
        // discard any mouse camera changes
        void reset_view();
        // glTF animation playback (looped): the graph's node transformations are changed as it plays
        void play_animation(neogfx::scene_graph::index aAnimation);
        void stop_animation();
        std::optional<neogfx::scene_graph::index> playing_animation() const;
        bool animation_paused() const;
        void set_animation_paused(bool aPaused);
    public:
        bool mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifier aKeyModifier) override;
        void mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override;
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override;
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
        void mouse_moved(const point& aPosition, key_modifier aKeyModifier) override;
    private:
        struct orbit_camera
        {
            vec3 target;
            scalar yaw;
            scalar pitch;
            scalar distance;
        };
        struct point_light
        {
            neogfx::scene_graph::index node;
            vec3 radiance;
            scalar range;
            bool castsShadows;
            scalar size;
        };
        struct primitive_entity
        {
            entity_id entity;
            neogfx::scene_graph::index node;
            neogfx::color color;
            bool doubleSided;
            neogfx::scene_graph::index skin = neogfx::scene_graph::invalid_index;
        };
    private:
        void init();
        void destroy_entities();
        void build();
        void build_node(neogfx::scene_graph::index aNode, std::optional<mat44> const& aWorld);
        void update_entities();
        neogfx::scene_graph::index scene_index() const;
        neogfx::scene_graph::index find_camera_node(std::vector<std::optional<mat44>> const& aWorld) const;
        std::optional<game::texture> load_texture(neogfx::scene_graph::index aImage, texture_sampling aSampling);
        static mat44 orbit_camera_world(orbit_camera const& aOrbit);
        void advance_animation();
        void begin_orbit();
    private:
        std::shared_ptr<neogfx::scene_graph::i_scene_graph> iGraph;
        std::optional<std::uint64_t> iBuiltRevision;
        // each node's mesh (invalid_index if none) when its entities were built
        std::vector<neogfx::scene_graph::index> iBuiltMeshes;
        neogfx::scene_graph::index iScene;
        neogfx::scene_graph::index iCameraNode;
        std::optional<scalar> iViewScale;
        bool iLighting;
        scene_lighting iLightingModel;
        std::vector<point_light> iPointLights;
        bool iShadows = false;
        std::optional<pbr_background_source> iEnvironmentBackground;
        scalar iEnvironmentBackgroundBlur = 0.0;
        scalar iGlowExtent = 0.25;
        scalar iGlowIntensity = 1.0;
        // 2D: the entities drawing the glow of emissive primitives, on an ECS of their own (see init)
        std::shared_ptr<game::i_ecs> iGlowEcs;
        std::vector<primitive_entity> iGlowEntities;
        std::vector<primitive_entity> iEntities;
        std::optional<std::pair<vec3, vec3>> iBounds;
        std::map<std::pair<neogfx::scene_graph::index, texture_sampling>, std::optional<game::texture>> iTextures;
        bool iMouseCameraControl;
        std::optional<mouse_button> iDragButton;
        bool iDragPan;
        point iLastMousePosition;
        std::optional<orbit_camera> iOrbit;
        mat44 iLastCameraWorld;
        scalar iLastYfov;
        vec2 iPan2D;
        scalar iZoom2D;
        scalar iLastScale2D;
        std::optional<neogfx::scene_graph::animation_player> iAnimationPlayer;
        std::optional<widget_timer> iAnimationTimer;
        std::chrono::steady_clock::time_point iAnimationClock;
        scalar iAnimationTime = 0.0;
        bool iAnimationPaused = false;
        sink iSink;
    };
}

namespace neogfx
{
    using game::scene_graph_canvas;
}
