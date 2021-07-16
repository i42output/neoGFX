// game_controller_dialog.hpp
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
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/hid/i_game_controllers.hpp>
#include <neogfx/gui/widget/tab_page_container.hpp>
#include <neogfx/gui/widget/tab_page.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/drop_list.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/image_widget.hpp>

namespace neogfx
{
    class game_controller_dialog : public dialog
    {
    public:
        game_controller_dialog();
        game_controller_dialog(i_widget& aParent);
        ~game_controller_dialog();
    private:
        void init();
    private:
        sink iSink;
        group_box iControllerSelectorGroupBox;
        drop_list iControllerSelector;
        tab_page_container<> iTabs;
        tab_page iCalibrateTab;
        tab_page iTestTab;
        vertical_layout iLayout1;
        group_box iSchematicGroupBox;
        image_widget iSchematic;
        vertical_layout iLayout2;
        group_box iTestGroupBox;
    };
}