// unit_spin_box.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gui/widget/spin_box.hpp>

namespace neogfx
{
    template <typename T, typename... Units>
    class basic_unit_spin_box : public basic_spin_box<T>
    {
        meta_object(basic_spin_box<T>)
    public:
        using value_type = T;
    public:
        basic_unit_spin_box();
        basic_unit_spin_box(i_widget& aParent);
        basic_unit_spin_box(i_layout& aLayout);
    };

    extern template class basic_unit_spin_box<float>;
    extern template class basic_unit_spin_box<double>;
    
    using float_unit_spin_box = basic_unit_spin_box<float>;
    using double_unit_spin_box = basic_unit_spin_box<double>;

    using unit_spin_box = double_unit_spin_box;
}