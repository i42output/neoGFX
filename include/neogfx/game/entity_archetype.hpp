// entity_archetype.hpp
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
#include <neolib/set.hpp>
#include <neolib/allocator.hpp>
#include <neolib/uuid.hpp>
#include <neolib/string.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_entity_archetype.hpp>

namespace neogfx::game
{
	class entity_archetype : public i_entity_archetype
	{
	private:
		typedef neolib::set<component_id, component_id, std::less<component_id>, neolib::fast_pool_allocator<component_id>> component_list;
	public:
		entity_archetype(const entity_archetype_id& aId, const std::string& aName, std::initializer_list<component_id> aComponents) :
			iId{ aId }, iName{ aName }, iComponents{ aComponents }
		{
		}
		template <typename ComponentIdIter>
		entity_archetype(const entity_archetype_id& aId, const std::string& aName, ComponentIdIter aFirstComponent, ComponentIdIter aLastComponent) :
			iId{ aId }, iName{ aName }, iComponents{ aFirstComponent, aLastComponent }
		{
		}
		entity_archetype(const entity_archetype& aOther) :
			iId{ aOther.iId }, iName{ aOther.iName }, iComponents{ aOther.iComponents }
		{
		}
		entity_archetype(entity_archetype&& aOther) :
			iId{ aOther.iId }, iName{ std::move(aOther.iName) }, iComponents{ std::move(aOther.iComponents) }
		{
		}
	public:
		const entity_archetype_id& id() const
		{
			return iId;
		}
		const neolib::i_string& name() const
		{
			return iName;
		}
		const neolib::i_set<component_id>& components() const
		{
			return iComponents;
		}
		neolib::i_set<component_id>& components()
		{
			return iComponents;
		}
	private:
		entity_archetype_id iId;
		neolib::string iName;
		component_list iComponents;
	};
}