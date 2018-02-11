// menu_item.cpp
/*
neogfx C++ GUI Library
Copyright(C) 2015-present Leigh Johnston

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
#include <neolib/raii.hpp>
#include <neogfx/gui/widget/menu_item.hpp>
#include <neogfx/gui/widget/i_menu.hpp>

namespace neogfx
{
	menu_item::menu_item(i_action& aAction) : iContents{ std::shared_ptr<i_action>{std::shared_ptr<i_action>{}, &aAction } }, iSelectAnySubMenuItem{ true }
	{
	}

	menu_item::menu_item(std::shared_ptr<i_action> aAction) : iContents{ aAction }, iSelectAnySubMenuItem{ true }
	{
	}

	menu_item::menu_item(i_menu& aSubMenu) : iContents{ std::shared_ptr<i_menu>{std::shared_ptr<i_menu>{}, &aSubMenu } }, iSelectAnySubMenuItem{ true }
	{
	}

	menu_item::menu_item(std::shared_ptr<i_menu> aSubMenu) : iContents{ aSubMenu }, iSelectAnySubMenuItem{ true }
	{
	}

	menu_item::type_e menu_item::type() const
	{
		if (iContents.is<action_pointer>())
			return Action;
		else
			return SubMenu;
	}

	const i_action& menu_item::action() const
	{
		if (type() != Action)
			throw wrong_type();
		return *static_variant_cast<const action_pointer&>(iContents);
	}

	i_action& menu_item::action()
	{
		return const_cast<i_action&>(const_cast<const menu_item*>(this)->action());
	}

	const i_menu& menu_item::sub_menu() const
	{
		if (type() != SubMenu)
			throw wrong_type();
		return *static_variant_cast<const menu_pointer&>(iContents);
	}

	i_menu& menu_item::sub_menu()
	{
		return const_cast<i_menu&>(const_cast<const menu_item*>(this)->sub_menu());
	}

	bool menu_item::available() const
	{
		if (type() == Action && (action().is_separator() || action().is_disabled()))
			return false;
		if (type() == SubMenu && sub_menu().count() == 0)
			return false;
		return true;
	}

	void menu_item::select(bool aSelectAnySubMenuItem)
	{
		neolib::scoped_flag sf{ iSelectAnySubMenuItem, aSelectAnySubMenuItem };
		selected.trigger();
	}

	bool menu_item::select_any_sub_menu_item() const
	{
		return iSelectAnySubMenuItem;
	}
}