// entity.hpp
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
#include <neogfx/core/event.hpp>
#include <neogfx/game/i_ecs.hpp>

namespace neogfx::game
{
    class entity
    {
    public:
        entity(i_ecs& aEcs, entity_id aId);
        entity(i_ecs& aEcs, const entity_archetype_id& aArchetypeId);
        template <typename... ComponentData>
        entity(i_ecs& aEcs, const entity_archetype_id& aArchetypeId, ComponentData&&... aComponentData) :
            entity{ aEcs, aEcs.create_entity(aArchetypeId, aComponentData...) } {}
        ~entity();
    public:
        entity(const entity& aOther) = delete;
        entity& operator=(const entity& aOther) = delete;
    public:
        i_ecs& ecs() const;
        entity_id id() const;
        bool detached_or_destroyed() const;
        entity_id detach();
    private:
        i_ecs& iEcs;
        entity_id iId;
        sink iSink;
    };
}