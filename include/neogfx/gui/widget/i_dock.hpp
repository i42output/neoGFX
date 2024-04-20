// i_dock.hpp
/*
  neogfx C++ App/Game Engine
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

#include <neolib/core/i_vector.hpp>

#include <neogfx/gui/widget/i_skinnable_item.hpp>

namespace neogfx
{
    class i_dockable;

    enum class dock_area : std::uint32_t
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
        return static_cast<dock_area>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr dock_area operator&(dock_area aLhs, dock_area aRhs)
    {
        return static_cast<dock_area>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }
}

begin_declare_enum(neogfx::dock_area)
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
end_declare_enum(neogfx::dock_area)

namespace neogfx
{
    class i_dock : public virtual i_skinnable_item
    {
        friend class dockable;
    public:
        struct item_not_found : std::logic_error { item_not_found() : std::logic_error{ "neogfx::i_dock::item_not_found" } {} };
    public:
        using item = i_ref_ptr<i_dockable>;
        using item_list = neolib::i_vector<item>;
    public:
        virtual dock_area area() const = 0;
        virtual void set_area(dock_area aArea) = 0;
    public:
        virtual ~i_dock() = default;
    public:
        virtual const item_list& items() const = 0;
    private:
        virtual void add(const item& aItem) = 0;
        virtual void remove(const item& aItem) = 0;
    };
}