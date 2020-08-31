// layout.inl
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/core/bresenham_counter.hpp>
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
            bool noSpace = (aAvailableSpace == std::nullopt || SpecializedPolicy::cx(*aAvailableSpace) <= SpecializedPolicy::cx(aLayout.minimum_size(aAvailableSpace)) || aLayout.items_visible(ItemTypeSpacer));
            for (auto const& item : aLayout.items())
            {
                if (!item.visible() && !aLayout.ignore_visibility())
                    continue;
                auto const sizeTest = noSpace ? item.minimum_size(aAvailableSpace) : item.maximum_size(aAvailableSpace);
                if (!item.is_spacer() && (SpecializedPolicy::cx(sizeTest) == 0.0 || SpecializedPolicy::cy(sizeTest) == 0.0))
                    ++result;
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
        static const neogfx::alignment AlignmentMask = neogfx::alignment::Top | neogfx::alignment::VCenter | neogfx::alignment::Bottom;
        static const neogfx::alignment InlineAlignmentMask = neogfx::alignment::Left | neogfx::alignment::Center | neogfx::alignment::Right;
        static const point::coordinate_type& x(const point& aPoint) { return aPoint.x; }
        static point::coordinate_type& x(point& aPoint) { return aPoint.x; }
        static const point::coordinate_type& y(const point& aPoint) { return aPoint.y; }
        static point::coordinate_type& y(point& aPoint) { return aPoint.y; }
        static const size::dimension_type& cx(const size& aSize) { return aSize.cx; }
        static size::dimension_type& cx(size& aSize) { return aSize.cx; }
        static const size::dimension_type& cy(const size& aSize) { return aSize.cy; }
        static size::dimension_type& cy(size& aSize) { return aSize.cy; }
        static size::dimension_type cx(const neogfx::padding& aPadding) { return aPadding.left + aPadding.right; }
        static size::dimension_type cy(const neogfx::padding& aPadding) { return aPadding.top + aPadding.bottom; }
        static neogfx::size_constraint size_policy_x(const neogfx::size_policy& aSizePolicy, bool aIgnoreBits = true) { return aSizePolicy.horizontal_size_policy(aIgnoreBits); }
        static neogfx::size_constraint size_policy_y(const neogfx::size_policy& aSizePolicy, bool aIgnoreBits = true) { return aSizePolicy.vertical_size_policy(aIgnoreBits); }
    };

    template <typename Layout>
    struct layout::row_major : common_axis_policy<row_major<Layout>>
    {
        typedef Layout layout_type;
        typedef vertical_layout major_layout;
        typedef horizontal_layout minor_layout;
        static const neogfx::alignment AlignmentMask = neogfx::alignment::Left | neogfx::alignment::Center | neogfx::alignment::Right;
        static const neogfx::alignment InlineAlignmentMask = neogfx::alignment::Top | neogfx::alignment::VCenter | neogfx::alignment::Bottom;
        static const point::coordinate_type& x(const point& aPoint) { return aPoint.y; }
        static point::coordinate_type& x(point& aPoint) { return aPoint.y; }
        static const point::coordinate_type& y(const point& aPoint) { return aPoint.x; }
        static point::coordinate_type& y(point& aPoint) { return aPoint.x; }
        static const size::dimension_type& cx(const size& aSize) { return aSize.cy; }
        static size::dimension_type& cx(size& aSize) { return aSize.cy; }
        static const size::dimension_type& cy(const size& aSize) { return aSize.cx; }
        static size::dimension_type& cy(size& aSize) { return aSize.cx; }
        static size::dimension_type cx(const neogfx::padding& aPadding) { return aPadding.top + aPadding.bottom; }
        static size::dimension_type cy(const neogfx::padding& aPadding) { return aPadding.left + aPadding.right; }
        static neogfx::size_constraint size_policy_x(const neogfx::size_policy& aSizePolicy, bool aIgnoreUniformity = true) { return aSizePolicy.vertical_size_policy(aIgnoreUniformity); }
        static neogfx::size_constraint size_policy_y(const neogfx::size_policy& aSizePolicy, bool aIgnoreUniformity = true) { return aSizePolicy.horizontal_size_policy(aIgnoreUniformity); }
    };

    namespace
    {
        inline size fix_aspect_ratio(const size_policy& aSizePolicy, const size& aSize)
        {
            if (aSizePolicy.maintain_aspect_ratio())
            {
                auto const& aspectRatio = aSizePolicy.aspect_ratio();
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
        inline size::dimension_type weighted_size(const neogfx::layout_item_proxy& aItem, const size& aTotalExpanderWeight, const size::dimension_type aLeftover, const size& aAvailableSize)
        {
            auto guess = AxisPolicy::cx(aItem.weight()) / AxisPolicy::cx(aTotalExpanderWeight) * aLeftover;
            if (!aItem.effective_size_policy().maintain_aspect_ratio())
                return std::floor(guess);
            else
            {
                size aspectCheckSize;
                AxisPolicy::cx(aspectCheckSize) = guess;
                AxisPolicy::cy(aspectCheckSize) = AxisPolicy::cy(aItem.maximum_size(aAvailableSize));
                return std::floor(AxisPolicy::cx(fix_aspect_ratio(aItem.effective_size_policy(), aspectCheckSize)));
            }
        }
    }

    template <typename AxisPolicy>
    size layout::do_minimum_size(const optional_size& aAvailableSpace) const
    {
        if (debug() == this)
            std::cerr << "layout(" << this << ")::do_minimum_size(" << aAvailableSpace << "): " << std::endl;
        uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
        size result;
        if (itemsVisible != 0)
        {
            auto availableSpaceForChildren = aAvailableSpace;
            if (availableSpaceForChildren != std::nullopt)
                *availableSpaceForChildren -= padding().size();
            uint32_t itemsZeroSized = 0;
            for (auto const& item : items())
            {
                if (!item.visible() && !ignore_visibility())
                    continue;
                auto const itemMinSize = item.minimum_size(availableSpaceForChildren);
                if (!item.is_spacer() && (AxisPolicy::cx(itemMinSize) == 0.0 || AxisPolicy::cy(itemMinSize) == 0.0))
                {
                    ++itemsZeroSized;
                    continue;
                }
                AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result), AxisPolicy::cy(itemMinSize));
                AxisPolicy::cx(result) += AxisPolicy::cx(itemMinSize);
            }
            AxisPolicy::cx(result) += AxisPolicy::cx(padding());
            AxisPolicy::cy(result) += AxisPolicy::cy(padding());
            if (itemsVisible - itemsZeroSized > 0)
                AxisPolicy::cx(result) += (AxisPolicy::cx(spacing()) * (itemsVisible - itemsZeroSized - 1));
            AxisPolicy::cx(result) = std::max(AxisPolicy::cx(result), AxisPolicy::cx(layout::minimum_size(aAvailableSpace)));
            AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result), AxisPolicy::cy(layout::minimum_size(aAvailableSpace)));
        }
        if (debug() == this)
            std::cerr << "layout(" << this << ")::do_minimum_size(" << aAvailableSpace << ") --> " << result << std::endl;
        return result;
    }

    template <typename AxisPolicy>
    size layout::do_maximum_size(const optional_size& aAvailableSpace) const
    {
        if (debug() == this)
            std::cerr << "layout(" << this << ")::do_maximum_size(" << aAvailableSpace << "): " << std::endl;
        if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
        {
            size result;
            if (AxisPolicy::size_policy_x(effective_size_policy()) == size_constraint::Expanding ||
                AxisPolicy::size_policy_x(effective_size_policy()) == size_constraint::Maximum)
                AxisPolicy::cx(result) = size::max_dimension();
            if (AxisPolicy::size_policy_y(effective_size_policy()) == size_constraint::Expanding ||
                AxisPolicy::size_policy_y(effective_size_policy()) == size_constraint::Maximum)
                AxisPolicy::cy(result) = size::max_dimension();
            return result;
        }
        auto availableSpaceForChildren = aAvailableSpace;
        if (availableSpaceForChildren != std::nullopt)
            *availableSpaceForChildren -= padding().size();
        uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
        uint32_t itemsZeroSized = 0;
        size result;
        for (auto const& item : items())
        {
            if (!item.visible() && !ignore_visibility())
                continue;
            auto const itemMaxSize = item.maximum_size(availableSpaceForChildren);
            if (!item.is_spacer() && (AxisPolicy::cx(itemMaxSize) == 0.0 || AxisPolicy::cy(itemMaxSize) == 0.0))
                ++itemsZeroSized;
            AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result), 
                AxisPolicy::size_policy_y(effective_size_policy()) == size_constraint::Expanding || AxisPolicy::size_policy_y(effective_size_policy()) == size_constraint::Maximum ? 
                    AxisPolicy::cy(itemMaxSize) : AxisPolicy::cy(item.minimum_size(availableSpaceForChildren)));
            if (AxisPolicy::cx(result) != size::max_dimension() && AxisPolicy::cx(itemMaxSize) != size::max_dimension())
                AxisPolicy::cx(result) += AxisPolicy::cx(itemMaxSize);
            else if (AxisPolicy::cx(itemMaxSize) == size::max_dimension())
                AxisPolicy::cx(result) = size::max_dimension();
        }
        if (AxisPolicy::cx(result) != size::max_dimension() && AxisPolicy::cx(result) != 0.0)
        {
            AxisPolicy::cx(result) += AxisPolicy::cx(padding());
            if (itemsVisible - itemsZeroSized > 1)
                AxisPolicy::cx(result) += (AxisPolicy::cx(spacing()) * (itemsVisible - itemsZeroSized - 1));
            AxisPolicy::cx(result) = std::min(AxisPolicy::cx(result), AxisPolicy::cx(layout::maximum_size(aAvailableSpace)));
        }
        if (AxisPolicy::cy(result) != size::max_dimension() && AxisPolicy::cy(result) != 0.0)
        {
            AxisPolicy::cy(result) += AxisPolicy::cy(padding());
            AxisPolicy::cy(result) = std::min(AxisPolicy::cy(result), AxisPolicy::cy(layout::maximum_size(aAvailableSpace)));
        }
        if (AxisPolicy::cx(result) == 0.0 && 
            (AxisPolicy::size_policy_x(effective_size_policy()) == size_constraint::Expanding ||
            AxisPolicy::size_policy_x(effective_size_policy()) == size_constraint::Maximum))
            AxisPolicy::cx(result) = size::max_dimension();
        if (AxisPolicy::cy(result) == 0.0 &&
            (AxisPolicy::size_policy_y(effective_size_policy()) == size_constraint::Expanding ||
            AxisPolicy::size_policy_y(effective_size_policy()) == size_constraint::Maximum))
            AxisPolicy::cy(result) = size::max_dimension();
        if (debug() == this)
            std::cerr << "layout(" << this << ")::do_maximum_size(" << aAvailableSpace << ") --> " << result << std::endl;
        return result;
    }

    template <typename AxisPolicy>
    void layout::do_layout_items(const point& aPosition, const size& aSize)
    {
        if (debug() == this)
            std::cerr << "layout::do_layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
        set_position(aPosition);
        set_extents(aSize);
        auto itemsVisibleIncludingSpacers = items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer));
        if (itemsVisibleIncludingSpacers == 0)
            return;
        uint32_t itemsVisible = items_visible();
        size availableSize = aSize;
        availableSize.cx -= padding().size().cx;
        availableSize.cy -= padding().size().cy;
        auto itemsZeroSized = AxisPolicy::items_zero_sized(static_cast<typename AxisPolicy::layout_type&>(*this), availableSize);
        auto spaces = (iAlwaysUseSpacing ? itemsVisibleIncludingSpacers : itemsVisible) - itemsZeroSized;
        if (spaces > 1)
            AxisPolicy::cx(availableSize) -= (AxisPolicy::cx(spacing()) * (spaces - 1));
        size::dimension_type leftover = AxisPolicy::cx(availableSize);
        uint32_t itemsUsingLeftover = 0;
        size totalExpanderWeight;
        enum disposition_e { Unknown, Weighted, Unweighted, TooSmall, FixedSize };
        std::unordered_map<const item*, disposition_e, std::hash<const item*>, std::equal_to<const item*>, neolib::fast_pool_allocator<std::pair<const item* const, disposition_e>>> itemDispositions;
        for (auto const& item : items())
        {
            if (!item.visible() && !ignore_visibility())
                continue;
            if (AxisPolicy::size_policy_x(item.effective_size_policy()) == size_constraint::Minimum)
            {
                itemDispositions[&item] = TooSmall;
                leftover -= AxisPolicy::cx(item.minimum_size(availableSize));
                if (leftover < 0.0)
                    leftover = 0.0;
            }
            else if (AxisPolicy::size_policy_x(item.effective_size_policy()) == size_constraint::Fixed)
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
            for (auto const& item : items())
            {
                if (!item.visible() && !ignore_visibility())
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
                    if (AxisPolicy::size_policy_x(item.effective_size_policy(), false) == size_constraint::ExpandingUniform)
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
            for (auto const& item : items())
                if ((item.visible() || ignore_visibility()) && itemDispositions[&item] == Weighted)
                    weightedAmount += weighted_size<AxisPolicy>(item, totalExpanderWeight, leftover, availableSize);
        uint32_t bitsLeft = 0;
        if (itemsUsingLeftover > 0)
            bitsLeft = static_cast<int32_t>(leftover - weightedAmount);
        neolib::bresenham_counter<int32_t> bits(bitsLeft, itemsUsingLeftover);
        uint32_t previousBit = 0;
        point nextPos = aPosition + padding().top_left();
        for (auto& item : *this)
        {
            if (!item.visible() && !ignore_visibility())
                continue;
            if (debug() == &item.subject())
                std::cerr << "Consideration by layout::do_layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
            auto itemMinSize = item.minimum_size(availableSize);
            auto itemMaxSize = item.maximum_size(availableSize);
            size s;
            AxisPolicy::cy(s) = std::min(std::max(AxisPolicy::cy(itemMinSize), AxisPolicy::cy(availableSize)), AxisPolicy::cy(itemMaxSize));
            auto disposition = itemDispositions[&item];
            if (disposition == FixedSize)
                AxisPolicy::cx(s) = AxisPolicy::cx(itemMinSize);
            else if (disposition == TooSmall)
                AxisPolicy::cx(s) = AxisPolicy::cx(AxisPolicy::size_policy_x(item.effective_size_policy()) == size_constraint::Minimum ? itemMinSize : itemMaxSize);
            else if (disposition == Weighted && leftover > 0.0)
            {
                uint32_t bit = 0;
                if (AxisPolicy::size_policy_x(item.effective_size_policy(), false) != size_constraint::ExpandingUniform)
                    bit = (bitsLeft != 0 ? bits() : 0);
                AxisPolicy::cx(s) = weighted_size<AxisPolicy>(item, totalExpanderWeight, leftover, availableSize) + static_cast<size::dimension_type>(bit - previousBit);
                previousBit = bit;
            }
            else
                AxisPolicy::cx(s) = AxisPolicy::cx(itemMinSize);
            s = fix_aspect_ratio(item.effective_size_policy(), s);
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
            case alignment::Center:
            case alignment::VCenter:
                AxisPolicy::y(alignmentAdjust) = std::ceil((AxisPolicy::cy(availableSize) - AxisPolicy::cy(s)) / 2.0);
                break;
            }
            if (AxisPolicy::y(alignmentAdjust) < 0.0)
                AxisPolicy::y(alignmentAdjust) = 0.0;
            item.layout_as(nextPos + alignmentAdjust, s);
            if (!item.is_spacer() && (AxisPolicy::cx(s) == 0.0 || AxisPolicy::cy(s) == 0.0))
                continue;
            AxisPolicy::x(nextPos) += AxisPolicy::cx(s);
            if (!item.is_spacer() || iAlwaysUseSpacing)
                AxisPolicy::x(nextPos) += AxisPolicy::cx(spacing());
        }
        point lastPos = aPosition + aSize;
        lastPos.x -= padding().right;
        lastPos.y -= padding().bottom;
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
            case alignment::Center:
            case alignment::VCenter:
                AxisPolicy::cx(adjust) = std::floor((AxisPolicy::x(lastPos) - AxisPolicy::x(nextPos)) / 2.0);
                break;
            }
            for (auto& item : *this)
            {
                if (!item.visible() && !ignore_visibility())
                    continue;
                item.set_position(item.position() + adjust);
            }
        }
    }
}
