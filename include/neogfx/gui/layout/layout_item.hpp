// layout_item.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present Leigh Johnston
  
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
#include <neolib/variant.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/i_layout.hpp>

namespace neogfx
{
	class layout_item : public i_widget_geometry
	{
	public:
		typedef std::shared_ptr<i_widget> widget_pointer;
		typedef std::shared_ptr<i_layout> layout_pointer;
		typedef std::shared_ptr<i_spacer> spacer_pointer;
		typedef neolib::variant<widget_pointer, layout_pointer, spacer_pointer> pointer_wrapper;
	public:
		layout_item(i_layout& aParent, i_widget& aWidget);
		layout_item(i_layout& aParent, std::shared_ptr<i_widget> aWidget);
		layout_item(i_layout& aParent, i_layout& aLayout);
		layout_item(i_layout& aParent, std::shared_ptr<i_layout> aLayout);
		layout_item(i_layout& aParent, i_spacer& aSpacer);
		layout_item(i_layout& aParent, std::shared_ptr<i_spacer> aSpacer);
	public:
		const pointer_wrapper& get() const;
		pointer_wrapper& get();
		const i_widget_geometry& wrapped_geometry() const;
		i_widget_geometry& wrapped_geometry();
		void set_owner(i_widget* aOwner);
		void layout(const point& aPosition, const size& aSize);
	public:
		virtual point position() const;
		virtual void set_position(const point& aPosition);
		virtual size extents() const;
		virtual void set_extents(const size& aExtents);
		virtual bool has_size_policy() const;
		virtual neogfx::size_policy size_policy() const;
		virtual void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true);
		virtual bool has_weight() const;
		virtual size weight() const;
		virtual void set_weight(const optional_size& aWeight, bool aUpdateLayout = true);
		virtual bool has_minimum_size() const;
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
		virtual void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true);
		virtual bool has_maximum_size() const;
		virtual size maximum_size(const optional_size& aAvailableSpace = optional_size()) const;
		virtual void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true);
	public:
		virtual bool has_margins() const;
		virtual neogfx::margins margins() const;
		virtual void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true);
	public:
		bool visible() const;
	public:
		bool operator==(const layout_item& aOther) const;
	private:
		i_layout& iParent;
		pointer_wrapper iPointerWrapper;
		i_widget* iOwner;
		mutable std::pair<uint32_t, uint32_t> iLayoutId;
		mutable size iMinimumSize;
		mutable size iMaximumSize;
	};
}
