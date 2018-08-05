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

namespace neogfx
{
	ecs::context_data::context_data(ecs& aOwner, context_id aId) :
		iOwner{ aOwner }, iId{ aId }, iReferenceCount{}, iNextEntityId{}
	{
	}

	ecs::context_id ecs::context_data::id() const
	{
		return iId;
	}

	const ecs::context_data::archetype_registry_t& ecs::context_data::archetypes() const
	{
		return iArchetypeRegistry;
	}

	ecs::context_data::archetype_registry_t& ecs::context_data::archetypes()
	{
		return iArchetypeRegistry;
	}

	const ecs::context_data::component_factories_t& ecs::context_data::component_factories() const
	{
		return iComponentFactories;
	}

	ecs::context_data::component_factories_t& ecs::context_data::component_factories()
	{
		return iComponentFactories;
	}

	const ecs::context_data::components_t& ecs::context_data::components() const
	{
		return iComponents;
	}

	ecs::context_data::components_t& ecs::context_data::components()
	{
		return iComponents;
	}

	const ecs::context_data::systems_t& ecs::context_data::systems() const
	{
		return iSystems;
	}

	ecs::context_data::systems_t& ecs::context_data::systems()
	{
		return iSystems;
	}

	const entity_archetype& ecs::context_data::archetype(entity_archetype_id aArchetypeId) const
	{
		auto existingArchetype = archetypes().find(aArchetypeId);
		if (existingArchetype != archetypes().end())
			return existingArchetype->second;
		throw entity_archetype_not_found();
	}

	entity_archetype& ecs::context_data::archetype(entity_archetype_id aArchetypeId)
	{
		return const_cast<entity_archetype&>(const_cast<const context_data*>(this)->archetype(aArchetypeId));
	}

	bool ecs::context_data::component_instantiated(component_id aComponentId) const
	{
		return components().find(aComponentId) != components().end();
	}

	const i_component& ecs::context_data::component(component_id aComponentId) const
	{
		auto existingComponent = components().find(aComponentId);
		if (existingComponent != components().end())
			return *existingComponent->second;
		auto existingFactory = component_factories().find(aComponentId);
		if (existingFactory != component_factories().end())
			return *iComponents.emplace(aComponentId, existingFactory->second()).first->second;
		throw component_not_found();
	}

	i_component& ecs::context_data::component(component_id aComponentId)
	{
		return const_cast<i_component&>(const_cast<const context_data*>(this)->component(aComponentId));
	}

	entity_id ecs::context_data::next_entity_id()
	{
		if (++iNextEntityId == 0ull)
			throw entity_ids_exhuasted(); // todo: handle this unlikely event gracefully
		return iNextEntityId;
	}

	void ecs::context_data::add_ref()
	{
		++iReferenceCount;
	}

	void ecs::context_data::release()
	{
		if (--iReferenceCount <= 0)
			iOwner.destroy_context(id());
	}

	ecs::context::context(context_data& aOwner) :
		iOwner{ aOwner }
	{
		add_ref();
	}

	ecs::context::context(const context& aOther) :
		iOwner(aOther.iOwner)
	{
		add_ref();
	}

	ecs::context::context(context&& aOther) :
		iOwner(aOther.iOwner)
	{
		add_ref();
	}

	ecs::context::~context()
	{
		release();
	}

	ecs::context_data& ecs::context::owner() const
	{
		return iOwner;
	}

	void ecs::context::add_ref()
	{
		iOwner.add_ref();
	}

	void ecs::context::release()
	{
		iOwner.release();
	}

	ecs::ecs() :
		iNextContextId{}
	{
	}

	ecs& ecs::instance()
	{
		static ecs sInstance;
		return sInstance;
	}

	ecs::context ecs::create_context()
	{
		++iNextContextId;
		return iContexts.emplace(iNextContextId, context_data{*this, iNextContextId}).first->second;
	}

	void ecs::destroy_context(context_id aContextId)
	{
		auto existingContext = iContexts.find(aContextId);
		if (existingContext == iContexts.end())
			throw invalid_context();
	}

	entity_id ecs::create_entity(const context& aContext, const entity_archetype_id& aArchetypeId)
	{
		auto newEntity = aContext.owner().next_entity_id();
		auto& archetype = aContext.owner().archetype(aArchetypeId);
		for (auto& cid : archetype.components())
			aContext.owner().component(cid);
		return newEntity;
	}
	
	void ecs::destroy_entity(const context& aContext, entity_id aEntityId)
	{
		if (component_instantiated<entity>(aContext))
			component<entity>(aContext).entity_record(aEntityId).destroyed = true;
	}

	bool ecs::component_instantiated(const context& aContext, component_id aComponentId) const
	{
		return aContext.owner().component_instantiated(aComponentId);
	}

	const i_component& ecs::component(const context& aContext, component_id aComponentId) const
	{
		return aContext.owner().component(aComponentId);
	}

	i_component& ecs::component(const context& aContext, component_id aComponentId)
	{
		return aContext.owner().component(aComponentId);
	}

	void ecs::register_archetype(const context& aContext, const entity_archetype& aArchetype)
	{
		aContext.owner().archetypes().emplace(aArchetype.id(), aArchetype);
	}

	void ecs::register_archetype(const context& aContext, entity_archetype&& aArchetype)
	{
		aContext.owner().archetypes().emplace(aArchetype.id(), std::move(aArchetype));
	}

	void ecs::register_component(const context& aContext, component_id aComponentId, component_factory aFactory)
	{
		aContext.owner().component_factories().emplace(aComponentId, aFactory);
	}
}