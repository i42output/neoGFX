// popup_menu.cpp
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
#include "popup_menu.hpp"
#include "menu_item_widget.hpp"
#include "app.hpp"

namespace neogfx
{
	popup_menu::popup_menu(const point& aPosition, i_menu& aMenu) :
		window(aPosition, size{}, None, framed_widget::SolidFrame), iMenu(aMenu), iLayout(*this)
	{
		init();
	}

	popup_menu::popup_menu(i_widget& aParent, const point& aPosition, i_menu& aMenu) :
		window(aParent, aPosition, size{}, None, framed_widget::SolidFrame), iMenu(aMenu), iLayout(*this)
	{
		init();
	}

	popup_menu::~popup_menu()
	{
		iMenu.item_added.unsubscribe(this);
		iMenu.item_removed.unsubscribe(this);
		iMenu.item_changed.unsubscribe(this);
		iMenu.close();
	}

	size_policy popup_menu::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy::Minimum;
	}

	colour popup_menu::background_colour() const
	{
		if (window::has_background_colour())
			return window::background_colour();
		return app::instance().current_style().colour().dark() ?
			app::instance().current_style().colour().darker(0x40) :
			app::instance().current_style().colour().lighter(0x40);
	}

	void popup_menu::init()
	{
		iMenu.open();
		iMenu.item_added([this](i_menu::item_index aIndex)
		{
			iLayout.add_item(std::make_shared<menu_item_widget>(*this, iMenu, iMenu.item(aIndex)));
			layout_items();
		}, this);
		iMenu.item_removed([this](i_menu::item_index aIndex)
		{
			iLayout.remove_item(aIndex);
			layout_items();
		}, this);
		iMenu.item_changed([this](i_menu::item_index)
		{
			layout_items();
		}, this);
		for (i_menu::item_index i = 0; i < iMenu.item_count(); ++i)
			iLayout.add_item(std::make_shared<menu_item_widget>(*this, iMenu, iMenu.item(i)));
		layout_items();
	}
}