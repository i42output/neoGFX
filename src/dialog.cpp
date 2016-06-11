// dialog.hpp
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
#include "app.hpp"
#include "dialog.hpp"

namespace neogfx
{
	dialog::dialog(style_e aStyle) :
		window(size{}, aStyle)
	{
	}

	dialog::dialog(const point& aPosition, const size& aDimensions, style_e aStyle) :
		window(aPosition, aDimensions, aStyle)
	{
	}

	dialog::dialog(const point& aPosition, const size& aDimensions, const std::string& aDialogTitle, style_e aStyle) :
		window(aPosition, aDimensions, aDialogTitle, aStyle)
	{
	}

	dialog::dialog(i_widget& aParent, style_e aStyle) :
		window(aParent, size{}, aStyle)
	{
	}

	dialog::dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, style_e aStyle) :
		window(aParent, aPosition, aDimensions, aStyle)
	{
	}

	dialog::dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aDialogTitle, style_e aStyle) :
		window(aParent, aPosition, aDimensions, aDialogTitle, aStyle)
	{
	}

	dialog::~dialog()
	{
	}

	dialog_button_box& dialog::button_box()
	{
		if (iButtonBox == boost::none)
		{
			if (!has_layout())
				set_layout(std::make_shared<vertical_layout>());
			iButtonBox.emplace(*this);
			iButtonBox->accepted([this]()
			{
				iResult = Accepted;
			});
			iButtonBox->rejected([this]()
			{
				iResult = Rejected;
			});
		}
		return *iButtonBox;
	}

	dialog::result_code_e dialog::exec()
	{
		while (iResult == boost::none)
		{
			app::instance().process_events();
			if (surface().destroyed() && iResult == boost::none)
				iResult = Rejected;
		}
		return *iResult;
	}

	void dialog::init()
	{

	}
}