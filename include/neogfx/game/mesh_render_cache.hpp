// mesh_render_cache.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/uuid.hpp>
#include <neolib/core/string.hpp>

#include <neogfx/game/component.hpp>
#include <neogfx/game/i_component_data.hpp>

namespace neogfx::game
{
    enum class cache_state : std::uint32_t
    {
        Invalid = 0x00000000,
        Dirty   = 0x00000001,
        Clean   = 0x00000002
    };

    struct mesh_render_cache
    {
        mutable std::atomic<cache_state> state;
        mutable vec2u32 meshVertexArrayIndices;
        mutable std::vector<vec2u32> patchVertexArrayIndices;

        mesh_render_cache() :
            state{ cache_state::Invalid }
        {
        }

        mesh_render_cache(const mesh_render_cache& other) : 
            state{ other.state.load() },
            meshVertexArrayIndices{ other.meshVertexArrayIndices },
            patchVertexArrayIndices{ other.patchVertexArrayIndices }
        {
        }

        mesh_render_cache(mesh_render_cache&& other) noexcept : 
            state{ other.state.load() },
            meshVertexArrayIndices{ std::move(other.meshVertexArrayIndices) },
            patchVertexArrayIndices{ std::move(other.patchVertexArrayIndices) }
        {
        }

        mesh_render_cache& operator=(const mesh_render_cache& other)
        {
            if (this != &other)
            {
                state = other.state.load();
                meshVertexArrayIndices = other.meshVertexArrayIndices;
                patchVertexArrayIndices = other.patchVertexArrayIndices;
            }
            return *this;
        }

        mesh_render_cache& operator=(mesh_render_cache&& other) noexcept
        {
            if (this != &other)
            {
                state = other.state.load();
                meshVertexArrayIndices = std::move(other.meshVertexArrayIndices);
                patchVertexArrayIndices = std::move(other.patchVertexArrayIndices);
            }
            return *this;
        }

        friend void swap(mesh_render_cache& lhs, mesh_render_cache& rhs) noexcept 
        {
            using std::swap;
            cache_state temp = rhs.state.exchange(lhs.state.load());
            lhs.state.exchange(temp);
            swap(lhs.meshVertexArrayIndices, rhs.meshVertexArrayIndices);
            swap(lhs.patchVertexArrayIndices, rhs.patchVertexArrayIndices);
        }

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0x4fdfc50f, 0x6562, 0x4cf1, 0x80cf, { 0x87, 0x2c, 0xff, 0xf, 0xa4, 0x2f } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "mesh_render_cache";
                return sName;
            }
            static std::uint32_t field_count()
            {
                return 3;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::Enum | component_data_field_type::Atomic | component_data_field_type::Internal;
                case 1:
                    return component_data_field_type::Vec2u32 | component_data_field_type::Internal;
                case 2:
                    return component_data_field_type::Vec2u32 | component_data_field_type::Array | component_data_field_type::Internal;
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "State",
                    "Mesh Vertex Array Indices",
                    "Patch Vertex Array Indices"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline bool is_render_cache_valid_no_lock(component<game::mesh_render_cache> const& aCache, entity_id aEntity)
    {
        return aCache.has_entity_record_no_lock(aEntity) &&
            aCache.entity_record_no_lock(aEntity).state != cache_state::Invalid;
    }

    inline bool is_render_cache_dirty_no_lock(component<game::mesh_render_cache> const& aCache, entity_id aEntity)
    {
        return aCache.has_entity_record_no_lock(aEntity) &&
            aCache.entity_record_no_lock(aEntity).state == cache_state::Dirty;
    }

    inline bool is_render_cache_clean_no_lock(component<game::mesh_render_cache> const& aCache, entity_id aEntity)
    {
        return aCache.has_entity_record_no_lock(aEntity) &&
            aCache.entity_record_no_lock(aEntity).state == cache_state::Clean;
    }

    inline void set_render_cache_invalid_no_lock(component<game::mesh_render_cache>& aCache, entity_id aEntity)
    {
        if (aCache.has_entity_record_no_lock(aEntity))
        {
            auto& cache = aCache.entity_record_no_lock(aEntity);
            cache.state = cache_state::Invalid;
        }
    }

    inline void set_render_cache_dirty_no_lock(component<game::mesh_render_cache>& aCache, entity_id aEntity)
    {
        if (aCache.has_entity_record_no_lock(aEntity))
        {
            auto& cache = aCache.entity_record_no_lock(aEntity);
            if (cache.state == cache_state::Clean)
                cache.state = cache_state::Dirty;
        }
    }

    inline void set_render_cache_clean_no_lock(component<game::mesh_render_cache>& aCache, entity_id aEntity)
    {
        if (aCache.has_entity_record_no_lock(aEntity))
        {
            auto& cache = aCache.entity_record_no_lock(aEntity);
            if (cache.state == cache_state::Dirty)
                cache.state = cache_state::Clean;
        }
    }

    inline bool is_render_cache_valid_no_lock(i_ecs const& aEcs, entity_id aEntity)
    {
        return is_render_cache_valid_no_lock(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline bool is_render_cache_dirty_no_lock(i_ecs const& aEcs, entity_id aEntity)
    {
        return is_render_cache_dirty_no_lock(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline bool is_render_cache_clean_no_lock(i_ecs const& aEcs, entity_id aEntity)
    {
        return is_render_cache_clean_no_lock(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline void set_render_cache_invalid_no_lock(i_ecs& aEcs, entity_id aEntity)
    {
        set_render_cache_invalid_no_lock(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline void set_render_cache_dirty_no_lock(i_ecs& aEcs, entity_id aEntity)
    {
        set_render_cache_dirty_no_lock(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline void set_render_cache_clean_no_lock(i_ecs& aEcs, entity_id aEntity)
    {
        set_render_cache_clean_no_lock(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline bool is_render_cache_valid(component<game::mesh_render_cache> const& aCache, entity_id aEntity)
    {
        return aCache.has_entity_record(aEntity) &&
            aCache.entity_record(aEntity).state != cache_state::Invalid;
    }

    inline bool is_render_cache_dirty(component<game::mesh_render_cache> const& aCache, entity_id aEntity)
    {
        return aCache.has_entity_record(aEntity) &&
            aCache.entity_record(aEntity).state == cache_state::Dirty;
    }

    inline bool is_render_cache_clean(component<game::mesh_render_cache> const& aCache, entity_id aEntity)
    {

        return aCache.has_entity_record(aEntity) &&
            aCache.entity_record(aEntity).state == cache_state::Clean;
    }

    inline void set_render_cache_invalid(component<game::mesh_render_cache>& aCache, entity_id aEntity)
    {
        auto& cache = aCache.entity_record(aEntity, true);
        cache.state = cache_state::Invalid;
    }

    inline void set_render_cache_dirty(component<game::mesh_render_cache>& aCache, entity_id aEntity)
    {
        auto& cache = aCache.entity_record(aEntity, true);
        if (cache.state == cache_state::Clean)
            cache.state = cache_state::Dirty;
    }

    inline void set_render_cache_clean(component<game::mesh_render_cache>& aCache, entity_id aEntity)
    {
        auto& cache = aCache.entity_record(aEntity, true);
        if (cache.state == cache_state::Dirty)
            cache.state = cache_state::Clean;
    }

    inline bool is_render_cache_valid(i_ecs const& aEcs, entity_id aEntity)
    {
        return is_render_cache_valid(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline bool is_render_cache_dirty(i_ecs const& aEcs, entity_id aEntity)
    {
        return is_render_cache_dirty(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline bool is_render_cache_clean(i_ecs const& aEcs, entity_id aEntity)
    {
        return is_render_cache_clean(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline void set_render_cache_invalid(i_ecs& aEcs, entity_id aEntity)
    {
        set_render_cache_invalid(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline void set_render_cache_dirty(i_ecs& aEcs, entity_id aEntity)
    {
        set_render_cache_dirty(aEcs.component<mesh_render_cache>(), aEntity);
    }

    inline void set_render_cache_clean(i_ecs& aEcs, entity_id aEntity)
    {
        set_render_cache_clean(aEcs.component<mesh_render_cache>(), aEntity);
    }
}