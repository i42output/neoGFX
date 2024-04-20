// layout.ipp
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
#include <neolib/core/bresenham_counter.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>

namespace neogfx
{
    template <typename SpecializedPolicy>
    struct layout::common_axis_policy
    {
        static bool item_zero_sized(layout const& aLayout, i_layout_item const& aItem, size const& aSizeTest)
        {
            auto const itemSizePolicy = aItem.effective_size_policy();
            auto const xSizePolicy = SpecializedPolicy::size_policy_x(itemSizePolicy);
            auto const ySizePolicy = SpecializedPolicy::size_policy_y(itemSizePolicy);
            if (!aItem.visible())
                return false;
            if (!aItem.is_spacer() &&
                ((xSizePolicy != size_constraint::Expanding && SpecializedPolicy::cx(aSizeTest) == 0.0) ||
                (ySizePolicy != size_constraint::Expanding && SpecializedPolicy::cy(aSizeTest) == 0.0)))
                return true;
            return false;
        }
        static std::uint32_t items_zero_sized(layout const& aLayout, optional_size const& aAvailableSpace = optional_size{})
        {
            std::uint32_t result = 0;
            bool const noSpace = (aAvailableSpace == std::nullopt ||
                SpecializedPolicy::cx(*aAvailableSpace) <= SpecializedPolicy::cx(aLayout.minimum_size(aAvailableSpace)) ||
                aLayout.items_visible(ItemTypeSpacer));
            for (auto const& itemRef : aLayout.items())
            {
                auto const& item = *itemRef;
                if (item_zero_sized(aLayout, item,
                    noSpace ? item.minimum_size(aAvailableSpace) : item.maximum_size(aAvailableSpace)))
                    ++result;
            }
            return result;
        }
    };

    template <typename Layout>
    struct layout::row_major : common_axis_policy<row_major<Layout>>
    {
        typedef Layout layout_type;
        typedef horizontal_layout major_layout;
        typedef vertical_layout minor_layout;
        static constexpr bool is_row_major = true;
        static constexpr bool is_column_major = false;
        static constexpr neogfx::alignment AlignmentMask = neogfx::alignment::Top | neogfx::alignment::VCenter | neogfx::alignment::Bottom;
        static constexpr neogfx::alignment InlineAlignmentMask = neogfx::alignment::Left | neogfx::alignment::Center | neogfx::alignment::Right;
        static constexpr const point::coordinate_type& x(const point& aPoint) { return aPoint.x; }
        static constexpr point::coordinate_type& x(point& aPoint) { return aPoint.x; }
        static constexpr const point::coordinate_type& y(const point& aPoint) { return aPoint.y; }
        static constexpr point::coordinate_type& y(point& aPoint) { return aPoint.y; }
        static constexpr const size::dimension_type& cx(const size& aSize) { return aSize.cx; }
        static constexpr size::dimension_type& cx(size& aSize) { return aSize.cx; }
        static constexpr const size::dimension_type& cy(const size& aSize) { return aSize.cy; }
        static constexpr size::dimension_type& cy(size& aSize) { return aSize.cy; }
        static constexpr size::dimension_type cx(const neogfx::padding& aPadding) { return aPadding.left + aPadding.right; }
        static constexpr size::dimension_type cy(const neogfx::padding& aPadding) { return aPadding.top + aPadding.bottom; }
        static constexpr neogfx::size_constraint size_policy_x(const neogfx::size_policy& aSizePolicy, bool aIgnoreUniformity = true) { return aSizePolicy.horizontal_constraint(aIgnoreUniformity); }
        static constexpr neogfx::size_constraint size_policy_y(const neogfx::size_policy& aSizePolicy, bool aIgnoreUniformity = true) { return aSizePolicy.vertical_constraint(aIgnoreUniformity); }
    };

    template <typename Layout>
    struct layout::column_major : common_axis_policy<column_major<Layout>>
    {
        typedef Layout layout_type;
        typedef vertical_layout major_layout;
        typedef horizontal_layout minor_layout;
        static constexpr bool is_row_major = false;
        static constexpr bool is_column_major = true;
        static constexpr neogfx::alignment AlignmentMask = neogfx::alignment::Left | neogfx::alignment::Center | neogfx::alignment::Right;
        static constexpr neogfx::alignment InlineAlignmentMask = neogfx::alignment::Top | neogfx::alignment::VCenter | neogfx::alignment::Bottom;
        static constexpr const point::coordinate_type& x(const point& aPoint) { return aPoint.y; }
        static constexpr point::coordinate_type& x(point& aPoint) { return aPoint.y; }
        static constexpr const point::coordinate_type& y(const point& aPoint) { return aPoint.x; }
        static constexpr point::coordinate_type& y(point& aPoint) { return aPoint.x; }
        static constexpr const size::dimension_type& cx(const size& aSize) { return aSize.cy; }
        static constexpr size::dimension_type& cx(size& aSize) { return aSize.cy; }
        static constexpr const size::dimension_type& cy(const size& aSize) { return aSize.cx; }
        static constexpr size::dimension_type& cy(size& aSize) { return aSize.cx; }
        static constexpr size::dimension_type cx(const neogfx::padding& aPadding) { return aPadding.top + aPadding.bottom; }
        static constexpr size::dimension_type cy(const neogfx::padding& aPadding) { return aPadding.left + aPadding.right; }
        static constexpr neogfx::size_constraint size_policy_x(const neogfx::size_policy& aSizePolicy, bool aIgnoreUniformity = true) { return aSizePolicy.vertical_constraint(aIgnoreUniformity); }
        static constexpr neogfx::size_constraint size_policy_y(const neogfx::size_policy& aSizePolicy, bool aIgnoreUniformity = true) { return aSizePolicy.horizontal_constraint(aIgnoreUniformity); }
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
        inline size::dimension_type weighted_size(const i_layout_item_cache& aItem, const size& aTotalExpanderWeight, const size::dimension_type aLeftover, const size& aAvailableSize)
        {
            auto guess = AxisPolicy::cx(aItem.weight()) / AxisPolicy::cx(aTotalExpanderWeight) * aLeftover;
            auto const itemSizePolicy = aItem.effective_size_policy();
            if (!itemSizePolicy.maintain_aspect_ratio())
                return std::floor(guess);
            else
            {
                size aspectCheckSize;
                AxisPolicy::cx(aspectCheckSize) = guess;
                AxisPolicy::cy(aspectCheckSize) = AxisPolicy::cy(aItem.maximum_size(aAvailableSize));
                return std::floor(AxisPolicy::cx(fix_aspect_ratio(itemSizePolicy, aspectCheckSize)));
            }
        }
    }

    template <typename AxisPolicy>
    inline size layout::do_minimum_size(optional_size const& aAvailableSpace) const
    {
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::do_minimum_size(" << aAvailableSpace << "): " << std::endl;
#endif // NEOGFX_DEBUG
        std::uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
        size result;
        if (has_minimum_size() && !querying_ideal_size())
            result = base_type::minimum_size(aAvailableSpace);
        else if (itemsVisible != 0)
        {
            auto availableSpaceForChildren = aAvailableSpace;
            if (availableSpaceForChildren != std::nullopt)
                *availableSpaceForChildren -= internal_spacing().size();
            std::uint32_t itemsZeroSized = 0;
            for (auto const& itemRef : items())
            {
                auto const& item = *itemRef;
                if (!item.visible())
                    continue;
                auto const itemMinSize = item.minimum_size(availableSpaceForChildren);
                if (!item.is_spacer() && (AxisPolicy::item_zero_sized(*this, item, itemMinSize)))
                {
                    ++itemsZeroSized;
                    continue;
                }
                AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result), AxisPolicy::cy(itemMinSize));
                AxisPolicy::cx(result) += AxisPolicy::cx(itemMinSize);
            }
            AxisPolicy::cx(result) += AxisPolicy::cx(internal_spacing());
            AxisPolicy::cy(result) += AxisPolicy::cy(internal_spacing());
            if (itemsVisible - itemsZeroSized > 0)
                AxisPolicy::cx(result) += (AxisPolicy::cx(spacing()) * (itemsVisible - itemsZeroSized - 1));
            AxisPolicy::cx(result) = std::max(AxisPolicy::cx(result), AxisPolicy::cx(layout::minimum_size(aAvailableSpace)));
            AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result), AxisPolicy::cy(layout::minimum_size(aAvailableSpace)));
        }
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::do_minimum_size(" << aAvailableSpace << ") --> " << result << std::endl;
#endif // NEOGFX_DEBUG
        return result;
    }

    template <typename AxisPolicy>
    inline size layout::do_maximum_size(optional_size const& aAvailableSpace) const
    {
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::do_maximum_size(" << aAvailableSpace << "): " << std::endl;
#endif // NEOGFX_DEBUG
        size result;
        auto const ourSizePolicy = effective_size_policy();
        if (has_maximum_size())
            result = base_type::maximum_size(aAvailableSpace);
        else if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
        {
            if (AxisPolicy::size_policy_x(ourSizePolicy) == size_constraint::Expanding ||
                AxisPolicy::size_policy_x(ourSizePolicy) == size_constraint::Maximum)
                AxisPolicy::cx(result) = size::max_dimension();
            if (AxisPolicy::size_policy_y(ourSizePolicy) == size_constraint::Expanding ||
                AxisPolicy::size_policy_y(ourSizePolicy) == size_constraint::Maximum)
                AxisPolicy::cy(result) = size::max_dimension();
        }
        else
        {
            auto availableSpaceForChildren = aAvailableSpace;
            if (availableSpaceForChildren != std::nullopt)
                *availableSpaceForChildren -= internal_spacing().size();
            std::uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
            std::uint32_t itemsZeroSized = 0;
            for (auto const& itemRef : items())
            {
                auto const& item = *itemRef;
                if (!item.visible())
                    continue;
                auto const itemMaxSize = item.maximum_size(availableSpaceForChildren);
                if (!item.is_spacer() && (AxisPolicy::item_zero_sized(*this, item, itemMaxSize)))
                    ++itemsZeroSized;
                AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result),
                    AxisPolicy::size_policy_y(ourSizePolicy) == size_constraint::Expanding ||
                    AxisPolicy::size_policy_y(ourSizePolicy) == size_constraint::Maximum ?
                    AxisPolicy::cy(itemMaxSize) : AxisPolicy::cy(item.minimum_size(availableSpaceForChildren)));
                if (AxisPolicy::cx(result) != size::max_dimension() && AxisPolicy::cx(itemMaxSize) != size::max_dimension())
                    AxisPolicy::cx(result) += AxisPolicy::cx(itemMaxSize);
                else if (AxisPolicy::cx(itemMaxSize) == size::max_dimension())
                    AxisPolicy::cx(result) = size::max_dimension();
            }
            if (AxisPolicy::cx(result) != size::max_dimension() && AxisPolicy::cx(result) != 0.0)
            {
                AxisPolicy::cx(result) += AxisPolicy::cx(internal_spacing());
                if (itemsVisible - itemsZeroSized > 0)
                    AxisPolicy::cx(result) += (AxisPolicy::cx(spacing()) * (itemsVisible - itemsZeroSized - 1));
                AxisPolicy::cx(result) = std::min(AxisPolicy::cx(result), AxisPolicy::cx(layout::maximum_size(aAvailableSpace)));
            }
            if (AxisPolicy::cy(result) != size::max_dimension() && AxisPolicy::cy(result) != 0.0)
            {
                AxisPolicy::cy(result) += AxisPolicy::cy(internal_spacing());
                AxisPolicy::cy(result) = std::min(AxisPolicy::cy(result), AxisPolicy::cy(layout::maximum_size(aAvailableSpace)));
            }
            if (AxisPolicy::cx(result) == 0.0 &&
                (AxisPolicy::size_policy_x(ourSizePolicy) == size_constraint::Expanding ||
                    AxisPolicy::size_policy_x(ourSizePolicy) == size_constraint::Maximum))
                AxisPolicy::cx(result) = size::max_dimension();
            if (AxisPolicy::cy(result) == 0.0 &&
                (AxisPolicy::size_policy_y(ourSizePolicy) == size_constraint::Expanding ||
                    AxisPolicy::size_policy_y(ourSizePolicy) == size_constraint::Maximum))
                AxisPolicy::cy(result) = size::max_dimension();
        }
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::do_maximum_size(" << aAvailableSpace << ") --> " << result << std::endl;
#endif // NEOGFX_DEBUG
        return result;
    }

    template <typename AxisPolicy>
    inline void layout::do_layout_items(const point& aPosition, const size& aSize)
    {
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::do_layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
#endif // NEOGFX_DEBUG
        set_position(aPosition);
        set_extents(aSize);
        auto const itemsVisible = (always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible());
        if (itemsVisible == 0u)
            return;
        size availableSpace = aSize;
        availableSpace.cx -= internal_spacing().size().cx;
        availableSpace.cy -= internal_spacing().size().cy;
        auto const itemsZeroSized = AxisPolicy::items_zero_sized(static_cast<typename AxisPolicy::layout_type&>(*this), availableSpace);
        if (itemsZeroSized >= itemsVisible)
            return;
        auto spaces = itemsVisible - itemsZeroSized - 1;
        AxisPolicy::cx(availableSpace) -= (AxisPolicy::cx(spacing()) * spaces);
        size::dimension_type leftover = AxisPolicy::cx(availableSpace);
        std::uint32_t itemsUsingLeftover = 0u;
        size totalExpanderWeight;
        for (auto const& itemRef : items())
        {
            auto const& item = *itemRef;
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == &item.subject())
                service<debug::logger>() << neolib::logger::severity::Debug << "Consideration (1) by " << typeid(*this).name() << "::do_layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
#endif // NEOGFX_DEBUG
            auto const itemSizePolicy = item.effective_size_policy();
            if (!item.visible())
                continue;
            if (AxisPolicy::item_zero_sized(*this, item, item.minimum_size(availableSpace)))
                continue;
            auto& disposition = item.cached_disposition();
            disposition = layout_item_disposition::Unknown;
            if (AxisPolicy::size_policy_x(itemSizePolicy) == size_constraint::Minimum)
            {
                disposition = layout_item_disposition::TooSmall;
                leftover -= AxisPolicy::cx(item.minimum_size(availableSpace));
                if (leftover < 0.0)
                    leftover = 0.0;
            }
            else if (AxisPolicy::size_policy_x(itemSizePolicy) == size_constraint::Fixed)
            {
                disposition = layout_item_disposition::FixedSize;
                leftover -= AxisPolicy::cx(item.has_fixed_size() ? item.fixed_size() : item.minimum_size(availableSpace));
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
            for (auto const& itemRef : items())
            {
                auto const& item = *itemRef;
                auto const itemSizePolicy = item.effective_size_policy();
                if (!item.visible())
                    continue;
                if (AxisPolicy::item_zero_sized(*this, item, item.minimum_size(availableSpace)))
                    continue;
                auto& disposition = item.cached_disposition();
                if (disposition != layout_item_disposition::Unknown && disposition != layout_item_disposition::Weighted)
                    continue;
#ifdef NEOGFX_DEBUG
                if (debug::layoutItem == &item.subject())
                    service<debug::logger>() << neolib::logger::severity::Debug << "Consideration (2) by " << typeid(*this).name() << "::do_layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
#endif // NEOGFX_DEBUG
                auto const minSize = AxisPolicy::cx(item.minimum_size(availableSpace));
                auto const maxSize = AxisPolicy::cx(item.maximum_size(availableSpace));
                auto const weightedSize = weighted_size<AxisPolicy>(item, totalExpanderWeight, leftover, availableSpace);
                if (minSize < weightedSize && maxSize > weightedSize)
                {
                    disposition = layout_item_disposition::Weighted;
                    if (AxisPolicy::size_policy_x(itemSizePolicy, false) == size_constraint::ExpandingUniform)
                    {
                        if (--itemsUsingLeftover == 0)
                            break;
                    }
                }
                else
                {
                    disposition = maxSize <= weightedSize ? layout_item_disposition::TooSmall : layout_item_disposition::Unweighted;
                    leftover -= disposition == layout_item_disposition::TooSmall ? maxSize : minSize;
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
            for (auto const& itemRef : items())
            {
                auto const& item = *itemRef;
                if (!item.visible())
                    continue;
                auto& disposition = item.cached_disposition();
                if (disposition == layout_item_disposition::Weighted)
                    weightedAmount += weighted_size<AxisPolicy>(item, totalExpanderWeight, leftover, availableSpace);
            }
        std::uint32_t bitsLeft = 0;
        if (itemsUsingLeftover > 0)
            bitsLeft = static_cast<std::int32_t>(leftover - weightedAmount);
        neolib::bresenham_counter<std::int32_t> bits(bitsLeft, itemsUsingLeftover);
        std::uint32_t previousBit = 0;
        point nextPos = aPosition + internal_spacing().top_left();
        bool addSpace = false;
        for (auto& itemRef : *this)
        {
            auto& item = *itemRef;
            auto const itemSizePolicy = item.effective_size_policy();
            if (!item.visible())
                continue;
            if (addSpace)
            {
                addSpace = false;
                AxisPolicy::x(nextPos) += AxisPolicy::cx(spacing());
            }
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == &item.subject())
                service<debug::logger>() << neolib::logger::severity::Debug << "Consideration (3) by " << typeid(*this).name() << "::do_layout_items(" << aPosition << ", " << aSize << ")" << std::endl;
#endif // NEOGFX_DEBUG
            auto const itemMinSize = item.minimum_size(availableSpace);
            auto const itemMaxSize = item.maximum_size(availableSpace);
            size s;
            AxisPolicy::cy(s) = std::min(std::max(AxisPolicy::cy(itemMinSize), AxisPolicy::cy(availableSpace)), AxisPolicy::cy(itemMaxSize));
            auto disposition = item.cached_disposition();
            if (disposition == layout_item_disposition::FixedSize)
                AxisPolicy::cx(s) = AxisPolicy::cx(itemMinSize);
            else if (disposition == layout_item_disposition::TooSmall)
                AxisPolicy::cx(s) = AxisPolicy::cx(AxisPolicy::size_policy_x(itemSizePolicy) == size_constraint::Minimum ? itemMinSize : itemMaxSize);
            else if (disposition == layout_item_disposition::Weighted && leftover > 0.0)
            {
                std::uint32_t bit = 0;
                if (AxisPolicy::size_policy_x(itemSizePolicy, false) != size_constraint::ExpandingUniform)
                    bit = (bitsLeft != 0 ? bits() : 0);
                AxisPolicy::cx(s) = weighted_size<AxisPolicy>(item, totalExpanderWeight, leftover, availableSpace) + static_cast<size::dimension_type>(bit - previousBit);
                previousBit = bit;
            }
            else
                AxisPolicy::cx(s) = AxisPolicy::cx(itemMinSize);
            s = fix_aspect_ratio(itemSizePolicy, s);
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
                AxisPolicy::y(alignmentAdjust) = AxisPolicy::cy(availableSpace) - AxisPolicy::cy(s);
                break;
            case alignment::VCenter:
            case alignment::Center:
                AxisPolicy::y(alignmentAdjust) = std::ceil((AxisPolicy::cy(availableSpace) - AxisPolicy::cy(s)) / 2.0);
                break;
            }
            if (AxisPolicy::y(alignmentAdjust) < 0.0)
                AxisPolicy::y(alignmentAdjust) = 0.0;
            item.layout_as(nextPos + alignmentAdjust, s);
            if (!item.is_spacer() && (AxisPolicy::cx(s) == 0.0 || AxisPolicy::cy(s) == 0.0))
                continue;
            AxisPolicy::x(nextPos) += AxisPolicy::cx(s);
            if (!item.is_spacer() || iAlwaysUseSpacing)
                addSpace = true;
        }
        point lastPos = aPosition + aSize;
        lastPos.x -= internal_spacing().right;
        lastPos.y -= internal_spacing().bottom;
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
            if (adjust != size{})
            {
                for (auto& itemRef : *this)
                {
                    auto& item = *itemRef;
                    if (!item.visible())
                        continue;
                    item.layout_as(item.position() + adjust, item.extents());
                }
            }
        }
    }
}
