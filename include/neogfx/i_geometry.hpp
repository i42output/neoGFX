// i_geometry.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include "geometry.hpp"

namespace neogfx
{
	enum class size_policy
	{
		Minimum,
		Maximum,
		Expanding
	};

	typedef boost::optional<size_policy> optional_size_policy;

	class i_geometry
	{
	public:
		virtual bool has_size_policy() const = 0;
		virtual neogfx::size_policy size_policy() const = 0;
		virtual void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) = 0;
		virtual bool has_weight() const = 0;
		virtual size weight() const = 0;
		virtual void set_weight(const optional_size& aWeight, bool aUpdateLayout = true) = 0;
		virtual bool has_minimum_size() const = 0;
		virtual size minimum_size() const = 0;
		virtual void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true) = 0;
		virtual bool has_maximum_size() const = 0;
		virtual size maximum_size() const = 0;
		virtual void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true) = 0;
		virtual bool is_fixed_size() const = 0;
		virtual void set_fixed_size(const optional_size& aFixedSize, bool aUpdateLayout = true) = 0;
	public:
		virtual bool has_margins() const = 0;
		virtual neogfx::margins margins() const = 0;
		virtual void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true) = 0;
	};
}