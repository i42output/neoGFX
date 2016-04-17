// i_menu.hpp
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
#include "i_widget.hpp"
#include "i_action.hpp"
#include "i_menu_item.hpp"

namespace neogfx
{
	class i_menu
	{
	public:
		typedef uint32_t item_index;
	public:
		event<item_index> item_added;
		event<item_index> item_removed;
		event<item_index> item_changed;
		event<i_menu&> open_sub_menu;
	public:
		enum type_e
		{
			MenuBar,
			Popup
		};
		typedef uint32_t item_index;
	public:
		struct bad_item_index : std::logic_error { bad_item_index() : std::logic_error("neogfx::i_menu::bad_item_index") {} };
	public:
		virtual type_e type() const = 0;
		virtual const std::string& title() const = 0;
		virtual uint32_t item_count() const = 0;
		virtual const i_menu_item& item(item_index aItemIndex) const = 0;
		virtual i_menu_item& item(item_index aItemIndex) = 0;
		virtual i_menu& add_sub_menu(const std::string& aSubMenuTitle) = 0;
		virtual void add_action(i_action& aAction) = 0;
		virtual void add_separator() = 0;
		virtual i_menu& insert_sub_menu(item_index aItemIndex, const std::string& aSubMenuTitle) = 0;
		virtual void insert_action(item_index aItemIndex, i_action& aAction) = 0;
		virtual void insert_separator(item_index aItemIndex) = 0;
		virtual void remove_item(item_index aItemIndex) = 0;
	};
}