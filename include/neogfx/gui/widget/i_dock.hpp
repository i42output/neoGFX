// i_dock.hpp
/*
  neogfx C++ GUI Library
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neolib/i_vector.hpp>
#include <neolib/i_enum.hpp>
#include <neogfx/gui/widget/i_skinnable_item.hpp>
#include <neogfx/gui/widget/i_dockable.hpp>

namespace neogfx
{
    enum class dock_area : uint32_t
    {
        None        = 0x00000000,
        Top         = 0x00000001,
        Bottom      = 0x00000002,
        Left        = 0x00000004,
        Right       = 0x00000008,
        North       = Top,
        South       = Bottom,
        East        = Right,
        West        = Left,
        Horizontal  = Top | Bottom,
        Vertical    = Left | Right,
        Any         = Horizontal | Vertical
    };

    inline constexpr dock_area operator|(dock_area aLhs, dock_area aRhs)
    {
        return static_cast<dock_area>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr dock_area operator&(dock_area aLhs, dock_area aRhs)
    {
        return static_cast<dock_area>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }
}

template <>
const neolib::enum_enumerators_t<neogfx::dock_area> neolib::enum_enumerators_v<neogfx::dock_area>
{
    declare_enum_string(neogfx::dock_area, Top)
    declare_enum_string(neogfx::dock_area, Bottom)
    declare_enum_string(neogfx::dock_area, Left)
    declare_enum_string(neogfx::dock_area, Right)
    declare_enum_string(neogfx::dock_area, North)
    declare_enum_string(neogfx::dock_area, South)
    declare_enum_string(neogfx::dock_area, East)
    declare_enum_string(neogfx::dock_area, West)
    declare_enum_string(neogfx::dock_area, Horizontal)
    declare_enum_string(neogfx::dock_area, Vertical)
    declare_enum_string(neogfx::dock_area, Any)
};

namespace neogfx
{
    class i_dock : public virtual i_skinnable_item
    {
    public:
        virtual dock_area area() const = 0;
        virtual void set_area(dock_area aArea) = 0;
    public:
        virtual ~i_dock() = default;
    public:
        virtual const neolib::i_vector<i_dockable>& items() const = 0;
    };
}