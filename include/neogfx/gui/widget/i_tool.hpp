// i_tool.hpp
/*
  neogfx C++ GUI Library
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neolib/i_string.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/i_skinnable_item.hpp>

namespace neogfx
{
    class i_tool : public virtual i_standard_layout_container, public virtual i_skinnable_item
    {
    public:
        virtual ~i_tool() = default;
    public:
        using i_standard_layout_container::is_widget;
        using i_standard_layout_container::as_widget;
    public:
        virtual const neolib::i_string& title() const = 0;
    };
}