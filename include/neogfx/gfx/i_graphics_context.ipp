// graphics_context.ipp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>

namespace neogfx
{
    template <typename Filter>
    scoped_filter<Filter>::scoped_filter(i_rendering_context& aRc, Filter const& aFilter, bool aSubtractRadius) :
        iRc{ aRc },
        iFilter{ aFilter },
        iBufferRect{ point{}, aFilter.region.extents() + size{ aFilter.radius * 2.0 } },
        iBuffers{ std::move(create_ping_pong_buffers(aRc, iBufferRect.extents(), texture_sampling::Multisample, color{}, aFilter.radius + 1.0)) },
        iRenderTarget{ front_buffer() },
        iSubtractRadius{ aSubtractRadius }
    {
        front_buffer().begin_redirect(aRc, aRc.origin());
        front_buffer().set_origin(aRc.origin() - aFilter.region.top_left() + point{ aFilter.radius, aFilter.radius });
    }

    template <typename Filter>
    scoped_filter<Filter>::~scoped_filter()
    {
        execute();
    }

    template <typename Filter>
    i_graphics_context& scoped_filter<Filter>::front_buffer() const
    {
        return iBuffers.buffer1->gc();
    }

    template <typename Filter>
    i_graphics_context& scoped_filter<Filter>::back_buffer() const
    {
        return iBuffers.buffer2->gc();
    }

    template <typename Filter>
    void scoped_filter<Filter>::execute()
    {
        if (iExecuted)
            return;
        iExecuted = true;

        front_buffer().end_redirect();
        front_buffer().set_origin({});
        
        {
            scoped_render_target srt{ back_buffer() };
            back_buffer().blit(iBufferRect, front_buffer(), iBufferRect);
        }

        if constexpr (std::is_same_v<Filter, blur_filter>)
            back_buffer().blur(iBufferRect, front_buffer(), iBufferRect, iFilter.radius, iFilter.algorithm, iFilter.parameter1, iFilter.parameter2);
        
        rect const drawRect{ iFilter.region.top_left() - (iSubtractRadius ? point{ iFilter.radius, iFilter.radius } : point{}), iBufferRect.extents() };
        
        auto& finalBuffer = static_cast<std::int32_t>(iFilter.radius) % 2 == 0   ? front_buffer() : back_buffer();
        
        scoped_render_target srt{ iRc };
        iRc.blit(drawRect, finalBuffer.render_target().target_texture(), iBufferRect, blending_mode::FilterFinish);
    }
}