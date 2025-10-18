// line_edit.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include "text_edit.hpp"

namespace neogfx
{
    class line_edit : public text_edit
    {
        meta_object(text_edit)
    public:
        line_edit() :
            line_edit{ text_edit_caps::SingleLine, frame_style::SolidFrame } {}
        line_edit(i_widget& aParent) :
            line_edit{ aParent, text_edit_caps::SingleLine, frame_style::SolidFrame } {}
        line_edit(i_layout& aLayout) :
            line_edit{ aLayout, text_edit_caps::SingleLine, frame_style::SolidFrame } {}
        line_edit(text_edit_caps aType) :
            line_edit{ aType, frame_style::SolidFrame } {}
        line_edit(i_widget& aParent, text_edit_caps aType) :
            line_edit{ aParent, aType, frame_style::SolidFrame } {}
        line_edit(i_layout& aLayout, text_edit_caps aType) :
            line_edit{ aLayout, aType, frame_style::SolidFrame } {}
        line_edit(frame_style aFrameStyle) :
            line_edit{ text_edit_caps::SingleLine, aFrameStyle } {}
        line_edit(i_widget& aParent, frame_style aFrameStyle) :
            line_edit{ aParent, text_edit_caps::SingleLine, aFrameStyle } {}
        line_edit(i_layout& aLayout, frame_style aFrameStyle) :
            line_edit{ aLayout, text_edit_caps::SingleLine, aFrameStyle } {}
        line_edit(text_edit_caps aType, frame_style aFrameStyle);
        line_edit(i_widget& aParent, text_edit_caps aType, frame_style aFrameStyle);
        line_edit(i_layout& aLayout, text_edit_caps aType, frame_style aFrameStyle);
    protected:
        neogfx::size_policy size_policy() const override;
    };
}