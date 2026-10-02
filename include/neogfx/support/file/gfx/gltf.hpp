// gltf.hpp
/*
  neogfx C++ App/Game Engine
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

#include <vector>
#include <variant>
#include <optional>
#include <string>
#include <memory>
#include <iosfwd>
#include <stdexcept>

#include <neogfx/core/numerical.hpp>
#include <neogfx/gfx/scene_graph.hpp>

namespace neogfx::file
{
    // glTF 2.0 file: .gltf (JSON, buffers embedded as base64 data URIs when written) or .glb (binary
    // container, buffer 0 in the BIN chunk). The document is held as a scene graph model.
    // Not supported (dropped on load with a warning): extensions other than
    // NEOGFX_scene_2d and vertex attributes other than those in scene_graph::vertex_attribute.
    class gltf
    {
    public:
        enum class format
        {
            Json,   // .gltf
            Binary  // .glb
        };
        struct error : std::runtime_error { using std::runtime_error::runtime_error; };
        static constexpr char const* extension_2d = "NEOGFX_scene_2d";
    public:
        struct asset
        {
            std::string version;
            std::optional<std::string> minVersion;
            std::string generator;
            std::string copyright;
        };
        typedef scene_graph::i_node::matrix_transform matrix_transform;
        typedef scene_graph::i_node::trs_transform trs_transform;
        typedef std::variant<matrix_transform, trs_transform> local_transform;
        typedef scene_graph::mesh mesh;
        typedef scene_graph::camera camera;
        typedef scene_graph::node node;
        typedef std::vector<node*> nodes;
        typedef scene_graph::scene scene;
        typedef std::vector<scene*> scene_list;
    public:
        // load from a file (format detected from content); relative buffer and image URIs are resolved against its directory
        gltf(std::string const& aUri);
        // load from a stream; relative URIs are resolved against aBaseDirectory
        gltf(std::istream& aInput, std::string const& aBaseDirectory = {});
        // wrap an existing model (e.g. for saving)
        gltf(std::shared_ptr<scene_graph::scene_graph_model> aModel);
    public:
        gltf::asset asset_info() const;
        static local_transform transform(node const& aNode);
        scene_graph::scene_graph_model const& model() const;
        scene_graph::scene_graph_model& model();
        std::shared_ptr<scene_graph::scene_graph_model> const& shared_model() const;
        std::vector<std::string> const& warnings() const;
    public:
        // format chosen from the extension (.glb is binary, anything else JSON)
        void save(std::string const& aUri) const;
        void save(std::ostream& aOutput, format aFormat) const;
        static void write(scene_graph::i_scene_graph const& aGraph, std::string const& aUri);
        static void write(scene_graph::i_scene_graph const& aGraph, std::ostream& aOutput, format aFormat);
        static std::string to_json(scene_graph::i_scene_graph const& aGraph);
    private:
        void read(std::istream& aInput, std::string const& aBaseDirectory);
        void update_scenes();
    public:
        scene_list scenes;
        // aliases the model, so keeps it alive
        std::shared_ptr<scene> displayScene;
    private:
        std::shared_ptr<scene_graph::scene_graph_model> iModel;
        std::vector<std::string> iWarnings;
    };
}
