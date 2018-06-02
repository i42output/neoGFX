// layout_item.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
		struct no_parent : std::logic_error { no_parent() : std::logic_error("neogfx::layout_item::no_parent") {} };
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
		const i_layout& parent() const;
		i_layout& parent();
		void set_parent(i_layout* aParent);
		void set_owner(i_widget* aOwner);
		void layout(const point& aPosition, const size& aSize);
	public:
		bool high_dpi() const override;
		dimension dpi_scale_factor() const override;
	public:
		bool device_metrics_available() const override;
		const i_device_metrics& device_metrics() const override;
		neogfx::units units() const override;
		neogfx::units set_units(neogfx::units aUnits) const override;
	public:
		point position() const override;
		void set_position(const point& aPosition) override;
		size extents() const override;
		void set_extents(const size& aExtents) override;
		bool has_size_policy() const override;
		neogfx::size_policy size_policy() const override;
		void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) override;
		bool has_weight() const override;
		size weight() const override;
		void set_weight(const optional_size& aWeight, bool aUpdateLayout = true) override;
		bool has_minimum_size() const override;
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true) override;
		bool has_maximum_size() const override;
		size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true) override;
	public:
		bool has_margins() const override;
		neogfx::margins margins() const override;
		void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true) override;
	public:
		bool visible() const;
	public:
		bool operator==(const layout_item& aOther) const;
	private:
		i_layout* iParent;
		pointer_wrapper iPointerWrapper;
		i_widget* iOwner;
		mutable std::pair<uint32_t, uint32_t> iLayoutId;
		mutable size iMinimumSize;
		mutable size iMaximumSize;
	};
}
