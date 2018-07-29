// ecs.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <set>
#include <map>
#include <neolib/allocator.hpp>
#include <neolib/uuid.hpp>
#include <neogfx/game/ecs_ids.hpp>

namespace neogfx
{
	class entity_archetype
	{
	public:
		typedef std::set<component_id, std::less<component_id>, neolib::fast_pool_allocator<component_id>> component_list;
	public:
		template <ComponentIdIter>
		entity_archetype(const entity_archetype_id& aId, ComponentIdIter aFirstComponent, ComponentIdIter aLastComponent) :
			iId{ aId }, iComponents{ aFirstComponent, aLastComponent }
		{
		}
		entity_archetype(const entity_archetype& aOther) :
			iId{ aOther.iId }, iComponents{ aOther.iComponents }
		{
		}
		entity_archetype(entity_archetype&& aOther) :
			iId{ aOther.iId }, iComponents{ std::move(aOther.iComponents) }
		{
		}
	public:
		const entity_archetype_id& id() const
		{
			return iId;
		}
		const component_list& components() const
		{
			return iComponents;
		}
		component_list& components()
		{
			return iComponents;
		}
	private:
		entity_archetype_id iId;
		component_list iComponents;
	};

	class ecs
	{
	private:
		typedef std::map<entity_archetype_id, entity_archetype, std::less<entity_archetype_id>, neolib::fast_pool_allocator<std::pair<const entity_archetype_id, entity_archetype>>> archetype_registry_t;
	public:
		ecs()
		{
		}
	public:
		entity_id create_entity(const entity_archetype_id& aArchetypeId)
		{
			// todo
		}
		void destroy_entity(entity_id aEntityId)
		{
			// todo
		}
	public:
		void register_archetype(const entity_archetype& aArchetype)
		{
			iArchetypeRegistry.emplace(aArchetype.id(), aArchetype);
		}
		void register_archetype(entity_archetype&& aArchetype)
		{
			iArchetypeRegistry.emplace(aArchetype.id(), std::move(aArchetype));
		}
	private:
		archetype_registry_t iArchetypeRegistry;
	};
}