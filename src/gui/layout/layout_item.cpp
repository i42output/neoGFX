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
#include <neogfx/gui/layout/i_spacer.hpp>

namespace neogfx
{
	layout_item::layout_item(i_layout& aParent, i_widget& aWidget) :
		iParent{ aParent }, iPointerWrapper{ widget_pointer{widget_pointer{}, &aWidget} }, iLayoutId{ -1, -1 }
	{
	}

	layout_item::layout_item(i_layout& aParent, std::shared_ptr<i_widget> aWidget) :
		iParent{ aParent }, iPointerWrapper{ aWidget }, iLayoutId{ -1, -1 }
	{
	}

	layout_item::layout_item(i_layout& aParent, i_layout& aLayout) :
		iParent{ aParent }, iPointerWrapper{ layout_pointer{layout_pointer{}, &aLayout} }, iLayoutId{ -1, -1 }
	{
	}

	layout_item::layout_item(i_layout& aParent, std::shared_ptr<i_layout> aLayout) :
		iParent{ aParent }, iPointerWrapper{ aLayout }, iLayoutId{ -1, -1 }
	{
	}

	layout_item::layout_item(i_layout& aParent, i_spacer& aSpacer) :
		iParent{ aParent }, iPointerWrapper{ spacer_pointer{spacer_pointer{}, &aSpacer} }, iLayoutId{ -1, -1 }
	{
	}

	layout_item::layout_item(i_layout& aParent, std::shared_ptr<i_spacer> aSpacer) :
		iParent{ aParent }, iPointerWrapper{ aSpacer }, iLayoutId{ -1, -1 }
	{
	}

	const layout_item::pointer_wrapper& layout_item::get() const
	{
		return iPointerWrapper;
	}

	layout_item::pointer_wrapper& layout_item::get()
	{
		return iPointerWrapper;
	}

	const i_widget_geometry& layout_item::wrapped_geometry() const
	{
		return iPointerWrapper.is<widget_pointer>() ?
			static_cast<const i_widget_geometry&>(*static_variant_cast<const widget_pointer&>(iPointerWrapper)) :
			iPointerWrapper.is<layout_pointer>() ?
				static_cast<const i_widget_geometry&>(*static_variant_cast<const layout_pointer&>(iPointerWrapper)) :
				static_cast<const i_widget_geometry&>(*static_variant_cast<const spacer_pointer&>(iPointerWrapper));
	}

	i_widget_geometry& layout_item::wrapped_geometry()
	{
		return const_cast<i_widget_geometry&>(const_cast<const layout_item*>(this)->wrapped_geometry());
	}

	void layout_item::set_owner(i_widget* aOwner)
	{
		iOwner = aOwner;
		if (iPointerWrapper.is<widget_pointer>())
			iOwner->add(static_variant_cast<widget_pointer&>(iPointerWrapper));
		else if (iPointerWrapper.is<layout_pointer>())
			static_variant_cast<layout_pointer&>(iPointerWrapper)->set_owner(aOwner);
	}

	void layout_item::layout(const point& aPosition, const size& aSize)
	{
		size adjustedSize = aSize.min(maximum_size());
		point adjustedPosition = aPosition;
		if (adjustedSize != aSize)
		{
			adjustedPosition += point{
			(iParent.alignment() & alignment::Centre) == alignment::Centre ?
				(aSize - adjustedSize).cx / 2.0 :
				(iParent.alignment() & alignment::Right) == alignment::Right ?
					(aSize - adjustedSize).cx :
					0.0,
			(iParent.alignment() & alignment::VCentre) == alignment::VCentre ?
				(aSize - adjustedSize).cy / 2.0 :
				(iParent.alignment() & alignment::Bottom) == alignment::Bottom ?
					(aSize - adjustedSize).cy :
					0.0 }.floor();
		}
					
		if (iPointerWrapper.is<widget_pointer>())
		{
			auto& w = *static_variant_cast<widget_pointer&>(iPointerWrapper);
			w.move(adjustedPosition);
			if (w.extents() != adjustedSize)
				w.resize(adjustedSize);
			else if (w.has_layout() && w.layout().invalidated())
				w.layout_items();
		}
		else if (iPointerWrapper.is<layout_pointer>())
		{
			static_variant_cast<layout_pointer&>(iPointerWrapper)->layout_items(adjustedPosition, adjustedSize);
		}
		else if (iPointerWrapper.is<spacer_pointer>())
		{
			static_variant_cast<spacer_pointer&>(iPointerWrapper)->set_extents(adjustedSize);
		}
	}

	bool layout_item::high_dpi() const
	{
		return wrapped_geometry().high_dpi();
	}

	dimension layout_item::dpi_scale_factor() const
	{
		return wrapped_geometry().dpi_scale_factor();
	}

	bool layout_item::device_metrics_available() const
	{
		return iParent.device_metrics_available();
	}

	const i_device_metrics& layout_item::device_metrics() const
	{
		return iParent.device_metrics();
	}

	neogfx::units layout_item::units() const
	{
		return iParent.units();
	}

	neogfx::units layout_item::set_units(neogfx::units aUnits) const
	{
		return iParent.set_units(aUnits);
	}

	point layout_item::position() const
	{
		return wrapped_geometry().position();
	}

	void layout_item::set_position(const point& aPosition)
	{
		wrapped_geometry().set_position(aPosition);
	}

	size layout_item::extents() const
	{
		return wrapped_geometry().extents();
	}

	void layout_item::set_extents(const size& aExtents)
	{
		wrapped_geometry().set_extents(aExtents);
	}

	bool layout_item::has_size_policy() const
	{
		return wrapped_geometry().has_size_policy();
	}

	size_policy layout_item::size_policy() const
	{
		return wrapped_geometry().size_policy();
	}

	void layout_item::set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout)
	{
		wrapped_geometry().set_size_policy(aSizePolicy, aUpdateLayout);
	}

	bool layout_item::has_weight() const
	{
		return wrapped_geometry().has_weight();
	}

	size layout_item::weight() const
	{
		return wrapped_geometry().weight();
	}

	void layout_item::set_weight(const optional_size& aWeight, bool aUpdateLayout)
	{
		wrapped_geometry().set_weight(aWeight, aUpdateLayout);
	}

	bool layout_item::has_minimum_size() const
	{
		return wrapped_geometry().has_minimum_size();
	}

	size layout_item::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (!visible())
			return size{};
		if (iLayoutId.first == iParent.layout_id() && iLayoutId.first != -1)
			return iMinimumSize;
		else
		{
			iMinimumSize = wrapped_geometry().minimum_size(aAvailableSpace);
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
			iLayoutId.first = iParent.layout_id();
			return iMinimumSize;
		}
	}

	void layout_item::set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout)
	{
		wrapped_geometry().set_minimum_size(aMinimumSize, aUpdateLayout);
		if (aMinimumSize != boost::none)
			iMinimumSize = *aMinimumSize;
	}

	bool layout_item::has_maximum_size() const
	{
		return wrapped_geometry().has_maximum_size();
	}

	size layout_item::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (!visible())
			return size::max_size();
		if (iLayoutId.second == iParent.layout_id())
			return iMaximumSize;
		else
		{
			iMaximumSize = wrapped_geometry().maximum_size(aAvailableSpace);
			iLayoutId.second = iParent.layout_id();
			return iMaximumSize;
		}
	}

	void layout_item::set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout)
	{
		wrapped_geometry().set_maximum_size(aMaximumSize, aUpdateLayout);
		if (aMaximumSize != boost::none)
			iMaximumSize = *aMaximumSize;
	}

	bool layout_item::has_margins() const
	{
		return wrapped_geometry().has_margins();
	}

	margins layout_item::margins() const
	{
		return wrapped_geometry().margins();
	}

	void layout_item::set_margins(const optional_margins& aMargins, bool aUpdateLayout)
	{
		wrapped_geometry().set_margins(aMargins, aUpdateLayout);
	}

	bool layout_item::visible() const
	{
		if (iPointerWrapper.is<widget_pointer>())
			return static_variant_cast<const widget_pointer&>(iPointerWrapper)->visible();
		else
			return true;
	}

	bool layout_item::operator==(const layout_item& aOther) const
	{
		return iPointerWrapper == aOther.iPointerWrapper;
	}
}
