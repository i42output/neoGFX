// colour_picker_dialog.cpp
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

#include "neogfx.hpp"
#include "colour_picker_dialog.hpp"

namespace neogfx
{
	colour_picker_dialog::colour_picker_dialog() : 
		dialog("Colour", Modal | Titlebar | Close)
	{
		init();
	}

	colour_picker_dialog::colour_picker_dialog(i_widget& aParent) :
		dialog(aParent, "Colour", Modal | Titlebar | Close)
	{
		init();
	}

	colour_picker_dialog::~colour_picker_dialog()
	{
	}

	void colour_picker_dialog::init()
	{
		button_box().add_button(dialog_button_box::Ok);
		button_box().add_button(dialog_button_box::Cancel);
		resize(minimum_size());
		rect desktopRect{ app::instance().surface_manager().desktop_rect(surface()) };
		move_surface((desktopRect.extents() - surface_size()) / 2.0);
	}
}