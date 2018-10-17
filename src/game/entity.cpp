// entity.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/game/entity.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_filter.hpp>

namespace neogfx::game
{
	entity::entity(i_ecs& aEcs, entity_id aId) :
		iEcs{ aEcs }, iId{ aId }
	{
		iSink += ecs().entity_destroyed([this](entity_id aId)
		{
			if (aId == iId)
				iId = null_entity;
		});
	}

	entity::entity(i_ecs& aEcs, const entity_archetype_id& aArchetypeId) :
		entity{ aEcs, aEcs.create_entity(aArchetypeId) }
	{
	}

	entity::~entity()
	{
		if (!detached_or_destroyed())
			ecs().destroy_entity(iId);
	}

	i_ecs& entity::ecs() const
	{
		return iEcs;
	}

	entity_id entity::id() const
	{
		return iId;
	}

	bool entity::detached_or_destroyed() const
	{
		return iId == null_entity;
	}

	entity_id entity::detach(bool aForRendering)
	{
		if (aForRendering)
		{
			if (!ecs().component<mesh_renderer>().has_entity_record(id()) && ecs().component<material>().has_entity_record(id()))
				ecs().component<mesh_renderer>().populate(id(), mesh_renderer{ ecs().component<material>().entity_record(id()) });
			if (!ecs().component<mesh_filter>().has_entity_record(id()) && ecs().component<mesh>().has_entity_record(id()))
				ecs().component<mesh_filter>().populate(id(), mesh_filter{ {}, ecs().component<mesh>().entity_record(id()) });
		}
		auto id = iId;
		iId = null_entity;
		return id;
	}
}