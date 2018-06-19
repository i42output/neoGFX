// i_properties.hpp
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

namespace neogfx
{
	class i_property;

	typedef std::map<std::string, i_property*> property_map;

	class i_properties
	{
	public:
		virtual void register_property(i_property& aProperty) = 0;
		virtual const property_map& map() const = 0;
		// helpers
	public:
		uint32_t count() const
		{
			return property_map().size();
		}
		std::string name(uint32_t aIndex) const
		{
			return std::next(property_map().begin(), aIndex)->first;
		}
		const i_property& property(uint32_t aIndex) const
		{
			return *std::next(property_map().begin(), aIndex)->second;
		}
		i_property& property(uint32_t aIndex)
		{
			return *std::next(property_map().begin(), aIndex)->second;
		}
	};
}
