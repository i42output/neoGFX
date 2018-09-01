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
#include <neolib/reference_counted.hpp>
#include <neolib/uuid.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/entity_archetype.hpp>
#include <neogfx/game/component.hpp>
#include <neogfx/game/system.hpp>

namespace neogfx::game
{
	class ecs
	{
	public:
		struct entity_archetype_not_found : std::logic_error { entity_archetype_not_found() : std::logic_error("neogfx::ecs::entity_archetype_not_found") {} };
		struct component_not_found : std::logic_error { component_not_found() : std::logic_error("neogfx::ecs::component_not_found") {} };
		struct system_not_found : std::logic_error { system_not_found() : std::logic_error("neogfx::ecs::system_not_found") {} };
		struct invalid_context : std::logic_error { invalid_context() : std::logic_error("neogfx::ecs::invalid_context") {} };
		struct entity_ids_exhuasted : std::runtime_error { entity_ids_exhuasted() : std::runtime_error("neogfx::ecs::entity_ids_exhuasted") {} };
	public:
		typedef std::function<std::unique_ptr<i_component>()> component_factory;
		typedef std::function<std::unique_ptr<i_shared_component>()> shared_component_factory;
		typedef std::function<std::unique_ptr<i_system>()> system_factory;
	private:
		typedef std::map<entity_archetype_id, entity_archetype> archetype_registry_t;
		typedef std::map<component_id, component_factory> component_factories_t;
		typedef std::map<component_id, std::unique_ptr<i_component>> components_t;
		typedef std::map<component_id, shared_component_factory> shared_component_factories_t;
		typedef std::map<component_id, std::unique_ptr<i_shared_component>> shared_components_t;
		typedef std::map<system_id, system_factory> system_factories_t;
		typedef std::map<system_id, std::unique_ptr<i_system>> systems_t;
	public:
		ecs();
	public:
		entity_id create_entity(const entity_archetype_id& aArchetypeId);
		template <typename... ComponentData>
		entity_id create_entity(const entity_archetype_id& aArchetypeId, ComponentData&&... aComponentData)
		{
			auto newEntity = create_entity(aArchetypeId);
			populate(newEntity, std::forward<ComponentData>(aComponentData)...);
			return newEntity;
		}
		void destroy_entity(entity_id aEntityId);
	public:
		const archetype_registry_t& archetypes() const;
		archetype_registry_t& archetypes();
		const component_factories_t& component_factories() const;
		component_factories_t& component_factories();
		const components_t& components() const;
		components_t& components();
		const shared_component_factories_t& shared_component_factories() const;
		shared_component_factories_t& shared_component_factories();
		const shared_components_t& shared_components() const;
		shared_components_t& shared_components();
		const system_factories_t& system_factories() const;
		system_factories_t& system_factories();
		const systems_t& systems() const;
		systems_t& systems();
	public:
		const entity_archetype& archetype(entity_archetype_id aArchetypeId) const;
		entity_archetype& archetype(entity_archetype_id aArchetypeId);
		bool component_instantiated(component_id aComponentId) const;
		const i_component& component(component_id aComponentId) const;
		i_component& component(component_id aComponentId);
		bool shared_component_instantiated(component_id aComponentId) const;
		const i_shared_component& shared_component(component_id aComponentId) const;
		i_shared_component& shared_component(component_id aComponentId);
		bool system_instantiated(system_id aSystemId) const;
		const i_system& system(system_id aSystemId) const;
		i_system& system(system_id aSystemId);
	public:
		entity_id next_entity_id();
		void free_entity_id(entity_id aId);
	public:
		template <typename... ComponentData, typename ComponentDataTail>
		void populate(entity_id aEntity, ComponentData&&... aComponentData, ComponentDataTail&& aComponentDataTail)
		{
			populate(aEntity, std::forward<ComponentDataTail>(aComponentDataTail));
			populate(aEntity, std::forward<ComponentData>(aComponentData)...);
		}
		template <typename ComponentData>
		void populate(entity_id aEntity, ComponentData&& aComponentData)
		{
			auto& c = static_cast<static_component<ComponentData>&>(*components().find(ComponentData::meta::id())->second);
			c.populate(aEntity, std::forward<ComponentData>(aComponentData));
		}
		template <typename... ComponentData, typename ComponentDataTail>
		void populate_shared(ComponentData&&... aComponentData, ComponentDataTail&& aComponentDataTail)
		{
			populate_shared(std::forward<ComponentDataTail>(aComponentDataTail));
			populate_shared(std::forward<ComponentData>(aComponentData)...);
		}
		template <typename ComponentData>
		void populate_shared(ComponentData&& aComponentData)
		{
			auto& c = static_cast<static_shared_component<ComponentData>&>(*shared_components().find(ComponentData::meta::id())->second);
			c.populate(std::forward<ComponentData>(aComponentData));
		}
		template <typename ComponentData>
		bool component_instantiated() const
		{
			return component_instantiated(ComponentData::meta::id());
		}
		template <typename ComponentData>
		const static_component<ComponentData>& component() const
		{
			return static_cast<const static_component<ComponentData>&>(component(ComponentData::meta::id()));
		}
		template <typename ComponentData>
		static_component<ComponentData>& component()
		{
			return const_cast<static_component<ComponentData>&>(const_cast<const ecs*>(this)->component<ComponentData>());
		}
		template <typename ComponentData>
		bool shared_component_instantiated() const
		{
			return shared_component_instantiated(ComponentData::meta::id());
		}
		template <typename ComponentData>
		const static_shared_component<ComponentData>& shared_component() const
		{
			return static_cast<const static_shared_component<ComponentData>&>(shared_component(ComponentData::meta::id()));
		}
		template <typename ComponentData>
		static_shared_component<ComponentData>& shared_component()
		{
			return const_cast<static_shared_component<ComponentData>&>(const_cast<const ecs*>(this)->shared_component<ComponentData>());
		}
		template <typename System>
		bool system_instantiated() const
		{
			return system_instantiated(System::meta::id());
		}
		template <typename System>
		const System& system() const
		{
			return static_cast<const System&>(system(System::meta::id()));
		}
		template <typename System>
		System& system()
		{
			return const_cast<System>&>(const_cast<const ecs*>(this)->system<System>());
		}
	public:
		bool archetype_registered(const entity_archetype& aArchetype) const;
		void register_archetype(const entity_archetype& aArchetype);
		void register_archetype(entity_archetype&& aArchetype);
		bool component_registered(component_id aComponentId) const;
		template <typename ComponentData>
		bool component_registered() const
		{
			return component_registered(ComponentData::meta::id());
		}
		void register_component(component_id aComponentId, component_factory aFactory);
		template <typename ComponentData>
		void register_component()
		{
			register_component(
				ComponentData::meta::id(),
				[&]() { return std::unique_ptr<i_component>{std::make_unique<static_component<ComponentData>>(*this)}; });
		}
		bool shared_component_registered(component_id aComponentId) const;
		template <typename ComponentData>
		bool shared_component_registered() const
		{
			return shared_component_registered(ComponentData::meta::id());
		}
		void register_shared_component(component_id aComponentId, shared_component_factory aFactory);
		template <typename ComponentData>
		void register_shared_component()
		{
			register_shared_component(
				ComponentData::meta::id(),
				[&]() { return std::unique_ptr<i_shared_component>{std::make_unique<static_shared_component<ComponentData>>(*this)}; });
		}
		bool system_registered(system_id aSystemId) const;
		template <typename System>
		bool system_registered() const
		{
			return system_registered(System::meta::id());
		}
		void register_system(system_id aSystemId, system_factory aFactory);
		template <typename System>
		void register_system()
		{
			register_system(
				System::meta::id(),
				[&]() { return std::unique_ptr<i_system>{std::make_unique<System>(*this)}; });
		}
	private:
		archetype_registry_t iArchetypeRegistry;
		component_factories_t iComponentFactories;
		mutable components_t iComponents;
		shared_component_factories_t iSharedComponentFactories;
		mutable shared_components_t iSharedComponents;
		system_factories_t iSystemFactories;
		mutable systems_t iSystems;
		entity_id iNextEntityId;
		std::vector<entity_id> iFreedEntityIds;
	};
}