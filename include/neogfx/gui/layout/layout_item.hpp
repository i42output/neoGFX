// layout_item.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/layout/i_layout_item.hpp>
#include <neogfx/gui/layout/anchor.hpp>
#include <neogfx/gui/layout/anchorable_object.hpp>

namespace neogfx
{
    template <typename Base>
    class layout_item : public anchorable_object<Base>
    {
        typedef layout_item<Base> self_type;
        typedef anchorable_object<Base> base_type;
        // types
    public:
        using typename base_type::abstract_type;
        // properties / anchors
    public:
        define_property(property_category::hard_geometry, optional_logical_coordinate_system, LogicalCoordinateSystem)
        define_property(property_category::soft_geometry, point, Position)
        define_property(property_category::soft_geometry, size, Size)
        define_optional_property(property_category::hard_geometry, optional_margins, Margins, margins)
        define_optional_property(property_category::hard_geometry, optional_size_policy, SizePolicy, size_policy)
        define_optional_property(property_category::hard_geometry, optional_size, Weight, weight)
        define_optional_property(property_category::hard_geometry, optional_size, MinimumSize, minimum_size)
        define_optional_property(property_category::hard_geometry, optional_size, MaximumSize, maximum_size)
        define_anchor(Position)
        define_anchor(Size)
        define_anchor(Margins)
        define_anchor(MinimumSize)
        define_anchor(MaximumSize)
    };
}
