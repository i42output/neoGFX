// system.hpp
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
#include <neogfx/game/ecs.hpp>

namespace neogfx::game
{
	class system : public i_system
	{
	private:
		typedef neolib::set<component_id, component_id, std::less<component_id>, neolib::fast_pool_allocator<component_id>> component_list;
	public:
		system(const ecs::context& aContext) :
			iContext{ aContext }
		{
		}
		template <typename ComponentIdIter>
		system(const ecs::context& aContext, ComponentIdIter aFirstComponent, ComponentIdIter aLastComponent) :
			iContext{ aContext }, iComponents{ aFirstComponent, aLastComponent }
		{
		}
		system(const system& aOther) :
			iContext{ aOther.iContext }, iComponents{ aOther.iComponents }
		{
		}
		system(system&& aOther) :
			iContext{ aOther.iContext }, iComponents{ std::move(aOther.iComponents) }
		{
		}
	public:
		const ecs::context& context() const
		{
			return iContext;
		}
	public:
		const neolib::i_set<component_id>& components() const override
		{
			return iComponents;
		}
		neolib::i_set<component_id>& components() override
		{
			return iComponents;
		}
	public:
		const i_component& component(component_id aComponentId) const override
		{
			return ecs::instance().component(iContext, aComponentId);
		}
		i_component& component(component_id aComponentId) override
		{
			return ecs::instance().component(iContext, aComponentId);
		}
	private:
		const ecs::context& iContext;
		component_list iComponents;
	};
}