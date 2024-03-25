// render_target.cpp
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

#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/i_render_target.hpp>

namespace neogfx
{
    scoped_render_target::scoped_render_target() : iRenderTarget{ nullptr }, iPreviouslyActivatedTarget{ nullptr }
    {
        iPreviouslyActivatedTarget = service<i_rendering_engine>().active_target();
    }

    scoped_render_target::scoped_render_target(const i_render_target& aRenderTarget) : iRenderTarget{ &aRenderTarget }, iPreviouslyActivatedTarget{ nullptr }
    {
        iPreviouslyActivatedTarget = service<i_rendering_engine>().active_target();
        if (iPreviouslyActivatedTarget != iRenderTarget)
        {
            if (iPreviouslyActivatedTarget)
                iPreviouslyActivatedTarget->deactivate_target();
            iRenderTarget->activate_target();
        }
    }

    scoped_render_target::scoped_render_target(const i_rendering_context& aRenderingContext) :
        scoped_render_target{ aRenderingContext.render_target() }
    {
    }

    scoped_render_target::~scoped_render_target()
    {
        if (iRenderTarget && iPreviouslyActivatedTarget != iRenderTarget)
        {
            if (iRenderTarget->target_active())
                iRenderTarget->deactivate_target();
            if (iPreviouslyActivatedTarget)
                iPreviouslyActivatedTarget->activate_target();
        }
        else if (!iRenderTarget && iPreviouslyActivatedTarget)
            iPreviouslyActivatedTarget->activate_target();
    }
}