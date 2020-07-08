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
        void fix_weightings() override
        {
            auto& self = static_cast<i_layout_item&>(*this);
            auto& layout = (self.is_layout() ? self.as_layout() : self.as_widget().layout());
            if (base_type::debug() == this)
                std::cout << typeid(*this).name() << "::fix_weightings(): ";
            for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
            {
                auto& item = layout.item_at(itemIndex);
                item.set_weight(calculate_relative_weight(layout, item), false);
                if (base_type::debug() == this)
                    std::cout << "(" << typeid(item).name() << ")" << item.extents() << ":" << item.weight() << ", ";
                item.set_fixed_size({}, false);
            }
            if (base_type::debug() == this)
                std::cout << std::endl;
        }
        void clear_weightings() override
        {
            auto& self = static_cast<i_layout_item&>(*this);
            auto& layout = (self.is_layout() ? self.as_layout() : self.as_widget().layout());
            for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
            {
                auto& item = layout.item_at(itemIndex);
                item.set_weight({});
            }
        }
        void anchor_to(i_anchorable& aRhs, const i_string& aLhsAnchor, anchor_constraint_function aLhsFunction, const i_string& aRhsAnchor, anchor_constraint_function aRhsFunction) override
        {
            base_type::anchor_to(aRhs, aLhsAnchor, aLhsFunction, aRhsAnchor, aRhsFunction);
            auto& self = static_cast<i_layout_item&>(*this);
            if (self.is_layout())
                self.as_layout().invalidate();
            else if (self.has_parent_layout())
                self.parent_layout().invalidate();
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
