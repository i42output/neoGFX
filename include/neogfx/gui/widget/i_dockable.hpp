// i_dockable.hpp
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
#include <neolib/i_enum.hpp>
#include <neogfx/gui/widget/i_tool.hpp>

namespace neogfx
{
    class i_widget;
    class i_dock;

    struct not_docked : std::logic_error { not_docked() : std::logic_error{ "neogfx::not_docked" } {} };

    class i_dockable : public i_tool, public i_reference_counted
    {
    public:
        declare_event(docked, i_dock&)
        declare_event(undocked, i_dock&)
    public:
        typedef i_dockable abstract_type;
    public:
        virtual ~i_dockable() = default;
    public:
        virtual bool can_dock(const i_dock& aDock) const = 0;
        virtual bool is_docked() const = 0;
        virtual const i_dock& which_dock() const = 0;
        virtual i_dock& which_dock() = 0;
        virtual void dock(i_dock& aDock) = 0;
        virtual void undock() = 0;
    public:
        virtual const i_widget& docked_widget() const = 0;
        virtual i_widget& docked_widget() = 0;
    };
}