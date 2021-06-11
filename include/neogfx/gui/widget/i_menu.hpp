// i_menu.hpp
/*
neogfx C++ App/Game Engine
Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.

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
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/i_menu_item.hpp>

namespace neogfx
{
    enum class menu_type : uint32_t
    {
        MenuBar,
        Popup
    };

    class i_menu : public i_action_container
    {
    public:
        struct no_widget : std::logic_error { no_widget() : std::logic_error{ "neogfx::i_menu::no_widget" } {} };
    public:
        typedef uint32_t item_index;
    public:
        declare_event(menu_changed)
        declare_event(item_added, item_index)
        declare_event(item_removed, item_index)
        declare_event(item_changed, item_index)
        declare_event(opened)
        declare_event(closed)
        declare_event(item_selected, i_menu_item&)
        declare_event(selection_cleared)
        declare_event(open_sub_menu, i_menu&)
    public:
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
        virtual ~i_menu() = default;
    public:
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
        virtual bool has_parent() const = 0;
        virtual i_menu& parent() = 0;
        virtual void set_parent(i_menu& aParent) = 0;
        virtual menu_type type() const = 0;
        virtual uuid const& group() const = 0;
        virtual void set_group(uuid const& aGroup) = 0;
        virtual i_string const& title() const = 0;
        virtual void set_title(i_string const& aTitle) = 0;
        virtual const i_texture& image() const = 0;
        virtual void set_image(i_string const& aUri) = 0;
        virtual void set_image(i_image const& aImage) = 0;
        virtual void set_image(i_texture const& aTexture) = 0;
        virtual uint32_t count() const = 0;
        virtual uint32_t ideal_insert_index(uuid const& aGroup) const = 0;
        virtual const i_menu_item& item_at(item_index aItemIndex) const = 0;
        virtual i_menu_item& item_at(item_index aItemIndex) = 0;
        virtual void add_sub_menu(i_menu& aSubMenu) = 0;
        virtual i_menu& add_sub_menu(i_string const& aSubMenuTitle, uuid const& aGroup = {}) = 0;
        virtual void add_separator(uuid const& aGroup = {}) = 0;
        virtual void insert_sub_menu_at(item_index aItemIndex, i_menu& aSubMenu) = 0;
        virtual i_menu& insert_sub_menu_at(item_index aItemIndex, i_string const& aSubMenuTitle, uuid const& aGroup = {}) = 0;
        virtual void insert_action_at(item_index aItemIndex, i_action& aAction) = 0;
        virtual void insert_action_at(item_index aItemIndex, i_ref_ptr<i_action> const& aAction) = 0;
        virtual void insert_separator_at(item_index aItemIndex, uuid const& aGroup = {}) = 0;
        virtual void remove_at(item_index aItemIndex) = 0;
        virtual item_index find_sub_menu(uuid const& aGroup) const = 0;
        virtual item_index find(i_menu_item const& aItem) const = 0;
        virtual item_index find(i_menu const& aSubMenu) const = 0;
        virtual bool has_selected_item() const = 0;
        virtual item_index selected_item() const = 0;
        virtual void select_item_at(item_index aItemIndex, bool aOpenAnySubMenu = true) = 0;
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
    public:
        uint32_t ideal_insert_index(i_action const& aAction) const
        {
            return ideal_insert_index(aAction.group());
        }
        uint32_t ideal_insert_index(i_menu const& aSubMenu) const
        {
            return ideal_insert_index(aSubMenu.group());
        }
    };
}