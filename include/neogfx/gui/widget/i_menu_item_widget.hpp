// i_menu_item_widget.hpp
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
#include <neogfx/app/i_mnemonic.hpp>
#include <neogfx/app/i_help.hpp>
#include <neogfx/gui/widget/text_widget.hpp>
#include <neogfx/gui/widget/image_widget.hpp>
#include <neogfx/gui/widget/i_menu_item.hpp>

namespace neogfx
{
    class popup_menu;

    class i_menu_item_widget : protected i_mnemonic, protected i_help_source
    {
    public:
        virtual i_menu& menu() const = 0;
        virtual i_menu_item& menu_item() const = 0;
    public:
        virtual image_widget& item_icon() = 0; // todo: i_image_widget
        virtual text_widget& item_text() = 0; // todo: i_text_widget
   };
}