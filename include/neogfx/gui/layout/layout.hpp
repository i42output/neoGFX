// layout.hpp
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
	private:
		class device_metrics_forwarder : public i_device_metrics
		{
		public:
			device_metrics_forwarder(i_layout& aOwner);
		public:
			virtual bool metrics_available() const;
			virtual size extents() const;
			virtual dimension horizontal_dpi() const;
			virtual dimension vertical_dpi() const;
			virtual dimension em_size() const;
		private:
			i_layout& iOwner;
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
		virtual i_widget* owner() const;
		virtual void set_owner(i_widget* aOwner);
		virtual i_layout* parent() const;
		virtual void set_parent(i_layout* aParent);
		virtual void add_item(i_widget& aWidget);
		virtual void add_item_at(item_index aPosition, i_widget& aWidget);
		virtual void add_item(std::shared_ptr<i_widget> aWidget);
		virtual void add_item_at(item_index aPosition, std::shared_ptr<i_widget> aWidget);
		virtual void add_item(i_layout& aLayout);
		virtual void add_item_at(item_index aPosition, i_layout& aLayout);
		virtual void add_item(std::shared_ptr<i_layout> aLayout);
		virtual void add_item_at(item_index aPosition, std::shared_ptr<i_layout> aLayout);
		virtual void add_item(i_spacer& aSpacer);
		virtual void add_item_at(item_index aPosition, i_spacer& aSpacer);
		virtual void add_item(std::shared_ptr<i_spacer> aSpacer);
		virtual void add_item_at(item_index aPosition, std::shared_ptr<i_spacer> aSpacer);
		virtual void add_item(const item& aItem);
		virtual void remove_item_at(item_index aIndex);
		virtual bool remove_item(i_layout& aItem);
		virtual bool remove_item(i_widget& aItem);
		virtual void remove_items();
		virtual item_index item_count() const;
		virtual optional_item_index find_item(i_layout& aItem) const;
		virtual optional_item_index find_item(i_widget& aItem) const;
		virtual optional_item_index find_item(const layout_item& aItem) const;
		virtual bool is_widget_at(item_index aIndex) const;
		virtual const i_widget_geometry& get_item_at(item_index aIndex) const;
		virtual i_widget_geometry& get_item_at(item_index aIndex);
		using i_layout::get_widget_at;
		virtual const i_widget& get_widget_at(item_index aIndex) const;
		virtual i_widget& get_widget_at(item_index aIndex);
		virtual const i_layout& get_layout_at(item_index aIndex) const;
		virtual i_layout& get_layout_at(item_index aIndex);
	public:
		virtual bool has_margins() const;
		virtual neogfx::margins margins() const;
		virtual void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true);
	public:
		virtual size spacing() const;
		virtual void set_spacing(const size& aSpacing, bool aUpdateLayout = true);
		virtual bool always_use_spacing() const;
		virtual void set_always_use_spacing(bool aAlwaysUseSpacing);
		virtual neogfx::alignment alignment() const;
		virtual void set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout = true);
	public:
		virtual void enable();
		virtual void disable();
		virtual bool enabled() const;
		virtual uint32_t layout_id() const;
		virtual void next_layout_id();
		virtual bool invalidated() const;
		virtual void invalidate();
		virtual void validate();
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
		virtual const i_device_metrics& device_metrics() const;
		virtual units_e units() const;
		virtual units_e set_units(units_e aUnits) const;
	protected:
		const item_list& items() const;
		item_list& items();
		void remove_item(item_list::const_iterator aItem);
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
		device_metrics_forwarder iDeviceMetricsForwarder;
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