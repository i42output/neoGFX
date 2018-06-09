// layout_item.cpp
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
#include <neogfx/gui/layout/layout_item.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/layout/i_spacer.hpp>

namespace neogfx
{
	layout_item::layout_item(i_layout_item& aItem) :
		layout_item{ std::shared_ptr<i_layout_item>{ std::shared_ptr<i_layout_item>{}, &aItem } } 
	{
	}

	layout_item::layout_item(std::shared_ptr<i_layout_item> aItem) :
		iSubject{ aItem }, iLayoutId{ -1, -1 }
	{
	}

	layout_item::layout_item(const layout_item& aOther) :
		iSubject{ aOther.iSubject }, iLayoutId{ -1, -1 }
	{
	}

	layout_item::~layout_item()
	{
	}

	const i_layout_item& layout_item::subject() const
	{
		if (iSubject->is_proxy())
			return iSubject->layout_item_proxy().subject();
		return *iSubject;
	}

	i_layout_item& layout_item::subject()
	{
		if (iSubject->is_proxy())
			return iSubject->layout_item_proxy().subject();
		return *iSubject;
	}

	std::shared_ptr<i_layout_item> layout_item::subject_ptr()
	{
		return iSubject;
	}

	bool layout_item::is_layout() const
	{
		return subject().is_layout();
	}

	const i_layout& layout_item::as_layout() const
	{
		return subject().as_layout();
	}

	i_layout& layout_item::as_layout()
	{
		return subject().as_layout();
	}

	bool layout_item::is_widget() const
	{
		return subject().is_widget();
	}

	const i_widget& layout_item::as_widget() const
	{
		return subject().as_widget();
	}

	i_widget& layout_item::as_widget()
	{
		return subject().as_widget();
	}

	bool layout_item::is_spacer() const
	{
		return !is_layout() && !is_widget();
	}

	bool layout_item::has_parent_layout() const
	{
		return subject().has_parent_layout();
	}

	const i_layout& layout_item::parent_layout() const
	{
		if (has_parent_layout())
			return subject().parent_layout();
		throw no_parent_layout();
	}

	i_layout& layout_item::parent_layout()
	{
		return const_cast<i_layout&>(const_cast<const layout_item*>(this)->parent_layout());
	}

	void layout_item::set_parent_layout(i_layout* aParentLayout)
	{
		if (!subject().is_proxy())
			subject().set_parent_layout(aParentLayout);
	}

	bool layout_item::has_layout_owner() const
	{
		return subject().has_layout_owner();
	}

	const i_widget& layout_item::layout_owner() const
	{
		if (has_layout_owner())
			return subject().layout_owner();
		throw no_layout_owner();
	}

	i_widget& layout_item::layout_owner()
	{
		return const_cast<i_widget&>(const_cast<const layout_item*>(this)->layout_owner());
	}

	void layout_item::set_layout_owner(i_widget* aOwner)
	{
		if (!subject().is_proxy())
			subject().set_layout_owner(aOwner);
	}

	bool layout_item::is_proxy() const
	{
		return true;
	}

	const i_layout_item_proxy& layout_item::layout_item_proxy() const
	{
		return *this;
	}

	i_layout_item_proxy& layout_item::layout_item_proxy()
	{
		return *this;
	}

	void layout_item::layout_as(const point& aPosition, const size& aSize)
	{
		point adjustedPosition = aPosition;
		size adjustedSize = aSize.min(maximum_size());
		if (adjustedSize != aSize)
		{
			adjustedPosition += point{
			(parent_layout().alignment() & alignment::Centre) == alignment::Centre ?
				(aSize - adjustedSize).cx / 2.0 :
				(parent_layout().alignment() & alignment::Right) == alignment::Right ?
					(aSize - adjustedSize).cx :
					0.0,
			(parent_layout().alignment() & alignment::VCentre) == alignment::VCentre ?
				(aSize - adjustedSize).cy / 2.0 :
				(parent_layout().alignment() & alignment::Bottom) == alignment::Bottom ?
					(aSize - adjustedSize).cy :
					0.0 }.floor();
		}

		subject().layout_as(adjustedPosition, adjustedSize);
	}

	uint32_t layout_item::layout_id() const
	{
		return subject().layout_id();
	}
	
	void layout_item::next_layout_id()
	{
		subject().next_layout_id();
	}

	bool layout_item::high_dpi() const
	{
		return subject().high_dpi();
	}

	dimension layout_item::dpi_scale_factor() const
	{
		return subject().dpi_scale_factor();
	}

	bool layout_item::device_metrics_available() const
	{
		return parent_layout().device_metrics_available();
	}

	const i_device_metrics& layout_item::device_metrics() const
	{
		return parent_layout().device_metrics();
	}

	neogfx::units layout_item::units() const
	{
		return parent_layout().units();
	}

	neogfx::units layout_item::set_units(neogfx::units aUnits) const
	{
		return parent_layout().set_units(aUnits);
	}

	point layout_item::position() const
	{
		return subject().position();
	}

	void layout_item::set_position(const point& aPosition)
	{
		subject().set_position(aPosition);
	}

	size layout_item::extents() const
	{
		return subject().extents();
	}

	void layout_item::set_extents(const size& aExtents)
	{
		subject().set_extents(aExtents);
	}

	bool layout_item::has_size_policy() const
	{
		return subject().has_size_policy();
	}

	size_policy layout_item::size_policy() const
	{
		return subject().size_policy();
	}

	void layout_item::set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout)
	{
		subject().set_size_policy(aSizePolicy, aUpdateLayout);
	}

	bool layout_item::has_weight() const
	{
		return subject().has_weight();
	}

	size layout_item::weight() const
	{
		return subject().weight();
	}

	void layout_item::set_weight(const optional_size& aWeight, bool aUpdateLayout)
	{
		subject().set_weight(aWeight, aUpdateLayout);
	}

	bool layout_item::has_minimum_size() const
	{
		return subject().has_minimum_size();
	}

	size layout_item::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (!visible())
			return size{};
		if (iLayoutId.first == parent_layout().layout_id() && iLayoutId.first != -1)
			return iMinimumSize;
		else
		{
			iMinimumSize = subject().minimum_size(aAvailableSpace);
			if (size_policy().maintain_aspect_ratio())
			{
				const auto& aspectRatio = size_policy().aspect_ratio();
				if (aspectRatio.cx < aspectRatio.cy)
				{
					if (iMinimumSize.cx < iMinimumSize.cy)
						iMinimumSize = size{ iMinimumSize.cx, iMinimumSize.cx * (aspectRatio.cy / aspectRatio.cx) };
					else
						iMinimumSize = size{ iMinimumSize.cy * (aspectRatio.cx / aspectRatio.cy), iMinimumSize.cy };
				}
				else
				{
					if (iMinimumSize.cx < iMinimumSize.cy)
						iMinimumSize = size{ iMinimumSize.cy * (aspectRatio.cx / aspectRatio.cy), iMinimumSize.cy };
					else
						iMinimumSize = size{ iMinimumSize.cx, iMinimumSize.cx * (aspectRatio.cy / aspectRatio.cx) };
				}
			}
			iLayoutId.first = parent_layout().layout_id();
			return iMinimumSize;
		}
	}

	void layout_item::set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout)
	{
		subject().set_minimum_size(aMinimumSize, aUpdateLayout);
		if (aMinimumSize != boost::none)
			iMinimumSize = *aMinimumSize;
	}

	bool layout_item::has_maximum_size() const
	{
		return subject().has_maximum_size();
	}

	size layout_item::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (!visible())
			return size::max_size();
		if (iLayoutId.second == parent_layout().layout_id() && iLayoutId.second != -1)
			return iMaximumSize;
		else
		{
			iMaximumSize = subject().maximum_size(aAvailableSpace);
			iLayoutId.second = parent_layout().layout_id();
			return iMaximumSize;
		}
	}

	void layout_item::set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout)
	{
		subject().set_maximum_size(aMaximumSize, aUpdateLayout);
		if (aMaximumSize != boost::none)
			iMaximumSize = *aMaximumSize;
	}

	bool layout_item::has_margins() const
	{
		return subject().has_margins();
	}

	margins layout_item::margins() const
	{
		return subject().margins();
	}

	void layout_item::set_margins(const optional_margins& aMargins, bool aUpdateLayout)
	{
		subject().set_margins(aMargins, aUpdateLayout);
	}

	bool layout_item::visible() const
	{
		return subject().visible();
	}

	bool layout_item::operator==(const layout_item& aOther) const
	{
		return iSubject == aOther.iSubject;
	}
}
