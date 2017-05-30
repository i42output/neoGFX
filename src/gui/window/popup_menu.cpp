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
	popup_menu::popup_menu(const point& aPosition, i_menu& aMenu, window_style aStyle) :
		window(aPosition, size{}, aStyle, scrollbar_style::Menu, frame_style::SolidFrame), iParentWidget(0), iMenu(nullptr), iLayout(*this), iOpeningSubMenu(false)
	{
		init();
		set_menu(aMenu, aPosition);
	}

	popup_menu::popup_menu(i_widget& aParent, const point& aPosition, i_menu& aMenu, window_style aStyle) :
		window(aParent, aPosition, size{}, aStyle, scrollbar_style::Menu, frame_style::SolidFrame), iParentWidget(&aParent), iMenu(nullptr), iLayout(*this), iOpeningSubMenu(false)
	{
		init();
		set_menu(aMenu, aPosition);
	}

	popup_menu::popup_menu(const point& aPosition, window_style aStyle) :
		window(aPosition, size{}, aStyle, scrollbar_style::Menu, frame_style::SolidFrame), iParentWidget(0), iMenu(nullptr), iLayout(*this), iOpeningSubMenu(false)
	{
		init();
	}

	popup_menu::popup_menu(i_widget& aParent, const point& aPosition, window_style aStyle) :
		window(aParent, aPosition, size{}, aStyle, scrollbar_style::Menu, frame_style::SolidFrame), iParentWidget(&aParent), iMenu(nullptr), iLayout(*this), iOpeningSubMenu(false)
	{
		init();
	}

	popup_menu::~popup_menu()
	{
		if (app::instance().keyboard().is_keyboard_grabbed_by(*this))
			app::instance().keyboard().ungrab_keyboard(*this);
		close_sub_menu();
		iOpenSubMenu.reset();
		if (has_menu() && menu().is_open())
			menu().close();
	}

	bool popup_menu::can_dismiss(const i_widget* aClickedWidget) const
	{
		return aClickedWidget == 0 || 
			iParentWidget == 0 || 
			(iParentWidget == aClickedWidget && (style() & (window_style::DismissOnParentClick | window_style::HideOnParentClick)) != window_style::Invalid) ||
			(iParentWidget != aClickedWidget && !iParentWidget->is_ancestor_of(*aClickedWidget));
	}

	i_surface::dismissal_type_e popup_menu::dismissal_type() const
	{
		if ((style() & window_style::DismissOnOwnerClick) == window_style::DismissOnOwnerClick)
			return CloseOnDismissal;
		else if ((style() & window_style::HideOnOwnerClick) == window_style::HideOnOwnerClick)
			return HideOnDismissal;
		else
			return CannotDismiss;
	}

	bool popup_menu::dismissed() const
	{
		switch (dismissal_type())
		{
		case CloseOnDismissal:
			return is_closed();
		case HideOnDismissal:
			return hidden();
		default:
			return false;
		}
	}

	void popup_menu::dismiss()
	{
		if ((style() & window_style::DismissOnOwnerClick) == window_style::DismissOnOwnerClick)
			close();
		else if ((style() & window_style::HideOnOwnerClick) == window_style::HideOnOwnerClick)
		{
			hide();
			if (has_menu() && menu().is_open())
				menu().close();
		}
		if (app::instance().keyboard().is_keyboard_grabbed_by(*this))
			app::instance().keyboard().ungrab_keyboard(*this);
	}

	bool popup_menu::has_rendering_priority() const
	{
		return window::has_rendering_priority() || visible();
	}

	bool popup_menu::has_menu() const
	{
		return iMenu != nullptr;
	}

	i_menu& popup_menu::menu() const
	{
		if (has_menu())
			return *iMenu;
		throw no_menu();
	}

	void popup_menu::set_menu(i_menu& aMenu, const point& aPosition)
	{
		if (has_menu())
			clear_menu();
		iMenu = &aMenu;
		if (iOpenSubMenu.get() == nullptr)
			iOpenSubMenu = std::make_unique<popup_menu>(*this, point{});
		if (!app::instance().keyboard().is_keyboard_grabbed_by(*this))
			app::instance().keyboard().grab_keyboard(*this);
		for (i_menu::item_index i = 0; i < menu().item_count(); ++i)
			iLayout.add_item(std::make_shared<menu_item_widget>(*this, menu(), menu().item_at(i)));
		layout_items();
		menu().open();
		iSink += menu().item_added([this](i_menu::item_index aIndex)
		{
			iLayout.add_item_at(aIndex, std::make_shared<menu_item_widget>(*this, menu(), menu().item_at(aIndex)));
			layout_items();
		});
		iSink += menu().item_removed([this](i_menu::item_index aIndex)
		{
			iLayout.remove_item_at(aIndex);
			layout_items();
		});
		iSink += menu().item_changed([this](i_menu::item_index)
		{
			layout_items();
		});
		iSink += menu().item_selected([this](i_menu_item& aMenuItem)
		{
			if (!app::instance().keyboard().is_keyboard_grabbed_by(*this))
				app::instance().keyboard().grab_keyboard(*this);
			if (iOpenSubMenu->has_menu() && iOpenSubMenu->menu().is_open())
			{
				if (aMenuItem.type() == i_menu_item::Action ||
					(aMenuItem.type() == i_menu_item::SubMenu && &iOpenSubMenu->menu() != &aMenuItem.sub_menu()))
				{
					iOpenSubMenu->menu().close();
				}
			}
			scroll_to(layout().get_widget_at<menu_item_widget>(menu().find_item(aMenuItem)));
			update();
		});
		iSink += menu().open_sub_menu([this](i_menu& aSubMenu)
		{
			if (!iOpeningSubMenu && aSubMenu.item_count() > 0)
			{
				neolib::scoped_flag sf{ iOpeningSubMenu };
				auto& itemWidget = layout().get_widget_at<menu_item_widget>(menu().find_item(aSubMenu));
				close_sub_menu();
				iOpenSubMenu->set_menu(aSubMenu, itemWidget.sub_menu_position());
				iSink2 += iOpenSubMenu->menu().closed([this]()
				{
					if (iOpenSubMenu->has_menu())
						iOpenSubMenu->clear_menu();
				});
				iSink2 += iOpenSubMenu->closed([this]()
				{
					close_sub_menu();
					iOpenSubMenu.reset();
				});
			}
		});
		move_surface(aPosition);
		resize(minimum_size());
		update_position();
		show();
	}

	void popup_menu::clear_menu()
	{
		if (iOpenSubMenu.get() != nullptr)
			iOpenSubMenu->clear_menu();
		if (has_menu() && menu().is_open())
			menu().close();
		iSink = sink{};
		iSink2 = sink{};
		iMenu = nullptr;
		hide();
		iLayout.remove_items();
		if (app::instance().keyboard().is_keyboard_grabbed_by(*this))
			app::instance().keyboard().ungrab_keyboard(*this);
	}

	void popup_menu::resized()
	{
		window::resized();
		update_position();
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
			if (menu().has_selected_item())
				menu().select_item_at(menu().previous_available_item(menu().selected_item()));
			else if (menu().has_available_items())
				menu().select_item_at(menu().first_available_item());
			break;
		case ScanCode_DOWN:
			if (menu().has_selected_item())
				menu().select_item_at(menu().next_available_item(menu().selected_item()));
			else if (menu().has_available_items())
				menu().select_item_at(menu().first_available_item());
			break;
		case ScanCode_LEFT:
			if (menu().has_parent())
			{
				if (menu().parent().type() == i_menu::Popup)
					menu().close();
				else if (menu().parent().has_selected_item())
					menu().parent().select_item_at(menu().parent().previous_available_item(menu().parent().selected_item()), true);
			}
			break;
		case ScanCode_RIGHT:
			if (menu().has_selected_item())
			{
				if (menu().item_at(menu().selected_item()).type() == i_menu_item::SubMenu)
				{
					auto& subMenu = menu().item_at(menu().selected_item()).sub_menu();
					if (!subMenu.is_open())
						menu().open_sub_menu.trigger(subMenu);
					if (subMenu.has_available_items())
						subMenu.select_item_at(subMenu.first_available_item());
				}
				else
				{
					i_menu* m = &menu();
					while (m->has_parent())
						m = &m->parent();
					if (m != &menu())
					{
						if (m->has_selected_item())
							m->select_item_at(m->next_available_item(m->selected_item()), true);
					}
				}
			}
			else if (menu().has_parent() && menu().parent().type() == i_menu::MenuBar)
			{
				if (menu().parent().has_selected_item())
					menu().parent().select_item_at(menu().parent().next_available_item(menu().parent().selected_item()), true);
			}
			break;
		case ScanCode_RETURN:
			if (menu().has_selected_item() && menu().item_at(menu().selected_item()).available())
			{
				auto& selectedItem = menu().item_at(menu().selected_item());
				if (selectedItem.type() == i_menu_item::Action)
				{
					selectedItem.action().triggered.async_trigger();
					if (selectedItem.action().is_checkable())
						selectedItem.action().toggle();
					menu().clear_selection();
					i_menu* menuToClose = &menu();
					while (menuToClose->has_parent() && menuToClose->parent().type() == i_menu::Popup)
						menuToClose = &menuToClose->parent();
					if (menuToClose->type() == i_menu::Popup)
						menuToClose->close();
				}
				else if (selectedItem.type() == i_menu_item::SubMenu && !selectedItem.sub_menu().is_open())
					menu().open_sub_menu.trigger(selectedItem.sub_menu());
			}
			break;
		case ScanCode_ESCAPE:
			menu().clear_selection();
			menu().close();
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

	void popup_menu::init()
	{
		iLayout.set_margins(neogfx::margins{});
		iLayout.set_spacing(neogfx::size{});
		closed([this]()
		{
			if (has_menu() && menu().is_open())
				menu().close();
		});
		if (has_menu())
			show();
	}

	void popup_menu::close_sub_menu()
	{
		if (iOpenSubMenu != nullptr)
		{
			iSink2 = sink{};
			iOpenSubMenu->clear_menu();
		}
	}

	void popup_menu::update_position()
	{
		rect desktopRect{ app::instance().surface_manager().desktop_rect(surface()) };
		rect surfaceRect{ surface().surface_position(), surface().surface_size() };
		if (surfaceRect.bottom() > desktopRect.bottom())
			surfaceRect.position().y += (desktopRect.bottom() - surfaceRect.bottom());
		if (surfaceRect.right() > desktopRect.right())
			surfaceRect.position().x += (desktopRect.right() - surfaceRect.right());
		if (has_menu() && menu().has_parent())
		{
			if (menu().parent().type() == i_menu::MenuBar)
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
}