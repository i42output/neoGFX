// i_vertex_provider.hpp
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
#include <neogfx/game/mesh_render_cache.hpp>

namespace neogfx
{
    class i_vertex_provider
    {
    public:
        struct not_cacheable : std::logic_error { not_cacheable() : std::logic_error{ "neogfx::i_vertex_provider::not_cacheable" } {} };
    public:
        virtual ~i_vertex_provider() = default;
    public:
        virtual bool cacheable() const = 0;
        virtual const game::component<game::mesh_render_cache>& cache() const = 0;
        virtual game::component<game::mesh_render_cache>& cache() = 0;
    };
}