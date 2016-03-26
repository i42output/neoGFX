// horizontal_layout.cpp
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

#include "neogfx.hpp"
#include <unordered_map>
#include <unordered_set>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/bresenham_counter.hpp>
#include "horizontal_layout.hpp"
#include "i_widget.hpp"
#include "spacer.hpp"

namespace neogfx
{
	horizontal_layout::horizontal_layout(neogfx::alignment aAlignment) :
		layout(aAlignment)
	{
	}

	horizontal_layout::horizontal_layout(i_widget& aParent, neogfx::alignment aAlignment) :
		layout(aParent, aAlignment)
	{
	}

	horizontal_layout::horizontal_layout(i_layout& aParent, neogfx::alignment aAlignment) :
		layout(aParent, aAlignment)
	{
	}

	i_spacer& horizontal_layout::add_spacer()
	{
		auto s = std::make_shared<horizontal_spacer>();
		add_item(s);
		return *s;
	}

	i_spacer& horizontal_layout::add_spacer(uint32_t aPosition)
	{
		auto s = std::make_shared<horizontal_spacer>();
		add_item(aPosition, s);
		return *s;
	}

	neogfx::size_policy horizontal_layout::size_policy() const
	{
		if (layout::has_size_policy())
			return layout::size_policy();
		neogfx::size_policy result{neogfx::size_policy::Minimum, neogfx::size_policy::Expanding};
		for (auto& i : items())
			if (i.size_policy().horizontal_size_policy() == neogfx::size_policy::Expanding)
				result.set_horizontal_size_policy(neogfx::size_policy::Expanding);
		return result;
	}

	size horizontal_layout::minimum_size() const
	{
		uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
		if (itemsVisible == 0)
			return size{};
		size result;
		uint32_t itemsZeroSized = 0;
		for (const auto& item : items())
		{
			if (!item.visible())
				continue;
			if (!item.get().is<item::spacer_pointer>() && (item.minimum_size().cx == 0.0 || item.minimum_size().cy == 0.0))
			{
				++itemsZeroSized;
				continue;
			}
			result.cy = std::max(result.cy, item.minimum_size().cy);
			result.cx += item.minimum_size().cx;
		}
		result.cx += (margins().left + margins().right);
		result.cy += (margins().top + margins().bottom);
		if (result.cx != std::numeric_limits<size::dimension_type>::max() && (itemsVisible - itemsZeroSized) > 0)
			result.cx += (spacing().cx * (itemsVisible - itemsZeroSized - 1));
		result.cx = std::max(result.cx, layout::minimum_size().cx);
		result.cy = std::max(result.cy, layout::minimum_size().cy);
		return result;
	}

	size horizontal_layout::maximum_size() const
	{
		if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
			return size{ std::numeric_limits<size::dimension_type>::max(), std::numeric_limits<size::dimension_type>::max() };
		uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
		size result{ std::numeric_limits<size::dimension_type>::max(), 0.0 };
		for (const auto& item : items())
		{
			if (!item.visible())
				continue;
			result.cy = std::max(result.cy, item.maximum_size().cy);
			auto cx = std::min(result.cx, item.maximum_size().cx);
			if (cx != std::numeric_limits<size::dimension_type>::max())
				result.cx += cx;
			else
				result.cx = std::numeric_limits<size::dimension_type>::max();
		}
		if (result.cx != std::numeric_limits<size::dimension_type>::max())
			result.cx += (margins().left + margins().right);
		if (result.cy != std::numeric_limits<size::dimension_type>::max())
			result.cy += (margins().top + margins().bottom);
		if (result.cx != std::numeric_limits<size::dimension_type>::max() && itemsVisible > 1)
			result.cx += (spacing().cx * (itemsVisible - 1));
		if (result.cx != std::numeric_limits<size::dimension_type>::max())
			result.cx = std::min(result.cx, layout::maximum_size().cx);
		if (result.cy != std::numeric_limits<size::dimension_type>::max())
			result.cy = std::min(result.cy, layout::maximum_size().cy);
		return result;
	}

	void horizontal_layout::layout_items(const point& aPosition, const size& aSize)
	{
		if (!enabled())
			return;
		owner()->layout_items_started();
		next_layout_id();
		layout::do_layout_items<layout::column_major<horizontal_layout>>(aPosition, aSize);
		owner()->layout_items_completed();
	}
}