// i_widget_geometry.hpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/core/geometry.hpp>

namespace neogfx
{
	class size_policy
	{
	public:
		enum size_policy_e
		{
			Fixed,
			Minimum,
			Maximum,
			Expanding,
			Manual
		};
	public:
		size_policy(size_policy_e aSizePolicy) :
			iHorizontalSizePolicy(aSizePolicy), iVerticalSizePolicy(aSizePolicy)
		{
		}
		size_policy(size_policy_e aHorizontalSizePolicy, size_policy_e aVerticalSizePolicy) :
			iHorizontalSizePolicy(aHorizontalSizePolicy), iVerticalSizePolicy(aVerticalSizePolicy)
		{
		}
	public:
		bool operator==(const size_policy& aRhs) const
		{
			return iHorizontalSizePolicy == aRhs.iHorizontalSizePolicy && iVerticalSizePolicy == aRhs.iVerticalSizePolicy;
		}
		bool operator!=(const size_policy& aRhs) const
		{
			return !(*this == aRhs);
		}
	public:
		size_policy_e horizontal_size_policy() const
		{
			return iHorizontalSizePolicy;
		}
		size_policy_e vertical_size_policy() const
		{
			return iVerticalSizePolicy;
		}
		void set_size_policy(size_policy_e aSizePolicy)
		{
			iHorizontalSizePolicy = aSizePolicy;
			iVerticalSizePolicy = aSizePolicy;
		}
		void set_horizontal_size_policy(size_policy_e aHorizontalSizePolicy)
		{
			iHorizontalSizePolicy = aHorizontalSizePolicy;
		}
		void set_vertical_size_policy(size_policy_e aVerticalSizePolicy)
		{
			iVerticalSizePolicy = aVerticalSizePolicy;
		}
	private:
		size_policy_e iHorizontalSizePolicy;
		size_policy_e iVerticalSizePolicy;
	};

	typedef boost::optional<size_policy> optional_size_policy;

	class i_widget_geometry
	{
	public:
		virtual point position() const = 0;
		virtual void set_position(const point& aPosition) = 0;
		virtual size extents() const = 0;
		virtual void set_extents(const size& aExtents) = 0;
		virtual bool has_size_policy() const = 0;
		virtual neogfx::size_policy size_policy() const = 0;
		virtual void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) = 0;
		virtual bool has_weight() const = 0;
		virtual size weight() const = 0;
		virtual void set_weight(const optional_size& aWeight, bool aUpdateLayout = true) = 0;
		virtual bool has_minimum_size() const = 0;
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size{}) const = 0;
		virtual void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true) = 0;
		virtual bool has_maximum_size() const = 0;
		virtual size maximum_size(const optional_size& aAvailableSpace = optional_size{}) const = 0;
		virtual void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true) = 0;
	public:
		virtual bool has_margins() const = 0;
		virtual neogfx::margins margins() const = 0;
		virtual void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true) = 0;
		// helpers
	public:
		void set_size_policy(neogfx::size_policy::size_policy_e aSizePolicy, bool aUpdateLayout = true)
		{
			set_size_policy(neogfx::size_policy{aSizePolicy}, aUpdateLayout);
		}
		void set_size_policy(neogfx::size_policy::size_policy_e aHorizontalSizePolicy, neogfx::size_policy::size_policy_e aVerticalSizePolicy, bool aUpdateLayout = true)
		{
			set_size_policy(neogfx::size_policy{aHorizontalSizePolicy, aVerticalSizePolicy}, aUpdateLayout);
		}
		void set_fixed_size(const size& aSize, bool aUpdateLayout = true)
		{
			set_minimum_size(aSize, aUpdateLayout);
			set_maximum_size(aSize, aUpdateLayout);
		}
		void set_minimum_width(dimension aWidth, bool aUpdateLayout = true)
		{
			auto newSize = minimum_size();
			newSize.cx = aWidth;
			set_minimum_size(newSize, aUpdateLayout);
		}
		void set_minimum_height(dimension aHeight, bool aUpdateLayout = true)
		{
			auto newSize = minimum_size();
			newSize.cy = aHeight;
			set_minimum_size(newSize, aUpdateLayout);
		}
		void set_maximum_width(dimension aWidth, bool aUpdateLayout = true)
		{
			auto newSize = maximum_size();
			newSize.cx = aWidth;
			set_maximum_size(newSize, aUpdateLayout);
		}
		void set_maximum_height(dimension aHeight, bool aUpdateLayout = true)
		{
			auto newSize = maximum_size();
			newSize.cy = aHeight;
			set_maximum_size(newSize, aUpdateLayout);
		}
	};
}