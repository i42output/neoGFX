// layout_item.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/anchor.hpp>
#include <neogfx/gui/layout/anchorable.hpp>

namespace neogfx
{
    template <typename Base>
    class layout_item : public anchorable<Base>
    {
        typedef layout_item<Base> self_type;
        typedef anchorable<Base> base_type;
        // types
    public:
        typedef i_layout_item abstract_type;
    private:
        typedef self_type property_context_type;
        // construction
    public:
        ~layout_item()
        {
            base_type::set_destroying();
        }
        // implementation
    public:
        bool has_layout_manager() const override
        {
            if (!as_layout_item().has_layout_owner())
                return false;
            const i_widget* w = &as_layout_item().layout_owner();
            if (w->is_managing_layout())
                return true;
            while (w->has_parent())
            {
                w = &w->parent();
                if (w->is_managing_layout())
                    return true;
            };
            return false;
        }
        const i_widget& layout_manager() const override
        {
            const i_widget* w = &as_layout_item().layout_owner();
            if (w->is_managing_layout())
                return *w;
            while (w->has_parent())
            {
                w = &w->parent();
                if (w->is_managing_layout())
                    return *w;
            };
            throw no_layout_manager();
        }
        i_widget& layout_manager() override
        {
            return const_cast<i_widget&>(to_const(*this).layout_manager());
        }
    public:
        bool has_size_policy() const override
        {
            return SizePolicy != std::nullopt || has_fixed_size();
        }
        neogfx::size_policy size_policy() const override
        {
            if (SizePolicy != std::nullopt)
                return *SizePolicy;
            else if (has_fixed_size())
                return size_constraint::Fixed;
            else
                return size_constraint::Minimum;
        }
        using i_geometry::set_size_policy;
        void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) override
        {
            if (SizePolicy != aSizePolicy)
            {
                if (base_type::debug() == this)
                    std::cerr << typeid(*this).name() << "::set_size_policy(" << aSizePolicy << ", " << aUpdateLayout << ")" << std::endl;
                SizePolicy = aSizePolicy;
                if (aUpdateLayout)
                {
                    if (as_layout_item().is_widget() && as_layout_item().has_layout_manager())
                        as_layout_item().layout_manager().layout_items(true);
                    else if (as_layout_item().is_layout())
                        as_layout_item().as_layout().invalidate();
                    else if (as_layout_item().has_layout_owner())
                        as_layout_item().layout_owner().layout_root(true);
                }
            }
        }
        bool has_weight() const override
        {
            return Weight != std::nullopt;
        }
        size weight() const override
        {
            if (has_weight())
                return *Weight;
            return size{ 1.0 };
        }
        void set_weight(optional_size const& aWeight, bool aUpdateLayout = true) override
        {
            if (Weight != aWeight)
            {
                if (base_type::debug() == this)
                    std::cerr << typeid(*this).name() << "::set_weight(" << aWeight << ", " << aUpdateLayout << ")" << std::endl;
                Weight.assign(aWeight, aUpdateLayout);
                if (aUpdateLayout)
                {
                    if (as_layout_item().is_widget() && as_layout_item().has_layout_manager())
                        as_layout_item().layout_manager().layout_items(true);
                    else if (as_layout_item().is_layout())
                        as_layout_item().as_layout().invalidate();
                    else if (as_layout_item().has_layout_owner())
                        as_layout_item().layout_owner().layout_root(true);
                }
            }
        }
        bool has_minimum_size() const override
        {
            return MinimumSize != std::nullopt;
        }
        size minimum_size(const optional_size&) const override
        {
            size result;
            if (has_minimum_size())
                result = units_converter(*this).from_device_units(*MinimumSize);
            else
                result = {};
            return result;
        }
        void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true) override
        {
            optional_size newMinimumSize = (aMinimumSize != std::nullopt ? units_converter(*this).to_device_units(*aMinimumSize) : optional_size());
            if (MinimumSize != newMinimumSize)
            {
                if (base_type::debug() == this)
                    std::cerr << typeid(*this).name() << "::set_minimum_size(" << aMinimumSize << ", " << aUpdateLayout << ")" << std::endl;
                MinimumSize.assign(newMinimumSize, aUpdateLayout);
                if (aUpdateLayout)
                {
                    if (as_layout_item().is_widget() && as_layout_item().has_layout_manager())
                        as_layout_item().layout_manager().layout_items(true);
                    else if (as_layout_item().is_layout())
                        as_layout_item().as_layout().invalidate();
                    else if (as_layout_item().has_layout_owner())
                        as_layout_item().layout_owner().layout_root(true);
                }
            }
        }
        bool has_maximum_size() const override
        {
            return MaximumSize != std::nullopt;
        }
        size maximum_size(const optional_size&) const override
        {
            size result;
            if (has_maximum_size())
                result = units_converter(*this).from_device_units(*MaximumSize);
            else
                result = size::max_size();
            return result;
        }
        void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true) override
        {
            optional_size newMaximumSize = (aMaximumSize != std::nullopt ? units_converter(*this).to_device_units(*aMaximumSize) : optional_size());
            if (MaximumSize != newMaximumSize)
            {
                if (base_type::debug() == this)
                    std::cerr << typeid(*this).name() << "::set_maximum_size(" << aMaximumSize << ", " << aUpdateLayout << ")" << std::endl;
                MaximumSize.assign(newMaximumSize, aUpdateLayout);
                if (aUpdateLayout)
                {
                    if (as_layout_item().is_widget() && as_layout_item().has_layout_manager())
                        as_layout_item().layout_manager().layout_items(true);
                    else if (as_layout_item().is_layout())
                        as_layout_item().as_layout().invalidate();
                    else if (as_layout_item().has_layout_owner())
                        as_layout_item().layout_owner().layout_root(true);
                }
            }
        }
        bool has_fixed_size() const override
        {
            return FixedSize != std::nullopt;
        }
        size fixed_size(const optional_size& aAvailableSpace = {}) const override
        {
            if (has_fixed_size())
                return units_converter(*this).from_device_units(*FixedSize);
            return minimum_size(aAvailableSpace);
        }
        void set_fixed_size(const optional_size& aFixedSize, bool aUpdateLayout = true)
        {
            optional_size newFixedSize = (aFixedSize != std::nullopt ? units_converter(*this).to_device_units(*aFixedSize) : optional_size());
            if (FixedSize != newFixedSize)
            {
                if (base_type::debug() == this)
                    std::cerr << typeid(*this).name() << "::set_fixed_size(" << aFixedSize << ", " << aUpdateLayout << ")" << std::endl;
                FixedSize.assign(newFixedSize, aUpdateLayout);
                if (aUpdateLayout)
                {
                    if (as_layout_item().is_widget() && as_layout_item().has_layout_manager())
                        as_layout_item().layout_manager().layout_items(true);
                    else if (as_layout_item().is_layout())
                        as_layout_item().as_layout().invalidate();
                    else if (as_layout_item().has_layout_owner())
                        as_layout_item().layout_owner().layout_root(true);
                }
            }
        }
    public:
        bool has_padding() const override
        {
            return Padding != std::nullopt;
        }
        void set_padding(const optional_padding& aPadding, bool aUpdateLayout = true) override
        {
            auto newPadding = (aPadding != std::nullopt ? units_converter(*this).to_device_units(*aPadding) : optional_padding{});
            if (Padding != newPadding)
            {
                Padding = newPadding;
                if (aUpdateLayout)
                {
                    if (as_layout_item().is_widget() && as_layout_item().has_layout_manager())
                        as_layout_item().layout_manager().layout_items(true);
                    else if (as_layout_item().is_layout())
                        as_layout_item().as_layout().invalidate();
                    else if (as_layout_item().has_layout_owner())
                        as_layout_item().layout_owner().layout_root(true);
                }
            }
        }
    public:
        void fix_weightings(optional_size_policy const& aFixWeightsPolicy = size_constraint::MinimumExpanding) override
        {
            auto& self = static_cast<i_layout_item&>(*this);
            auto& layout = (self.is_layout() ? self.as_layout() : self.as_widget().layout());
            if (base_type::debug() == this)
                std::cerr << typeid(*this).name() << "::fix_weightings(): " << std::endl;
            bool causeWasExplicitResize = false;
            size totalSize;
            size remainingSize;
            size remainingWeight;
            for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
            {
                auto& item = layout.item_at(itemIndex);
                if (!item.visible())
                    continue;
                if (item.has_fixed_size() && item.extents() != item.fixed_size())
                    causeWasExplicitResize = true;
                totalSize += item.extents();
            }
            if (!causeWasExplicitResize)
            {
                for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
                {
                    auto& item = layout.item_at(itemIndex);
                    if (!item.visible())
                        continue;
                    item.set_weight(calculate_relative_weight(layout, item), false);
                }
            }
            else
            {
                remainingSize = totalSize;
                for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
                {
                    auto& item = layout.item_at(itemIndex);
                    if (!item.visible())
                        continue;
                    if (item.size_policy() != size_constraint::Expanding)
                    {
                        auto const& itemExtents = (item.has_fixed_size() && item.extents() != item.fixed_size() ?
                            item.fixed_size() : item.extents());
                        item.set_weight(itemExtents / totalSize, false);
                        remainingSize -= itemExtents;
                    }
                    else
                        remainingWeight += item.weight();
                }
                for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
                {
                    auto& item = layout.item_at(itemIndex);
                    if (!item.visible())
                        continue;
                    if (item.size_policy() == size_constraint::Expanding)
                        item.set_weight(item.weight() / remainingWeight * remainingSize / totalSize, false);
                }
            }
            for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
            {
                auto& item = layout.item_at(itemIndex);
                if (!item.visible())
                    continue;
                if (item.size_policy() != size_constraint::Expanding)
                    item.set_size_policy(aFixWeightsPolicy);
                if (base_type::debug() == this)
                    std::cerr << "(" << typeid(item).name() << ")" << item.extents() << ":" << item.weight() << ", ";
            }
            if (base_type::debug() == this)
                std::cerr << typeid(*this).name() << "::fix_weightings() done" << std::endl;
        }
        void clear_weightings(optional_size_policy const& aFixSizesPolicy = {}) override
        {
            auto& self = static_cast<i_layout_item&>(*this);
            auto& layout = (self.is_layout() ? self.as_layout() : self.as_widget().layout());
            for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
            {
                auto& item = layout.item_at(itemIndex);
                item.set_weight({});
                if (!item.visible())
                    continue;
                if (aFixSizesPolicy && item.size_policy() == size_constraint::MinimumExpanding)
                {
                    item.set_fixed_size(item.extents());
                    item.set_size_policy(aFixSizesPolicy);
                }
            }
        }
        void anchor_to(i_anchorable& aRhs, i_string const& aLhsAnchor, anchor_constraint_function aLhsFunction, i_string const& aRhsAnchor, anchor_constraint_function aRhsFunction) override
        {
            base_type::anchor_to(aRhs, aLhsAnchor, aLhsFunction, aRhsAnchor, aRhsFunction);
            auto& self = static_cast<i_layout_item&>(*this);
            if (self.is_layout())
                self.as_layout().invalidate();
            else if (self.has_parent_layout())
                self.parent_layout().invalidate();
        }
    private:
        i_layout_item const& as_layout_item() const
        {
            return *this;
        }
        i_layout_item& as_layout_item()
        {
            return *this;
        }
        // properties / anchors
    public:
        // todo: declarations for these in i_layout_item when supported
        define_property(property_category::soft_geometry, point, Position, position)
        define_property(property_category::soft_geometry, size, Size, extents)
        define_property(property_category::hard_geometry, optional_padding, Padding, padding)
        define_property(property_category::hard_geometry, optional_size_policy, SizePolicy, size_policy)
        define_property(property_category::hard_geometry, optional_size, Weight, weight)
        define_property(property_category::hard_geometry, optional_size, MinimumSize, minimum_size)
        define_property(property_category::hard_geometry, optional_size, MaximumSize, maximum_size)
        define_property(property_category::hard_geometry, optional_size, FixedSize, fixed_size)
        define_anchor(Position)
        define_anchor(Size)
        define_anchor(Padding)
        define_anchor(MinimumSize)
        define_anchor(MaximumSize)
    };
}
