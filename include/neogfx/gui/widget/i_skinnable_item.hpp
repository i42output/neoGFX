// i_skinnable_item.hpp
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

#include <neogfx/core/geometrical.hpp>

namespace neogfx
{
    class i_widget;

    enum class skin_element : uint32_t
    {
        ClickableArea,
        Scrollbar,
        ScrollbarUpArrow,
        ScrollbarLeftArrow,
        ScrollbarDownArrow,
        ScrollbarRightArrow,
        ScrollbarPageUpArea,
        ScrollbarPageLeftArea,
        ScrollbarPageDownArea,
        ScrollbarPageRightArea,
        ScrollbarThumb,
        ButtonFace,
        CheckBox,
        RadioButton,
        SliderTrack,
        SliderThumb,
        SpinnerUpArrow,
        SpinnerDownArrow,
        TabClose,
        TreeExpander,
        ProgressBar,
        Separator,
        Custom1,
        Custom2,
        Custom3,
        Custom4,
        Custom5,
        Custom6,
        Custom7,
        Custom8,
        Custom9,
        Custom10
    };

    class i_skinnable_item
    {
    public:
        virtual ~i_skinnable_item() = default;
    public:
        virtual bool is_widget() const = 0;
        virtual const i_widget& as_widget() const = 0;
    public:
        virtual rect element_rect(skin_element aElement) const = 0;
    };
}
