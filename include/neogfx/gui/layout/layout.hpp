// layout.hpp
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
#include <list>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/variant.hpp>
#include "i_layout.hpp"
#include "layout_item.hpp"

namespace neogfx
{
	class i_spacer;

	class layout : public i_layout
	{
	public:
		struct widget_already_added : std::logic_error { widget_already_added() : std::logic_error("neogfx::layout::widget_already_added") {} };
	protected:
		typedef layout_item item;
		typedef std::list<item, boost::pool_allocator<item>> item_list;
		enum item_type_e
		{
			ItemTypeNone = 0x00,
			ItemTypeWidget = 0x01,
			ItemTypeLayout = 0x02,
			ItemTypeSpacer = 0x04
		};
	protected:
		template <typename SpecializedPolicy>
		struct common_axis_policy;
		template <typename Layout>
		struct column_major;
		template <typename Layout>
		struct row_major;
	public:
		layout(neogfx::alignment aAlignment = neogfx::alignment::Centre | neogfx::alignment::VCentre);
		layout(i_widget& aParent, neogfx::alignment aAlignment = neogfx::alignment::Centre | neogfx::alignment::VCentre);
		layout(i_layout& aParent, neogfx::alignment aAlignment = neogfx::alignment::Centre | neogfx::alignment::VCentre);
		layout(const layout&) = delete;
		~layout();
	public:
		i_widget* owner() const override;
		void set_owner(i_widget* aOwner) override;
		i_layout* parent() const override;
		void set_parent(i_layout* aParent) override;
		void add(i_widget& aWidget) override;
		void add_at(item_index aPosition, i_widget& aWidget) override;
		void add(std::shared_ptr<i_widget> aWidget) override;
		void add_at(item_index aPosition, std::shared_ptr<i_widget> aWidget) override;
		void add(i_layout& aLayout) override;
		void add_at(item_index aPosition, i_layout& aLayout) override;
		void add(std::shared_ptr<i_layout> aLayout) override;
		void add_at(item_index aPosition, std::shared_ptr<i_layout> aLayout) override;
		void add(i_spacer& aSpacer) override;
		void add_at(item_index aPosition, i_spacer& aSpacer) override;
		void add(std::shared_ptr<i_spacer> aSpacer) override;
		void add_at(item_index aPosition, std::shared_ptr<i_spacer> aSpacer) override;
		void add(const item& aItem) override;
		void remove_at(item_index aIndex) override;
		bool remove(i_layout& aItem) override;
		bool remove(i_widget& aItem) override;
		void remove_all() override;
		item_index count() const override;
		optional_item_index find(i_layout& aItem) const override;
		optional_item_index find(i_widget& aItem) const override;
		optional_item_index find(const layout_item& aItem) const override;
		bool is_widget_at(item_index aIndex) const override;
		const i_widget_geometry& get_item_at(item_index aIndex) const override;
		i_widget_geometry& get_item_at(item_index aIndex) override;
		using i_layout::get_widget_at;
		const i_widget& get_widget_at(item_index aIndex) const override;
		i_widget& get_widget_at(item_index aIndex) override;
		const i_layout& get_layout_at(item_index aIndex) const override;
		i_layout& get_layout_at(item_index aIndex) override;
	public:
		bool has_margins() const override;
		neogfx::margins margins() const override;
		void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true) override;
	public:
		size spacing() const override;
		void set_spacing(const size& aSpacing, bool aUpdateLayout = true) override;
		bool always_use_spacing() const override;
		void set_always_use_spacing(bool aAlwaysUseSpacing) override;
		neogfx::alignment alignment() const override;
		void set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout = true) override;
	public:
		void enable() override;
		void disable() override;
		bool enabled() const override;
		uint32_t layout_id() const override;
		void next_layout_id() override;
		bool invalidated() const override;
		void invalidate() override;
		void validate() override;
	public:
		point position() const override;
		void set_position(const point& aPosition) override;
		size extents() const override;
		void set_extents(const size& aExtents) override;
		bool has_size_policy() const override;
		neogfx::size_policy size_policy() const override;
		void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) override;
		using i_layout::set_size_policy;
		bool has_weight() const override;
		size weight() const override;
		void set_weight(const optional_size& aWeight, bool aUpdateLayout = true) override;
		bool has_minimum_size() const override;
		size minimum_size(const optional_size& aAvailableSpace = optional_size{}) const override;
		void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true) override;
		bool has_maximum_size() const override;
		size maximum_size(const optional_size& aAvailableSpace = optional_size{}) const override;
		void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true) override;
	public:
		bool device_metrics_available() const override;
		const i_device_metrics& device_metrics() const override;
		neogfx::units units() const override;
		neogfx::units set_units(neogfx::units aUnits) const override;
	protected:
		const item_list& items() const;
		item_list& items();
		void remove(item_list::const_iterator aItem);
		const i_widget_geometry& item_geometry(item_list::size_type aItem) const;
		uint32_t spacer_count() const;
		uint32_t items_visible(item_type_e aItemType = static_cast<item_type_e>(ItemTypeWidget|ItemTypeLayout)) const;
		template <typename AxisPolicy>
		size do_minimum_size(const optional_size& aAvailableSpace) const;
		template <typename AxisPolicy>
		size do_maximum_size(const optional_size& aAvailableSpace) const;
		template <typename AxisPolicy>
		void do_layout_items(const point& aPosition, const size& aSize);
	private:
		mutable i_widget* iOwner;
		i_layout* iParent;
		units_context iUnitsContext;
		optional_margins iMargins;
		size iSpacing;
		bool iAlwaysUseSpacing;
		neogfx::alignment iAlignment;
		bool iEnabled;
		point iPosition;
		size iExtents;
		optional_size_policy iSizePolicy;
		optional_size iWeight;
		optional_size iMinimumSize;
		optional_size iMaximumSize;
		item_list iItems;
		bool iLayoutStarted;
		uint32_t iLayoutId;
		bool iInvalidated;
	};
}

#include "layout.inl"