// layout.inl
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

#include <unordered_map>
#include <unordered_set>
#include "horizontal_layout.hpp"
#include "vertical_layout.hpp"

namespace neogfx
{
	template <typename AxisPolicy>
	size flow_layout::do_minimum_size(const optional_size& aAvailableSpace) const
	{
		uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
		if (itemsVisible == 0)
			return size{};
		size result;
		uint32_t itemsZeroSized = 0;
		size availableSpace = *aAvailableSpace - margins().size();
		size extent;
		point pos;
		bool previousNonZeroSize = false;
		for (const auto& item : items())
		{
			if (!item.visible())
				continue;
			auto itemMinimumSize = item.minimum_size(availableSpace);
			if (!item.get().is<item::spacer_pointer>() && (AxisPolicy::cx(itemMinimumSize) == 0.0 || AxisPolicy::cy(itemMinimumSize) == 0.0))
			{
				++itemsZeroSized;
				previousNonZeroSize = false;
				continue;
			}
			if (previousNonZeroSize)
				AxisPolicy::x(pos) += AxisPolicy::cx(spacing());
			if (AxisPolicy::x(pos) + AxisPolicy::cx(itemMinimumSize) > AxisPolicy::cx(availableSpace))
			{
				AxisPolicy::x(pos) = AxisPolicy::cx(itemMinimumSize);
				AxisPolicy::y(pos) += (AxisPolicy::cy(extent) + AxisPolicy::cy(spacing()));
			}
			else
				AxisPolicy::x(pos) += AxisPolicy::cx(itemMinimumSize);
			AxisPolicy::cx(extent) = std::max(AxisPolicy::cx(extent), AxisPolicy::x(pos));
			AxisPolicy::cy(extent) = std::max(AxisPolicy::cy(extent), AxisPolicy::cy(itemMinimumSize));
			previousNonZeroSize = true;
		}
		AxisPolicy::cx(result) = AxisPolicy::cx(extent);
		AxisPolicy::cy(result) = AxisPolicy::y(pos) + AxisPolicy::cy(extent);
		AxisPolicy::cx(result) += AxisPolicy::cx(margins());
		AxisPolicy::cy(result) += AxisPolicy::cy(margins());
		AxisPolicy::cx(result) = std::max(std::min(AxisPolicy::cx(result), AxisPolicy::cx(*aAvailableSpace)), AxisPolicy::cx(layout::minimum_size(aAvailableSpace)));
		AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result), AxisPolicy::cy(layout::minimum_size(aAvailableSpace)));
		return result;
	}

	template <typename AxisPolicy>
	size flow_layout::do_maximum_size(const optional_size& aAvailableSpace) const
	{
		if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
			return size{ std::numeric_limits<size::dimension_type>::max(), std::numeric_limits<size::dimension_type>::max() };
		uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
		size result;
		size availableSpace = *aAvailableSpace - margins().size();
		coordinate extent = 0.0;
		point pos;
		for (const auto& item : items())
		{
			if (!item.visible())
				continue;
			auto itemMaximumSize = item.maximum_size(availableSpace);
			if (AxisPolicy::cx(itemMaximumSize) != std::numeric_limits<size::dimension_type>::max())
			{
				if (AxisPolicy::x(pos) != std::numeric_limits<size::dimension_type>::max())
				{
					if (AxisPolicy::x(pos) + AxisPolicy::cx(itemMaximumSize) > AxisPolicy::cx(availableSpace))
					{
						AxisPolicy::x(pos) = (AxisPolicy::cx(itemMaximumSize) + AxisPolicy::cx(spacing()));
						if (AxisPolicy::cy(itemMaximumSize) != std::numeric_limits<size::dimension_type>::max())
						{
							if (AxisPolicy::y(pos) != std::numeric_limits<size::dimension_type>::max())
								AxisPolicy::y(pos) += (AxisPolicy::cy(itemMaximumSize) + AxisPolicy::cy(spacing()));
						}
						else
							AxisPolicy::y(pos) = std::numeric_limits<size::dimension_type>::max();
					}
					else
					{
						AxisPolicy::x(pos) += (AxisPolicy::cx(itemMaximumSize) + AxisPolicy::cx(spacing()));
					}
				}
				else
				{
					if (AxisPolicy::cy(itemMaximumSize) != std::numeric_limits<size::dimension_type>::max())
					{
						if (AxisPolicy::y(pos) != std::numeric_limits<size::dimension_type>::max())
							AxisPolicy::y(pos) += (AxisPolicy::cy(itemMaximumSize) + AxisPolicy::cy(spacing()));
					}
					else
						AxisPolicy::y(pos) = std::numeric_limits<size::dimension_type>::max();
				}
			}
			else
				AxisPolicy::x(pos) = std::numeric_limits<size::dimension_type>::max();
			extent = std::max(extent, AxisPolicy::x(pos));
		}
		AxisPolicy::cx(result) = extent;
		AxisPolicy::cy(result) = AxisPolicy::y(pos);
		if (AxisPolicy::cx(result) != std::numeric_limits<size::dimension_type>::max())
		{
			AxisPolicy::cx(result) += AxisPolicy::cx(margins());
			if (itemsVisible > 1)
				AxisPolicy::cx(result) += (AxisPolicy::cx(spacing()) * (itemsVisible - 1));
			AxisPolicy::cx(result) = std::min(AxisPolicy::cx(result), AxisPolicy::cx(layout::maximum_size(aAvailableSpace)));
		}
		if (AxisPolicy::cy(result) != std::numeric_limits<size::dimension_type>::max())
		{
			AxisPolicy::cy(result) += AxisPolicy::cy(margins());
			AxisPolicy::cy(result) = std::min(AxisPolicy::cy(result), AxisPolicy::cy(layout::maximum_size(aAvailableSpace)));
		}
		if (AxisPolicy::cx(result) == 0.0 && AxisPolicy::size_policy_x(size_policy()) == neogfx::size_policy::Expanding)
			AxisPolicy::cx(result) = std::numeric_limits<size::dimension_type>::max();
		if (AxisPolicy::cy(result) == 0.0 && AxisPolicy::size_policy_y(size_policy()) == neogfx::size_policy::Expanding)
			AxisPolicy::cy(result) = std::numeric_limits<size::dimension_type>::max();
		return result;
	}

	template <typename AxisPolicy>
	void flow_layout::do_layout_items(const point& aPosition, const size& aSize)
	{
		set_position(aPosition);
		set_extents(aSize);
		size availableSpace = aSize;
		availableSpace.cx -= (margins().left + margins().right);
		availableSpace.cy -= (margins().top + margins().bottom);
		point pos;
		bool previousNonZeroSize = false;
		typename AxisPolicy::minor_layout rows(*this);
		for (const auto& item : items())
		{
			if (!item.visible())
				continue;
			if (&item == &items().back())
				continue;
			auto itemMinimumSize = item.minimum_size(availableSpace);
			if (!item.get().is<item::spacer_pointer>() && (AxisPolicy::cx(itemMinimumSize) == 0.0 || AxisPolicy::cy(itemMinimumSize) == 0.0))
			{
				previousNonZeroSize = false;
				continue;
			}
			if (previousNonZeroSize)
				AxisPolicy::x(pos) += AxisPolicy::cx(spacing());
			if (AxisPolicy::x(pos) + AxisPolicy::cx(itemMinimumSize) > AxisPolicy::cx(availableSpace))
			{
				rows.add_item(std::make_shared<typename AxisPolicy::major_layout>());
				rows.get_layout(rows.item_count() - 1).add_item(item);
				AxisPolicy::x(pos) = AxisPolicy::cx(itemMinimumSize);
			}
			else
			{
				if (rows.item_count() == 0)
					rows.add_item(std::make_shared<typename AxisPolicy::major_layout>());
				rows.get_layout(rows.item_count() - 1).add_item(item);
				AxisPolicy::x(pos) += AxisPolicy::cx(itemMinimumSize);
			}
			previousNonZeroSize = true;
		}
		rows.set_margins(neogfx::margins{});
		rows.set_spacing(spacing());
		for (uint32_t i = 0; i < rows.item_count(); ++i)
		{
			rows.get_layout(i).set_margins(neogfx::margins{});
			rows.get_layout(i).set_spacing(spacing());
		}
		rows.layout_items(aPosition + margins().top_left(), availableSpace);
	}
}
