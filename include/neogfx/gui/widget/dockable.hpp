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
#include <neogfx/gui/widget/framed_widget.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>

namespace neogfx
{
    class dockable_title_bar : public widget
    {
    public:
        dockable_title_bar(i_dockable& aDockable);
    private:
        i_dockable& iDockable;
        horizontal_layout iLayout;
        label iTitle;
    };

    class dockable : public widget, public reference_counted<i_dockable>
    {
    public:
        define_declared_event(Docked, docked, i_dock&)
        define_declared_event(Undocked, undocked, i_dock&)
    public:
        dockable(const std::shared_ptr<i_widget>& aDockableWidget, const std::string& aTitle = "", dock_area aAcceptableDocks = dock_area::Any);
    public:
        const neolib::string& title() const override;
    public:
        bool can_dock(const i_dock& aDock) const override;
        bool is_docked() const override;
        void dock(i_dock& aDock) override;
        void undock() override;
    public:
        const i_widget& as_widget() const override;
        i_widget& as_widget() override;
        const i_widget& docked_widget() const override;
        i_widget& docked_widget() override;
    private:
        neolib::string iTitle;
        dock_area iAcceptableDocks;
        vertical_layout iLayout;
        dockable_title_bar iTitleBar;
        std::shared_ptr<i_widget> iDockedWidget;
        i_dock* iDock;
    };

    template <typename WidgetType, typename... Args>
    inline dockable make_dockable(const std::string& aTitle = "", dock_area aAcceptableDocks = dock_area::Any, Args&&... aArgs)
    {
        return dockable{ std::make_shared<WidgetType>(std::forward<Args>(aArgs)...), aTitle, aAcceptableDocks };
    }

    template <typename WidgetType, typename... Args>
    inline std::shared_ptr<i_dockable> make_shared_dockable(const std::string& aTitle = "", dock_area aAcceptableDocks = dock_area::Any, Args&&... aArgs)
    {
        return std::make_shared<dockable>(std::make_shared<WidgetType>(std::forward<Args>(aArgs)...), aTitle, aAcceptableDocks);
    }
}