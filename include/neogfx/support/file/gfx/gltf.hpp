// gltf.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/numerical.hpp>

namespace neogfx::file
{
    class gltf
    {
    public:
        struct asset
        {
            std::string version;
            std::optional<std::string> minVersion;
            std::string generator;
            std::string copyright;
        };
        typedef mat44 matrix_transform;
        typedef struct { vec3 translation; vec4 rotation; vec3 scale; } trs_transform;
        typedef std::variant<matrix_transform, trs_transform> local_transform;
        struct mesh
        {

        };
        struct camera
        {
        };
        struct node;
        typedef std::vector<node*> nodes;
        struct node
        {
            std::optional<std::string> name;
            std::optional<nodes> children;
            std::optional<local_transform> transform;
            mesh const* mesh;
            camera const* camera;
        };
        struct scene
        {
            nodes nodes;
        };
        typedef std::vector<scene> scenes;
    public:
        gltf(std::string const& aUri) {}
    public:
        scenes scenes;
        std::shared_ptr<scene> displayScene;
    };
}