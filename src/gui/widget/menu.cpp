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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/gui/widget/menu_item.hpp>
#include <neogfx/gui/widget/menu.hpp>

namespace neogfx
{
	menu::menu(type_e aType, const std::string& aTitle) : iParent(0), iType(aType), iTitle(aTitle), iOpenCount(0)
	{
	}

	menu::menu(i_menu& aParent, type_e aType, const std::string& aTitle) : iParent(&aParent), iType(aType), iTitle(aTitle), iOpenCount(0)
	{
	}

	menu::~menu()
	{
	}

	bool menu::has_parent() const
	{
		return iParent != 0;
	}

	i_menu& menu::parent()
	{
		if (iParent != 0)
			return *iParent;
		throw no_parent();
	}

	menu::type_e menu::type() const
	{
		return iType;
	}

	const std::string& menu::title() const
	{
		return iTitle;
	}

	void menu::set_title(const std::string& aTitle)
	{
		iTitle = aTitle;
		menu_changed.trigger();
	}

	const i_texture& menu::image() const
	{
		return iImage;
	}

	void menu::set_image(const std::string& aUri)
	{
		iImage = neogfx::image{aUri};
		menu_changed.trigger();
	}

	void menu::set_image(const i_image& aImage)
	{
		iImage = aImage;
		menu_changed.trigger();
	}

	void menu::set_image(const i_texture& aTexture)
	{
		iImage = aTexture;
		menu_changed.trigger();
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

	i_action& menu::add_action(i_action& aAction)
	{
		insert_action(item_count(), aAction);
		return aAction;
	}

	void menu::add_separator()
	{
		insert_separator(item_count());
	}

	i_menu& menu::insert_sub_menu(item_index aItemIndex, const std::string& aSubMenuTitle)
	{
		auto newItem = iItems.insert(iItems.begin() + aItemIndex, std::make_unique<menu_item>(std::make_shared<menu>(*this, Popup, aSubMenuTitle)));
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
		if (aItemIndex >= item_count())
			throw bad_item_index();
		iItems.erase(iItems.begin() + aItemIndex);
		item_removed.trigger(aItemIndex);
	}

	menu::item_index menu::find_item(const i_menu_item& aItem) const
	{
		for (item_index i = 0; i < iItems.size(); ++i)
			if (&*iItems[i] == &aItem)
				return i;
		throw item_not_found();
	}

	menu::item_index menu::find_item(const i_menu& aSubMenu) const
	{
		for (item_index i = 0; i < iItems.size(); ++i)
			if (iItems[i]->type() == i_menu_item::SubMenu && &iItems[i]->sub_menu() == &aSubMenu)
				return i;
		throw item_not_found();
	}

	bool menu::has_selected_item() const
	{
		return iSelection != boost::none;
	}

	menu::item_index menu::selected_item() const
	{
		if (iSelection != boost::none)
			return *iSelection;
		throw no_selected_item();
	}

	void menu::select_item(item_index aItemIndex)
	{
		if (has_selected_item() && selected_item() == aItemIndex)
			return;
		if (!item_available(aItemIndex))
			throw cannot_select_item();
		if (has_selected_item())
			item(selected_item()).deselected.trigger();
		iSelection = aItemIndex;
		item_selected.trigger(item(aItemIndex));
		item(aItemIndex).selected.trigger();
	}

	void menu::clear_selection()
	{
		if (has_selected_item())
		{
			iSelection = boost::none;
			selection_cleared.trigger();
		}
	}

	bool menu::has_available_items() const
	{
		for (const auto& i : iItems)
			if (i->availabie())
				return true;
		return false;
	}

	bool menu::item_available(item_index aItemIndex) const
	{
		return item(aItemIndex).availabie();
	}

	menu::item_index menu::first_available_item() const
	{
		for (item_index i = 0; i < item_count(); ++i)
			if (item(i).availabie())
				return i;
		throw no_available_items();
	}

	menu::item_index menu::previous_available_item(item_index aCurrentIndex) const
	{
		if (aCurrentIndex >= item_count())
			throw bad_item_index();
		auto previous = [this](item_index aCurrent) -> item_index
		{
			if (aCurrent > 0)
				return aCurrent - 1;
			else
				return item_count() - 1;
		};
		for (item_index previousIndex = previous(aCurrentIndex); previousIndex != aCurrentIndex; previousIndex = previous(previousIndex))
			if (item(previousIndex).availabie())
				return previousIndex;
		return aCurrentIndex;
	}

	menu::item_index menu::next_available_item(item_index aCurrentIndex) const
	{
		if (aCurrentIndex >= item_count())
			throw bad_item_index();
		auto next = [this](item_index aCurrent) -> item_index
		{
			if (aCurrent < item_count() - 1)
				return aCurrent + 1;
			else
				return 0;
		};
		for (item_index nextIndex = next(aCurrentIndex); nextIndex != aCurrentIndex; nextIndex = next(nextIndex))
			if (item(nextIndex).availabie())
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
			opened.trigger();
	}

	void menu::close()
	{
		if (!is_open())
			throw already_closed();
		if (--iOpenCount == 0)
		{
			if (has_selected_item())
				clear_selection();
			closed.trigger();
		}
	}
}