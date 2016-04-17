// menu.cpp
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
#include "menu_item.hpp"
#include "menu.hpp"

namespace neogfx
{
	menu::menu(type_e aType, const std::string& aTitle) : iType(aType), iTitle(aTitle)
	{
	}

	menu::type_e menu::type() const
	{
		return iType;
	}

	const std::string& menu::title() const
	{
		return iTitle;
	}

	uint32_t menu::item_count() const
	{
		return iItems.size();
	}

	const i_menu_item& menu::item(item_index aItemIndex) const
	{
		if (aItemIndex >= item_count())
			throw bad_item_index();
		return *iItems[aItemIndex];
	}

	i_menu_item& menu::item(item_index aItemIndex)
	{
		return const_cast<i_menu_item&>(const_cast<const menu*>(this)->item(aItemIndex));
	}

	i_menu& menu::add_sub_menu(const std::string& aSubMenuTitle)
	{
		return insert_sub_menu(item_count(), aSubMenuTitle);
	}

	void menu::add_action(i_action& aAction)
	{
		insert_action(item_count(), aAction);
	}

	void menu::add_separator()
	{
		insert_separator(item_count());
	}

	i_menu& menu::insert_sub_menu(item_index aItemIndex, const std::string& aSubMenuTitle)
	{
		auto newItem = iItems.insert(iItems.begin() + aItemIndex, std::make_unique<menu_item>(std::make_shared<menu>(Popup, aSubMenuTitle)));
		item_added.trigger(aItemIndex);
		return (**newItem).sub_menu();
	}

	void menu::insert_action(item_index aItemIndex, i_action& aAction)
	{
		iItems.insert(iItems.begin() + aItemIndex, std::make_unique<menu_item>(aAction));
		item_added.trigger(aItemIndex);
		aAction.changed([this, &aAction]()
		{
			for (item_index i = 0; i < iItems.size(); ++i)
				if (&(iItems[i]->action()) == &aAction)
				{
					item_changed.trigger(i);
					return;
				}
		});
	}

	void menu::insert_separator(item_index aItemIndex)
	{
		insert_action(aItemIndex, iSeparator);
	}

	void menu::remove_item(item_index aItemIndex)
	{
		iItems.erase(iItems.begin() + aItemIndex);
		item_removed.trigger(aItemIndex);
	}
}