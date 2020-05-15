// entity_archetype.cpp
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
#include <neogfx/game/entity_archetype.hpp>
#include <neogfx/game/i_ecs.hpp>

namespace neogfx::game
{
    entity_archetype::entity_archetype(const entity_archetype_id& aId, const std::string& aName, std::initializer_list<component_id> aComponents) :
        iId{ aId }, iName{ aName }, iComponents{ aComponents }
    {
    }
    
    entity_archetype::entity_archetype(const std::string& aName, std::initializer_list<component_id> aComponents) :
        iId{ neolib::generate_uuid() }, iName{ aName }, iComponents{ aComponents }
    {
    }
    
    entity_archetype::entity_archetype(const entity_archetype& aOther) :
        iId{ aOther.iId }, iName{ aOther.iName }, iComponents{ aOther.iComponents }
    {
    }

    entity_archetype::entity_archetype(entity_archetype&& aOther) :
        iId{ aOther.iId }, iName{ std::move(aOther.iName) }, iComponents{ std::move(aOther.iComponents) }
    {
    }

    const entity_archetype_id& entity_archetype::id() const
    {
        return iId;
    }

    const i_string& entity_archetype::name() const
    {
        return iName;
    }

    const neolib::i_set<component_id>& entity_archetype::components() const
    {
        return iComponents;
    }

    neolib::i_set<component_id>& entity_archetype::components()
    {
        return iComponents;
    }

    void entity_archetype::populate_default_components(i_ecs&, entity_id)
    {
        // nothing to do.
    }
}