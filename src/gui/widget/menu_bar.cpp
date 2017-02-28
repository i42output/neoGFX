// menu_bar.cpp
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
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/menu_bar.hpp>
#include <neogfx/gui/widget/menu_item_widget.hpp>
#include <neogfx/gui/window/popup_menu.hpp>

namespace neogfx
{
	menu_bar::menu_bar() : menu(MenuBar), iLayout(*this), iOpenSubMenu{ std::make_unique<popup_menu>(*this, point{}) }
	{
		init();
	}

	menu_bar::menu_bar(i_widget& aParent) : widget(aParent), menu(MenuBar), iLayout(*this), iOpenSubMenu{ std::make_unique<popup_menu>(*this, point{}) }
	{
		init();
	}

	menu_bar::menu_bar(i_layout& aLayout) : widget(aLayout), menu(MenuBar), iLayout(*this), iOpenSubMenu{ std::make_unique<popup_menu>(*this, point{}) }
	{
		init();
	}

	menu_bar::~menu_bar()
	{
		close_sub_menu();
		iOpenSubMenu.reset();
		remove_widgets();
	}

	size_policy menu_bar::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy::Minimum;
	}

	bool menu_bar::visible() const
	{
		if (app::instance().basic_services().has_system_menu_bar())
			return false;
		return widget::visible();
	}

	bool menu_bar::key_pressed(scan_code_e aScanCode, key_code_e, key_modifiers_e)
	{
		switch (aScanCode)
		{
		case ScanCode_LEFT:
			if (has_selected_item())
				select_item(previous_available_item(selected_item()));
			break;
		case ScanCode_RIGHT:
			if (has_selected_item())
				select_item(next_available_item(selected_item()));
			break;
		case ScanCode_DOWN:
			if (has_selected_item() && item(selected_item()).availabie())
			{
				auto& selectedItem = item(selected_item());
				if (selectedItem.type() == i_menu_item::SubMenu)
				{
					if (!selectedItem.sub_menu().is_open())
						open_sub_menu.trigger(selectedItem.sub_menu());
					if (selectedItem.sub_menu().has_available_items())
						selectedItem.sub_menu().select_item(selectedItem.sub_menu().first_available_item());
				}
			}
			break;
		case ScanCode_RETURN:
			if (has_selected_item() && item(selected_item()).availabie())
			{
				auto& selectedItem = item(selected_item());
				if (selectedItem.type() == i_menu_item::Action)
				{
					selectedItem.action().triggered.trigger();
					if (selectedItem.action().is_checkable())
						selectedItem.action().toggle();
					clear_selection();
				}
				else if (selectedItem.type() == i_menu_item::SubMenu && !selectedItem.sub_menu().is_open())
					open_sub_menu.trigger(selectedItem.sub_menu());
			}
			break;
		case ScanCode_ESCAPE:
			close_sub_menu(false);
			clear_selection();
			update();
			break;
		default:
			break;
		}
		return true;
	}

	bool menu_bar::key_released(scan_code_e, key_code_e, key_modifiers_e)
	{
		return true;
	}

	bool menu_bar::text_input(const std::string&)
	{
		app::instance().basic_services().system_beep();
		return true;
	}

	void menu_bar::init()
	{
		set_margins(neogfx::margins{});
		layout().set_margins(neogfx::margins{});
		iSink += surface().dismissing_children([this](const i_widget* aClickedWidget)
		{
			if (aClickedWidget != this && (aClickedWidget == 0 || !is_ancestor_of(*aClickedWidget)))
				clear_selection();
			update();
		});
		iSink += item_added([this](item_index aItemIndex)
		{
			layout().add_item(aItemIndex, std::make_shared<menu_item_widget>(*this, item(aItemIndex)));
		});
		iSink += item_removed([this](item_index aItemIndex)
		{
			if (layout().is_widget(aItemIndex))
			{
				i_widget& w = layout().get_widget(aItemIndex);
				w.parent().remove_widget(w);
			}
			layout().remove_item(aItemIndex);
		});
		iSink += item_selected([this](i_menu_item& aMenuItem)
		{
			if (iOpenSubMenu->has_menu())
			{
				if (aMenuItem.type() == i_menu_item::Action ||
					(aMenuItem.type() == i_menu_item::SubMenu && &iOpenSubMenu->menu() != &aMenuItem.sub_menu() && aMenuItem.availabie()))
				{
					close_sub_menu(false);
					if (aMenuItem.type() == i_menu_item::SubMenu)
						open_sub_menu.trigger(aMenuItem.sub_menu());
				}
			}
			update();
		});
		iSink += selection_cleared([this]()
		{
			if (app::instance().keyboard().is_keyboard_grabbed_by(*this))
				app::instance().keyboard().ungrab_keyboard(*this);
			close_sub_menu(false);
		});
		iSink += open_sub_menu([this](i_menu& aSubMenu)
		{
			auto& itemWidget = layout().get_widget<menu_item_widget>(find_item(aSubMenu));
			close_sub_menu(false);
			if (!app::instance().keyboard().is_keyboard_grabbed_by(*this))
				app::instance().keyboard().grab_keyboard(*this);
			iOpenSubMenu->set_menu(aSubMenu, itemWidget.sub_menu_position());
			iSink2 = iOpenSubMenu->menu().closed([this]()
			{
				if (iOpenSubMenu->has_menu())
					iOpenSubMenu->clear_menu();
			});
			iSink2 += iOpenSubMenu->closed([this]()
			{
				close_sub_menu();
				iOpenSubMenu.reset();
			});
		});
	}

	void menu_bar::close_sub_menu(bool aClearSelection)
	{
		iSink2 = sink{};
		iOpenSubMenu->clear_menu();
		if (aClearSelection)
			clear_selection();
	}

}