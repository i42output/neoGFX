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

#include <neogfx/gui/dialog/color_dialog.hpp>
#include <neogfx/gui/widget/color_widget.hpp>

namespace neogfx
{
    color_widget::color_widget(const neogfx::color& aColor) : 
        base_type{}, iColor{ aColor }
    {
        init();
    }

    color_widget::color_widget(i_widget& aParent, const neogfx::color& aColor) :
        base_type{ aParent }, iColor{ aColor }
    {
        init();
    }

    color_widget::color_widget(i_layout& aLayout, const neogfx::color& aColor) :
        base_type{ aLayout }, iColor{ aColor }
    {
        init();
    }

    color const& color_widget::color() const
    {
        return iColor;
    }

    void color_widget::set_color(neogfx::color const& aColor)
    {
        if (iColor != aColor)
        {
            iColor = aColor;
            ColorChanged();
            update();
        }
    }

    size_policy color_widget::size_policy() const
    {
        if (has_size_policy())
            return base_type::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    size color_widget::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return base_type::minimum_size(aAvailableSpace);
        return size{ 26.0_dip, 26.0_dip };
    }

    void color_widget::paint(i_graphics_context& aGc) const
    {
        auto r = client_rect(false);
        r.deflate(0.5, 0.5); // todo: change to use a refactored snap_to_pixel to accomodate SDF polygon primitives
        aGc.draw_rounded_rect(r, 3.0_dip, pen{ palette_color(color_role::Void), 1.0 });
        r.deflate(0.5, 0.5); // todo: this shouldn't be necessary as draw_rect should behave the same as draw_rounded_rect
        r.deflate(2.0_dip, 2.0_dip);
        aGc.draw_rect(r, pen{ palette_color(color_role::Void), 1.0 });
        r.deflate(1.0, 1.0);
        draw_alpha_background(aGc, r, 4.0);
        if (effectively_enabled())
            aGc.fill_rect(r, iColor);
    }

    void color_widget::init()
    {
        Clicked([&]()
        {
            neogfx::color oldColor = color();
            color_dialog cd{ *this, iColor };
            cd.SelectionChanged([&]()
            {
                set_color(cd.selected_color());
            });
            if (cd.exec() == dialog_result::Accepted)
                set_color(cd.selected_color());
            else
                set_color(oldColor);
        });
    }
}