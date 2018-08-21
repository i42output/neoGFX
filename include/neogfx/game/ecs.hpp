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
#include <neogfx/game/i_system.hpp>

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
		typedef uint64_t context_id;
		class context_data
		{
		private:
			typedef std::map<entity_archetype_id, entity_archetype> archetype_registry_t;
			typedef std::map<component_id, component_factory> component_factories_t;
			typedef std::map<component_id, std::unique_ptr<i_component>> components_t;
			typedef std::map<component_id, shared_component_factory> shared_component_factories_t;
			typedef std::map<component_id, std::unique_ptr<i_shared_component>> shared_components_t;
			typedef std::map<system_id, system_factory> system_factories_t;
			typedef std::map<system_id, std::unique_ptr<i_system>> systems_t;
		public:
			context_data(ecs& aOwner, context_id aId);
		public:
			context_id id() const;
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
			void add_ref();
			void release();
		private:
			ecs& iOwner;
			context_id iId;
			int32_t iReferenceCount;
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
		typedef std::map<context_id, context_data> contexts_t;
	public:
		class context
		{
			friend class ecs;
		public:
			context(context_data& aOwner);
			context(const context& aOther);
			context(context&& aOther);
			~context();
		public:
			context& operator=(const context&) = delete;
			context& operator=(context&&) = delete;
		private:
			context_data& owner() const;
		private:
			void add_ref();
			void release();
		private:
			context_data& iOwner;
		};
	public:
		ecs();
	public:
		static ecs& instance();
	public:
		context create_context();
		void destroy_context(context_id aContextId);
	public:
		entity_id create_entity(const context& aContext, const entity_archetype_id& aArchetypeId);
		template <typename... ComponentData>
		entity_id create_entity(const context& aContext, const entity_archetype_id& aArchetypeId, ComponentData&&... aComponentData)
		{
			auto newEntity = create_entity(aContext, aArchetypeId);
			populate(aContext, newEntity, std::forward<ComponentData>(aComponentData)...);
			return newEntity;
		}
		void destroy_entity(const context& aContext, entity_id aEntityId);
	public:
		template <typename... ComponentData, typename ComponentDataTail>
		void populate(const context& aContext, entity_id aEntity, ComponentData&&... aComponentData, ComponentDataTail&& aComponentDataTail)
		{
			populate(aContext, aEntity, std::forward<ComponentDataTail>(aComponentDataTail));
			populate(aContext, aEntity, std::forward<ComponentData>(aComponentData)...);
		}
		template <typename ComponentData>
		void populate(const context& aContext, entity_id aEntity, ComponentData&& aComponentData)
		{
			auto& c = static_cast<static_component<ComponentData>&>(*aContext.owner().components().find(ComponentData::meta::id())->second);
			c.populate(aEntity, std::forward<ComponentData>(aComponentData));
		}
		template <typename... ComponentData, typename ComponentDataTail>
		void populate_shared(const context& aContext, ComponentData&&... aComponentData, ComponentDataTail&& aComponentDataTail)
		{
			populate_shared(aContext, std::forward<ComponentDataTail>(aComponentDataTail));
			populate_shared(aContext, std::forward<ComponentData>(aComponentData)...);
		}
		template <typename ComponentData>
		void populate_shared(const context& aContext, ComponentData&& aComponentData)
		{
			auto& c = static_cast<static_shared_component<ComponentData>&>(*aContext.owner().shared_components().find(ComponentData::meta::id())->second);
			c.populate(std::forward<ComponentData>(aComponentData));
		}
		bool component_instantiated(const context& aContext, component_id aComponentId) const;
		template <typename ComponentData>
		bool component_instantiated(const context& aContext) const
		{
			return component_instantiated(aContext, ComponentData::meta::id());
		}
		const i_component& component(const context& aContext, component_id aComponentId) const;
		i_component& component(const context& aContext, component_id aComponentId);
		template <typename ComponentData>
		const static_component<ComponentData>& component(const context& aContext) const
		{
			return static_cast<const static_component<ComponentData>&>(component(aContext, ComponentData::meta::id()));
		}
		template <typename ComponentData>
		static_component<ComponentData>& component(const context& aContext)
		{
			return const_cast<static_component<ComponentData>&>(const_cast<const ecs*>(this)->component<ComponentData>(aContext));
		}
		bool shared_component_instantiated(const context& aContext, component_id aComponentId) const;
		template <typename ComponentData>
		bool shared_component_instantiated(const context& aContext) const
		{
			return shared_component_instantiated(aContext, ComponentData::meta::id());
		}
		const i_shared_component& shared_component(const context& aContext, component_id aComponentId) const;
		i_shared_component& shared_component(const context& aContext, component_id aComponentId);
		template <typename ComponentData>
		const static_shared_component<ComponentData>& shared_component(const context& aContext) const
		{
			return static_cast<const static_shared_component<ComponentData>&>(shared_component(aContext, ComponentData::meta::id()));
		}
		template <typename ComponentData>
		static_shared_component<ComponentData>& shared_component(const context& aContext)
		{
			return const_cast<static_shared_component<ComponentData>&>(const_cast<const ecs*>(this)->shared_component<ComponentData>(aContext));
		}
		bool system_instantiated(const context& aContext, system_id aSystemId) const;
		template <typename System>
		bool system_instantiated(const context& aContext) const
		{
			return system_instantiated(aContext, System::meta::id());
		}
		const i_component& system(const context& aContext, system_id aSystemId) const;
		i_component& system(const context& aContext, system_id aSystemId);
		template <typename System>
		const System& system(const context& aContext) const
		{
			return static_cast<const System&>(system(aContext, System::meta::id()));
		}
		template <typename System>
		System& system(const context& aContext)
		{
			return const_cast<System>&>(const_cast<const ecs*>(this)->system<System>(aContext));
		}
	public:
		bool archetype_registered(const context& aContext, const entity_archetype& aArchetype) const;
		void register_archetype(const context& aContext, const entity_archetype& aArchetype);
		void register_archetype(const context& aContext, entity_archetype&& aArchetype);
		bool component_registered(const context& aContext, component_id aComponentId) const;
		template <typename ComponentData>
		bool component_registered(const context& aContext) const
		{
			return component_registered(aContext, ComponentData::meta::id());
		}
		void register_component(const context& aContext, component_id aComponentId, component_factory aFactory);
		template <typename ComponentData>
		void register_component(const context& aContext)
		{
			register_component(
				aContext,
				ComponentData::meta::id(),
				[]() { return std::unique_ptr<i_component>{std::make_unique<static_component<ComponentData>>()}; });
		}
		bool shared_component_registered(const context& aContext, component_id aComponentId) const;
		template <typename ComponentData>
		bool shared_component_registered(const context& aContext) const
		{
			return shared_component_registered(aContext, ComponentData::meta::id());
		}
		void register_shared_component(const context& aContext, component_id aComponentId, shared_component_factory aFactory);
		template <typename ComponentData>
		void register_shared_component(const context& aContext)
		{
			register_shared_component(
				aContext,
				ComponentData::meta::id(),
				[]() { return std::unique_ptr<i_shared_component>{std::make_unique<static_shared_component<ComponentData>>()}; });
		}
		bool system_registered(const context& aContext, system_id aSystemId) const;
		template <typename System>
		bool system_registered(const context& aContext) const
		{
			return system_registered(aContext, System::meta::id());
		}
		void register_system(const context& aContext, system_id aSystemId, system_factory aFactory);
		template <typename System>
		void register_system(const context& aContext)
		{
			register_system(
				aContext,
				System::meta::id(),
				[](return std::unique_ptr<i_system>{std::make_unique<System>()}; ));
		}
	private:
		context_id iNextContextId;
		contexts_t iContexts;
	};
}