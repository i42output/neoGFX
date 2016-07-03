// layout.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/layout.hpp>
#include <neogfx/gui/layout/layout_item.hpp>
#include <neogfx/gui/layout/i_spacer.hpp>
#include <neogfx/app/app.hpp>

namespace neogfx
{
	layout::device_metrics_forwarder::device_metrics_forwarder(i_layout& aOwner) :
		iOwner(aOwner)
	{
	}

	size layout::device_metrics_forwarder::extents() const
	{
		if (iOwner.owner() == 0)
			throw no_widget();
		return iOwner.owner()->device_metrics().extents();
	}

	dimension layout::device_metrics_forwarder::horizontal_dpi() const
	{
		if (iOwner.owner() == 0)
			throw no_widget();
		return iOwner.owner()->device_metrics().horizontal_dpi();
	}

	dimension layout::device_metrics_forwarder::vertical_dpi() const
	{
		if (iOwner.owner() == 0)
			throw no_widget();
		return iOwner.owner()->device_metrics().vertical_dpi();
	}

	dimension layout::device_metrics_forwarder::em_size() const
	{
		if (iOwner.owner() == 0)
			throw no_widget();
		return iOwner.owner()->device_metrics().em_size();
	}

	layout::layout(neogfx::alignment aAlignment) :
		iOwner(0),
		iParent(0),
		iDeviceMetricsForwarder(*this), iUnitsContext(iDeviceMetricsForwarder),
		iSpacing(app::instance().current_style().spacing()),
		iAlwaysUseSpacing(false),
		iAlignment(aAlignment),
		iEnabled(true),
		iMinimumSize{},
		iMaximumSize{},
		iLayoutStarted(false),
		iLayoutId(0)
	{
	}

	layout::layout(i_widget& aParent, neogfx::alignment aAlignment) :
		iOwner(&aParent),
		iParent(0),
		iDeviceMetricsForwarder(*this), iUnitsContext(iDeviceMetricsForwarder),
		iSpacing(app::instance().current_style().spacing()),
		iAlwaysUseSpacing(false),
		iAlignment(aAlignment),
		iEnabled(true),
		iMinimumSize{},
		iMaximumSize{},
		iLayoutStarted(false),
		iLayoutId(0)
	{
		aParent.set_layout(*this);
	}

	layout::layout(i_layout& aParent, neogfx::alignment aAlignment) :
		iOwner(aParent.owner()), 
		iParent(&aParent),
		iDeviceMetricsForwarder(*this), iUnitsContext(iDeviceMetricsForwarder),
		iMargins(neogfx::margins(0)),
		iSpacing(app::instance().current_style().spacing()),
		iAlwaysUseSpacing(false),
		iAlignment(aAlignment),
		iEnabled(true),
		iMinimumSize{},
		iMaximumSize{},
		iLayoutStarted(false),
		iLayoutId(0)
	{
		aParent.add_item(*this);
	}

	layout::~layout()
	{
		remove_items();
		if (iParent != 0)
			iParent->remove_item(*this);
		if (owner() && owner()->has_layout() && &owner()->layout() == this)
			owner()->set_layout(nullptr);
	}

	i_widget* layout::owner() const
	{
		return iOwner;
	}

	void layout::set_owner(i_widget* aOwner)
	{
		iOwner = aOwner;
	}

	i_layout* layout::parent() const
	{
		return iParent;
	}
	
	void layout::set_parent(i_layout* aParent)
	{
		iParent = aParent;
	}

	void layout::add_item(i_widget& aWidget)
	{
		if (aWidget.has_layout() && &aWidget.layout() == this)
			throw widget_already_added();
		iItems.push_back(item(*this, aWidget));
		if (iOwner != 0)
			iItems.back().set_owner(iOwner);
	}

	void layout::add_item(uint32_t aPosition, i_widget& aWidget)
	{
		if (aWidget.has_layout() && &aWidget.layout() == this)
			throw widget_already_added();
		while (aPosition > iItems.size())
			add_spacer(0);
		auto i = iItems.insert(std::next(iItems.begin(), aPosition), item(*this, aWidget));
		if (iOwner != 0)
			i->set_owner(iOwner);
	}

	void layout::add_item(std::shared_ptr<i_widget> aWidget)
	{
		if (aWidget->has_layout() && &aWidget->layout() == this)
			throw widget_already_added();
		iItems.push_back(item(*this, aWidget));
		if (iOwner != 0)
			iItems.back().set_owner(iOwner);
	}

	void layout::add_item(uint32_t aPosition, std::shared_ptr<i_widget> aWidget)
	{
		if (aWidget->has_layout() && &aWidget->layout() == this)
			throw widget_already_added();
		while (aPosition > iItems.size())
			add_spacer(0);
		auto i = iItems.insert(std::next(iItems.begin(), aPosition), item(*this, aWidget));
		if (iOwner != 0)
			i->set_owner(iOwner);
	}

	void layout::add_item(i_layout& aLayout)
	{
		iItems.push_back(item(*this, aLayout));
		if (iOwner != 0)
			iItems.back().set_owner(iOwner);
		aLayout.set_parent(this);
	}

	void layout::add_item(uint32_t aPosition, i_layout& aLayout)
	{
		while (aPosition > iItems.size())
			add_spacer(0);
		auto i = iItems.insert(std::next(iItems.begin(), aPosition), item(*this, aLayout));
		if (iOwner != 0)
			i->set_owner(iOwner);
		aLayout.set_parent(this);
	}

	void layout::add_item(std::shared_ptr<i_layout> aLayout)
	{
		iItems.push_back(item(*this, aLayout));
		if (iOwner != 0)
			iItems.back().set_owner(iOwner);
		aLayout->set_parent(this);
	}

	void layout::add_item(uint32_t aPosition, std::shared_ptr<i_layout> aLayout)
	{
		while (aPosition > iItems.size())
			add_spacer(0);
		auto i = iItems.insert(std::next(iItems.begin(), aPosition), item(*this, aLayout));
		if (iOwner != 0)
			i->set_owner(iOwner);
		aLayout->set_parent(this);
	}

	void layout::add_item(i_spacer& aSpacer)
	{
		iItems.push_back(item(*this, aSpacer));
		if (iOwner != 0)
			iItems.back().set_owner(iOwner);
		aSpacer.set_parent(*this);
	}

	void layout::add_item(uint32_t aPosition, i_spacer& aSpacer)
	{
		while (aPosition > iItems.size())
			add_spacer(0);
		auto i = iItems.insert(std::next(iItems.begin(), aPosition), item(*this, aSpacer));
		if (iOwner != 0)
			i->set_owner(iOwner);
		aSpacer.set_parent(*this);
	}

	void layout::add_item(std::shared_ptr<i_spacer> aSpacer)
	{
		iItems.push_back(item(*this, aSpacer));
		if (iOwner != 0)
			iItems.back().set_owner(iOwner);
		aSpacer->set_parent(*this);
	}

	void layout::add_item(uint32_t aPosition, std::shared_ptr<i_spacer> aSpacer)
	{
		while (aPosition > iItems.size())
			add_spacer(0);
		auto i = iItems.insert(std::next(iItems.begin(), aPosition), item(*this, aSpacer));
		if (iOwner != 0)
			i->set_owner(iOwner);
		aSpacer->set_parent(*this);
	}

	void layout::add_item(const item& aItem)
	{
		iItems.push_back(aItem);
		if (iOwner != 0)
			iItems.back().set_owner(iOwner);
	}

	void layout::remove_item(item_index aIndex)
	{
		remove_item(std::next(iItems.begin(), aIndex));
	}

	bool layout::remove_item(i_layout& aItem)
	{
		for (auto i = items().begin(); i != items().end(); ++i)
			if (i->get().is<item::layout_pointer>() && &aItem == &*static_variant_cast<item::layout_pointer&>(i->get()))
			{
				remove_item(i);
				return true;
			}
		return false;
	}

	bool layout::remove_item(i_widget& aItem)
	{
		for (auto i = items().begin(); i != items().end(); ++i)
			if (i->get().is<item::widget_pointer>() && &aItem == &*static_variant_cast<item::widget_pointer&>(i->get()))
			{
				remove_item(i);
				return true;
			}
		for (auto i = items().begin(); i != items().end(); ++i)
			if (i->get().is<item::layout_pointer>())
			{
				if (static_variant_cast<item::layout_pointer&>(i->get())->remove_item(aItem))
					return true;
			}
		return false;
	}

	void layout::remove_items()
	{
		item_list toRemove;
		toRemove.splice(toRemove.begin(), items());
		if (iOwner != 0)
			iOwner->ultimate_ancestor().layout_items(true);
	}

	layout::item_index layout::item_count() const
	{
		return iItems.size();
	}

	layout::optional_item_index layout::find_item(i_layout& aItem) const
	{
		for (auto i = iItems.begin(); i != iItems.end(); ++i)
		{
			const auto& item = *i;
			if (item.get().is<item::layout_pointer>() && &*static_variant_cast<item::layout_pointer>(item.get()) == &aItem)
				return std::distance(iItems.begin(), i);
		}
		return optional_item_index();
	}

	layout::optional_item_index layout::find_item(i_widget& aItem) const
	{
		for (auto i = iItems.begin(); i != iItems.end(); ++i)
		{
			const auto& item = *i;
			if (item.get().is<item::widget_pointer>() && &*static_variant_cast<item::widget_pointer>(item.get()) == &aItem)
				return std::distance(iItems.begin(), i);
		}
		return optional_item_index();
	}

	layout::optional_item_index layout::find_item(const layout_item& aItem) const
	{
		for (auto i = iItems.begin(); i != iItems.end(); ++i)
		{
			if (aItem == *i)
				return std::distance(iItems.begin(), i);
		}
		return optional_item_index();
	}

	bool layout::is_widget(item_index aIndex) const
	{
		if (aIndex >= iItems.size())
			throw bad_item_index();
		auto item = std::next(iItems.begin(), aIndex);
		return item->get().is<item::widget_pointer>();
	}

	const i_widget_geometry& layout::get_item(item_index aIndex) const
	{
		if (aIndex >= iItems.size())
			throw bad_item_index();
		auto item = std::next(iItems.begin(), aIndex);
		if (item->get().is<item::widget_pointer>())
			return *static_variant_cast<const item::widget_pointer&>(item->get());
		else if (item->get().is<item::layout_pointer>())
			return *static_variant_cast<const item::layout_pointer&>(item->get());
		else
			throw wrong_item_type();
	}

	i_widget_geometry& layout::get_item(item_index aIndex)
	{
		return const_cast<i_widget_geometry&>(const_cast<const layout*>(this)->get_item(aIndex));
	}
		
	const i_widget& layout::get_widget(item_index aIndex) const
	{
		if (aIndex >= iItems.size())
			throw bad_item_index();
		auto item = std::next(iItems.begin(), aIndex);
		if (item->get().is<item::widget_pointer>())
			return *static_variant_cast<const item::widget_pointer&>(item->get());
		else
			throw wrong_item_type();
	}

	i_widget& layout::get_widget(item_index aIndex)
	{
		return const_cast<i_widget&>(const_cast<const layout*>(this)->get_widget(aIndex));
	}
		
	const i_layout& layout::get_layout(item_index aIndex) const
	{
		if (aIndex >= iItems.size())
			throw bad_item_index();
		auto item = std::next(iItems.begin(), aIndex);
		if (item->get().is<item::layout_pointer>())
			return *static_variant_cast<const item::layout_pointer&>(item->get());
		else
			throw wrong_item_type();
	}

	i_layout& layout::get_layout(item_index aIndex)
	{
		return const_cast<i_layout&>(const_cast<const layout*>(this)->get_layout(aIndex));
	}
		
	bool layout::has_margins() const
	{
		return iMargins != boost::none;
	}

	margins layout::margins() const
	{
		return units_converter(*this).from_device_units(has_margins() ? *iMargins : app::instance().current_style().margins());
	}

	void layout::set_margins(const optional_margins& aMargins, bool aUpdateLayout)
	{
		optional_margins newMargins = (aMargins != boost::none ? units_converter(*this).to_device_units(*aMargins) : optional_margins());
		if (iMargins != newMargins)
		{
			iMargins = newMargins;
			if (iOwner != 0 && aUpdateLayout)
				iOwner->ultimate_ancestor().layout_items(true);
		}
	}

	size layout::spacing() const
	{
		return units_converter(*this).from_device_units(iSpacing);
	}

	void layout::set_spacing(const size& aSpacing)
	{
		if (iSpacing != aSpacing)
		{
			iSpacing = units_converter(*this).to_device_units(aSpacing);
			if (iOwner != 0)
				iOwner->ultimate_ancestor().layout_items(true);
		}
	}

	bool layout::always_use_spacing() const
	{
		return iAlwaysUseSpacing;
	}

	void layout::set_always_use_spacing(bool aAlwaysUseSpacing)
	{
		iAlwaysUseSpacing = aAlwaysUseSpacing;
	}

	neogfx::alignment layout::alignment() const
	{
		return iAlignment;
	}

	void layout::set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout)
	{
		if (iAlignment != aAlignment)
		{
			iAlignment = aAlignment;
			alignment_changed.trigger();
			if (iOwner != 0 && aUpdateLayout)
				iOwner->ultimate_ancestor().layout_items(true);
		}
	}

	void layout::enable()
	{
		if (iEnabled == false)
		{
			iEnabled = true;
			owner()->layout_items();
		}
	}

	void layout::disable()
	{
		if (iEnabled == true)
		{
			iEnabled = false;
		}
	}

	bool layout::enabled() const
	{
		return iEnabled;
	}

	uint32_t layout::layout_id() const
	{
		return iLayoutId;
	}

	void layout::next_layout_id()
	{
		if (++iLayoutId == static_cast<uint32_t>(-1))
			iLayoutId = 0;
		for (auto& item : items())
			if (item.get().is<item::widget_pointer>())
			{
				auto& widget = *static_variant_cast<item::widget_pointer&>(item.get());
				if (widget.has_layout())
					widget.layout().next_layout_id();
			}
			else if (item.get().is<item::layout_pointer>())
				static_variant_cast<item::layout_pointer&>(item.get())->next_layout_id();
	}

	point layout::position() const
	{
		return units_converter(*this).from_device_units(iPosition);
	}
	
	void layout::set_position(const point& aPosition)
	{
		iPosition = units_converter(*this).to_device_units(aPosition);
	}

	size layout::extents() const
	{
		return units_converter(*this).from_device_units(iExtents);
	}

	void layout::set_extents(const size& aExtents)
	{
		iExtents = units_converter(*this).to_device_units(aExtents);
	}

	bool layout::has_size_policy() const
	{
		return iSizePolicy != boost::none;
	}

	size_policy layout::size_policy() const
	{
		if (has_size_policy())
			return *iSizePolicy;
		neogfx::size_policy result{ neogfx::size_policy::Minimum, neogfx::size_policy::Minimum };
		for (auto& i : items())
		{
			if (i.get().is<item::spacer_pointer>())
				continue;
			if (i.size_policy().horizontal_size_policy() == neogfx::size_policy::Expanding)
				result.set_horizontal_size_policy(neogfx::size_policy::Expanding);
			if (i.size_policy().vertical_size_policy() == neogfx::size_policy::Expanding)
				result.set_vertical_size_policy(neogfx::size_policy::Expanding);
		}
		return result;
	}

	void layout::set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout)
	{
		if (iSizePolicy != aSizePolicy)
		{
			iSizePolicy = aSizePolicy;
			if (iOwner != 0 && aUpdateLayout)
				iOwner->ultimate_ancestor().layout_items(true);
		}
	}

	bool layout::has_weight() const
	{
		return iWeight != boost::none;
	}

	size layout::weight() const
	{
		if (has_weight())
			return *iWeight;
		else
			return 1.0;
	}

	void layout::set_weight(const optional_size& aWeight, bool aUpdateLayout)
	{
		if (iWeight != aWeight)
		{
			iWeight = aWeight;
			if (iOwner != 0 && aUpdateLayout)
				iOwner->ultimate_ancestor().layout_items(true);
		}
	}

	bool layout::has_minimum_size() const
	{
		return iMinimumSize != boost::none;
	}

	size layout::minimum_size(const optional_size&) const
	{
		return has_minimum_size() ?
			units_converter(*this).from_device_units(*iMinimumSize) :
			size{};
	}

	void layout::set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout)
	{
		optional_size newMinimumSize = (aMinimumSize != boost::none ? units_converter(*this).to_device_units(*aMinimumSize) : optional_size());
		if (iMinimumSize != newMinimumSize)
		{
			iMinimumSize = newMinimumSize;
			if (iOwner != 0 && aUpdateLayout)
				iOwner->ultimate_ancestor().layout_items(true);
		}
	}

	bool layout::has_maximum_size() const
	{
		return iMaximumSize != boost::none;
	}

	size layout::maximum_size(const optional_size&) const
	{
		return has_maximum_size() ?
			units_converter(*this).from_device_units(*iMaximumSize) :
			size(std::numeric_limits<size::dimension_type>::max(), std::numeric_limits<size::dimension_type>::max());
	}

	void layout::set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout)
	{
		optional_size newMaximumSize = (aMaximumSize != boost::none ? units_converter(*this).to_device_units(*aMaximumSize) : optional_size());
		if (iMaximumSize != newMaximumSize)
		{
			iMaximumSize = newMaximumSize;
			if (iOwner != 0 && aUpdateLayout)
				iOwner->ultimate_ancestor().layout_items(true);
		}
	}

	const i_device_metrics& layout::device_metrics() const
	{
		return iDeviceMetricsForwarder;
	}

	units_e layout::units() const
	{
		return iUnitsContext.units();
	}

	units_e layout::set_units(units_e aUnits) const
	{
		return iUnitsContext.set_units(aUnits);
	}

	const layout::item_list& layout::items() const
	{
		return iItems;
	}

	layout::item_list& layout::items()
	{
		return iItems;
	}

	uint32_t layout::spacer_count() const
	{
		uint32_t count = 0u;
		for (const auto& i : iItems)
			if (i.get().is<item::spacer_pointer>())
				++count;
		return count;
	}

	void layout::remove_item(item_list::const_iterator aItem)
	{
		item_list toRemove;
		toRemove.splice(toRemove.begin(), items(), aItem);
		if (iOwner != 0)
			iOwner->ultimate_ancestor().layout_items(true);
	}

	uint32_t layout::items_visible(item_type_e aItemType) const
	{
		uint32_t count = 0u;
		for (const auto& i : iItems)
			if (i.visible())
			{
				if ((aItemType & ItemTypeWidget) && i.get().is<item::widget_pointer>())
					++count;
				else if ((aItemType & ItemTypeLayout) && i.get().is<item::layout_pointer>())
					++count;
				else if ((aItemType & ItemTypeSpacer) && i.get().is<item::spacer_pointer>())
					++count;
			}
		return count;
	}
}