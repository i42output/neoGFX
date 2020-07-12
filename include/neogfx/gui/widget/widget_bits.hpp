// widget_bits.hpp
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

namespace neogfx
{
    enum class widget_part : uint32_t
    {
        Client,
        NonClient,
        NonClientOther,
        TitleBar,
        Grab,
        Border,
        BorderLeft,
        BorderTopLeft,
        BorderTop,
        BorderTopRight,
        BorderRight,
        BorderBottomRight,
        BorderBottom,
        BorderBottomLeft,
        GrowBox,
        CloseButton,
        MaximizeButton,
        MinimizeButton,
        RestoreButton,
        Menu,
        SystemMenu,
        VerticalScrollbar,
        HorizontalScrollbar,
        Nowhere,
        NowhereError
    };

    inline constexpr widget_part operator|(widget_part aLhs, widget_part aRhs)
    {
        return static_cast<widget_part>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr widget_part operator&(widget_part aLhs, widget_part aRhs)
    {
        return static_cast<widget_part>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr widget_part& operator|=(widget_part& aLhs, widget_part aRhs)
    {
        return aLhs = static_cast<widget_part>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr widget_part& operator&=(widget_part& aLhs, widget_part aRhs)
    {
        return aLhs = static_cast<widget_part>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline bool capture_ok(widget_part aWidgetPart)
    {
        switch (aWidgetPart)
        {
        case widget_part::Client:
        case widget_part::VerticalScrollbar:
        case widget_part::HorizontalScrollbar:
        case widget_part::CloseButton:
        case widget_part::MaximizeButton:
        case widget_part::MinimizeButton:
        case widget_part::RestoreButton: 
            return true;
        default:
            return false;
        }
    }

    enum class focus_policy : uint32_t
    {
        NoFocus           = 0x00000000,
        ClickFocus        = 0x00000001,
        TabFocus          = 0x00000002,
        ClickTabFocus     = ClickFocus | TabFocus,
        StrongFocus       = ClickTabFocus,
        WheelFocus        = 0x00000004, // todo
        PointerFocus      = 0x00000008, // todo
        ConsumeTabKey     = 0x10000000,
        ConsumeReturnKey  = 0x20000000,
        IgnoreNonClient   = 0x40000000
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::widget_part> neolib::enum_enumerators_v<neogfx::widget_part>
{
    declare_enum_string(neogfx::widget_part, Client)
    declare_enum_string(neogfx::widget_part, NonClient)
    declare_enum_string(neogfx::widget_part, NonClientOther)
    declare_enum_string(neogfx::widget_part, TitleBar)
    declare_enum_string(neogfx::widget_part, Grab)
    declare_enum_string(neogfx::widget_part, Border)
    declare_enum_string(neogfx::widget_part, BorderLeft)
    declare_enum_string(neogfx::widget_part, BorderTopLeft)
    declare_enum_string(neogfx::widget_part, BorderTop)
    declare_enum_string(neogfx::widget_part, BorderTopRight)
    declare_enum_string(neogfx::widget_part, BorderRight)
    declare_enum_string(neogfx::widget_part, BorderBottomRight)
    declare_enum_string(neogfx::widget_part, BorderBottom)
    declare_enum_string(neogfx::widget_part, BorderBottomLeft)
    declare_enum_string(neogfx::widget_part, GrowBox)
    declare_enum_string(neogfx::widget_part, CloseButton)
    declare_enum_string(neogfx::widget_part, MaximizeButton)
    declare_enum_string(neogfx::widget_part, MinimizeButton)
    declare_enum_string(neogfx::widget_part, RestoreButton)
    declare_enum_string(neogfx::widget_part, Menu)
    declare_enum_string(neogfx::widget_part, SystemMenu)
    declare_enum_string(neogfx::widget_part, VerticalScrollbar)
    declare_enum_string(neogfx::widget_part, HorizontalScrollbar)
    declare_enum_string(neogfx::widget_part, Nowhere)
    declare_enum_string(neogfx::widget_part, NowhereError)
};

template <>
const neolib::enum_enumerators_t<neogfx::focus_policy> neolib::enum_enumerators_v<neogfx::focus_policy>
{
    declare_enum_string(neogfx::focus_policy, NoFocus)
    declare_enum_string(neogfx::focus_policy, ClickFocus)
    declare_enum_string(neogfx::focus_policy, TabFocus)
    declare_enum_string(neogfx::focus_policy, ClickTabFocus)
    declare_enum_string(neogfx::focus_policy, StrongFocus)
    declare_enum_string(neogfx::focus_policy, WheelFocus)
    declare_enum_string(neogfx::focus_policy, PointerFocus)
    declare_enum_string(neogfx::focus_policy, ConsumeTabKey)
    declare_enum_string(neogfx::focus_policy, ConsumeReturnKey)
    declare_enum_string(neogfx::focus_policy, IgnoreNonClient)
};

namespace neogfx
{
    enum class focus_event
    {
        FocusGained,
        FocusLost
    };

    enum class focus_reason
    {
        ClickNonClient,
        ClickClient,
        Tab,
        Wheel,
        Pointer,
        WindowActivation,
        Other
    };

    enum class capture_reason
    {
        MouseEvent,
        Other
    };

    inline focus_policy operator|(focus_policy aLhs, focus_policy aRhs)
    {
        return static_cast<focus_policy>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline focus_policy& operator|=(focus_policy& aLhs, focus_policy aRhs)
    {
        aLhs = static_cast<focus_policy>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
        return aLhs;
    }

    inline focus_policy operator&(focus_policy aLhs, focus_policy aRhs)
    {
        return static_cast<focus_policy>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline focus_policy& operator&=(focus_policy& aLhs, focus_policy aRhs)
    {
        aLhs = static_cast<focus_policy>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
        return aLhs;
    }

    inline focus_policy operator~(focus_policy aLhs)
    {
        return static_cast<focus_policy>(~static_cast<uint32_t>(aLhs));
    }
}