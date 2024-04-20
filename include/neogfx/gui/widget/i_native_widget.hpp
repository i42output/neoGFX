// i_native_widget.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
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
    enum native_widget_style : std::uint32_t
    {
        Default         = 0x00000000,
        Transparent     = 0x00000001,
        ClickThrough    = 0x00000002,
    };

    inline constexpr native_widget_style operator|(native_widget_style aLhs, native_widget_style aRhs)
    {
        return static_cast<native_widget_style>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr native_widget_style operator&(native_widget_style aLhs, native_widget_style aRhs)
    {
        return static_cast<native_widget_style>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    class i_native_widget
    {
    public:
        virtual ~i_native_widget() = default;
    public:
        virtual native_widget_style style() const = 0;
        virtual void set_style(native_widget_style aStyle) = 0;
    };
}
