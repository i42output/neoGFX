// i_menu_item.hpp
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
#include <neogfx/app/i_action.hpp>

namespace neogfx
{
    class i_menu;

    enum class menu_item_type : uint32_t
    {
        Action,
        SubMenu
    };

    class i_menu_item_widget;

    class i_menu_item
    {
    public:
        declare_event(selected)
        declare_event(deselected)
    public:
        struct wrong_type : std::logic_error { wrong_type() : std::logic_error("neogfx::i_menu_item::wrong_type") {} };
        struct no_widget : std::logic_error { no_widget() : std::logic_error("neogfx::i_menu_item::no_widget") {} };
    public:
        virtual ~i_menu_item() = default;
    public:
        virtual menu_item_type type() const = 0;
        virtual uuid const& group() const = 0;
        virtual const i_action& action() const = 0;
        virtual i_action& action() = 0;
        virtual const i_menu& sub_menu() const = 0;
        virtual i_menu& sub_menu() = 0;
    public:
        virtual const i_menu_item_widget& as_widget() const = 0;
        virtual i_menu_item_widget& as_widget() = 0;
        virtual void set_widget(i_menu_item_widget& aWidget) = 0;
        virtual void unset_widget() = 0;
    public:
        virtual bool available() const = 0;
        virtual void select(bool aOpenAnySubMenu = true) = 0;
        virtual bool open_any_sub_menu() const = 0;
    public:
        bool is_separator() const
        {
            return type() == menu_item_type::Action && action().is_separator();
        }
    };
}