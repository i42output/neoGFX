// color_widget.cpp
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
#include <neogfx/gui/widget/color_widget.hpp>

namespace neogfx
{
    color_widget::color_widget(const neogfx::color& aColor) : 
        base_type{}, iColor{ aColor }
    {
    }

    color_widget::color_widget(i_widget& aParent, const neogfx::color& aColor) :
        base_type{ aParent }, iColor{ aColor }
    {
    }

    color_widget::color_widget(i_layout& aLayout, const neogfx::color& aColor) :
        base_type{ aLayout }, iColor{ aColor }
    {
    }

    color const& color_widget::color() const
    {
        return iColor;
    }

    size_policy color_widget::size_policy() const
    {
        if (has_size_policy())
            return base_type::size_policy();
        return neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum };
    }

    size color_widget::minimum_size(const optional_size& aAvailableSpace) const
    {
        if (has_minimum_size())
            return base_type::minimum_size(aAvailableSpace);
        return base_type::minimum_size(aAvailableSpace) + size{ 16.0_dip, 16.0_dip };
    }

    void color_widget::paint(i_graphics_context& aGc) const
    {
        aGc.fill_rect(client_rect(), iColor);
    }
}