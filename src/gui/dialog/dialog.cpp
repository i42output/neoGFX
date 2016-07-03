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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/dialog/dialog.hpp>

namespace neogfx
{
	dialog::dialog(style_e aStyle) :
		window(size{}, aStyle)
	{
		init();
	}

	dialog::dialog(const std::string& aDialogTitle, style_e aStyle) :
		window(size{}, aDialogTitle, aStyle)
	{
		init();
	}

	dialog::dialog(const size& aDimensions, style_e aStyle) :
		window(aDimensions, aStyle)
	{
		init();
	}

	dialog::dialog(const size& aDimensions, const std::string& aDialogTitle, style_e aStyle) :
		window(aDimensions, aDialogTitle, aStyle)
	{
		init();
	}

	dialog::dialog(const point& aPosition, const size& aDimensions, style_e aStyle) :
		window(aPosition, aDimensions, aStyle)
	{
		init();
	}

	dialog::dialog(const point& aPosition, const size& aDimensions, const std::string& aDialogTitle, style_e aStyle) :
		window(aPosition, aDimensions, aDialogTitle, aStyle)
	{
		init();
	}

	dialog::dialog(i_widget& aParent, style_e aStyle) :
		window(aParent, size{}, aStyle)
	{
		init();
	}

	dialog::dialog(i_widget& aParent, const std::string& aDialogTitle, style_e aStyle) :
		window(aParent, size{}, aDialogTitle, aStyle)
	{
		init();
	}

	dialog::dialog(i_widget& aParent, const size& aDimensions, style_e aStyle) :
		window(aParent, aDimensions, aStyle)
	{
		init();
	}

	dialog::dialog(i_widget& aParent, const size& aDimensions, const std::string& aDialogTitle, style_e aStyle) :
		window(aParent, aDimensions, aDialogTitle, aStyle)
	{
		init();
	}

	dialog::dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, style_e aStyle) :
		window(aParent, aPosition, aDimensions, aStyle)
	{
		init();
	}

	dialog::dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aDialogTitle, style_e aStyle) :
		window(aParent, aPosition, aDimensions, aDialogTitle, aStyle)
	{
		init();
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
			iButtonBox.emplace(layout());
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
		app::event_processing_context epc(app::instance(), "neogfx::dialog");
		while (iResult == boost::none)
		{
			app::instance().process_events(epc);
			if (surface().destroyed() && iResult == boost::none)
				iResult = Rejected;
		}
		return *iResult;
	}

	void dialog::init()
	{
	}
}