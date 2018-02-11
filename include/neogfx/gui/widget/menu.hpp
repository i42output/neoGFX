// menu.hpp
/*
neogfx C++ GUI Library
Copyright (c) 2015-present, Leigh Johnston.

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
#include <neogfx/core/event.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/gfx/texture.hpp>
#include "i_menu.hpp"

namespace neogfx
{
	class menu : public i_menu
	{
	private:
		typedef std::unique_ptr<i_menu_item> item_pointer;
		typedef std::vector<item_pointer> item_list;
	public:
		menu(i_menu& aParent, type_e aType = Popup, const std::string& aTitle = std::string());
		menu(type_e aType = Popup, const std::string& aTitle = std::string());
		~menu();
	public:
		virtual bool has_parent() const;
		virtual i_menu& parent();
		virtual type_e type() const;
		virtual const std::string& title() const;
		virtual void set_title(const std::string& aTitle);
		virtual const i_texture& image() const;
		virtual void set_image(const std::string& aUri);
		virtual void set_image(const i_image& aImage);
		virtual void set_image(const i_texture& aTexture);
		virtual uint32_t count() const;
		virtual const i_menu_item& item_at(item_index aItemIndex) const;
		virtual i_menu_item& item_at(item_index aItemIndex);
		virtual i_menu& add_sub_menu(const std::string& aSubMenuTitle);
		virtual i_action& add_action(i_action& aAction);
		virtual i_action& add_action(std::shared_ptr<i_action> aAction);
		virtual void add_separator();
		virtual i_menu& insert_sub_menu_at(item_index aItemIndex, const std::string& aSubMenuText);
		virtual void insert_action_at(item_index aItemIndex, i_action& aAction);
		virtual void insert_action_at(item_index aItemIndex, std::shared_ptr<i_action> aAction);
		virtual void insert_separator_at(item_index aItemIndex);
		virtual void remove_at(item_index aItemIndex);
		virtual item_index find(const i_menu_item& aItem) const;
		virtual item_index find(const i_menu& aSubMenu) const;
		virtual bool has_selected_item() const;
		virtual item_index selected_item() const;
		virtual void select_item_at(item_index aItemIndex, bool aSelectAnySubMenuItem = true);
		virtual void clear_selection();
		virtual bool has_available_items() const;
		virtual bool item_available_at(item_index aItemIndex) const;
		virtual item_index first_available_item() const;
		virtual item_index previous_available_item(item_index aCurrentIndex) const;
		virtual item_index next_available_item(item_index aCurrentIndex) const;
		virtual bool is_open() const;
		virtual void open();
		virtual void close();
		virtual bool is_modal() const;
		virtual void set_modal(bool aModal);
	private:
		i_menu* iParent;
		type_e iType;
		std::string iTitle;
		texture iImage;
		item_list iItems;
		action iSeparator;
		uint32_t iOpenCount;
		boost::optional<item_index> iSelection;
		bool iModal;
	};
}