// vertical_layout.cpp
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
#include "vertical_layout.hpp"
#include "i_widget.hpp"
#include "spacer.hpp"

namespace neogfx
{
	vertical_layout::vertical_layout(i_widget& aParent, alignment aHorizontalAlignment) :
		layout(aParent), iHorizontalAlignment(aHorizontalAlignment)
	{
	}

	vertical_layout::vertical_layout(i_layout& aParent, alignment aHorizontalAlignment) :
		layout(aParent), iHorizontalAlignment(aHorizontalAlignment)
	{
	}

	i_spacer& vertical_layout::add_spacer()
	{
		auto s = std::make_shared<vertical_spacer>();
		add_spacer(s);
		return *s;
	}

	i_spacer& vertical_layout::add_spacer(uint32_t aPosition)
	{
		auto s = std::make_shared<vertical_spacer>();
		add_spacer(aPosition, s);
		return *s;
	}

	size vertical_layout::minimum_size() const
	{
		if (items_visible() == 0)
			return size{};
		size result;
		uint32_t itemsZeroSized = 0;
		for (const auto& item : items())
		{
			if (!item.visible())
				continue;
			if (item.get().is<item::widget_pointer>() && (item.minimum_size().cx == 0.0 || item.minimum_size().cy == 0.0))
			{
				++itemsZeroSized;
				continue;
			}
			result.cx = std::max(result.cx, item.minimum_size().cx);
			result.cy += item.minimum_size().cy;
		}
		result.cx += (margins().left + margins().right);
		result.cy += (margins().top + margins().bottom);
		if (result.cy != std::numeric_limits<size::dimension_type>::max() && (items_visible() - itemsZeroSized) > 0)
			result.cy += (spacing().cy * (items_visible() - itemsZeroSized - 1));
		result.cx = std::max(result.cx, layout::minimum_size().cx);
		result.cy = std::max(result.cy, layout::minimum_size().cy);
		return result;
	}

	size vertical_layout::maximum_size() const
	{
		if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
			return size{ std::numeric_limits<size::dimension_type>::max(), std::numeric_limits<size::dimension_type>::max() };
		size result{ 0.0, std::numeric_limits<size::dimension_type>::max() };
		for (const auto& item : items())
		{
			if (!item.visible())
				continue;
			result.cx = std::max(result.cx, item.maximum_size().cx);
			auto cy = std::min(result.cy, item.maximum_size().cy);
			if (cy != std::numeric_limits<size::dimension_type>::max())
				result.cy += cy;
			else
				result.cy = std::numeric_limits<size::dimension_type>::max();
		}
		if (result.cx != std::numeric_limits<size::dimension_type>::max())
			result.cx += (margins().left + margins().right);
		if (result.cy != std::numeric_limits<size::dimension_type>::max())
			result.cy += (margins().top + margins().bottom);
		if (result.cy != std::numeric_limits<size::dimension_type>::max() && items_visible() > 0)
			result.cy += (spacing().cy * (items_visible() - 1 - spacer_count()));
		if (result.cx != std::numeric_limits<size::dimension_type>::max())
			result.cx = std::min(result.cx, layout::maximum_size().cx);
		if (result.cy != std::numeric_limits<size::dimension_type>::max())
			result.cy = std::min(result.cy, layout::maximum_size().cy);
		return result;
	}

	void vertical_layout::layout_items(const point& aPosition, const size& aSize)
	{
		if (!enabled())
			return;
		if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
			return;
		size availableSize = aSize;
		availableSize.cx -= (margins().left + margins().right);
		availableSize.cy -= (margins().top + margins().bottom);
		uint32_t itemsZeroSized = 0;
		if (aSize.cy <= minimum_size().cy)
		{
			for (const auto& item : items())
			{
				if (!item.visible())
					continue;
				if (item.get().is<item::widget_pointer>() && (item.minimum_size().cx == 0.0 || item.minimum_size().cy == 0.0))
					++itemsZeroSized;
			}
		}
		if (items_visible() - itemsZeroSized > 0)
			availableSize.cy -= (spacing().cy * (items_visible() - itemsZeroSized - 1));
		size::dimension_type leftover = availableSize.cy;
		size::dimension_type eachLeftover = std::floor(leftover / items_visible());
		size totalExpanderWeight;
		enum disposition_e { Unknown, Normal, TooSmall, TooBig, FixedSize };
		std::unordered_map<const item*, disposition_e, std::hash<const item*>, std::equal_to<const item*>, boost::pool_allocator<std::pair<const item*, disposition_e>>> itemDispositions;
		std::unordered_set<const item*, std::hash<const item*>, std::equal_to<const item*>, boost::pool_allocator<const item*>> expandersUsingLeftover;
		auto items_not_using_leftover = [&itemDispositions]() -> std::size_t 
		{ 
			std::size_t result = 0;
			for (auto& i : itemDispositions)
				if (i.second == TooSmall || i.second == TooBig || i.second == FixedSize)
					++result;
			return result;
		};
		bool done = false;
		while (!done)
		{
			done = true;
			for (const auto& item : items())
			{
				if (!item.visible())
					continue;
				if (expandersUsingLeftover.find(&item) != expandersUsingLeftover.end())
					continue;
				if (item.size_policy() == size_policy::Expanding && item.maximum_size().cy >= leftover)
				{
					if (expandersUsingLeftover.empty())
					{
						itemDispositions.clear();
						leftover = availableSize.cy;
						totalExpanderWeight = size{};
						eachLeftover = 0.0;
					}
					expandersUsingLeftover.insert(&item);
					totalExpanderWeight += item.weight();
					done = false;
					break;
				}
				else if (item.size_policy() != size_policy::Expanding && !expandersUsingLeftover.empty())
				{
					if (itemDispositions[&item] != TooBig)
					{
						if (itemDispositions[&item] == TooSmall)
							leftover += item.maximum_size().cy;
						itemDispositions[&item] = TooBig;
						leftover -= item.minimum_size().cy;
						done = false;
					}
				}
				else if (item.maximum_size().cy < eachLeftover)
				{
					if (itemDispositions[&item] != TooSmall && itemDispositions[&item] != Normal && itemDispositions[&item] != FixedSize)
					{
						if (itemDispositions[&item] == TooBig)
							leftover += item.minimum_size().cy;
						itemDispositions[&item] = item.is_fixed_size() ? FixedSize : TooSmall;
						leftover -= item.maximum_size().cy;
						if (expandersUsingLeftover.empty())
							eachLeftover = std::floor(leftover / (items_visible() - items_not_using_leftover()));
						done = false;
					}
				}
				else if (item.minimum_size().cy > eachLeftover)
				{
					if (itemDispositions[&item] != TooBig && itemDispositions[&item] != FixedSize)
					{
						if (itemDispositions[&item] == TooSmall)
							leftover += item.maximum_size().cy;
						itemDispositions[&item] = item.is_fixed_size() ? FixedSize : TooBig;
						leftover -= item.minimum_size().cy;
						if (expandersUsingLeftover.empty())
							eachLeftover = std::floor(leftover / (items_visible() - items_not_using_leftover()));
						done = false;
					}
				}
				else if (itemDispositions[&item] != Normal && itemDispositions[&item] != FixedSize)
				{
					if (itemDispositions[&item] == TooSmall)
						leftover += item.maximum_size().cy;
					else if (itemDispositions[&item] == TooBig)
						leftover += item.minimum_size().cy;
					itemDispositions[&item] = item.is_fixed_size() ? FixedSize : Normal;
					if (expandersUsingLeftover.empty())
						eachLeftover = std::floor(leftover / (items_visible() - items_not_using_leftover()));
					done = false;
				}
			}	
		}
		if (leftover < 0.0)
		{
			leftover = 0.0;
			eachLeftover = 0.0;
		}
		uint32_t numberUsingLeftover = items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) - items_not_using_leftover();
		uint32_t bitsLeft = static_cast<int32_t>(leftover - (eachLeftover * numberUsingLeftover));
		if (!expandersUsingLeftover.empty())
		{
			size::dimension_type totalIntegralAmount = 0.0;
			for (const auto& s : expandersUsingLeftover)
				totalIntegralAmount += std::floor(s->weight().cy / totalExpanderWeight.cy * leftover);
			bitsLeft = static_cast<int32_t>(leftover - totalIntegralAmount);
		}
		neolib::bresenham_counter<int32_t> bits(bitsLeft, numberUsingLeftover);
		uint32_t previousBit = 0;
		point nextPos = aPosition;
		nextPos.x += margins().left;
		nextPos.y += margins().top;
		for (auto& item : items())
		{
			if (!item.visible())
				continue;
			size s{ std::min(std::max(item.minimum_size().cx, availableSize.cx), item.maximum_size().cx), 0 };
			point alignmentAdjust;
			switch (iHorizontalAlignment)
			{
			case alignment::Left:
				alignmentAdjust.x = 0;
				break;
			case alignment::Right:
				alignmentAdjust.x = availableSize.cx - s.cx;
				break;
			case alignment::Centre:
			default:
				alignmentAdjust.x = std::ceil((availableSize.cx - s.cx) / 2.0);
				break;
			}
			if (itemDispositions[&item] == TooBig || itemDispositions[&item] == FixedSize)
				s.cy = item.minimum_size().cy;
			else if (itemDispositions[&item] == TooSmall)
				s.cy = item.maximum_size().cy;
			else if (expandersUsingLeftover.find(&item) != expandersUsingLeftover.end())
			{
				uint32_t bit = bitsLeft != 0 ? bits() : 0;
				s.cy = std::floor(item.weight().cy / totalExpanderWeight.cy * leftover) + static_cast<size::dimension_type>(bit - previousBit);
				previousBit = bit;
			}
			else
			{
				uint32_t bit = bitsLeft != 0 ? bits() : 0;
				s.cy = eachLeftover + static_cast<size::dimension_type>(bit - previousBit);
				previousBit = bit;
			}
			item.layout(nextPos + alignmentAdjust, s);
			if (!item.get().is<item::spacer_pointer>() && (s.cx == 0.0 || s.cy == 0.0))
				continue;
			nextPos.y += s.cy;
			if (!item.get().is<item::spacer_pointer>())
				nextPos.y += spacing().cy;
		}
		owner()->layout_items_completed();
	}
}