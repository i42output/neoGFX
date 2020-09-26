// line_edit.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/line_edit.hpp>

namespace neogfx
{
    line_edit::line_edit(frame_style aFrameStyle) :
        text_edit{ SingleLine, aFrameStyle }
    {
    }

    line_edit::line_edit(i_widget& aParent, frame_style aFrameStyle) :
        text_edit{ aParent, SingleLine, aFrameStyle }
    {
    }

    line_edit::line_edit(i_layout& aLayout, frame_style aFrameStyle) :
        text_edit{ aLayout, SingleLine, aFrameStyle }
    {
    }

    neogfx::size_policy line_edit::size_policy() const
    {
        if (has_size_policy())
            return text_edit::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum };
    }
}