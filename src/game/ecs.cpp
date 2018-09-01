// ecs.cpp
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
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/entity.hpp>

namespace neogfx::game
{
	const ecs::archetype_registry_t& ecs::archetypes() const
	{
		return iArchetypeRegistry;
	}

	ecs::archetype_registry_t& ecs::archetypes()
	{
		return iArchetypeRegistry;
	}

	const ecs::component_factories_t& ecs::component_factories() const
	{
		return iComponentFactories;
	}

	ecs::component_factories_t& ecs::component_factories()
	{
		return iComponentFactories;
	}

	const ecs::components_t& ecs::components() const
	{
		return iComponents;
	}

	ecs::components_t& ecs::components()
	{
		return iComponents;
	}

	const ecs::shared_component_factories_t& ecs::shared_component_factories() const
	{
		return iSharedComponentFactories;
	}

	ecs::shared_component_factories_t& ecs::shared_component_factories()
	{
		return iSharedComponentFactories;
	}

	const ecs::shared_components_t& ecs::shared_components() const
	{
		return iSharedComponents;
	}

	ecs::shared_components_t& ecs::shared_components()
	{
		return iSharedComponents;
	}

	const ecs::system_factories_t& ecs::system_factories() const
	{
		return iSystemFactories;
	}

	ecs::system_factories_t& ecs::system_factories()
	{
		return iSystemFactories;
	}

	const ecs::systems_t& ecs::systems() const
	{
		return iSystems;
	}

	ecs::systems_t& ecs::systems()
	{
		return iSystems;
	}

	const entity_archetype& ecs::archetype(entity_archetype_id aArchetypeId) const
	{
		auto existingArchetype = archetypes().find(aArchetypeId);
		if (existingArchetype != archetypes().end())
			return existingArchetype->second;
		throw entity_archetype_not_found();
	}

	entity_archetype& ecs::archetype(entity_archetype_id aArchetypeId)
	{
		return const_cast<entity_archetype&>(const_cast<const ecs*>(this)->archetype(aArchetypeId));
	}

	bool ecs::component_instantiated(component_id aComponentId) const
	{
		return components().find(aComponentId) != components().end();
	}

	const i_component& ecs::component(component_id aComponentId) const
	{
		auto existingComponent = components().find(aComponentId);
		if (existingComponent != components().end())
			return *existingComponent->second;
		auto existingFactory = component_factories().find(aComponentId);
		if (existingFactory != component_factories().end())
			return *iComponents.emplace(aComponentId, existingFactory->second()).first->second;
		throw component_not_found();
	}

	i_component& ecs::component(component_id aComponentId)
	{
		return const_cast<i_component&>(const_cast<const ecs*>(this)->component(aComponentId));
	}

	bool ecs::shared_component_instantiated(component_id aComponentId) const
	{
		return shared_components().find(aComponentId) != shared_components().end();
	}

	const i_shared_component& ecs::shared_component(component_id aComponentId) const
	{
		auto existingComponent = shared_components().find(aComponentId);
		if (existingComponent != shared_components().end())
			return *existingComponent->second;
		auto existingFactory = shared_component_factories().find(aComponentId);
		if (existingFactory != shared_component_factories().end())
			return *iSharedComponents.emplace(aComponentId, existingFactory->second()).first->second;
		throw component_not_found();
	}

	i_shared_component& ecs::shared_component(component_id aComponentId)
	{
		return const_cast<i_shared_component&>(const_cast<const ecs*>(this)->shared_component(aComponentId));
	}

	bool ecs::system_instantiated(system_id aSystemId) const
	{
		return systems().find(aSystemId) != systems().end();
	}

	const i_system& ecs::system(system_id aSystemId) const
	{
		auto existingSystem = systems().find(aSystemId);
		if (existingSystem != systems().end())
			return *existingSystem->second;
		auto existingFactory = system_factories().find(aSystemId);
		if (existingFactory != system_factories().end())
			return *iSystems.emplace(aSystemId, existingFactory->second()).first->second;
		throw system_not_found();
	}

	i_system& ecs::system(system_id aSystemId)
	{
		return const_cast<i_system&>(const_cast<const ecs*>(this)->system(aSystemId));
	}

	entity_id ecs::next_entity_id()
	{
		if (!iFreedEntityIds.empty())
		{
			auto nextId = iFreedEntityIds.back();
			iFreedEntityIds.pop_back();
			return nextId;
		}
		if (++iNextEntityId == 0ull)
			throw entity_ids_exhuasted();
		return iNextEntityId;
	}

	void ecs::free_entity_id(entity_id aId)
	{
		iFreedEntityIds.push_back(aId);
	}

	ecs::ecs() :
		iNextEntityId{}
	{
	}

	entity_id ecs::create_entity(const entity_archetype_id& aArchetypeId)
	{
		auto newEntity = next_entity_id();
		for (auto& cid : archetype(aArchetypeId).components())
			component(cid);
		return newEntity;
	}
	
	void ecs::destroy_entity(entity_id aEntityId)
	{
		if (component_instantiated<entity>())
			component<entity>().entity_record(aEntityId).destroyed = true;
		// todo: mark entity ID as invalid in all component data
		free_entity_id(aEntityId);
	}

	bool ecs::archetype_registered(const entity_archetype& aArchetype) const
	{
		return archetypes().find(aArchetype.id()) != archetypes().end();
	}

	void ecs::register_archetype(const entity_archetype& aArchetype)
	{
		archetypes().emplace(aArchetype.id(), aArchetype);
	}

	void ecs::register_archetype(entity_archetype&& aArchetype)
	{
		archetypes().emplace(aArchetype.id(), std::move(aArchetype));
	}

	bool ecs::component_registered(component_id aComponentId) const
	{
		return component_factories().find(aComponentId) != component_factories().end();
	}

	void ecs::register_component(component_id aComponentId, component_factory aFactory)
	{
		component_factories().emplace(aComponentId, aFactory);
	}

	bool ecs::shared_component_registered(component_id aComponentId) const
	{
		return shared_component_factories().find(aComponentId) != shared_component_factories().end();
	}

	void ecs::register_shared_component(component_id aComponentId, shared_component_factory aFactory)
	{
		shared_component_factories().emplace(aComponentId, aFactory);
	}

	bool ecs::system_registered(system_id aSystemId) const
	{
		return system_factories().find(aSystemId) != system_factories().end();
	}

	void ecs::register_system(system_id aSystemId, system_factory aFactory)
	{
		system_factories().emplace(aSystemId, aFactory);
	}
}