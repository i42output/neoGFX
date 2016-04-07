// i_layout.hpp
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
#include "i_geometry.hpp"
#include "event.hpp"

namespace neogfx
{
	class i_widget;
	class i_spacer;

	class layout_item;

	class i_layout : public i_geometry, public i_units_context
	{
	public:
		event<> alignment_changed;
	protected:
		class item;
	public:
		struct no_widget : std::logic_error { no_widget() : std::logic_error("neogfx::i_layout::no_widget") {} };
		struct wrong_item_type : std::logic_error { wrong_item_type() : std::logic_error("neogfx::i_layout::wrong_item_type") {} };
	public:
		virtual ~i_layout() {}
	public:
		virtual i_widget* owner() const = 0;
		virtual void set_owner(i_widget* aOwner) = 0;
		virtual void add_item(i_widget& aWidget) = 0;
		virtual void add_item(uint32_t aPosition, i_widget& aWidget) = 0;
		virtual void add_item(std::shared_ptr<i_widget> aWidget) = 0;
		virtual void add_item(uint32_t aPosition, std::shared_ptr<i_widget> aWidget) = 0;
		virtual void add_item(i_layout& aLayout) = 0;
		virtual void add_item(uint32_t aPosition, i_layout& aLayout) = 0;
		virtual void add_item(std::shared_ptr<i_layout> aLayout) = 0;
		virtual void add_item(uint32_t aPosition, std::shared_ptr<i_layout> aLayout) = 0;
		virtual void add_item(i_spacer& aSpacer) = 0;
		virtual void add_item(uint32_t aPosition, i_spacer& aSpacer) = 0;
		virtual void add_item(std::shared_ptr<i_spacer> aSpacer) = 0;
		virtual void add_item(uint32_t aPosition, std::shared_ptr<i_spacer> aSpacer) = 0;
		virtual void add_item(const	layout_item& aItem) = 0;
		virtual i_spacer& add_spacer() = 0;
		virtual i_spacer& add_spacer(uint32_t aPosition) = 0;
		virtual void remove_item(std::size_t aIndex) = 0;
		virtual void remove_items() = 0;
		virtual std::size_t item_count() const = 0;
		virtual i_geometry& get_item(std::size_t aIndex) = 0;
		virtual i_widget& get_widget(std::size_t aIndex) = 0;
		virtual i_layout& get_layout(std::size_t aIndex) = 0;
	public:
		virtual size spacing() const = 0;
		virtual void set_spacing(const size& sSpacing) = 0;
		virtual bool always_use_spacing() const = 0;
		virtual void set_always_use_spacing(bool aAlwaysUseSpacing) = 0;
		virtual neogfx::alignment alignment() const = 0;
		virtual void set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout = true) = 0;
	public:
		virtual void enable() = 0;
		virtual void disable() = 0;
		virtual bool enabled() const = 0;
		virtual void layout_items(const point& aPosition, const size& aSize) = 0;
		virtual uint32_t layout_id() const = 0;
		virtual void next_layout_id() = 0;
		// helpers
	public:
		template <typename ItemType>
		void replace_item(uint32_t aPosition, ItemType&& aItem)
		{
			if (aPosition < item_count())
				remove_item(aPosition);
			add_item(aPosition, aItem);
		}
		template <typename WidgetT>
		WidgetT& get_widget(std::size_t aIndex)
		{
			return static_cast<WidgetT&>(get_widget(aIndex));
		}
	};
}