// renderable_entity_archetype.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/game/renderable_entity_archetype.hpp>
#include <neogfx/game/i_ecs.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_filter.hpp>

namespace neogfx::game
{
    void renderable_entity_archetype::populate_default_components(i_ecs& aEcs, entity_id aEntity)
    {
        entity_archetype::populate_default_components(aEcs, aEntity);
        for (auto const& c : components())
        {
            if (aEcs.component_registered(c) && aEcs.component(c).has_entity_record(aEntity))
                continue;
            if (c == mesh_renderer::meta::id() && aEcs.component<material>().has_entity_record(aEntity))
                aEcs.populate(aEntity, mesh_renderer{ aEcs.component<material>().entity_record(aEntity) });
            else if (c == mesh_filter::meta::id() && aEcs.component<mesh>().has_entity_record(aEntity))
                aEcs.populate(aEntity, mesh_filter{ {}, aEcs.component<mesh>().entity_record(aEntity) });
        }
    }
}