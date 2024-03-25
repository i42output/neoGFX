// context_menu.hpp
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

#include "popup_menu.hpp"

namespace neogfx
{
    class context_menu
    {
    public:
        define_event(PopupCreated, popup_created, popup_menu&)
    public:
        struct widget_not_created_yet : std::logic_error { widget_not_created_yet() : std::logic_error("neogfx::context_menu::widget_not_created_yet") {} };
    public:
        context_menu(const point& aPosition, window_style aStyle = popup_menu::DEFAULT_STYLE | window_style::DismissOnParentClick);
        context_menu(i_widget& aParent, const point& aPosition, window_style aStyle = popup_menu::DEFAULT_STYLE | window_style::DismissOnParentClick);
        ~context_menu();
    public:
        i_menu& menu();
        popup_menu& root_widget();
        void exec();
    private:
        std::unique_ptr<i_menu> iMenu;
        i_widget* iParent;
        point iPosition;
        window_style iStyle;
        static std::unique_ptr<popup_menu> sWidget;
    };
}