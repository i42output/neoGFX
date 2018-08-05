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

namespace neogfx
{
	class ecs
	{
	public:
		struct entity_archetype_not_found : std::logic_error { entity_archetype_not_found() : std::logic_error("neogfx::ecs::entity_archetype_not_found") {} };
		struct component_not_found : std::logic_error { component_not_found() : std::logic_error("neogfx::ecs::component_not_found") {} };
		struct invalid_context : std::logic_error { invalid_context() : std::logic_error("neogfx::ecs::invalid_context") {} };
		struct entity_ids_exhuasted : std::runtime_error { entity_ids_exhuasted() : std::runtime_error("neogfx::ecs::entity_ids_exhuasted") {} };
	public:
		typedef std::function<std::unique_ptr<i_component>()> component_factory;
	private:
		typedef uint64_t context_id;
		class context_data
		{
		private:
			typedef std::map<entity_archetype_id, entity_archetype> archetype_registry_t;
			typedef std::map<component_id, component_factory> component_factories_t;
			typedef std::map<component_id, std::unique_ptr<i_component>> components_t;
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
			const systems_t& systems() const;
			systems_t& systems();
		public:
			const entity_archetype& archetype(entity_archetype_id aArchetypeId) const;
			entity_archetype& archetype(entity_archetype_id aArchetypeId);
			bool component_instantiated(component_id aComponentId) const;
			const i_component& component(component_id aComponentId) const;
			i_component& component(component_id aComponentId);
		public:
			entity_id next_entity_id();
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
			systems_t iSystems;
			entity_id iNextEntityId;
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
			aContext.owner().components().find(ComponentData::meta::id())->second->populate(aEntity, std::forward<ComponentData>(aComponentData));
		}
		const i_component& component(const context& aContext, component_id aComponentId) const;
		i_component& component(const context& aContext, component_id aComponentId);
	public:
		void register_archetype(const context& aContext, const entity_archetype& aArchetype);
		void register_archetype(const context& aContext, entity_archetype&& aArchetype);
		void register_component(const context& aContext, component_id aComponentId, component_factory aFactory);
		template <typename ComponentData>
		void register_component(const context& aContext)
		{
			register_component(
				aContext,
				ComponentData::meta::id(),
				[]( return std::unique_ptr<i_component>{std::make_unique<static_component<ComponentData>>()}; ));
		}
	private:
		context_id iNextContextId;
		contexts_t iContexts;
	};
}