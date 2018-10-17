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
#include <neogfx/core/object.hpp>
#include <neogfx/game/i_ecs.hpp>

namespace neogfx::game
{
	class ecs : public object<i_ecs>
	{
	private:
		typedef std::vector<handle_t> handles_t;
	public:
		ecs(ecs_flags aCreationFlags = ecs_flags::PopulateEntityInfo);
	public:
		ecs_flags flags() const override;
		entity_id create_entity(const entity_archetype_id& aArchetypeId) override;
		void destroy_entity(entity_id aEntityId) override;
	public:
		const archetype_registry_t& archetypes() const override;
		archetype_registry_t& archetypes() override;
		const component_factories_t& component_factories() const override;
		component_factories_t& component_factories() override;
		const components_t& components() const override;
		components_t& components() override;
		const shared_component_factories_t& shared_component_factories() const override;
		shared_component_factories_t& shared_component_factories() override;
		const shared_components_t& shared_components() const override;
		shared_components_t& shared_components() override;
		const system_factories_t& system_factories() const override;
		system_factories_t& system_factories() override;
		const systems_t& systems() const override;
		systems_t& systems() override;
	public:
		const entity_archetype& archetype(entity_archetype_id aArchetypeId) const override;
		entity_archetype& archetype(entity_archetype_id aArchetypeId) override;
		bool component_instantiated(component_id aComponentId) const override;
		const i_component& component(component_id aComponentId) const override;
		i_component& component(component_id aComponentId) override;
		bool shared_component_instantiated(component_id aComponentId) const override;
		const i_shared_component& shared_component(component_id aComponentId) const override;
		i_shared_component& shared_component(component_id aComponentId) override;
		bool system_instantiated(system_id aSystemId) const override;
		const i_system& system(system_id aSystemId) const override;
		i_system& system(system_id aSystemId) override;
	public:
		entity_id next_entity_id() override;
		void free_entity_id(entity_id aId) override;
	public:
		bool archetype_registered(const entity_archetype& aArchetype) const override;
		void register_archetype(const entity_archetype& aArchetype) override;
		void register_archetype(entity_archetype&& aArchetype) override;
		bool component_registered(component_id aComponentId) const override;
		void register_component(component_id aComponentId, component_factory aFactory) override;
		bool shared_component_registered(component_id aComponentId) const override;
		void register_shared_component(component_id aComponentId, shared_component_factory aFactory) override;
		bool system_registered(system_id aSystemId) const override;
		void register_system(system_id aSystemId, system_factory aFactory) override;
	public:
		handle_t to_handle(handle_id aId) const override;
		handle_id add_handle(const std::type_info& aTypeInfo, handle_t aHandle) override;
		handle_t update_handle(handle_id aId, const std::type_info& aTypeInfo, handle_t aHandle) override;
		handle_t release_handle(handle_id aId) override;
	private:
		handle_id next_handle_id();
		void free_handle_id(handle_id aId);
	public:
		using i_ecs::create_entity;
	public:
		using i_ecs::populate;
		using i_ecs::populate_shared;
		using i_ecs::component_instantiated;
		using i_ecs::component;
		using i_ecs::shared_component_instantiated;
		using i_ecs::shared_component;
		using i_ecs::system_instantiated;
		using i_ecs::system;
	public:
		using i_ecs::component_registered;
		using i_ecs::register_component;
		using i_ecs::shared_component_registered;
		using i_ecs::register_shared_component;
		using i_ecs::system_registered;
		using i_ecs::register_system;
	private:
		ecs_flags iFlags;
		archetype_registry_t iArchetypeRegistry;
		component_factories_t iComponentFactories;
		mutable components_t iComponents;
		shared_component_factories_t iSharedComponentFactories;
		mutable shared_components_t iSharedComponents;
		system_factories_t iSystemFactories;
		mutable systems_t iSystems;
		entity_id iNextEntityId;
		std::vector<entity_id> iFreedEntityIds;
		handle_id iNextHandleId;
		std::vector<handle_id> iFreedHandleIds;
		handles_t iHandles;
		neolib::callback_timer iSystemTimer;
	};
}