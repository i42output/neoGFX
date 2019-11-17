// menu_bar.hpp
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
#include <neogfx/gui/layout/flow_layout.hpp>
#include "widget.hpp"
#include "menu.hpp"

namespace neogfx
{
    class popup_menu;

    class menu_bar : public widget, public menu
    {
    public:
        menu_bar();
        menu_bar(i_widget& aParent);
        menu_bar(i_layout& aLayout);
        ~menu_bar();
    public:
        const i_widget& as_widget() const override;
        i_widget& as_widget() override;
    public:
        using widget::find;
        using menu::find;
    public:
        neogfx::size_policy size_policy() const override;    
    public:
        double opacity() const override;
    public:
        bool visible() const override;
    protected:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool text_input(const std::string& aText) override;
    protected:
        widget_part hit_test(const point& aPosition) const override;
    private:
        void init();
        void close_sub_menu(bool aClearSelection = true);
    private:
        sink iSink;
        sink iSink2;
        flow_layout iLayout;
        std::unique_ptr<popup_menu> iOpenSubMenu;
    };
}