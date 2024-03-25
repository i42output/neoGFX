// i_push_button.hpp
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

#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/i_button.hpp>

namespace neogfx
{
    enum class push_button_style
    {
        Normal,
        ButtonBox,
        ItemViewHeader,
        Tab,
        DropList,
        SpinBox,
        TitleBar,
        Toolbar
    };

    class i_push_button : public i_button
    {
    public:
        virtual push_button_style style() const = 0;
    };
}