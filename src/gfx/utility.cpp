// utility.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/utility.hpp>

namespace neogfx
{
    texture colored_icon(const texture& aSource, const optional_color& aColor, scalar const aOutline)
    {
        texture result{ aSource.extents(), 1.0, texture_sampling::Multisample };
        graphics_context gc{ result };
        scoped_render_target srt{ gc };
        rect const targetRect{ point{}, aSource.extents() };
        if (aColor)
        {
            // draw a black outline for a non-text color icon...
            scoped_filter sf{ gc, blur_filter{ targetRect, aOutline } };
            sf.front_buffer().draw_texture(targetRect, aSource, color::Black);
        }
        gc.draw_texture(targetRect, aSource, aColor ? *aColor : service<i_app>().current_style().palette().color(color_role::Text), shader_effect::ColorizeAlpha);
        return result;
    };
}