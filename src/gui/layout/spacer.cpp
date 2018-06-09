// spacer.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>

namespace neogfx
{
	spacer::spacer(expansion_policy_e aExpansionPolicy) :
		iParentLayout{ nullptr }, 
		iUnitsContext{ *this }, 
		iExpansionPolicy{ aExpansionPolicy }
	{
	}

	spacer::spacer(i_layout& aParentLayout, expansion_policy_e aExpansionPolicy) :
		iParentLayout{ nullptr }, 
		iUnitsContext{ *this },
		iExpansionPolicy{ aExpansionPolicy }
	{
		aParentLayout.add(*this);
	}

	spacer::~spacer()
	{
		if (has_parent_layout())
			parent_layout().remove(*this);
	}

	bool spacer::is_layout() const
	{
		return false;
	}

	const i_layout& spacer::as_layout() const
	{
		throw not_a_layout();
	}

	i_layout& spacer::as_layout()
	{
		throw not_a_layout();
	}

	bool spacer::is_widget() const
	{
		return false;
	}

	const i_widget& spacer::as_widget() const
	{
		throw not_a_widget();
	}

	i_widget& spacer::as_widget()
	{
		throw not_a_widget();
	}

	bool spacer::has_parent_layout() const
	{
		return iParentLayout != nullptr;
	}

	const i_layout& spacer::parent_layout() const
	{
		if (has_parent_layout())
			return *iParentLayout;
		throw no_parent_layout();
	}

	i_layout& spacer::parent_layout()
	{
		return const_cast<i_layout&>(const_cast<const spacer*>(this)->parent_layout());
	}

	void spacer::set_parent_layout(i_layout* aParentLayout)
	{
		iParentLayout = aParentLayout;
	}

	bool spacer::has_layout_owner() const
	{
		if (has_parent_layout())
			return parent_layout().has_layout_owner();
		else
			return false;
	}

	const i_widget& spacer::layout_owner() const
	{
		if (has_layout_owner())
			return parent_layout().layout_owner();
		throw no_layout_owner();
	}

	i_widget& spacer::layout_owner()
	{
		return const_cast<i_widget&>(const_cast<const spacer*>(this)->layout_owner());
	}

	void spacer::set_layout_owner(i_widget*)
	{
		// do nothing
	}

	bool spacer::is_proxy() const
	{
		return false;
	}

	const i_layout_item_proxy& spacer::layout_item_proxy() const
	{
		return parent_layout().find_proxy(*this);
	}

	i_layout_item_proxy& spacer::layout_item_proxy()
	{
		return parent_layout().find_proxy(*this);
	}

	spacer::expansion_policy_e spacer::expansion_policy() const
	{
		return iExpansionPolicy;
	}

	void spacer::set_expansion_policy(expansion_policy_e aExpansionPolicy)
	{
		if (iExpansionPolicy != aExpansionPolicy)
		{
			iExpansionPolicy = aExpansionPolicy;
			if (has_layout_owner())
				layout_owner().ultimate_ancestor().layout_items(true);
		}
	}

	size spacer::extents() const
	{
		return units_converter(*this).from_device_units(iExtents);
	}

	bool spacer::high_dpi() const
	{
		return has_layout_owner() && layout_owner().has_surface() ? 
			layout_owner().surface().ppi() >= 150.0 : 
			app::instance().surface_manager().display().metrics().ppi() >= 150.0;
	}

	dimension spacer::dpi_scale_factor() const
	{
		return default_dpi_scale_factor(
			has_layout_owner() && layout_owner().has_surface() ?
				layout_owner().surface().ppi() : 
				app::instance().surface_manager().display().metrics().ppi());
	}

	point spacer::position() const
	{
		return units_converter(*this).from_device_units(iPosition);
	}

	void spacer::set_position(const point& aPosition)
	{
		iPosition = units_converter(*this).to_device_units(aPosition);
	}

	void spacer::set_extents(const size& aExtents)
	{
		iExtents = units_converter(*this).to_device_units(aExtents);
	}

	bool spacer::has_size_policy() const
	{
		return iSizePolicy != boost::none;
	}

	size_policy spacer::size_policy() const
	{
		if (has_size_policy())
			return *iSizePolicy;
		neogfx::size_policy result{neogfx::size_policy::Minimum};
		if (iExpansionPolicy & ExpandHorizontally)
			result.set_horizontal_size_policy(neogfx::size_policy::Expanding);
		if (iExpansionPolicy & ExpandVertically)
			result.set_vertical_size_policy(neogfx::size_policy::Expanding);
		return result;
	}

	void spacer::set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout)
	{
		if (iSizePolicy != aSizePolicy)
		{
			iSizePolicy = aSizePolicy;
			if (has_layout_owner() && aUpdateLayout)
				layout_owner().ultimate_ancestor().layout_items(true);
		}
	}

	bool spacer::has_weight() const
	{
		return iWeight != boost::none;
	}

	size spacer::weight() const
	{
		if (has_weight())
			return *iWeight;
		else
			return 1.0;
	}

	void spacer::set_weight(const optional_size& aWeight, bool aUpdateLayout)
	{
		if (iWeight != aWeight)
		{
			iWeight = aWeight;
			if (has_layout_owner() && aUpdateLayout)
				layout_owner().ultimate_ancestor().layout_items(true);
		}
	}

	bool spacer::has_minimum_size() const
	{
		return iMinimumSize != boost::none;
	}

	size spacer::minimum_size(const optional_size&) const
	{
		return has_minimum_size() ?
			units_converter(*this).from_device_units(*iMinimumSize) :
			size{};
	}

	void spacer::set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout)
	{
		optional_size newMinimumSize = (aMinimumSize != boost::none ? units_converter(*this).to_device_units(*aMinimumSize) : optional_size());
		if (iMinimumSize != newMinimumSize)
		{
			iMinimumSize = newMinimumSize;
			if (has_layout_owner() && aUpdateLayout)
				layout_owner().ultimate_ancestor().layout_items(true);
		}
	}

	bool spacer::has_maximum_size() const
	{
		return iMaximumSize != boost::none;
	}

	size spacer::maximum_size(const optional_size&) const
	{
		return has_maximum_size() ?
			units_converter(*this).from_device_units(*iMaximumSize) :
			size::max_size();
	}

	void spacer::set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout)
	{
		optional_size newMaximumSize = (aMaximumSize != boost::none ? units_converter(*this).to_device_units(*aMaximumSize) : optional_size());
		if (iMaximumSize != newMaximumSize)
		{
			iMaximumSize = newMaximumSize;
			if (has_layout_owner() && aUpdateLayout)
				layout_owner().ultimate_ancestor().layout_items(true);
		}
	}

	bool spacer::has_margins() const
	{
		return false;
	}

	neogfx::margins spacer::margins() const
	{
		return neogfx::margins{};
	}

	void spacer::set_margins(const optional_margins&, bool)
	{
		/* do nothing */
	}

	bool spacer::device_metrics_available() const
	{
		return has_parent_layout() && parent_layout().device_metrics_available();
	}

	const i_device_metrics& spacer::device_metrics() const
	{
		if (device_metrics_available())
			return parent_layout().device_metrics();
		throw no_device_metrics();
	}

	units spacer::units() const
	{
		return iUnitsContext.units();
	}

	units spacer::set_units(neogfx::units aUnits) const
	{
		return iUnitsContext.set_units(aUnits);
	}

	void spacer::layout_as(const point&, const size& aSize)
	{
		set_extents(aSize);
	}

	uint32_t spacer::layout_id() const
	{
		return iLayoutId;
	}

	void spacer::next_layout_id()
	{
		if (++iLayoutId == static_cast<uint32_t>(-1))
			iLayoutId = 0;
	}

	bool spacer::visible() const
	{
		if (has_layout_owner())
			return layout_owner().visible();
		return false;
	}

	horizontal_spacer::horizontal_spacer() :
		spacer(ExpandHorizontally)
	{
	}

	horizontal_spacer::horizontal_spacer(i_layout& aParent) :
		spacer(aParent, ExpandHorizontally)
	{
	}

	vertical_spacer::vertical_spacer() :
		spacer(ExpandVertically)
	{
	}

	vertical_spacer::vertical_spacer(i_layout& aParent) :
		spacer(aParent, ExpandVertically)
	{
	}
}