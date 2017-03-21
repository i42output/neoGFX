// new_project_dialog.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/group_box.hpp>

namespace neogui
{
	class new_project_dialog : public neogfx::dialog
	{
	public:
		enum result_code_e
		{
			Accepted,
			Rejected
		};
	public:
		new_project_dialog(i_widget& aParent) :
			dialog{ aParent, "New Project", Modal | Titlebar | Close },
			iLayout0{ *this }, iLayout1{ iLayout0 }, iGroupBox1{ iLayout1, "Project Type" }, iGroupBox2{ iLayout1, "Project Defaults" },
			iNewMVCApp{ iGroupBox1.item_layout(), "New MVC (Model-View-Controller) App" },
			iNewDialogApp{ iGroupBox1.item_layout(), "New Dialog App" },
			iNew2DGame{ iGroupBox1.item_layout(), "New 2D Game" },
			iNew25DGame{ iGroupBox1.item_layout(), "New 2.5D (Isometric) Game" },
			iNew3DGame{ iGroupBox1.item_layout(), "New 3D Game" },
			iLayout2{ iGroupBox2.item_layout() },
			iNameLabel{ iLayout2, "Name:"},
			iName{ iLayout2 },
			iSpacer1{ iGroupBox1.item_layout() },	
			iSpacer2{ iGroupBox2.item_layout() }
		{
			button_box().add_button(neogfx::dialog_button_box::Ok);
			button_box().add_button(neogfx::dialog_button_box::Cancel);
			iNewDialogApp.enable(false);
			iNew2DGame.enable(false);
			iNew25DGame.enable(false);
			iNew3DGame.enable(false);
			iName.set_hint("Medium sized project name");
			centre_on_parent();
		}
	public:
		neogfx::vertical_layout iLayout0;
		neogfx::horizontal_layout iLayout1;
		neogfx::group_box iGroupBox1;
		neogfx::radio_button iNewMVCApp;
		neogfx::radio_button iNewDialogApp;
		neogfx::radio_button iNew2DGame;
		neogfx::radio_button iNew25DGame;
		neogfx::radio_button iNew3DGame;
		neogfx::group_box iGroupBox2;
		neogfx::horizontal_layout iLayout2;
		neogfx::label iNameLabel;
		neogfx::line_edit iName;
		neogfx::vertical_spacer iSpacer1;
		neogfx::vertical_spacer iSpacer2;
	};
}