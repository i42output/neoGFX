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
		window(aPosition, size{}, None | NoActivate | RequiresOwnerFocus, framed_widget::SolidFrame), iMenu(aMenu), iLayout(*this)
	{
		init();
	}

	popup_menu::popup_menu(i_widget& aParent, const point& aPosition, i_menu& aMenu) :
		window(aParent, aPosition, size{}, None | NoActivate | RequiresOwnerFocus, framed_widget::SolidFrame), iMenu(aMenu), iLayout(*this)
	{
		init();
	}

	popup_menu::~popup_menu()
	{
		iOpenSubMenu.reset();
		iMenu.item_added.unsubscribe(this);
		iMenu.item_removed.unsubscribe(this);
		iMenu.item_changed.unsubscribe(this);
		iMenu.item_selected.unsubscribe(this);
		iMenu.open_sub_menu.unsubscribe(this);
		iMenu.close();
	}

	void popup_menu::resized()
	{
		window::resized();
		rect desktopRect{ app::instance().surface_manager().desktop_rect(surface()) };
		rect surfaceRect{ surface().surface_position(), surface().surface_size() };
		point newPosition = surface().surface_position();
		if (surfaceRect.bottom() > desktopRect.bottom())
			newPosition.y += (desktopRect.bottom() - surfaceRect.bottom());
		if (surfaceRect.right() > desktopRect.right())
			newPosition.x += (desktopRect.right() - surfaceRect.right());
		if (newPosition != surface().surface_position())
			surface().move_surface(newPosition);
	}

	size_policy popup_menu::size_policy() const
	{
		if (window::has_size_policy())
			return window::size_policy();
		return neogfx::size_policy::Minimum;
	}

	size popup_menu::minimum_size(const optional_size& aAvailableSpace) const
	{
		size result = window::minimum_size(aAvailableSpace);
		rect desktopRect = app::instance().surface_manager().desktop_rect(surface());
		result.cx = std::min(result.cx, desktopRect.cx);
		result.cy = std::min(result.cy, desktopRect.cy);
		return result;
	}
	
	size popup_menu::maximum_size(const optional_size& aAvailableSpace) const
	{
		return minimum_size(aAvailableSpace);
	}

	colour popup_menu::background_colour() const
	{
		if (window::has_background_colour())
			return window::background_colour();
		return app::instance().current_style().colour().dark() ?
			app::instance().current_style().colour().darker(0x40) :
			app::instance().current_style().colour().lighter(0x40);
	}

	i_menu& popup_menu::menu() const
	{
		return iMenu;
	}

	void popup_menu::init()
	{
		iLayout.set_margins(neogfx::margins{});
		for (i_menu::item_index i = 0; i < iMenu.item_count(); ++i)
			iLayout.add_item(std::make_shared<menu_item_widget>(*this, iMenu, iMenu.item(i)));
		layout_items();
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
		iMenu.item_selected([this](i_menu_item& aMenuItem)
		{
			if (iOpenSubMenu.get() != 0)
			{
				if (aMenuItem.type() == i_menu_item::Action ||
					(aMenuItem.type() == i_menu_item::SubMenu && &iOpenSubMenu->menu() != &aMenuItem.sub_menu()))
				{
					iOpenSubMenu->menu().close();
				}
			}
			update();
		}, this);
		iMenu.open_sub_menu([this](i_menu& aSubMenu)
		{
			if (aSubMenu.item_count() > 0)
			{
				auto& itemWidget = layout().get_widget<menu_item_widget>(iMenu.find_item(aSubMenu));
				iOpenSubMenu.reset();
				iOpenSubMenu = std::make_unique<popup_menu>(*this, itemWidget.sub_menu_position(), aSubMenu);
				iOpenSubMenu->menu().closed([this]()
				{
					if (iOpenSubMenu.get() != 0)
						iOpenSubMenu->close();
				}, this);
				iOpenSubMenu->closed([this]()
				{
					iOpenSubMenu.reset();
				}, this);
			}
		}, this);
	}
}