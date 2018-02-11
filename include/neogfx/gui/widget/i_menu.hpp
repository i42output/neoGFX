// i_menu.hpp
/*
neogfx C++ GUI Library
Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.

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
#include <neogfx/app/i_action.hpp>
#include "i_widget.hpp"
#include "i_menu_item.hpp"

namespace neogfx
{
	class i_menu
	{
	public:
		typedef uint32_t item_index;
	public:
		event<> menu_changed;
		event<item_index> item_added;
		event<item_index> item_removed;
		event<item_index> item_changed;
		event<> opened;
		event<> closed;
		event<i_menu_item&> item_selected;
		event<> selection_cleared;
		event<i_menu&> open_sub_menu;
	public:
		enum type_e
		{
			MenuBar,
			Popup
		};
		typedef uint32_t item_index;
	public:
		struct no_parent : std::logic_error { no_parent() : std::logic_error("neogfx::i_menu::no_parent") {} };
		struct bad_item_index : std::logic_error { bad_item_index() : std::logic_error("neogfx::i_menu::bad_item_index") {} };
		struct item_not_found : std::logic_error { item_not_found() : std::logic_error("neogfx::i_menu::item_not_found") {} };
		struct no_selected_item : std::logic_error { no_selected_item() : std::logic_error("neogfx::i_menu::no_selected_item") {} };
		struct cannot_select_item : std::logic_error { cannot_select_item() : std::logic_error("neogfx::i_menu::cannot_select_item") {} };
		struct no_available_items : std::logic_error { no_available_items() : std::logic_error("neogfx::i_menu::no_available_items") {} };
		struct already_closed : std::logic_error { already_closed() : std::logic_error("neogfx::i_menu::already_closed") {} };
	public:
		virtual ~i_menu() {}
	public:
		virtual bool has_parent() const = 0;
		virtual i_menu& parent() = 0;
		virtual type_e type() const = 0;
		virtual const std::string& title() const = 0;
		virtual void set_title(const std::string& aTitle) = 0;
		virtual const i_texture& image() const = 0;
		virtual void set_image(const std::string& aUri) = 0;
		virtual void set_image(const i_image& aImage) = 0;
		virtual void set_image(const i_texture& aTexture) = 0;		
		virtual uint32_t count() const = 0;
		virtual const i_menu_item& item_at(item_index aItemIndex) const = 0;
		virtual i_menu_item& item_at(item_index aItemIndex) = 0;
		virtual i_menu& add_sub_menu(const std::string& aSubMenuTitle) = 0;
		virtual i_action& add_action(i_action& aAction) = 0;
		virtual i_action& add_action(std::shared_ptr<i_action> aAction) = 0;
		virtual void add_separator() = 0;
		virtual i_menu& insert_sub_menu_at(item_index aItemIndex, const std::string& aSubMenuTitle) = 0;
		virtual void insert_action_at(item_index aItemIndex, i_action& aAction) = 0;
		virtual void insert_separator_at(item_index aItemIndex) = 0;
		virtual void remove_at(item_index aItemIndex) = 0;
		virtual item_index find(const i_menu_item& aItem) const = 0;
		virtual item_index find(const i_menu& aSubMenu) const = 0;
		virtual bool has_selected_item() const = 0;
		virtual item_index selected_item() const = 0;
		virtual void select_item_at(item_index aItemIndex, bool aSelectAnySubMenuItem = true) = 0;
		virtual void clear_selection() = 0;
		virtual bool has_available_items() const = 0;
		virtual bool item_available_at(item_index aItemIndex) const = 0;
		virtual item_index first_available_item() const = 0;
		virtual item_index previous_available_item(item_index aCurrentIndex) const = 0;
		virtual item_index next_available_item(item_index aCurrentIndex) const = 0;
		virtual bool is_open() const = 0;
		virtual void open() = 0;
		virtual void close() = 0;
		virtual bool is_modal() const = 0;
		virtual void set_modal(bool aModal) = 0;
	};
}