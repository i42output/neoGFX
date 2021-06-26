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
    enum class widget_type : uint32_t
    {
        Client      = 0x00000001,
        NonClient   = 0x00000002,
        Floating    = 0x00010000
    };

    inline constexpr widget_type operator|(widget_type aLhs, widget_type aRhs)
    {
        return static_cast<widget_type>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr widget_type operator&(widget_type aLhs, widget_type aRhs)
    {
        return static_cast<widget_type>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr widget_type& operator|=(widget_type& aLhs, widget_type aRhs)
    {
        return aLhs = static_cast<widget_type>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr widget_type& operator&=(widget_type& aLhs, widget_type aRhs)
    {
        return aLhs = static_cast<widget_type>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    class i_widget; 

    struct widget_part
    {
        i_widget const& widget;
        enum widget_part_e
        {
            Invalid             = 0x00000000,
            Client              = 0x00000001,
            NonClient           = 0x00000002,
            NonClientOther      = 0x00000004,
            TitleBar            = 0x00000008,
            Grab                = 0x00000010,
            Border              = 0x00000020,
            BorderLeft          = 0x00000040,
            BorderTopLeft       = 0x00000080,
            BorderTop           = 0x00000100,
            BorderTopRight      = 0x00000200,
            BorderRight         = 0x00000400,
            BorderBottomRight   = 0x00000800,
            BorderBottom        = 0x00001000,
            BorderBottomLeft    = 0x00002000,
            GrowBox             = 0x00004000,
            CloseButton         = 0x00008000,
            MaximizeButton      = 0x00010000,
            MinimizeButton      = 0x00020000,
            RestoreButton       = 0x00040000,
            Menu                = 0x00080000,
            SystemMenu          = 0x00100000,
            VerticalScrollbar   = 0x00200000,
            HorizontalScrollbar = 0x00400000,
            Nowhere             = 0x40000000,
            NowhereError        = 0x80800000
        } part;
    };

    typedef widget_part::widget_part_e widget_part_e;

    inline bool capture_ok(widget_part aWidgetPart)
    {
        switch (aWidgetPart.part)
        {
        case widget_part::Client:
        case widget_part::NonClient:
        case widget_part::VerticalScrollbar:
        case widget_part::HorizontalScrollbar:
        case widget_part::CloseButton:
        case widget_part::MaximizeButton:
        case widget_part::MinimizeButton:
        case widget_part::RestoreButton: 
            return true;
        case widget_part::Border:
        case widget_part::BorderLeft:
        case widget_part::BorderTopLeft:
        case widget_part::BorderTop:
        case widget_part::BorderTopRight:
        case widget_part::BorderRight:
        case widget_part::BorderBottomRight:
        case widget_part::BorderBottom:
        case widget_part::BorderBottomLeft:
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

    typedef optional<focus_policy> optional_focus_policy;
}

begin_declare_enum(neogfx::widget_part_e)
declare_enum_string(neogfx::widget_part_e, Client)
declare_enum_string(neogfx::widget_part_e, NonClient)
declare_enum_string(neogfx::widget_part_e, NonClientOther)
declare_enum_string(neogfx::widget_part_e, TitleBar)
declare_enum_string(neogfx::widget_part_e, Grab)
declare_enum_string(neogfx::widget_part_e, Border)
declare_enum_string(neogfx::widget_part_e, BorderLeft)
declare_enum_string(neogfx::widget_part_e, BorderTopLeft)
declare_enum_string(neogfx::widget_part_e, BorderTop)
declare_enum_string(neogfx::widget_part_e, BorderTopRight)
declare_enum_string(neogfx::widget_part_e, BorderRight)
declare_enum_string(neogfx::widget_part_e, BorderBottomRight)
declare_enum_string(neogfx::widget_part_e, BorderBottom)
declare_enum_string(neogfx::widget_part_e, BorderBottomLeft)
declare_enum_string(neogfx::widget_part_e, GrowBox)
declare_enum_string(neogfx::widget_part_e, CloseButton)
declare_enum_string(neogfx::widget_part_e, MaximizeButton)
declare_enum_string(neogfx::widget_part_e, MinimizeButton)
declare_enum_string(neogfx::widget_part_e, RestoreButton)
declare_enum_string(neogfx::widget_part_e, Menu)
declare_enum_string(neogfx::widget_part_e, SystemMenu)
declare_enum_string(neogfx::widget_part_e, VerticalScrollbar)
declare_enum_string(neogfx::widget_part_e, HorizontalScrollbar)
declare_enum_string(neogfx::widget_part_e, Nowhere)
declare_enum_string(neogfx::widget_part_e, NowhereError)
end_declare_enum(neogfx::widget_part_e)

begin_declare_enum(neogfx::focus_policy)
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
end_declare_enum(neogfx::focus_policy)

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