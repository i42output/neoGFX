// i_mouse.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/variant.hpp>
#include <neogfx/core/event.hpp>

namespace neogfx
{
    enum class mouse_button : uint32_t
    {
        None    = 0x00,
        Left    = 0x01,
        Right    = 0x02,
        Middle    = 0x04,
        X1        = 0x08,
        X2        = 0x10,
        Other    = 0x20
    };

    enum class mouse_wheel : uint32_t
    {
        None        = 0x00,
        Vertical    = 0x01,
        Horizontal    = 0x02
    };

    enum class mouse_system_cursor
    {
        Arrow,
        Ibeam,
        Wait,
        Crosshair,
        WaitArrow,
        SizeNWSE,
        SizeNESW,
        SizeWE,
        SizeNS,
        SizeAll,
        No,
        Hand
    };

    class mouse_cursor
    {
    public:
        typedef neolib::variant<mouse_system_cursor> cursor_type;
        /* todo: support for custom mouse cursors. */
    public:
        struct wrong_type : std::logic_error { wrong_type() : std::logic_error("neogfx::mouse_cursor::wrong_type") {} };
    public:
        mouse_cursor(mouse_system_cursor aSystemCursor) : iType(aSystemCursor)
        {
        }
    public:
        bool is_system_cursor() const
        {
            return std::holds_alternative<mouse_system_cursor>(iType);
        }
        mouse_system_cursor system_cursor() const
        {
            if (is_system_cursor())
                return std::get<mouse_system_cursor>(iType);
            throw wrong_type();
        }
    public:
        cursor_type iType;
    };

    inline mouse_button operator|(mouse_button aLhs, mouse_button aRhs)
    {
        return static_cast<mouse_button>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline mouse_button operator&(mouse_button aLhs, mouse_button aRhs)
    {
        return static_cast<mouse_button>(static_cast<uint32_t>(aLhs)& static_cast<uint32_t>(aRhs));
    }

    inline mouse_button operator~(mouse_button aLhs)
    {
        return static_cast<mouse_button>(~static_cast<uint32_t>(aLhs));
    }

    inline mouse_button& operator|=(mouse_button& aLhs, mouse_button aRhs)
    {
        aLhs = aLhs | aRhs;
        return aLhs;
    }

    inline mouse_button& operator&=(mouse_button aLhs, mouse_button aRhs)
    {
        aLhs = aLhs & aRhs;
        return aLhs;
    }

    inline mouse_wheel operator|(mouse_wheel aLhs, mouse_wheel aRhs)
    {
        return static_cast<mouse_wheel>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline mouse_wheel operator&(mouse_wheel aLhs, mouse_wheel aRhs)
    {
        return static_cast<mouse_wheel>(static_cast<uint32_t>(aLhs)& static_cast<uint32_t>(aRhs));
    }

    inline mouse_wheel operator~(mouse_wheel aLhs)
    {
        return static_cast<mouse_wheel>(~static_cast<uint32_t>(aLhs));
    }

    class i_mouse
    {
    public:
        declare_event(button_pressed, mouse_button)
        declare_event(button_released, mouse_button)
    public:
        virtual ~i_mouse() = default;
    public:
        virtual point position() const = 0;
        virtual mouse_button button_state() const = 0;
    public:
        bool is_button_pressed(mouse_button aButton) const
        {
            return (button_state() & aButton) == aButton;
        }
    };
}