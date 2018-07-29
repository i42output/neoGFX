// component.hpp
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
#include <neogfx/game/ecs.hpp>
#include <neogfx/game/i_component.hpp>

namespace neogfx
{
	template <typename Data>
	class builtin_component : public i_component
	{
	public:
		typedef Data data_type;
		typedef data_type::meta meta_data_type;
	public:
		builtin_component(const component_id& aId) :
			iId{ aId }
		{
		}
	public:
		const component_id& id() const override
		{
			return iId;
		}
	public:
		bool shared_data() const override
		{
			return meta_data_type::shared_data();
		}
		const neolib::i_string& name() const override
		{
			return meta_data_type::name();
		}
		uint32_t field_count() const override
		{
			return meta_data_type::field_count();
		}
		component_data_field_type field_type(uint32_t aFieldIndex) const override
		{
			return meta_data_type::field_type(aFieldIndex);
		}
		const neolib::i_string& field_name(uint32_t aFieldIndex) const override
		{
			return meta_data_type::field_name(aFieldIndex);
		}
	private:
		component_id iId;
	};
}