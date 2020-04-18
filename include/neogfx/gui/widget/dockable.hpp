// dockable.hpp
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
#include <neogfx/gui/widget/i_dock.hpp>
#include <neogfx/gui/widget/i_dockable.hpp>

namespace neogfx
{
    template <typename Widget>
    class dockable : public Widget, public i_dockable
    {
    public:
        define_declared_event(Docked, docked, i_dock&)
        define_declared_event(Undocked, undocked, i_dock&)
    public:
        dockable(dock_area aAcceptableDocks = dock_area::Any) : 
            iAcceptableDocks{ aAcceptableDocks }, iDock{ nullptr }
        {
        }
    public:
        bool can_dock(const i_dock& aDock) const override
        {
            return (iAcceptableDocks & aDock.area()) != dock_area::None;
        }
        bool is_docked() const override
        {
            return iDock != nullptr;
        }
        void dock(i_dock& aDock) override
        {
            if (is_docked())
                undock()
            iDock = &aDock;
            Docked.trigger(aDock);
        }
        void undock() override
        {
            if (!docked())
                throw not_docked();
            auto& dock = *iDock;
            iDock = nullptr;
            Undocked.trigger(dock);
        }
    private:
        dock_area iAcceptableDocks;
        i_dock* iDock;
    };
}