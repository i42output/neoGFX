// stack_layout.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include <unordered_map>
#include <unordered_set>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/bresenham_counter.hpp>
#include <neogfx/gui/layout/stack_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
	stack_layout::stack_layout(i_widget& aParent) :
		layout(aParent)
	{
	}

	stack_layout::stack_layout(i_layout& aParent) :
		layout(aParent)
	{
	}

	i_spacer& stack_layout::add_spacer()
	{
		throw wrong_item_type();
	}

	i_spacer& stack_layout::add_spacer_at(item_index)
	{
		throw wrong_item_type();
	}

	size stack_layout::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (items_visible() == 0)
			return size{};
		size result;
		for (const auto& item : items())
		{
			result.cx = std::max(result.cx, item.minimum_size(aAvailableSpace).cx);
			result.cy = std::max(result.cy, item.minimum_size(aAvailableSpace).cy);
		}
		result.cx += (margins().left + margins().right);
		result.cy += (margins().top + margins().bottom);
		result.cx = std::max(result.cx, layout::minimum_size(aAvailableSpace).cx);
		result.cy = std::max(result.cy, layout::minimum_size(aAvailableSpace).cy);
		return result;
	}

	size stack_layout::maximum_size(const optional_size& aAvailableSpace) const
	{
		size result{ std::numeric_limits<size::dimension_type>::max(), std::numeric_limits<size::dimension_type>::max() };
		for (const auto& item : items())
			result = result.min(item.maximum_size(aAvailableSpace));
		if (result.cx != std::numeric_limits<size::dimension_type>::max())
			result.cx += (margins().left + margins().right);
		if (result.cy != std::numeric_limits<size::dimension_type>::max())
			result.cy += (margins().top + margins().bottom);
		if (result.cx != std::numeric_limits<size::dimension_type>::max())
			result.cx = std::min(result.cx, layout::maximum_size(aAvailableSpace).cx);
		if (result.cy != std::numeric_limits<size::dimension_type>::max())
			result.cy = std::min(result.cy, layout::maximum_size(aAvailableSpace).cy);
		return result;
	}

	void stack_layout::layout_items(const point& aPosition, const size& aSize)
	{
		if (!enabled())
			return;
		owner()->layout_items_started();
		next_layout_id();
		validate();
		for (auto& item : items())
		{
			if (!item.visible())
				continue;
			item.layout(aPosition + margins().top_left(), aSize - margins().size());
		}
		owner()->layout_items_completed();
	}
}