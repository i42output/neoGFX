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
#include <neogfx/gui/layout/i_layout_item_proxy.hpp>

namespace neogfx
{
	class layout_item : public i_layout_item_proxy
	{
	public:
		layout_item(i_layout_item& aItem);
		layout_item(std::shared_ptr<i_layout_item> aItem);
		layout_item(const layout_item& aOther);
		~layout_item();
	public:
		bool is_layout() const override;
		const i_layout& as_layout() const override;
		i_layout& as_layout() override;
		bool is_widget() const override;
		const i_widget& as_widget() const override;
		i_widget& as_widget() override;
	public:
		bool is_spacer() const;
	public:
		bool has_parent_layout() const override;
		const i_layout& parent_layout() const override;
		i_layout& parent_layout() override;
		void set_parent_layout(i_layout* aParentLayout);
		bool has_layout_owner() const override;
		const i_widget& layout_owner() const override;
		i_widget& layout_owner() override;
		void set_layout_owner(i_widget* aOwner) override;
		bool is_proxy() const override;
		const i_layout_item_proxy& layout_item_proxy() const override;
		i_layout_item_proxy& layout_item_proxy() override;
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
		bool visible() const override;
	public:
		void layout_as(const point& aPosition, const size& aSize) override;
	public:
		const i_layout_item& subject() const override;
		i_layout_item& subject() override;
		std::shared_ptr<i_layout_item> subject_ptr() override;
	public:
		bool operator==(const layout_item& aOther) const;
	private:
		std::shared_ptr<i_layout_item> iSubject;
		mutable std::pair<uint32_t, uint32_t> iLayoutId;
		mutable size iMinimumSize;
		mutable size iMaximumSize;
	};
}
