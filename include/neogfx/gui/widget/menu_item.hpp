// menu_item.hpp
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
#include <neogfx/gui/widget/i_menu_item.hpp>

namespace neogfx
{
    class i_menu;

    class menu_item : public i_menu_item
    {
    public:
        define_declared_event(Selected, selected)
        define_declared_event(Deselected, deselected)
    private:
        typedef std::shared_ptr<i_action> action_pointer;
        typedef std::shared_ptr<i_menu> menu_pointer;
        typedef neolib::variant<action_pointer, menu_pointer> contents;
    public:
        menu_item(i_action& aAction);
        menu_item(std::shared_ptr<i_action> aAction);
        menu_item(i_menu& aSubMenu);
        menu_item(std::shared_ptr<i_menu> aSubMenu);
    public:
        menu_item_type type() const override;
        const i_action& action() const override;
        i_action& action() override;
        const i_menu& sub_menu() const override;
        i_menu& sub_menu() override;
    public:
        const i_menu_item_widget& as_widget() const override;
        i_menu_item_widget& as_widget() override;
        void set_widget(i_menu_item_widget& aWidget) override;
        void unset_widget() override;
    public:
        bool available() const override;
        void select(bool aOpenAnySubMenu = true) override;
        bool open_any_sub_menu() const override;
    private:
        contents iContents;
        bool iOpenAnySubMenu;
        i_menu_item_widget* iWidget;
    };
}