// unit_spin_box.inl
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

#pragma once

#include <neogfx/gui/widget/unit_spin_box.hpp>

namespace neogfx
{
    template <typename T, typename... Units>
    basic_unit_spin_box<T, Units...>::basic_unit_spin_box() :
        base_type{}
    {
    }

    template <typename T, typename... Units>
    basic_unit_spin_box<T, Units...>::basic_unit_spin_box(i_widget& aParent) :
        base_type{ aParent }
    {
    }

    template <typename T, typename... Units>
    basic_unit_spin_box<T, Units...>::basic_unit_spin_box(i_layout& aLayout) :
        base_type{ aLayout }
    {
    }
}
