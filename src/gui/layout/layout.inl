// layout.inl
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

#include <unordered_map>
#include <unordered_set>
#include <neolib/bresenham_counter.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>

namespace neogfx
{
	template <typename SpecializedPolicy>
	struct layout::common_axis_policy
	{
		static uint32_t items_zero_sized(layout& aLayout, const optional_size& aAvailableSpace = optional_size())
		{
			uint32_t result = 0;
			if (aAvailableSpace == boost::none || SpecializedPolicy::cx(*aAvailableSpace) <= SpecializedPolicy::cx(aLayout.minimum_size(aAvailableSpace)) || aLayout.items_visible(ItemTypeSpacer))
			{
				for (const auto& item : aLayout.items())
				{
					if (!item.visible())
						continue;
					if (!item.get().is<item::spacer_pointer>() && (SpecializedPolicy::cx(item.minimum_size(aAvailableSpace)) == 0.0 || SpecializedPolicy::cy(item.minimum_size(aAvailableSpace)) == 0.0))
						++result;
				}
			}
			return result;
		}
	};

	template <typename Layout>
	struct layout::column_major : common_axis_policy<column_major<Layout>>
	{
		typedef Layout layout_type;
		typedef horizontal_layout major_layout;
		typedef vertical_layout minor_layout;
		static const neogfx::alignment AlignmentMask = neogfx::alignment::Top | neogfx::alignment::VCentre | neogfx::alignment::Bottom;
		static const neogfx::alignment InlineAlignmentMask = neogfx::alignment::Left | neogfx::alignment::Centre | neogfx::alignment::Right;
		static const point::coordinate_type& x(const point& aPoint) { return aPoint.x; }
		static point::coordinate_type& x(point& aPoint) { return aPoint.x; }
		static const point::coordinate_type& y(const point& aPoint) { return aPoint.y; }
		static point::coordinate_type& y(point& aPoint) { return aPoint.y; }
		static const size::dimension_type& cx(const size& aSize) { return aSize.cx; }
		static size::dimension_type& cx(size& aSize) { return aSize.cx; }
		static const size::dimension_type& cy(const size& aSize) { return aSize.cy; }
		static size::dimension_type& cy(size& aSize) { return aSize.cy; }
		static size::dimension_type cx(const neogfx::margins& aMargins) { return aMargins.left + aMargins.right; }
		static size::dimension_type cy(const neogfx::margins& aMargins) { return aMargins.top + aMargins.bottom; }
		static neogfx::size_policy::size_policy_e size_policy_x(const neogfx::size_policy& aSizePolicy, bool aIgnoreBits = true) { return aSizePolicy.horizontal_size_policy(aIgnoreBits); }
		static neogfx::size_policy::size_policy_e size_policy_y(const neogfx::size_policy& aSizePolicy, bool aIgnoreBits = true) { return aSizePolicy.vertical_size_policy(aIgnoreBits); }
	};

	template <typename Layout>
	struct layout::row_major : common_axis_policy<row_major<Layout>>
	{
		typedef Layout layout_type;
		typedef vertical_layout major_layout;
		typedef horizontal_layout minor_layout;
		static const neogfx::alignment AlignmentMask = neogfx::alignment::Left | neogfx::alignment::Centre | neogfx::alignment::Right;
		static const neogfx::alignment InlineAlignmentMask = neogfx::alignment::Top | neogfx::alignment::VCentre | neogfx::alignment::Bottom;
		static const point::coordinate_type& x(const point& aPoint) { return aPoint.y; }
		static point::coordinate_type& x(point& aPoint) { return aPoint.y; }
		static const point::coordinate_type& y(const point& aPoint) { return aPoint.x; }
		static point::coordinate_type& y(point& aPoint) { return aPoint.x; }
		static const size::dimension_type& cx(const size& aSize) { return aSize.cy; }
		static size::dimension_type& cx(size& aSize) { return aSize.cy; }
		static const size::dimension_type& cy(const size& aSize) { return aSize.cx; }
		static size::dimension_type& cy(size& aSize) { return aSize.cx; }
		static size::dimension_type cx(const neogfx::margins& aMargins) { return aMargins.top + aMargins.bottom; }
		static size::dimension_type cy(const neogfx::margins& aMargins) { return aMargins.left + aMargins.right; }
		static neogfx::size_policy::size_policy_e size_policy_x(const neogfx::size_policy& aSizePolicy, bool aIgnoreNoBits = true) { return aSizePolicy.vertical_size_policy(aIgnoreNoBits); }
		static neogfx::size_policy::size_policy_e size_policy_y(const neogfx::size_policy& aSizePolicy, bool aIgnoreNoBits = true) { return aSizePolicy.horizontal_size_policy(aIgnoreNoBits); }
	};

	namespace
	{
		inline size fix_aspect_ratio(const size_policy& aSizePolicy, const size& aSize)
		{
			if (aSizePolicy.maintain_aspect_ratio())
			{
				const auto& aspectRatio = aSizePolicy.aspect_ratio();
				if (aspectRatio.cx < aspectRatio.cy)
				{
					size result{ aSize.cx, aSize.cx * (aspectRatio.cy / aspectRatio.cx) };
					if (result.cy > aSize.cy)
						result = size{ aSize.cy * (aspectRatio.cx / aspectRatio.cy), aSize.cy };
					return result;
				}
				else // (aspectRatio.cx >= aspectRatio.cy)
				{
					size result{ aSize.cy * (aspectRatio.cx / aspectRatio.cy), aSize.cy };
					if (result.cx > aSize.cx)
						result = size{ aSize.cx, aSize.cx * (aspectRatio.cy / aspectRatio.cx) };
					return result;
				}
			}
			return aSize;
		}

		template <typename AxisPolicy>
		inline size::dimension_type weighted_size(const neogfx::layout_item& aItem, const size& aTotalExpanderWeight, const size::dimension_type aLeftover, const size& aAvailableSize)
		{
			auto guess = AxisPolicy::cx(aItem.weight()) / AxisPolicy::cx(aTotalExpanderWeight) * aLeftover;
			if (!aItem.size_policy().maintain_aspect_ratio())
				return std::floor(guess);
			else
			{
				size aspectCheckSize;
				AxisPolicy::cx(aspectCheckSize) = guess;
				AxisPolicy::cy(aspectCheckSize) = AxisPolicy::cy(aItem.maximum_size(aAvailableSize));
				return std::floor(AxisPolicy::cx(fix_aspect_ratio(aItem.size_policy(), aspectCheckSize)));
			}
		}
	}

	template <typename AxisPolicy>
	size layout::do_minimum_size(const optional_size& aAvailableSpace) const
	{
		uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
		if (itemsVisible == 0)
			return size{};
		auto availableSpaceForChildren = aAvailableSpace;
		if (availableSpaceForChildren != boost::none)
			*availableSpaceForChildren -= margins().size();
		size result;
		uint32_t itemsZeroSized = 0;
		for (const auto& item : items())
		{
			if (!item.visible())
				continue;
			const auto itemMinSize = item.minimum_size(availableSpaceForChildren);
			if (!item.get().is<item::spacer_pointer>() && (AxisPolicy::cx(itemMinSize) == 0.0 || AxisPolicy::cy(itemMinSize) == 0.0))
			{
				++itemsZeroSized;
				continue;
			}
			AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result), AxisPolicy::cy(itemMinSize));
			AxisPolicy::cx(result) += AxisPolicy::cx(itemMinSize);
		}
		AxisPolicy::cx(result) += AxisPolicy::cx(margins());
		AxisPolicy::cy(result) += AxisPolicy::cy(margins());
		if (itemsVisible - itemsZeroSized > 0)
			AxisPolicy::cx(result) += (AxisPolicy::cx(spacing()) * (itemsVisible - itemsZeroSized - 1));
		AxisPolicy::cx(result) = std::max(AxisPolicy::cx(result), AxisPolicy::cx(layout::minimum_size(aAvailableSpace)));
		AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result), AxisPolicy::cy(layout::minimum_size(aAvailableSpace)));
		return result;
	}

	template <typename AxisPolicy>
	size layout::do_maximum_size(const optional_size& aAvailableSpace) const
	{
//		if (debug == this)
//			_asm int 3;
		if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
		{
			size result;
			if (AxisPolicy::size_policy_x(size_policy()) == neogfx::size_policy::Expanding)
				AxisPolicy::cx(result) = size::max_dimension();
			if (AxisPolicy::size_policy_y(size_policy()) == neogfx::size_policy::Expanding)
				AxisPolicy::cy(result) = size::max_dimension();
			return result;
		}
		auto availableSpaceForChildren = aAvailableSpace;
		if (availableSpaceForChildren != boost::none)
			*availableSpaceForChildren -= margins().size();
		uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
		uint32_t itemsZeroSized = 0;
		size result;
		for (const auto& item : items())
		{
			if (!item.visible())
				continue;
			const auto itemMaxSize = item.maximum_size(availableSpaceForChildren);
			if (!item.get().is<item::spacer_pointer>() && (AxisPolicy::cx(itemMaxSize) == 0.0 || AxisPolicy::cy(itemMaxSize) == 0.0))
				++itemsZeroSized;
			AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result), 
				AxisPolicy::size_policy_y(size_policy()) == size_policy::Expanding || AxisPolicy::size_policy_y(size_policy()) == size_policy::Maximum ? 
					AxisPolicy::cy(itemMaxSize) : AxisPolicy::cy(item.minimum_size(availableSpaceForChildren)));
			if (AxisPolicy::cx(result) != size::max_dimension() && AxisPolicy::cx(itemMaxSize) != size::max_dimension())
				AxisPolicy::cx(result) += AxisPolicy::cx(itemMaxSize);
			else if (AxisPolicy::cx(itemMaxSize) == size::max_dimension())
				AxisPolicy::cx(result) = size::max_dimension();
			if (result == size::max_size())
				return result;
		}
		if (AxisPolicy::cx(result) != size::max_dimension())
		{
			AxisPolicy::cx(result) += AxisPolicy::cx(margins());
			if (itemsVisible - itemsZeroSized > 1)
				AxisPolicy::cx(result) += (AxisPolicy::cx(spacing()) * (itemsVisible - itemsZeroSized - 1));
			AxisPolicy::cx(result) = std::min(AxisPolicy::cx(result), AxisPolicy::cx(layout::maximum_size(aAvailableSpace)));
		}
		if (AxisPolicy::cy(result) != size::max_dimension())
		{
			AxisPolicy::cy(result) += AxisPolicy::cy(margins());
			AxisPolicy::cy(result) = std::min(AxisPolicy::cy(result), AxisPolicy::cy(layout::maximum_size(aAvailableSpace)));
		}
		if (AxisPolicy::cx(result) == 0.0 && AxisPolicy::size_policy_x(size_policy()) == neogfx::size_policy::Expanding)
			AxisPolicy::cx(result) = size::max_dimension();
		if (AxisPolicy::cy(result) == 0.0 && AxisPolicy::size_policy_y(size_policy()) == neogfx::size_policy::Expanding)
			AxisPolicy::cy(result) = size::max_dimension();
		return result;
	}

	template <typename AxisPolicy>
	void layout::do_layout_items(const point& aPosition, const size& aSize)
	{
		set_position(aPosition);
		set_extents(aSize);
		auto itemsVisibleIncludingSpacers = items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer));
		if (itemsVisibleIncludingSpacers == 0)
			return;
		uint32_t itemsVisible = items_visible();
		size availableSize = aSize;
		availableSize.cx -= (margins().left + margins().right);
		availableSize.cy -= (margins().top + margins().bottom);
		auto itemsZeroSized = AxisPolicy::items_zero_sized(static_cast<typename AxisPolicy::layout_type&>(*this), availableSize);
		auto spaces = (iAlwaysUseSpacing ? itemsVisibleIncludingSpacers : itemsVisible) - itemsZeroSized;
		if (spaces > 1)
			AxisPolicy::cx(availableSize) -= (AxisPolicy::cx(spacing()) * (spaces - 1));
		size::dimension_type leftover = AxisPolicy::cx(availableSize);
		uint32_t itemsUsingLeftover = 0;
		size totalExpanderWeight;
		enum disposition_e { Unknown, Weighted, Unweighted, TooSmall, FixedSize };
		std::unordered_map<const item*, disposition_e, std::hash<const item*>, std::equal_to<const item*>, boost::pool_allocator<std::pair<const item*, disposition_e>>> itemDispositions;
		for (const auto& item : items())
		{
			if (!item.visible())
				continue;
			if (AxisPolicy::size_policy_x(item.size_policy()) == neogfx::size_policy::Minimum)
			{
				itemDispositions[&item] = TooSmall;
				leftover -= AxisPolicy::cx(item.minimum_size(availableSize));
				if (leftover < 0.0)
					leftover = 0.0;
			}
			else if (AxisPolicy::size_policy_x(item.size_policy()) == neogfx::size_policy::Fixed)
			{
				itemDispositions[&item] = FixedSize;
				leftover -= AxisPolicy::cx(item.minimum_size(availableSize));
				if (leftover < 0.0)
					leftover = 0.0;
			}
			else
			{
				++itemsUsingLeftover;
				totalExpanderWeight += item.weight();
			}
		}
		bool done = false;
		while (!done && itemsUsingLeftover > 0)
		{
			done = true;
			for (const auto& item : items())
			{
				if (!item.visible())
					continue;
				auto& disposition = itemDispositions[&item];
				if (disposition != Unknown && disposition != Weighted)
					continue;
				auto minSize = AxisPolicy::cx(item.minimum_size(availableSize));
				auto maxSize = AxisPolicy::cx(item.maximum_size(availableSize));
				auto weightedSize = weighted_size<AxisPolicy>(item, totalExpanderWeight, leftover, availableSize);
				if (minSize < weightedSize && maxSize > weightedSize)
				{
					disposition = Weighted;
					if (AxisPolicy::size_policy_x(item.size_policy(), false) == neogfx::size_policy::ExpandingNoBits)
					{
						if (--itemsUsingLeftover == 0)
							break;
					}
				}
				else
				{
					disposition = maxSize <= weightedSize ? TooSmall : Unweighted;
					leftover -= AxisPolicy::cx(disposition == TooSmall ? item.maximum_size(availableSize) : item.minimum_size(availableSize));
					if (leftover < 0.0)
						leftover = 0.0;
					totalExpanderWeight -= item.weight();
					if (--itemsUsingLeftover == 0)
						break;
					done = false;
				}
			}
		}
		size::dimension_type weightedAmount = 0.0;
		if (AxisPolicy::cx(totalExpanderWeight) > 0.0)
			for (const auto& item : items())
				if (item.visible() && itemDispositions[&item] == Weighted)
					weightedAmount += weighted_size<AxisPolicy>(item, totalExpanderWeight, leftover, availableSize);
		uint32_t bitsLeft = 0;
		if (itemsUsingLeftover > 0)
			bitsLeft = static_cast<int32_t>(leftover - weightedAmount);
		neolib::bresenham_counter<int32_t> bits(bitsLeft, itemsUsingLeftover);
		uint32_t previousBit = 0;
		point nextPos = aPosition;
		nextPos.x += margins().left;
		nextPos.y += margins().top;
		for (auto& item : items())
		{
			if (!item.visible())
				continue;
			auto itemMinSize = item.minimum_size(availableSize);
			auto itemMaxSize = item.maximum_size(availableSize);
			size s;
			AxisPolicy::cy(s) = std::min(std::max(AxisPolicy::cy(itemMinSize), AxisPolicy::cy(availableSize)), AxisPolicy::cy(itemMaxSize));
			auto disposition = itemDispositions[&item];
			if (disposition == FixedSize)
				AxisPolicy::cx(s) = AxisPolicy::cx(itemMinSize);
			else if (disposition == TooSmall)
				AxisPolicy::cx(s) = AxisPolicy::cx(AxisPolicy::size_policy_x(item.size_policy()) == neogfx::size_policy::Minimum ? itemMinSize : itemMaxSize);
			else if (disposition == Weighted && leftover > 0.0)
			{
				uint32_t bit = 0;
				if (AxisPolicy::size_policy_x(item.size_policy(), false) != neogfx::size_policy::ExpandingNoBits)
					bit = (bitsLeft != 0 ? bits() : 0);
				AxisPolicy::cx(s) = weighted_size<AxisPolicy>(item, totalExpanderWeight, leftover, availableSize) + static_cast<size::dimension_type>(bit - previousBit);
				previousBit = bit;
			}
			else
				AxisPolicy::cx(s) = AxisPolicy::cx(itemMinSize);
			s = fix_aspect_ratio(item.size_policy(), s);
			point alignmentAdjust;
			switch (alignment() & AxisPolicy::AlignmentMask)
			{
			case alignment::Left:
			case alignment::Top:
			default:
				AxisPolicy::y(alignmentAdjust) = 0.0;
				break;
			case alignment::Right:
			case alignment::Bottom:
				AxisPolicy::y(alignmentAdjust) = AxisPolicy::cy(availableSize) - AxisPolicy::cy(s);
				break;
			case alignment::Centre:
			case alignment::VCentre:
				AxisPolicy::y(alignmentAdjust) = std::ceil((AxisPolicy::cy(availableSize) - AxisPolicy::cy(s)) / 2.0);
				break;
			}
			if (AxisPolicy::y(alignmentAdjust) < 0.0)
				AxisPolicy::y(alignmentAdjust) = 0.0;
			item.layout(nextPos + alignmentAdjust, s);
			if (!item.get().is<item::spacer_pointer>() && (AxisPolicy::cx(s) == 0.0 || AxisPolicy::cy(s) == 0.0))
				continue;
			AxisPolicy::x(nextPos) += AxisPolicy::cx(s);
			if (!item.get().is<item::spacer_pointer>() || iAlwaysUseSpacing)
				AxisPolicy::x(nextPos) += AxisPolicy::cx(spacing());
		}
		point lastPos = aPosition + aSize;
		lastPos.x -= margins().right;
		lastPos.y -= margins().bottom;
		if (AxisPolicy::x(nextPos) < AxisPolicy::x(lastPos))
		{
			size adjust;
			switch (alignment() & AxisPolicy::InlineAlignmentMask)
			{
			case alignment::Left:
			case alignment::Top:
			default:
				AxisPolicy::cx(adjust) = 0.0;
				break;
			case alignment::Right:
			case alignment::Bottom:
				AxisPolicy::cx(adjust) = AxisPolicy::x(lastPos) - AxisPolicy::x(nextPos);
				break;
			case alignment::Centre:
			case alignment::VCentre:
				AxisPolicy::cx(adjust) = std::floor((AxisPolicy::x(lastPos) - AxisPolicy::x(nextPos)) / 2.0);
				break;
			}
			for (auto& item : items())
			{
				if (!item.visible())
					continue;
				item.set_position(item.position() + adjust);
			}
		}
	}
}
