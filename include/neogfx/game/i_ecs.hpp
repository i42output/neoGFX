// i_ecs.hpp
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
#include <map>
#include <neogfx/core/event.hpp>
#include <neogfx/core/i_object.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/entity_archetype.hpp>
#include <neogfx/game/component.hpp>
#include <neogfx/game/system.hpp>

namespace neogfx::game
{
	class i_ecs : public virtual i_object
	{
	public:
		struct entity_archetype_not_found : std::logic_error { entity_archetype_not_found() : std::logic_error("neogfx::i_ecs::entity_archetype_not_found") {} };
		struct component_not_found : std::logic_error { component_not_found() : std::logic_error("neogfx::i_ecs::component_not_found") {} };
		struct system_not_found : std::logic_error { system_not_found() : std::logic_error("neogfx::i_ecs::system_not_found") {} };
		struct uuid_exists : std::runtime_error { uuid_exists(const std::string& aContext) : std::runtime_error("neogfx::i_ecs::uuid_exists: " + aContext) {} };
		struct entity_ids_exhuasted : std::runtime_error { entity_ids_exhuasted() : std::runtime_error("neogfx::i_ecs::entity_ids_exhuasted") {} };
	public:
		event<entity_id> entity_created;
		event<entity_id> entity_destroyed;
	public:
		typedef std::function<std::unique_ptr<i_component>()> component_factory;
		typedef std::function<std::unique_ptr<i_shared_component>()> shared_component_factory;
		typedef std::function<std::unique_ptr<i_system>()> system_factory;
	protected:
		typedef std::map<entity_archetype_id, entity_archetype> archetype_registry_t;
		typedef std::map<component_id, component_factory> component_factories_t;
		typedef std::map<component_id, std::unique_ptr<i_component>> components_t;
		typedef std::map<component_id, shared_component_factory> shared_component_factories_t;
		typedef std::map<component_id, std::unique_ptr<i_shared_component>> shared_components_t;
		typedef std::map<system_id, system_factory> system_factories_t;
		typedef std::map<system_id, std::unique_ptr<i_system>> systems_t;
	public:
		virtual entity_id create_entity(const entity_archetype_id& aArchetypeId) = 0;
		virtual void destroy_entity(entity_id aEntityId) = 0;
	public:
		virtual const archetype_registry_t& archetypes() const = 0;
		virtual archetype_registry_t& archetypes() = 0;
		virtual const component_factories_t& component_factories() const = 0;
		virtual component_factories_t& component_factories() = 0;
		virtual const components_t& components() const = 0;
		virtual components_t& components() = 0;
		virtual const shared_component_factories_t& shared_component_factories() const = 0;
		virtual shared_component_factories_t& shared_component_factories() = 0;
		virtual const shared_components_t& shared_components() const = 0;
		virtual shared_components_t& shared_components() = 0;
		virtual const system_factories_t& system_factories() const = 0;
		virtual system_factories_t& system_factories() = 0;
		virtual const systems_t& systems() const = 0;
		virtual systems_t& systems() = 0;
	public:
		virtual const entity_archetype& archetype(entity_archetype_id aArchetypeId) const = 0;
		virtual entity_archetype& archetype(entity_archetype_id aArchetypeId) = 0;
		virtual bool component_instantiated(component_id aComponentId) const = 0;
		virtual const i_component& component(component_id aComponentId) const = 0;
		virtual i_component& component(component_id aComponentId) = 0;
		virtual bool shared_component_instantiated(component_id aComponentId) const = 0;
		virtual const i_shared_component& shared_component(component_id aComponentId) const = 0;
		virtual i_shared_component& shared_component(component_id aComponentId) = 0;
		virtual bool system_instantiated(system_id aSystemId) const = 0;
		virtual const i_system& system(system_id aSystemId) const = 0;
		virtual i_system& system(system_id aSystemId) = 0;
	public:
		virtual entity_id next_entity_id() = 0;
		virtual void free_entity_id(entity_id aId) = 0;
	public:
		virtual bool archetype_registered(const entity_archetype& aArchetype) const = 0;
		virtual void register_archetype(const entity_archetype& aArchetype) = 0;
		virtual void register_archetype(entity_archetype&& aArchetype) = 0;
		virtual bool component_registered(component_id aComponentId) const = 0;
		virtual void register_component(component_id aComponentId, component_factory aFactory) = 0;
		virtual bool shared_component_registered(component_id aComponentId) const = 0;
		virtual void register_shared_component(component_id aComponentId, shared_component_factory aFactory) = 0;
		virtual bool system_registered(system_id aSystemId) const = 0;
		virtual void register_system(system_id aSystemId, system_factory aFactory) = 0;
		// helpers
	public:
		template <typename... ComponentData>
		entity_id create_entity(const entity_archetype_id& aArchetypeId, ComponentData&&... aComponentData)
		{
			auto newEntity = create_entity(aArchetypeId);
			populate(newEntity, std::forward<ComponentData>(aComponentData)...);
			return newEntity;
		}
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
			return const_cast<static_component<ComponentData>&>(const_cast<const i_ecs*>(this)->component<ComponentData>());
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
			return const_cast<static_shared_component<ComponentData>&>(const_cast<const i_ecs*>(this)->shared_component<ComponentData>());
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
			return const_cast<System&>(const_cast<const i_ecs*>(this)->system<System>());
		}
	public:
		template <typename ComponentData>
		bool component_registered() const
		{
			return component_registered(ComponentData::meta::id());
		}
		template <typename ComponentData>
		void register_component()
		{
			register_component(
				ComponentData::meta::id(),
				[&]() { return std::unique_ptr<i_component>{std::make_unique<static_component<ComponentData>>(*this)}; });
		}
		template <typename ComponentData>
		bool shared_component_registered() const
		{
			return shared_component_registered(ComponentData::meta::id());
		}
		template <typename ComponentData>
		void register_shared_component()
		{
			register_shared_component(
				ComponentData::meta::id(),
				[&]() { return std::unique_ptr<i_shared_component>{std::make_unique<static_shared_component<ComponentData>>(*this)}; });
		}
		template <typename System>
		bool system_registered() const
		{
			return system_registered(System::meta::id());
		}
		template <typename System>
		void register_system()
		{
			register_system(
				System::meta::id(),
				[&]() { return std::unique_ptr<i_system>{std::make_unique<System>(*this)}; });
		}
	};
}