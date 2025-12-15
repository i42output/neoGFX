// ecs.cpp
/*
  neolib C++ App/Game Engine
  Copyright (c)  2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/simple_physics.hpp>
#include <neogfx/game/collision_detector.hpp>
#include <neogfx/game/animator.hpp>
#include <neogfx/game/time.hpp>
#include <neogfx/game/mesh_render_cache.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>

namespace neogfx
{
    namespace game
    {
        ecs::ecs(ecs_flags aCreationFlags) : base_type{ aCreationFlags }
        {
            service<i_rendering_engine>().allocate_vertex_buffer(*this, vertex_buffer_type::DefaultECS);
        }

        ecs::~ecs()
        {
            service<i_rendering_engine>().deallocate_vertex_buffer(*this);
        }

        bool ecs::run_threaded(const system_id& aSystemId) const
        {
            if (system_instantiated<simple_physics_2d>() || system_instantiated<simple_physics_3d>())
            {
                if (aSystemId == time::meta::id() || aSystemId == animator::meta::id())
                    return false;
            }
            return base_type::run_threaded(aSystemId);
        }

        void ecs::destroy_entity(entity_id aEntityId, bool aNotify)
        {
            scoped_component_lock<mesh_render_cache> lock{ *this };
            if (component<mesh_render_cache>().has_entity_record(aEntityId) && service<i_rendering_engine>().vertex_buffer_allocated(*this))
            {
                auto const& cacheEntry = component<mesh_render_cache>().entity_record(aEntityId);
                service<i_rendering_engine>().vertex_buffer(*this).reclaim(cacheEntry.meshVertexArrayIndices[0], cacheEntry.meshVertexArrayIndices[1]);
                for (auto& indices : cacheEntry.patchVertexArrayIndices)
                    service<i_rendering_engine>().vertex_buffer(*this).reclaim(indices[0], indices[1]);
            }
            base_type::destroy_entity(aEntityId, aNotify);
        }

        bool ecs::cacheable() const
        {
            return true;
        }

        const game::component<game::mesh_render_cache>& ecs::cache() const
        {
            return component<game::mesh_render_cache>();
        }

        game::component<game::mesh_render_cache>& ecs::cache()
        {
            return component<game::mesh_render_cache>();
        }
    }
}