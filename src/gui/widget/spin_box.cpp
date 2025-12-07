// spin_box.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>

#include <neogfx/gui/widget/spin_box.ipp>

namespace neogfx
{
    template class basic_spin_box<std::int8_t>;
    template class basic_spin_box<std::uint8_t>;
    template class basic_spin_box<std::int16_t>;
    template class basic_spin_box<std::uint16_t>;
    template class basic_spin_box<std::int32_t>;
    template class basic_spin_box<std::uint32_t>;
    template class basic_spin_box<std::int64_t>;
    template class basic_spin_box<std::uint64_t>;
    template class basic_spin_box<float>;
    template class basic_spin_box<double>;
}

