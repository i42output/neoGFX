// i_component.hpp
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
#include <neolib/string.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_component_data.hpp>

namespace neogfx
{
	class i_component
	{
	public:
		virtual const component_id& id() const = 0;
	public:
		virtual bool shared_data() const = 0;
		virtual const neolib::i_string& name() const = 0;
		virtual uint32_t field_count() const = 0;
		virtual component_data_field_type field_type(uint32_t aFieldIndex) const = 0;
		virtual const neolib::i_string& field_name(uint32_t aFieldIndex) const = 0;
	};
}