// window_bits.hpp
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
#include <neogfx/core/geometrical.hpp>
#include <neogfx/hid/video_mode.hpp>

namespace neogfx
{
    enum class window_style : uint32_t
    {
        Invalid                     = 0x00000000,
        NoDecoration                = 0x00000001,    // No decoration at all (useful for splash screens, for example); this style cannot be combined with others
        TitleBar                    = 0x00000002,    // The window has a titlebar
        NativeTitleBar              = 0x00000004,    // The window has a native titlebar
        SystemMenu                  = 0x00000008,
        MinimizeBox                 = 0x00000010,
        MaximizeBox                 = 0x00000020,
        Resize                      = 0x00000040,    // The window can be resized and has a maximize button
        SizeGrip                    = 0x00000080,
        Close                       = 0x00000100,    // The window has a close button
        Nested                      = 0x00000200,    // The window is not a native desktop window but a part of an existing one
        Fullscreen                  = 0x00000400,    // The window is shown in fullscreen mode; this style cannot be combined with others, and requires a valid video mode
        Main                        = 0x00000800,    // The window is a main window so may go fullscreen or change video mode
        Dialog                      = 0x00001000,
        Popup                       = 0x00002000,    // The window is a popup (e.g. menu)
        Tool                        = 0x00004000,    // The window shouldn't appear on the operating system taskbar
        DropShadow                  = 0x00008000,
        Modal                       = 0x00010000,
        ApplicationModal            = 0x00020000,
        NoActivate                  = 0x00040000,
        RequiresOwnerFocus          = 0x00080000,
        DismissOnOwnerClick         = 0x00100000,
        DismissOnParentClick        = 0x00200000,
        HideOnOwnerClick            = 0x00400000,
        HideOnParentClick           = 0x00800000,
        InitiallyHidden             = 0x01000000,
        InitiallyCentered           = 0x02000000,    // Center on desktop or parent
        InitiallyRenderable         = 0x04000000,
        Weak                        = 0x80000000,
        Default                     = TitleBar | SystemMenu | MinimizeBox | MaximizeBox | Resize | SizeGrip | Close | DropShadow | InitiallyCentered | InitiallyRenderable,
        DefaultDialog               = (Default | Dialog) & ~InitiallyRenderable,
        DefaultNonResizableDialog   = (Default | Dialog) & ~InitiallyRenderable & ~Resize
    };

    inline constexpr window_style operator~(window_style aStyle)
    {
        return static_cast<window_style>(~static_cast<uint32_t>(aStyle));
    }

    inline constexpr window_style operator|(window_style aLhs, window_style aRhs)
    {
        return static_cast<window_style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr window_style operator&(window_style aLhs, window_style aRhs)
    {
        return static_cast<window_style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr window_style& operator|=(window_style& aLhs, window_style aRhs)
    {
        return aLhs = static_cast<window_style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr window_style& operator&=(window_style& aLhs, window_style aRhs)
    {
        return aLhs = static_cast<window_style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    enum class window_state : uint32_t
    {
        Normal      = 0x00000000,
        Iconized    = 0x00000001,
        Maximized   = 0x00000002
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::window_style> neolib::enum_enumerators_v<neogfx::window_style>
{
    declare_enum_string(neogfx::window_style, Invalid)
    declare_enum_string(neogfx::window_style, NoDecoration)
    declare_enum_string(neogfx::window_style, TitleBar)
    declare_enum_string(neogfx::window_style, NativeTitleBar)
    declare_enum_string(neogfx::window_style, SystemMenu)
    declare_enum_string(neogfx::window_style, MinimizeBox)
    declare_enum_string(neogfx::window_style, MaximizeBox)
    declare_enum_string(neogfx::window_style, Resize)
    declare_enum_string(neogfx::window_style, Close)
    declare_enum_string(neogfx::window_style, Nested)
    declare_enum_string(neogfx::window_style, Fullscreen)
    declare_enum_string(neogfx::window_style, Main)
    declare_enum_string(neogfx::window_style, Dialog)
    declare_enum_string(neogfx::window_style, Popup)
    declare_enum_string(neogfx::window_style, Tool)
    declare_enum_string(neogfx::window_style, Modal)
    declare_enum_string(neogfx::window_style, ApplicationModal)
    declare_enum_string(neogfx::window_style, NoActivate)
    declare_enum_string(neogfx::window_style, RequiresOwnerFocus)
    declare_enum_string(neogfx::window_style, DismissOnOwnerClick)
    declare_enum_string(neogfx::window_style, DismissOnParentClick)
    declare_enum_string(neogfx::window_style, HideOnOwnerClick)
    declare_enum_string(neogfx::window_style, HideOnParentClick)
    declare_enum_string(neogfx::window_style, InitiallyHidden)
    declare_enum_string(neogfx::window_style, InitiallyCentered)
    declare_enum_string(neogfx::window_style, DropShadow)
    declare_enum_string(neogfx::window_style, Weak)
    declare_enum_string(neogfx::window_style, Default)
    declare_enum_string(neogfx::window_style, DefaultDialog)
};

template <>
const neolib::enum_enumerators_t<neogfx::window_state> neolib::enum_enumerators_v<neogfx::window_state>
{
    declare_enum_string(neogfx::window_state, Normal)
    declare_enum_string(neogfx::window_state, Iconized)
    declare_enum_string(neogfx::window_state, Maximized)
};

namespace neogfx
{
    class window_placement
    {
    public:
        struct geometry_not_specified : std::logic_error { geometry_not_specified() : std::logic_error{ "neogfx::window_placement::geometry_not_specified" } {} };
        struct invalid_state : std::logic_error { invalid_state() : std::logic_error{ "neogfx::window_placement::invalid_state" } {} };
    public:
        window_placement() :
            iPositionSpecified{},
            iNormalGeometry{},
            iIconizedGeometry{},
            iMaximizedGeometry{},
            iVideoMode{},
            iState{window_state::Normal}
        {
        }
        window_placement(
            const rect& aNormalGeometry,
            const optional_rect& aIconizedGeometry = rect{},
            const optional_rect& aMaximizedGeometry = {},
            const optional_video_mode& aVideoMode = {},
            window_state aState = window_state::Normal) :
            iPositionSpecified{ true },
            iNormalGeometry{ aNormalGeometry },
            iIconizedGeometry{ aIconizedGeometry },
            iMaximizedGeometry{ aMaximizedGeometry },
            iVideoMode{ aVideoMode },
            iState{ aState }
        {
            check_state();
        }
        window_placement(
            const point& aNormalPosition,
            const optional_rect& aIconizedGeometry = rect{},
            const optional_rect& aMaximizedGeometry = {},
            const optional_video_mode& aVideoMode = {},
            window_state aState = window_state::Normal) :
            iPositionSpecified{ true },
            iNormalGeometry{ aNormalPosition },
            iIconizedGeometry{ aIconizedGeometry },
            iMaximizedGeometry{ aMaximizedGeometry },
            iVideoMode{ aVideoMode },
            iState{ aState }
        {
            check_state();
        }
        window_placement(
            const point& aNormalTopLeft,
            const point& aNormalBottomRight,
            const optional_rect& aIconizedGeometry = rect{},
            const optional_rect& aMaximizedGeometry = {},
            const optional_video_mode& aVideoMode = {},
            window_state aState = window_state::Normal) :
            iPositionSpecified{ true },
            iNormalGeometry{ rect{ aNormalTopLeft, aNormalBottomRight } },
            iIconizedGeometry{ aIconizedGeometry },
            iMaximizedGeometry{ aMaximizedGeometry },
            iVideoMode{ aVideoMode },
            iState{ aState }
        {
            check_state();
        }
        window_placement(
            const point& aNormalPosition,
            const size& aNormalSize,
            const optional_rect& aIconizedGeometry = rect{},
            const optional_rect& aMaximizedGeometry = {},
            const optional_video_mode& aVideoMode = {},
            window_state aState = window_state::Normal) :
            iPositionSpecified{ true },
            iNormalGeometry{ rect{ aNormalPosition, aNormalSize } },
            iIconizedGeometry{ aIconizedGeometry },
            iMaximizedGeometry{ aMaximizedGeometry },
            iVideoMode{ aVideoMode },
            iState{ aState }
        {
            check_state();
        }
        window_placement(
            const size& aNormalGeometry,
            const optional_rect& aIconizedGeometry = rect{},
            const optional_rect& aMaximizedGeometry = {},
            const optional_video_mode& aVideoMode = {},
            window_state aState = window_state::Normal) :
            iPositionSpecified{ false },
            iNormalGeometry{ aNormalGeometry },
            iIconizedGeometry{ aIconizedGeometry },
            iMaximizedGeometry{ aMaximizedGeometry },
            iVideoMode{ aVideoMode },
            iState{ aState }
        {
            check_state();
        }
        window_placement(
            const video_mode& aVideoMode) :
            iPositionSpecified{ true },
            iNormalGeometry{ rect{ aVideoMode.resolution() } },
            iIconizedGeometry{ rect{} },
            iMaximizedGeometry{ rect{ aVideoMode.resolution() } },
            iVideoMode{ aVideoMode },
            iState{ window_state::Normal }
        {
            check_state();
        }
    public:
        bool position_specified() const
        {
            return iPositionSpecified;
        }
        const optional_rect& normal_geometry() const
        {
            check_state();
            return iNormalGeometry;
        }
        const optional_rect& iconized_geometry() const
        { 
            check_state();
            return iIconizedGeometry;
        }
        const optional_rect& maximized_geometry() const 
        { 
            check_state();
            return iMaximizedGeometry;
        }
        const optional_video_mode& video_mode() const
        {
            check_state();
            return iVideoMode;
        }
        window_state state() const
        {
            check_state();
            return iState;
        }
        window_placement& set_normal_geometry(const optional_rect& aNormalGeometry)
        {
            iPositionSpecified = !!aNormalGeometry;
            iNormalGeometry = aNormalGeometry;
            return *this;
        }
        window_placement& set_iconized_geometry(const optional_rect& aIconizedGeometry)
        {
            iIconizedGeometry = aIconizedGeometry;
            return *this;
        }
        window_placement& set_maximized_geometry(const optional_rect& aMaximizedGeometry)
        {
            iMaximizedGeometry = aMaximizedGeometry;
            return *this;
        }
        window_placement& set_video_mode(const optional_video_mode& aVideoMode)
        {
            iVideoMode = aVideoMode;
            return *this;
        }
        window_placement& set_state(window_state aState)
        {
            iState = aState;
            return *this;
        }
    public:
        static window_placement default_placement();
    private:
        void check_state() const
        {
            switch (iState)
            {
            case window_state::Normal:
                if (!iNormalGeometry)
                    throw geometry_not_specified();
                break;
            case window_state::Iconized:
                if (!iIconizedGeometry)
                    throw geometry_not_specified();
                break;
            case window_state::Maximized:
                if (!iMaximizedGeometry)
                    throw geometry_not_specified();
                break;
            default:
                throw invalid_state();
            }
        }
    private:
        bool iPositionSpecified;
        optional_rect iNormalGeometry;
        optional_rect iIconizedGeometry;
        optional_rect iMaximizedGeometry;
        optional_video_mode iVideoMode;
        window_state iState;
    };

    typedef std::optional<window_placement> optional_window_placement;
}