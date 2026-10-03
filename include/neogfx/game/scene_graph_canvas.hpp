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
    //     y up); z translation is drawing order.
    // 3D: drawn through the scene's camera (or a default camera framing the scene), depth tested
    //     and back face culled (unless any material is double sided); lit (per vertex, on the GPU, by their
    //     normals) by a fixed directional light.
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
        // re-create the entities (done automatically when the graph's revision changes)
        void rebuild();
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
        neogfx::scene_graph::index iScene;
        neogfx::scene_graph::index iCameraNode;
        std::optional<scalar> iViewScale;
        bool iLighting;
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
