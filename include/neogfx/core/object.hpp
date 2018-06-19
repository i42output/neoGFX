// object.hpp
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
#include <neolib/lifetime.hpp>
#include <neogfx/core/i_object.hpp>

namespace neogfx
{
	template <typename Base>
	class object : public Base, public i_properties, protected virtual neolib::lifetime
	{
		// i_object
	public:
		neolib::i_lifetime& as_lifetime() override
		{
			return *this;
		}
	public:
		void property_changed(i_property&) override
		{
			// default is to do nothing
		}
	public:
		const i_properties& properties() const override
		{
			return *this;
		}
		i_properties& properties() override
		{
			return *this;
		}
		// i_properties
	public:
		virtual void register_property(i_property& aProperty)
		{
			iProperties.emplace(aProperty.name(), &aProperty);
		}
		virtual const property_map& map() const
		{
			return iProperties;
		}
		// state
	private:
		property_map iProperties;
	};
}
