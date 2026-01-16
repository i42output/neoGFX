// utility.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/app/i_app.hpp>

namespace neogfx
{
    inline texture colored_icon(const texture& aSource, const optional_color& aColor = {})
    {
        texture result{ aSource.extents(), 1.0, texture_sampling::Multisample };
        graphics_context gc{ result };
        scalar const outline = 4.0;
        auto const targetRect = aColor ? rect{ point{ outline, outline }, aSource.extents() - size{ outline * 2.0 } } : rect{ point{}, aSource.extents() };
        if (aColor)
        {
            // draw a black outline for a non-text color icon...
            // todo: provide an easier way to blur any drawing primitive
            rect const r{ point{}, size{aSource.extents()} };
            auto pingPongBuffers = create_ping_pong_buffers(gc, aSource.extents(), texture_sampling::Multisample);
            {
                scoped_render_target srt{ pingPongBuffers.buffer1->gc() };
                pingPongBuffers.buffer1->gc().draw_texture(targetRect, aSource, color::Black);
            }
            {
                scoped_render_target srt{ *pingPongBuffers.buffer2 };
                pingPongBuffers.buffer2->gc().blur(r, pingPongBuffers.buffer1->gc(), r, outline, blurring_algorithm::Gaussian, 5.0, 1.0);
            }
            scoped_render_target srt{ gc };
            gc.blit(r, pingPongBuffers.buffer2->gc(), r);
        }
        scoped_render_target srt{ gc };
        gc.draw_texture(targetRect, aSource, aColor ? *aColor : service<i_app>().current_style().palette().color(color_role::Text), shader_effect::ColorizeAlpha);
        return result;
    };
}