// dockable.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/dockable.hpp>

namespace neogfx
{
    dockable::dockable(std::shared_ptr<i_widget> aDockableWidget, const std::string& aTitle, dock_area aAcceptableDocks) :
        base_type{ neogfx::decoration_style::Tool },
        iTitle{ aTitle },
        iAcceptableDocks{ aAcceptableDocks },
        iDockedWidget{ aDockableWidget },
        iDock{ nullptr }
    {
        title_bar().set_title(aTitle);
        set_client(docked_widget());
        set_padding(neogfx::padding{});
    }

    const neolib::string& dockable::title() const
    {
        return iTitle;
    }

    bool dockable::can_dock(const i_dock& aDock) const
    {
        return (iAcceptableDocks & aDock.area()) != dock_area::None;
    }

    bool dockable::is_docked() const
    {
        return iDock != nullptr;
    }

    const i_dock& dockable::which_dock() const
    {
        if (is_docked())
            return *iDock;
        throw not_docked();
    }

    i_dock& dockable::which_dock()
    {
        if (is_docked())
            return *iDock;
        throw not_docked();
    }

    void dockable::dock(i_dock& aDock)
    {
        if (is_docked())
            undock();
        aDock.add(ref_ptr{ static_cast<i_dockable&>(*this) });
        iDock = &aDock;
        Docked.trigger(aDock);
    }

    void dockable::undock()
    {
        if (!is_docked())
            throw not_docked();
        auto& dock = *iDock;
        dock.remove(ref_ptr{ static_cast<i_dockable&>(*this) });
        iDock = nullptr;
        Undocked.trigger(dock);
    }

    const i_widget& dockable::docked_widget() const
    {
        return *iDockedWidget;
    }

    i_widget& dockable::docked_widget()
    {
        return *iDockedWidget;
    }

    color dockable::frame_color() const
    {
        return has_frame_color() ? base_type::frame_color() : base_type::frame_color().shaded(0x08);
    }
}