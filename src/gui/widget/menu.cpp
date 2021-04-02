// menu.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/gui/widget/menu_item.hpp>
#include <neogfx/gui/widget/menu.hpp>

namespace neogfx
{
    menu::menu(std::string const& aTitle, menu_type aType) :
        iParent{ nullptr }, 
        iType{ aType }, 
        iGroup{},
        iTitle{ aTitle }, 
        iOpenCount{ 0 }, 
        iModal{ false }
    {
    }

    menu::menu(i_menu& aParent, std::string const& aTitle, menu_type aType) :
        iParent{ &aParent }, 
        iType{ aType }, 
        iGroup{},
        iTitle{ aTitle },
        iOpenCount{ 0 }, 
        iModal{ false }
    {
        aParent.add_sub_menu(*this);
    }

    menu::~menu()
    {
    }

    const i_widget& menu::as_widget() const
    {
        throw no_widget();
    }
    
    i_widget& menu::as_widget()
    {
        throw no_widget();
    }

    bool menu::has_parent() const
    {
        return iParent != nullptr;
    }

    i_menu& menu::parent()
    {
        if (iParent != nullptr)
            return *iParent;
        throw no_parent();
    }

    void menu::set_parent(i_menu& aParent)
    {
        iParent = &aParent;
    }

    menu_type menu::type() const
    {
        return iType;
    }

    uuid const& menu::group() const
    {
        return iGroup;
    }

    void menu::set_group(uuid const& aGroup)
    {
        iGroup = aGroup;
    }

    std::string const& menu::title() const
    {
        return iTitle;
    }

    void menu::set_title(std::string const& aTitle)
    {
        iTitle = aTitle;
        MenuChanged.trigger();
    }

    const i_texture& menu::image() const
    {
        return iImage;
    }

    void menu::set_image(std::string const& aUri)
    {
        iImage = neogfx::image{aUri};
        MenuChanged.trigger();
    }

    void menu::set_image(const i_image& aImage)
    {
        iImage = aImage;
        MenuChanged.trigger();
    }

    void menu::set_image(const i_texture& aTexture)
    {
        iImage = aTexture;
        MenuChanged.trigger();
    }

    uint32_t menu::count() const
    {
        return static_cast<uint32_t>(iItems.size());
    }

    uint32_t menu::ideal_insert_index(uuid const& aGroup) const
    {
        if (count() == 0u)
            return 0u;
        for (auto index = count(); index --> 0u;)
        {
            if (item_at(index).group() == aGroup)
                return index + 1u;
        }
        return count();
    }

    const i_menu_item& menu::item_at(item_index aItemIndex) const
    {
        if (aItemIndex >= count())
            throw bad_item_index();
        return *iItems[aItemIndex];
    }

    i_menu_item& menu::item_at(item_index aItemIndex)
    {
        return const_cast<i_menu_item&>(to_const(*this).item_at(aItemIndex));
    }

    void menu::add_sub_menu(i_menu& aSubMenu)
    {
        insert_sub_menu_at(ideal_insert_index(aSubMenu), aSubMenu);
    }

    i_menu& menu::add_sub_menu(std::string const& aSubMenuTitle, uuid const& aGroup)
    {
        return insert_sub_menu_at(ideal_insert_index(aGroup), aSubMenuTitle);
    }

    i_action& menu::add_action(i_action& aAction)
    {
        insert_action_at(ideal_insert_index(aAction), aAction);
        return aAction;
    }

    i_action& menu::add_action(std::shared_ptr<i_action> aAction)
    {
        insert_action_at(ideal_insert_index(*aAction), aAction);
        return *aAction;
    }

    void menu::add_separator(uuid const& aGroup)
    {
        insert_separator_at(ideal_insert_index(aGroup));
    }

    void menu::insert_sub_menu_at(item_index aItemIndex, i_menu& aSubMenu)
    {
        auto& newItem = **iItems.insert(iItems.begin() + aItemIndex, std::make_unique<menu_item>(aSubMenu));
        newItem.sub_menu().set_parent(*this);
        aItemIndex = update_grouping_separators(aItemIndex);
        ItemAdded.trigger(aItemIndex);
    }

    i_menu& menu::insert_sub_menu_at(item_index aItemIndex, std::string const& aSubMenuTitle, uuid const& aGroup)
    {
        auto& newItem = **iItems.insert(iItems.begin() + aItemIndex, std::make_unique<menu_item>(std::make_shared<menu>(aSubMenuTitle)));
        newItem.sub_menu().set_parent(*this);
        newItem.sub_menu().set_group(aGroup);
        aItemIndex = update_grouping_separators(aItemIndex);
        ItemAdded.trigger(aItemIndex);
        return newItem.sub_menu();
    }

    void menu::insert_action_at(item_index aItemIndex, i_action& aAction)
    {
        insert_action_at(aItemIndex, std::shared_ptr<i_action>(std::shared_ptr<i_action>(), &aAction));
    }
    
    void menu::insert_action_at(item_index aItemIndex, std::shared_ptr<i_action> aAction)
    {
        iItems.insert(iItems.begin() + aItemIndex, std::make_unique<menu_item>(aAction));
        aItemIndex = update_grouping_separators(aItemIndex);
        ItemAdded.trigger(aItemIndex);
        aAction->changed([this, aAction]()
        {
            for (item_index i = 0; i < iItems.size(); ++i)
                if (iItems[i]->type() == menu_item_type::Action && &(iItems[i]->action()) == &*aAction)
                {
                    ItemChanged.trigger(i);
                    return;
                }
        });
    }

    void menu::insert_separator_at(item_index aItemIndex, uuid const& aGroup)
    {
        insert_action_at(aItemIndex, iSeparator);
        item_at(aItemIndex).action().set_group(aGroup);
    }

    void menu::remove_at(item_index aItemIndex)
    {
        if (aItemIndex >= count())
            throw bad_item_index();
        iItems.erase(iItems.begin() + aItemIndex);
        ItemRemoved.trigger(aItemIndex);
    }

    menu::item_index menu::find_sub_menu(uuid const& aGroup) const
    {
        for (item_index i = 0; i < iItems.size(); ++i)
            if (iItems[i]->group() == aGroup && iItems[i]->type() == menu_item_type::SubMenu)
                return i;
        throw item_not_found();
    }

    menu::item_index menu::find(const i_menu_item& aItem) const
    {
        for (item_index i = 0; i < iItems.size(); ++i)
            if (&*iItems[i] == &aItem)
                return i;
        throw item_not_found();
    }

    menu::item_index menu::find(const i_menu& aSubMenu) const
    {
        for (item_index i = 0; i < iItems.size(); ++i)
            if (iItems[i]->type() == menu_item_type::SubMenu && &iItems[i]->sub_menu() == &aSubMenu)
                return i;
        throw item_not_found();
    }

    bool menu::has_selected_item() const
    {
        return iSelection != std::nullopt;
    }

    menu::item_index menu::selected_item() const
    {
        if (iSelection != std::nullopt)
            return *iSelection;
        throw no_selected_item();
    }

    void menu::select_item_at(item_index aItemIndex, bool aOpenAnySubMenu)
    {
        if (has_selected_item() && selected_item() == aItemIndex)
            return;
        if (!item_available_at(aItemIndex))
            throw cannot_select_item();
        if (has_selected_item())
        {
            auto selection = selected_item();
            iSelection = std::nullopt;
            item_at(selection).deselected().trigger();
        }
        iSelection = aItemIndex;
        ItemSelected.trigger(item_at(aItemIndex));
        item_at(aItemIndex).select(aOpenAnySubMenu);
    }

    void menu::clear_selection()
    {
        if (has_selected_item())
        {
            auto selection = selected_item();
            iSelection = std::nullopt;
            item_at(selection).deselected().trigger();
            SelectionCleared.trigger();
        }
    }

    bool menu::has_available_items() const
    {
        for (auto const& i : iItems)
            if (i->available())
                return true;
        return false;
    }

    bool menu::item_available_at(item_index aItemIndex) const
    {
        return item_at(aItemIndex).available();
    }

    menu::item_index menu::first_available_item() const
    {
        for (item_index i = 0; i < count(); ++i)
            if (item_at(i).available())
                return i;
        throw no_available_items();
    }

    menu::item_index menu::previous_available_item(item_index aCurrentIndex) const
    {
        if (aCurrentIndex >= count())
            throw bad_item_index();
        auto previous = [this](item_index aCurrent) -> item_index
        {
            if (aCurrent > 0)
                return aCurrent - 1;
            else
                return count() - 1;
        };
        for (item_index previousIndex = previous(aCurrentIndex); previousIndex != aCurrentIndex; previousIndex = previous(previousIndex))
            if (item_at(previousIndex).available())
                return previousIndex;
        return aCurrentIndex;
    }

    menu::item_index menu::next_available_item(item_index aCurrentIndex) const
    {
        if (aCurrentIndex >= count())
            throw bad_item_index();
        auto next = [this](item_index aCurrent) -> item_index
        {
            if (aCurrent < count() - 1)
                return aCurrent + 1;
            else
                return 0;
        };
        for (item_index nextIndex = next(aCurrentIndex); nextIndex != aCurrentIndex; nextIndex = next(nextIndex))
            if (item_at(nextIndex).available())
                return nextIndex;
        return aCurrentIndex;
    }

    bool menu::is_open() const
    {
        return iOpenCount != 0;
    }

    void menu::open()
    {
        if (++iOpenCount == 1)
        {
            Opened.trigger();
        }
    }

    void menu::close()
    {
        if (!is_open())
            throw already_closed();
        if (--iOpenCount == 0)
        {
            if (has_selected_item())
                clear_selection();
            Closed.trigger();
        }
    }

    bool menu::is_modal() const
    {
        return iModal;
    }

    void menu::set_modal(bool aModal)
    {
        iModal = aModal;
    }

    menu::item_index menu::update_grouping_separators(item_index aItemIndex)
    {
        if (type() == menu_type::MenuBar)
            return aItemIndex;
        if (item_at(aItemIndex).is_separator())
            return aItemIndex;
        if (aItemIndex > 0u && item_at(aItemIndex).group() != item_at(aItemIndex - 1u).group() && !item_at(aItemIndex - 1u).is_separator())
            insert_separator_at(aItemIndex++);
        if (aItemIndex < count() - 1u && item_at(aItemIndex).group() != item_at(aItemIndex + 1u).group() && !item_at(aItemIndex + 1u).is_separator())
            insert_separator_at(aItemIndex + 1u);
        return aItemIndex;
    }
}