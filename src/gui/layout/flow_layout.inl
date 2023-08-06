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

namespace neogfx
{
    inline bool flow_layout_fit(scalar aXpos, scalar aAvailableSpace)
    {
        return aXpos <= aAvailableSpace + 0.5;
    }

    template <typename AxisPolicy>
    size flow_layout::do_minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size() || aAvailableSpace == std::nullopt)
            return layout::minimum_size(aAvailableSpace);
        uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
        if (itemsVisible == 0)
            return size{};
        auto availableSpaceForChildren = aAvailableSpace;
        *availableSpaceForChildren -= internal_spacing().size();
        size result;
        uint32_t itemsZeroSized = 0;
        size extent;
        point pos;
        bool previousNonZeroSize = false;
        for (auto const& itemRef : items())
        {
            auto const& item = *itemRef;
            auto const itemSizePolicy = item.effective_size_policy();
            if (!item.visible() && !ignore_child_visibility() && !itemSizePolicy.ignore_visibility())
                continue;
            auto itemMinimumSize = item.minimum_size(availableSpaceForChildren);
            if (!item.is_spacer() && (AxisPolicy::cx(itemMinimumSize) == 0.0 || AxisPolicy::cy(itemMinimumSize) == 0.0))
            {
                ++itemsZeroSized;
                previousNonZeroSize = false;
                continue;
            }
            if (previousNonZeroSize)
                AxisPolicy::x(pos) += AxisPolicy::cx(spacing());
            if (!flow_layout_fit(AxisPolicy::x(pos) + AxisPolicy::cx(itemMinimumSize), AxisPolicy::cx(*availableSpaceForChildren)))
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
        AxisPolicy::cx(result) += AxisPolicy::cx(internal_spacing());
        AxisPolicy::cy(result) += AxisPolicy::cy(internal_spacing());
        AxisPolicy::cx(result) = std::max(std::min(AxisPolicy::cx(result), AxisPolicy::cx(*aAvailableSpace)), AxisPolicy::cx(layout::minimum_size(aAvailableSpace)));
        AxisPolicy::cy(result) = std::max(AxisPolicy::cy(result), AxisPolicy::cy(layout::minimum_size(aAvailableSpace)));
        return result;
    }

    template <typename AxisPolicy>
    size flow_layout::do_maximum_size(optional_size const& aAvailableSpace) const
    {
        if (has_maximum_size())
            return layout::maximum_size(aAvailableSpace);
        if (items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) == 0)
            return size::max_size();
        auto availableSpaceForChildren = aAvailableSpace;
        if (availableSpaceForChildren != std::nullopt)
            *availableSpaceForChildren -= internal_spacing().size();
        uint32_t itemsVisible = always_use_spacing() ? items_visible(static_cast<item_type_e>(ItemTypeWidget | ItemTypeLayout | ItemTypeSpacer)) : items_visible();
        size result;
        coordinate extent = 0.0;
        point pos;
        auto const ourSizePolicy = effective_size_policy();
        for (auto const& itemRef : items())
        {
            auto const& item = *itemRef;
            auto const itemSizePolicy = item.effective_size_policy();
            if (!item.visible() && !ignore_child_visibility() && !itemSizePolicy.ignore_visibility())
                continue;
            auto itemMaximumSize = item.maximum_size(availableSpaceForChildren);
            if (AxisPolicy::cx(itemMaximumSize) != size::max_dimension())
            {
                if (AxisPolicy::x(pos) != size::max_dimension())
                {
                    if (!flow_layout_fit(AxisPolicy::x(pos) + AxisPolicy::cx(itemMaximumSize), AxisPolicy::cx(*availableSpaceForChildren)))
                    {
                        AxisPolicy::x(pos) = (AxisPolicy::cx(itemMaximumSize) + AxisPolicy::cx(spacing()));
                        if (AxisPolicy::cy(itemMaximumSize) != size::max_dimension())
                        {
                            if (AxisPolicy::y(pos) != size::max_dimension())
                                AxisPolicy::y(pos) += (AxisPolicy::cy(itemMaximumSize) + AxisPolicy::cy(spacing()));
                        }
                        else
                            AxisPolicy::y(pos) = size::max_dimension();
                    }
                    else
                    {
                        AxisPolicy::x(pos) += (AxisPolicy::cx(itemMaximumSize) + AxisPolicy::cx(spacing()));
                    }
                }
                else
                {
                    if (AxisPolicy::cy(itemMaximumSize) != size::max_dimension())
                    {
                        if (AxisPolicy::y(pos) != size::max_dimension())
                            AxisPolicy::y(pos) += (AxisPolicy::cy(itemMaximumSize) + AxisPolicy::cy(spacing()));
                    }
                    else
                        AxisPolicy::y(pos) = size::max_dimension();
                }
            }
            else
                AxisPolicy::x(pos) = size::max_dimension();
            extent = std::max(extent, AxisPolicy::x(pos));
        }
        AxisPolicy::cx(result) = extent;
        AxisPolicy::cy(result) = AxisPolicy::y(pos);
        if (AxisPolicy::cx(result) != size::max_dimension())
        {
            AxisPolicy::cx(result) += AxisPolicy::cx(internal_spacing());
            if (itemsVisible > 1)
                AxisPolicy::cx(result) += (AxisPolicy::cx(spacing()) * (itemsVisible - 1));
            AxisPolicy::cx(result) = std::min(AxisPolicy::cx(result), AxisPolicy::cx(layout::maximum_size(aAvailableSpace)));
        }
        if (AxisPolicy::cy(result) != size::max_dimension())
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
        return result;
    }

    template <typename AxisPolicy>
    void flow_layout::do_layout_items(const point& aPosition, const size& aSize)
    {
        set_position(aPosition);
        set_extents(aSize);
        size availableSpace = aSize;
        availableSpace.cx -= internal_spacing().size().cx;
        availableSpace.cy -= internal_spacing().size().cy;
        point pos;
        bool previousNonZeroSize = false;
        typename AxisPolicy::minor_layout rows(*this);
        for (auto& itemRef : *this)
        {
            auto& item = *itemRef;
            auto const itemSizePolicy = item.effective_size_policy();
            if (!item.visible() && !ignore_child_visibility() && !itemSizePolicy.ignore_visibility())
                continue;
            if (&item == &*items().back())
                continue;
            auto itemMinimumSize = item.minimum_size(availableSpace);
            if (!item.is_spacer() && (AxisPolicy::cx(itemMinimumSize) == 0.0 || AxisPolicy::cy(itemMinimumSize) == 0.0))
            {
                previousNonZeroSize = false;
                continue;
            }
            if (previousNonZeroSize)
                AxisPolicy::x(pos) += AxisPolicy::cx(spacing());
            if (!flow_layout_fit(AxisPolicy::x(pos) + AxisPolicy::cx(itemMinimumSize),  AxisPolicy::cx(availableSpace)))
            {
                rows.add(make_ref<typename AxisPolicy::major_layout>());
                rows.get_layout_at(rows.count() - 1).set_size_policy(size_constraint::Minimum);
                rows.get_layout_at(rows.count() - 1).add(item);
                AxisPolicy::x(pos) = AxisPolicy::cx(itemMinimumSize);
            }
            else
            {
                if (rows.count() == 0)
                {
                    rows.add(make_ref<typename AxisPolicy::major_layout>());
                    rows.get_layout_at(rows.count() - 1).set_size_policy(size_constraint::Minimum);
                }
                rows.get_layout_at(rows.count() - 1).add(item);
                AxisPolicy::x(pos) += AxisPolicy::cx(itemMinimumSize);
            }
            previousNonZeroSize = true;
        }
        rows.set_padding(neogfx::padding{});
        rows.set_spacing(spacing());
        for (uint32_t i = 0; i < rows.count(); ++i)
        {
            rows.get_layout_at(i).set_padding(neogfx::padding{});
            rows.get_layout_at(i).set_spacing(spacing());
        }
        rows.layout_items(aPosition + internal_spacing().top_left(), availableSpace);
        rows.remove_all();
        for (auto& itemRef : *this)
        {
            auto& item = *itemRef;
            item.set_parent_layout(this);
            if (has_parent_widget())
                item.set_parent_widget(&parent_widget());
        }
    }
}
