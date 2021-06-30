// object_type.hpp
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

namespace neogfx
{
    enum class object_type : uint64_t
    {
        Invalid                         = 0x0000000000000000,
        None                            = 0x0000000000000000,

        MASK_RESERVED                   = 0xFF0FF0FF0000FFFF,
        MASK_USER                       = 0xFFFFFFFFFFFF0000,

        MASK_TYPE                       = 0x00000000FFFFFFFF,
        MASK_TRAITS                     = 0x00000FFF00000000,
        MASK_CATEGORY                   = 0x00FFF00000000000,
        MASK_GENERIC                    = MASK_TRAITS | MASK_CATEGORY,
        MASK_SPECIFIC                   = MASK_TYPE | MASK_TRAITS | MASK_CATEGORY,
        MASK_CONTEXT                    = 0xF000000000000000,

        MASK_RESERVED_TYPE              = MASK_RESERVED & MASK_TYPE,
        MASK_RESERVED_TRAITS            = MASK_RESERVED & MASK_TRAITS,
        MASK_RESERVED_CATEGORY          = MASK_RESERVED & MASK_CATEGORY,
        MASK_RESERVED_GENERIC           = MASK_RESERVED & MASK_GENERIC,
        MASK_RESERVED_SPECIFIC          = MASK_RESERVED & MASK_SPECIFIC,

        HasGeometry                     = 0x0000000100000000,
        HasAlignment                    = 0x0000000200000000,
        LayoutItem                      = 0x0000000400000000 | HasGeometry,
        HasActions                      = 0x0000000800000000,
        HasText                         = 0x0000001000000000,
        HasLabel                        = 0x0000002000000000,
        HasImage                        = 0x0000004000000000,
        HasColor                        = 0x0000008000000000,

        Action                          = 0x0000100000000000,
        Widget                          = 0x0000200000000000 | LayoutItem | HasColor,
        Window                          = 0x0000400000000000 | Widget,
        Layout                          = 0x0000800000000000 | HasAlignment | LayoutItem,
        Menu                            = 0x0001000000000000 | HasActions,
        Button                          = 0x0002000000000000 | Widget | HasText | HasLabel | HasImage,

        App                             = 0x0000000000000001 | HasActions,
        SurfaceWindow                   = 0x0000000000000002 | Window,
        NestedWindow                    = 0x0000000000000003 | Window,
        Dialog                          = 0x0000000000000004 | Window,
        TextWidget                      = 0x0000000000000010 | Widget | HasAlignment | HasText,
        ImageWidget                     = 0x0000000000000011 | Widget | HasImage,
        MenuBar                         = 0x0000000000000020 | Widget | Menu,
        Toolbar                         = 0x0000000000000021 | Widget | HasActions,
        StatusBar                       = 0x0000000000000022 | Widget,
        TabPageContainer                = 0x0000000000000030 | Widget,
        TabPage                         = 0x0000000000000031 | Widget,
        GroupBox                        = 0x0000000000000040 | Widget | HasLabel,
        Canvas                          = 0x0000000000000090 | Widget,
        PushButton                      = 0x0000000000000100 | Button,
        CheckBox                        = 0x0000000000000101 | Button,
        RadioButton                     = 0x0000000000000102 | Button,
        Label                           = 0x0000000000000110 | Widget | HasText | HasImage,
        TextEdit                        = 0x0000000000000120 | Widget | HasAlignment | HasText,
        LineEdit                        = 0x0000000000000121 | Widget | HasAlignment | HasText,
        TextField                       = 0x0000000000000122 | Widget | HasText | HasLabel,
        DropList                        = 0x0000000000000130 | Widget,
        TableView                       = 0x0000000000000131 | Widget,
        Slider                          = 0x0000000000000140 | Widget,
        DoubleSlider                    = 0x0000000000000141 | Widget,
        SpinBox                         = 0x0000000000000142 | Widget,
        DoubleSpinBox                   = 0x0000000000000143 | Widget,
        SliderBox                       = 0x0000000000000144 | Widget,
        DoubleSliderBox                 = 0x0000000000000145 | Widget,
        GradientWidget                  = 0x0000000000000800 | Widget,
        VerticalLayout                  = 0x0000000000000900 | Layout,
        HorizontalLayout                = 0x0000000000000901 | Layout,
        GridLayout                      = 0x0000000000000902 | Layout,
        FlowLayout                      = 0x0000000000000903 | Layout,
        StackLayout                     = 0x0000000000000904 | Layout,
        BorderLayout                    = 0x0000000000000905 | Layout,
        Spacer                          = 0x0000000000000920 | LayoutItem,
        VerticalSpacer                  = 0x0000000000000921 | LayoutItem,
        HorizontalSpacer                = 0x0000000000000922 | LayoutItem,

        Value                           = 0x0000000000000000,
        Reference                       = 0x1000000000000000,
    };

    inline constexpr object_type operator|(object_type aLhs, object_type aRhs)
    {
        return static_cast<object_type>(static_cast<uint64_t>(aLhs) | static_cast<uint64_t>(aRhs));
    }

    inline constexpr object_type operator&(object_type aLhs, object_type aRhs)
    {
        return static_cast<object_type>(static_cast<uint64_t>(aLhs)& static_cast<uint64_t>(aRhs));
    }

    inline constexpr object_type category(object_type aType)
    {
        return aType & object_type::MASK_CATEGORY;
    }
}
