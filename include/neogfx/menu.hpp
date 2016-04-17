// menu.hpp
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

#include "neogfx.hpp"
#include "event.hpp"
#include "action.hpp"
#include "i_menu.hpp"

namespace neogfx
{
	class menu : public i_menu
	{
	private:
		typedef std::unique_ptr<i_menu_item> item_pointer;
		typedef std::vector<item_pointer> item_list;
	public:
		menu(type_e aType = Popup, const std::string& aTitle = std::string());
	public:
		virtual type_e type() const;
		virtual const std::string& title() const;
		virtual uint32_t item_count() const;
		virtual const i_menu_item& item(item_index aItemIndex) const;
		virtual i_menu_item& item(item_index aItemIndex);
		virtual i_menu& add_sub_menu(const std::string& aSubMenuTitle);
		virtual void add_action(i_action& aAction);
		virtual void add_separator();
		virtual i_menu& insert_sub_menu(item_index aItemIndex, const std::string& aSubMenuText);
		virtual void insert_action(item_index aItemIndex, i_action& aAction);
		virtual void insert_separator(item_index aItemIndex);
		virtual void remove_item(item_index aItemIndex);
	private:
		type_e iType;
		std::string iTitle;
		item_list iItems;
		action iSeparator;
	};
}