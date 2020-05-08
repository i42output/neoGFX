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
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_entity_archetype.hpp>
#include <neogfx/game/component.hpp>
#include <neogfx/game/system.hpp>
#include <neogfx/game/chrono.hpp>

namespace neogfx::game
{
    enum class ecs_flags : uint32_t
    {
        None                = 0x0000,
        PopulateEntityInfo  = 0x0001,
        Turbo               = 0x0002,
        CreatePaused        = 0x0004,

        Default             = PopulateEntityInfo | Turbo
    };

    inline constexpr ecs_flags operator|(ecs_flags aLhs, ecs_flags aRhs)
    {
        return static_cast<ecs_flags>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr ecs_flags operator&(ecs_flags aLhs, ecs_flags aRhs)
    {
        return static_cast<ecs_flags>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr ecs_flags& operator|=(ecs_flags& aLhs, ecs_flags aRhs)
    {
        return aLhs = static_cast<ecs_flags>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr ecs_flags& operator&=(ecs_flags& aLhs, ecs_flags aRhs)
    {
        return aLhs = static_cast<ecs_flags>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    class i_ecs
    {
    public:
        declare_event(systems_paused)
        declare_event(systems_resumed)
        declare_event(entity_created, entity_id)
        declare_event(entity_destroyed, entity_id)
        declare_event(handle_updated, handle_id)
    public:
        struct entity_archetype_not_found : std::logic_error { entity_archetype_not_found() : std::logic_error("neogfx::i_ecs::entity_archetype_not_found") {} };
        struct component_not_found : std::logic_error { component_not_found() : std::logic_error("neogfx::i_ecs::component_not_found") {} };
        struct system_not_found : std::logic_error { system_not_found() : std::logic_error("neogfx::i_ecs::system_not_found") {} };
        struct uuid_exists : std::runtime_error { uuid_exists(const std::string& aContext) : std::runtime_error("neogfx::i_ecs::uuid_exists: " + aContext) {} };
        struct entity_ids_exhausted : std::runtime_error { entity_ids_exhausted() : std::runtime_error("neogfx::i_ecs::entity_ids_exhausted") {} };
        struct handle_ids_exhausted : std::runtime_error { handle_ids_exhausted() : std::runtime_error("neogfx::i_ecs::handle_ids_exhausted") {} };
        struct invalid_handle_id : std::logic_error { invalid_handle_id() : std::logic_error("neogfx::i_ecs::invalid_handle_id") {} };
    public:
        typedef std::function<std::unique_ptr<i_component>()> component_factory;
        typedef std::function<std::unique_ptr<i_shared_component>()> shared_component_factory;
        typedef std::function<std::unique_ptr<i_system>()> system_factory;
    protected:
        typedef std::map<entity_archetype_id, std::shared_ptr<const i_entity_archetype>> archetype_registry_t;
        typedef std::map<component_id, component_factory> component_factories_t;
        typedef std::map<component_id, std::unique_ptr<i_component>> components_t;
        typedef std::map<component_id, shared_component_factory> shared_component_factories_t;
        typedef std::map<component_id, std::unique_ptr<i_shared_component>> shared_components_t;
        typedef std::map<system_id, system_factory> system_factories_t;
        typedef std::map<system_id, std::unique_ptr<i_system>> systems_t;
    public:
        typedef id_t handle_id;
        typedef void* handle_t;
    public:
        virtual std::recursive_mutex& mutex() const = 0; // todo: use a polymorphic mutex
    public:
        virtual ecs_flags flags() const = 0;
        virtual entity_id create_entity(const entity_archetype_id& aArchetypeId) = 0;
        virtual void destroy_entity(entity_id aEntityId) = 0;
    public:
        virtual bool all_systems_paused() const = 0;
        virtual void pause_all_systems() = 0;
        virtual void resume_all_systems() = 0;
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
        virtual const i_entity_archetype& archetype(entity_archetype_id aArchetypeId) const = 0;
        virtual i_entity_archetype& archetype(entity_archetype_id aArchetypeId) = 0;
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
        virtual bool archetype_registered(const i_entity_archetype& aArchetype) const = 0;
        virtual void register_archetype(const i_entity_archetype& aArchetype) = 0;
        virtual void register_archetype(std::shared_ptr<const i_entity_archetype> aArchetype) = 0;
        virtual bool component_registered(component_id aComponentId) const = 0;
        virtual void register_component(component_id aComponentId, component_factory aFactory) = 0;
        virtual bool shared_component_registered(component_id aComponentId) const = 0;
        virtual void register_shared_component(component_id aComponentId, shared_component_factory aFactory) = 0;
        virtual bool system_registered(system_id aSystemId) const = 0;
        virtual void register_system(system_id aSystemId, system_factory aFactory) = 0;
    public:
        virtual handle_t to_handle(handle_id aId) const = 0;
        virtual handle_id add_handle(const std::type_info& aTypeInfo, handle_t aHandle) = 0;
        virtual handle_t update_handle(handle_id aId, const std::type_info& aTypeInfo, handle_t aHandle) = 0;
        virtual handle_t release_handle(handle_id aId) = 0;
        // helpers
    public:
        template <typename... ComponentData>
        entity_id create_entity(const entity_archetype_id& aArchetypeId, ComponentData&&... aComponentData)
        {
            auto newEntity = create_entity(aArchetypeId);
            populate(newEntity, std::forward<ComponentData>(aComponentData)...);
            archetype(aArchetypeId).populate_default_components(*this, newEntity);
            return newEntity;
        }
        template <typename Archetype, typename... ComponentData>
        entity_id create_entity(const Archetype& aArchetype, ComponentData&&... aComponentData)
        {
            if (!archetype_registered(aArchetype))
                register_archetype(aArchetype);
            return create_entity(aArchetype.id(), std::forward<ComponentData>(aComponentData)...);
        }
    public:
        template <typename ComponentData, typename... ComponentDataRest>
        void populate(entity_id aEntity, ComponentData&& aComponentData, ComponentDataRest&&... aComponentDataRest)
        {
            populate(aEntity, std::forward<ComponentData>(aComponentData));
            populate(aEntity, std::forward<ComponentDataRest>(aComponentDataRest)...);
        }
        template <typename ComponentData>
        void populate(entity_id aEntity, ComponentData&& aComponentData)
        {
            component<ecs_data_type_t<ComponentData>>().populate(aEntity, std::forward<ComponentData>(aComponentData));
        }
        template <typename ComponentData, typename... ComponentDataRest>
        void populate_shared(const std::string& aName, ComponentData&& aComponentData, ComponentDataRest&&... aComponentDataRest)
        {
            populate_shared(aName, std::forward<ComponentData>(aComponentData));
            populate_shared(aName, std::forward<ComponentDataRest>(aComponentDataRest)...);
        }
        template <typename ComponentData>
        void populate_shared(const std::string& aName, ComponentData&& aComponentData)
        {
            shared_component<ecs_data_type_t<ComponentData>>().populate(aName, std::forward<ComponentData>(aComponentData));
        }
        template <typename ComponentData>
        bool component_instantiated() const
        {
            return component_instantiated(ecs_data_type_t<ComponentData>::meta::id());
        }
        template <typename ComponentData>
        const static_component<ComponentData>& component() const
        {
            return static_cast<const static_component<ecs_data_type_t<ComponentData>>&>(component(ecs_data_type_t<ComponentData>::meta::id()));
        }
        template <typename ComponentData>
        static_component<ComponentData>& component()
        {
            if (!component_registered<ecs_data_type_t<ComponentData>>())
                register_component<ecs_data_type_t<ComponentData>>();
            return const_cast<static_component<ecs_data_type_t<ComponentData>>&>(to_const(*this).component<ecs_data_type_t<ComponentData>>());
        }
        template <typename ComponentData>
        bool shared_component_instantiated() const
        {
            return shared_component_instantiated(ecs_data_type_t<ComponentData>::meta::id());
        }
        template <typename ComponentData>
        const static_shared_component<ComponentData>& shared_component() const
        {
            return static_cast<const static_shared_component<ecs_data_type_t<ComponentData>>&>(shared_component(ComponentData::meta::id()));
        }
        template <typename ComponentData>
        static_shared_component<ComponentData>& shared_component()
        {
            if (!shared_component_registered<ecs_data_type_t<ComponentData>>())
                register_shared_component<ecs_data_type_t<ComponentData>>();
            return const_cast<static_shared_component<ecs_data_type_t<ComponentData>>&>(to_const(*this).shared_component<ecs_data_type_t<ComponentData>>());
        }
        template <typename System>
        bool system_instantiated() const
        {
            return system_instantiated(ecs_data_type_t<System>::meta::id());
        }
        template <typename System>
        const ecs_data_type_t<System>& system() const
        {
            return static_cast<const ecs_data_type_t<System>&>(system(ecs_data_type_t<System>::meta::id()));
        }
        template <typename System>
        ecs_data_type_t<System>& system()
        {
            if (!system_registered<ecs_data_type_t<System>>())
                register_system<ecs_data_type_t<System>>();
            return const_cast<ecs_data_type_t<System>&>(to_const(*this).system<ecs_data_type_t<System>>());
        }
    public:
        template <typename ComponentData>
        bool component_registered() const
        {
            return component_registered(ecs_data_type_t<ComponentData>::meta::id());
        }
        template <typename ComponentData>
        void register_component()
        {
            register_component(ecs_data_type_t<ComponentData>::meta::id(), [&]() { return std::unique_ptr<i_component>{std::make_unique<static_component<ecs_data_type_t<ComponentData>>>(*this)}; });
        }
        template <typename ComponentData>
        bool shared_component_registered() const
        {
            return shared_component_registered(ecs_data_type_t<ComponentData>::meta::id());
        }
        template <typename ComponentData>
        void register_shared_component()
        {
            register_shared_component(ecs_data_type_t<ComponentData>::meta::id(), [&]() { return std::unique_ptr<i_shared_component>{std::make_unique<static_shared_component<ecs_data_type_t<ComponentData>>>(*this)}; });
        }
        template <typename System>
        bool system_registered() const
        {
            return system_registered(ecs_data_type_t<System>::meta::id());
        }
        template <typename System>
        void register_system()
        {
            register_system(
                ecs_data_type_t<System>::meta::id(),
                [&]() { return std::unique_ptr<i_system>{std::make_unique<ecs_data_type_t<System>>(*this)}; });
        }
    public:
        template <typename Handle>
        Handle to_handle(handle_id aId) const
        {
            return reinterpret_cast<Handle>(to_handle(aId));
        }
        template <typename Context, typename Handle>
        handle_id add_handle(Handle aHandle)
        {
            return add_handle(typeid(Context), reinterpret_cast<handle_t>(aHandle));
        }
        template <typename Context, typename Handle>
        Handle update_handle(handle_id aId, Handle aHandle)
        {
            if constexpr(std::is_pointer<Handle>::value)
                return reinterpret_cast<Handle>(update_handle(aId, typeid(Context), reinterpret_cast<handle_t>(aHandle)));
            else
                return static_cast<Handle>(reinterpret_cast<intptr_t>(update_handle(aId, typeid(Context), reinterpret_cast<handle_t>(aHandle))));
        }
        template <typename Handle>
        Handle release_handle(handle_id aId)
        {
            if constexpr(std::is_pointer<Handle>::value)
                return reinterpret_cast<Handle>(release_handle(aId));
            else
                return static_cast<Handle>(reinterpret_cast<intptr_t>(release_handle(aId)));
        }
    };

    template <typename Data>
    class shared_component_scoped_lock
    {
    public:
        shared_component_scoped_lock(const i_ecs& aEcs) :
            iLockGuard{ aEcs.shared_component<Data>().mutex() }
        {
        }
    private:
        std::scoped_lock<std::recursive_mutex> iLockGuard;
    };

    template <typename Data>
    class scoped_component_lock
    {
    public:
        scoped_component_lock(const i_ecs& aEcs) :
            iLockGuard{ aEcs.component<Data>().mutex() }
        {
        }
    private:
        std::scoped_lock<std::recursive_mutex> iLockGuard;
    };
}