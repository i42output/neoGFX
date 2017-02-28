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

#include <neogfx/neogfx.hpp>
#include <neolib/raii.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/window/popup_menu.hpp>
#include <neogfx/gui/widget/menu_item_widget.hpp>

namespace neogfx
{
	popup_menu::popup_menu(const point& aPosition, i_menu& aMenu, style_e aStyle) :
		window(aPosition, size{}, aStyle, i_scrollbar::Button, framed_widget::SolidFrame), iParentWidget(0), iMenu(aMenu), iLayout(*this), iOpeningSubMenu(false)
	{
		init();
	}

	popup_menu::popup_menu(i_widget& aParent, const point& aPosition, i_menu& aMenu, style_e aStyle) :
		window(aParent, aPosition, size{}, aStyle, i_scrollbar::Button, framed_widget::SolidFrame), iParentWidget(&aParent), iMenu(aMenu), iLayout(*this), iOpeningSubMenu(false)
	{
		init();
	}

	popup_menu::~popup_menu()
	{
		if (app::instance().keyboard().is_keyboard_grabbed_by(*this))
			app::instance().keyboard().ungrab_keyboard(*this);
		close_sub_menu();
		if (iMenu.is_open())
			iMenu.close();
	}

	bool popup_menu::can_dismiss(const i_widget* aClickedWidget) const
	{
		return aClickedWidget == 0 || 
			iParentWidget == 0 || 
			(iParentWidget == aClickedWidget && (style() & DismissOnParentClick)) || 
			(iParentWidget != aClickedWidget && !iParentWidget->is_ancestor_of(*aClickedWidget));
	}

	void popup_menu::resized()
	{
		window::resized();
		rect desktopRect{ app::instance().surface_manager().desktop_rect(surface()) };
		rect surfaceRect{ surface().surface_position(), surface().surface_size() };
		if (surfaceRect.bottom() > desktopRect.bottom())
			surfaceRect.position().y += (desktopRect.bottom() - surfaceRect.bottom());
		if (surfaceRect.right() > desktopRect.right())
			surfaceRect.position().x += (desktopRect.right() - surfaceRect.right());
		if (iMenu.has_parent())
		{
			if (iMenu.parent().type() == i_menu::MenuBar)
			{
				if (iParentWidget != 0)
				{
					rect menuBarRect = iParentWidget->window_rect() + parent_surface().surface_position();
					if (!surfaceRect.intersection(menuBarRect).empty())
					{
						if (desktopRect.bottom() - menuBarRect.bottom() > surfaceRect.height())
							surfaceRect.position().y = menuBarRect.bottom();
						else
							surfaceRect.position().y = menuBarRect.top() - surfaceRect.height();
					}
				}
			}
			else
			{
				rect parentSurfaceRect{ parent_surface().surface_position(), parent_surface().surface_size() };
				if (surfaceRect.intersection(parentSurfaceRect).width() > 8.0)
				{
					if (parentSurfaceRect.right() + surfaceRect.width() < desktopRect.right())
						surfaceRect.position().x = parentSurfaceRect.right();
					else
						surfaceRect.position().x = parentSurfaceRect.position().x - surfaceRect.width();
				}
			}
		}
		if (surfaceRect.position() != surface().surface_position())
			surface().move_surface(surfaceRect.position());
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

	void popup_menu::set_capture()
	{
		/* do nothing */
	}

	bool popup_menu::key_pressed(scan_code_e aScanCode, key_code_e, key_modifiers_e)
	{
		switch (aScanCode)
		{
		case ScanCode_UP:
			if (iMenu.has_selected_item())
				iMenu.select_item(iMenu.previous_available_item(iMenu.selected_item()));
			else if (iMenu.has_available_items())
				iMenu.select_item(iMenu.first_available_item());
			break;
		case ScanCode_DOWN:
			if (iMenu.has_selected_item())
				iMenu.select_item(iMenu.next_available_item(iMenu.selected_item()));
			else if (iMenu.has_available_items())
				iMenu.select_item(iMenu.first_available_item());
			break;
		case ScanCode_LEFT:
			if (iMenu.has_parent())
			{
				if (iMenu.parent().type() == i_menu::Popup)
					iMenu.close();
				else if (iMenu.parent().has_selected_item())
					iMenu.parent().select_item(iMenu.parent().previous_available_item(iMenu.parent().selected_item()), true);
			}
			break;
		case ScanCode_RIGHT:
			if (iMenu.has_selected_item())
			{
				if (iMenu.item(iMenu.selected_item()).type() == i_menu_item::SubMenu)
				{
					auto& subMenu = iMenu.item(iMenu.selected_item()).sub_menu();
					if (!subMenu.is_open())
						iMenu.open_sub_menu.trigger(subMenu);
					if (subMenu.has_available_items())
						subMenu.select_item(subMenu.first_available_item());
				}
				else
				{
					i_menu* m = &iMenu;
					while (m->has_parent())
						m = &m->parent();
					if (m != &iMenu)
					{
						if (m->has_selected_item())
							m->select_item(m->next_available_item(m->selected_item()), true);
					}
				}
			}
			else if (iMenu.has_parent() && iMenu.parent().type() == i_menu::MenuBar)
			{
				if (iMenu.parent().has_selected_item())
					iMenu.parent().select_item(iMenu.parent().next_available_item(iMenu.parent().selected_item()), true);
			}
			break;
		case ScanCode_RETURN:
			if (iMenu.has_selected_item() && iMenu.item(iMenu.selected_item()).availabie())
			{
				auto& selectedItem = iMenu.item(iMenu.selected_item());
				if (selectedItem.type() == i_menu_item::Action)
				{
					selectedItem.action().triggered.trigger();
					if (selectedItem.action().is_checkable())
						selectedItem.action().toggle();
					iMenu.clear_selection();
					i_menu* menuToClose = &iMenu;
					while (menuToClose->has_parent() && menuToClose->parent().type() == i_menu::Popup)
						menuToClose = &menuToClose->parent();
					if (menuToClose->type() == i_menu::Popup)
						menuToClose->close();
				}
				else if (selectedItem.type() == i_menu_item::SubMenu && !selectedItem.sub_menu().is_open())
					iMenu.open_sub_menu.trigger(selectedItem.sub_menu());
			}
			break;
		case ScanCode_ESCAPE:
			iMenu.clear_selection();
			iMenu.close();
			break;
		default:
			break;
		}
		return true;
	}

	bool popup_menu::key_released(scan_code_e, key_code_e, key_modifiers_e)
	{
		return true;
	}

	bool popup_menu::text_input(const std::string&)
	{
		app::instance().basic_services().system_beep();
		return true;
	}

	i_menu& popup_menu::menu() const
	{
		return iMenu;
	}

	void popup_menu::init()
	{
		app::instance().keyboard().grab_keyboard(*this);
		iLayout.set_margins(neogfx::margins{});
		closed([this]()
		{
			if (iMenu.is_open())
				iMenu.close();
		});
		for (i_menu::item_index i = 0; i < iMenu.item_count(); ++i)
			iLayout.add_item(std::make_shared<menu_item_widget>(*this, iMenu, iMenu.item(i)));
		layout_items();
		iMenu.open();
		iSink += iMenu.item_added([this](i_menu::item_index aIndex)
		{
			iLayout.add_item(aIndex, std::make_shared<menu_item_widget>(*this, iMenu, iMenu.item(aIndex)));
			layout_items();
		});
		iSink += iMenu.item_removed([this](i_menu::item_index aIndex)
		{
			iLayout.remove_item(aIndex);
			layout_items();
		});
		iSink += iMenu.item_changed([this](i_menu::item_index)
		{
			layout_items();
		});
		iSink += iMenu.item_selected([this](i_menu_item& aMenuItem)
		{
			if (!app::instance().keyboard().is_keyboard_grabbed_by(*this))
				app::instance().keyboard().grab_keyboard(*this);
			if (iOpenSubMenu != nullptr)
			{
				if (aMenuItem.type() == i_menu_item::Action ||
					(aMenuItem.type() == i_menu_item::SubMenu && &iOpenSubMenu->menu() != &aMenuItem.sub_menu()))
				{
					iOpenSubMenu->menu().close();
				}
			}
			scroll_to(layout().get_widget<menu_item_widget>(iMenu.find_item(aMenuItem)));
			update();
		});
		iSink += iMenu.open_sub_menu([this](i_menu& aSubMenu)
		{
			if (!iOpeningSubMenu && aSubMenu.item_count() > 0)
			{
				neolib::scoped_flag sf(iOpeningSubMenu);
				auto& itemWidget = layout().get_widget<menu_item_widget>(iMenu.find_item(aSubMenu));
				close_sub_menu();
				iOpenSubMenu = std::make_unique<popup_menu>(*this, itemWidget.sub_menu_position(), aSubMenu);
				iSink2 += iOpenSubMenu->menu().closed([this]()
				{
					if (iOpenSubMenu != nullptr)
						iOpenSubMenu->close();
				});
				iSink2 += iOpenSubMenu->closed([this]()
				{
					close_sub_menu();
				});
			}
		});
		show();
	}

	void popup_menu::close_sub_menu()
	{
		if (iOpenSubMenu != nullptr)
		{
			iSink2 = sink{};
			iOpenSubMenu.reset();
		}
	}
}