// menu_item_widget.cpp
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
#include "menu_item_widget.hpp"
#include "i_menu.hpp"
#include "popup_menu.hpp"
#include "app.hpp"

namespace neogfx
{
	menu_item_widget::menu_item_widget(i_menu& aMenu, i_menu_item& aMenuItem) :
		iMenu(aMenu), iMenuItem(aMenuItem), iLayout(*this), iIcon(iLayout, texture{}), iText(iLayout), iSpacer(iLayout), iShortcutText(iLayout)
	{
		init();
	}

	menu_item_widget::menu_item_widget(i_widget& aParent, i_menu& aMenu, i_menu_item& aMenuItem) :
		widget(aParent), iMenu(aMenu), iMenuItem(aMenuItem), iLayout(*this), iIcon(iLayout, texture{}), iText(iLayout), iSpacer(iLayout), iShortcutText(iLayout)
	{
		init();
	}

	menu_item_widget::menu_item_widget(i_layout& aLayout, i_menu& aMenu, i_menu_item& aMenuItem) :
		widget(aLayout), iMenu(aMenu), iMenuItem(aMenuItem), iLayout(*this), iIcon(iLayout, texture{}), iText(iLayout), iSpacer(iLayout), iShortcutText(iLayout)
	{
		init();
	}

	menu_item_widget::~menu_item_widget()
	{
		iMenuItem.selected.unsubscribe(this);
		iSubMenuOpener.reset();
		if (iMenuItem.type() == i_menu_item::SubMenu)
		{
			iMenuItem.sub_menu().opened.unsubscribe(this);
			iMenuItem.sub_menu().closed.unsubscribe(this);
			iMenuItem.sub_menu().menu_changed.unsubscribe(this);
		}
		if (iMenuItem.type() == i_menu_item::Action)
		{
			iMenuItem.action().changed.unsubscribe(this);
			iMenuItem.action().checked.unsubscribe(this);
			iMenuItem.action().unchecked.unsubscribe(this);
			iMenuItem.action().enabled.unsubscribe(this);
			iMenuItem.action().disabled.unsubscribe(this);
		}
	}

	neogfx::size_policy menu_item_widget::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy{ iMenu.type() == i_menu::type_e::Popup ? neogfx::size_policy::Expanding : neogfx::size_policy::Minimum, neogfx::size_policy::Minimum };
	}

	size menu_item_widget::minimum_size(const optional_size&) const
	{
		size result = widget::minimum_size();
		if (iMenuItem.type() == i_menu_item::Action && iMenuItem.action().is_separator())
			result.cy = units_converter(*this).from_device_units(3.0);
		return result;
	}

	void menu_item_widget::paint_non_client(graphics_context& aGraphicsContext) const
	{
		if (iMenu.has_selected_item() && iMenu.selected_item() == (iMenu.find_item(iMenuItem)))
		{
			bool openSubMenu = (iMenuItem.type() == i_menu_item::SubMenu && iMenuItem.sub_menu().is_open());
			colour background;
			if (openSubMenu && iMenu.type() == i_menu::MenuBar)
			{
				background = app::instance().current_style().colour().dark() ?
					app::instance().current_style().colour().darker(0x40) :
					app::instance().current_style().colour().lighter(0x40);
				if (background.similar_intensity(app::instance().current_style().colour(), 0.05))
				{
					background = app::instance().current_style().selection_colour();
					background.set_alpha(0x80);
				}
			}
			else
			{
				background = background_colour().light() ? background_colour().darker(0x40) : background_colour().lighter(0x40);
				background.set_alpha(0x80);
			}
			aGraphicsContext.fill_rect(client_rect(), background);
		}
	}

	void menu_item_widget::paint(graphics_context& aGraphicsContext) const
	{
		if (iMenuItem.type() != i_menu_item::Action || !iMenuItem.action().is_separator())
		{
			widget::paint(aGraphicsContext);
			if (iMenuItem.type() == i_menu_item::SubMenu && iMenu.type() == i_menu::Popup)
			{
				bool openSubMenu = (iMenuItem.type() == i_menu_item::SubMenu && iMenuItem.sub_menu().is_open());
				colour ink = openSubMenu ? app::instance().current_style().selection_colour()
					: background_colour().light() ? background_colour().darker(0x80) : background_colour().lighter(0x80);
				if (iSubMenuArrow == boost::none || iSubMenuArrow->first != ink)
				{
					const uint8_t sArrowImagePattern[9][6]
					{
						{ 0, 0, 0, 0, 0, 0 },
						{ 0, 1, 0, 0, 0, 0 },
						{ 0, 1, 1, 0, 0, 0 },
						{ 0, 1, 1, 1, 0, 0 },
						{ 0, 1, 1, 1, 1, 0 },
						{ 0, 1, 1, 1, 0, 0 },
						{ 0, 1, 1, 0, 0, 0 },
						{ 0, 1, 0, 0, 0, 0 },
						{ 0, 0, 0, 0, 0, 0 },
					};
					iSubMenuArrow = std::make_pair(ink, image{ sArrowImagePattern, { { 0, colour{} },{ 1, ink } } });
				}
				rect rect = client_rect(false);
				aGraphicsContext.draw_texture(
					point{ rect.right() - iGap + std::floor((iGap - iSubMenuArrow->second.extents().cx) / 2.0), std::floor((rect.height() - iSubMenuArrow->second.extents().cy) / 2.0) },
					iSubMenuArrow->second);
			}
		}
		else
		{
			scoped_units su(*this, aGraphicsContext, UnitsPixels);
			rect line = client_rect(false);
			++line.y;
			line.cy = 1.0;
			line.x += (iIconSize + iGap * 2.0);
			line.cx -= (iIconSize + iGap * 3.0);
			colour ink = background_colour().light() ? background_colour().darker(0x60) : background_colour().lighter(0x60);
			ink.set_alpha(0x80);
			aGraphicsContext.fill_rect(line, ink);
		}
	}

	void menu_item_widget::mouse_entered()
	{
		widget::mouse_entered();
		update();
		if (iMenuItem.availabie())
			iMenu.select_item(iMenu.find_item(iMenuItem));
	}

	void menu_item_widget::mouse_left()
	{
		widget::mouse_left();
		update();
		iSubMenuOpener.reset();
	}

	void menu_item_widget::mouse_button_pressed(mouse_button aButton, const point&)
	{
		if (aButton == mouse_button::Left && iMenuItem.type() == i_menu_item::SubMenu)
			handle_pressed();
	}

	void menu_item_widget::mouse_button_released(mouse_button aButton, const point&)
	{
		if (aButton == mouse_button::Left && iMenuItem.type() == i_menu_item::Action)
			handle_pressed();
	}

	bool menu_item_widget::key_pressed(scan_code_e aScanCode, key_code_e, key_modifiers_e)
	{
		if (aScanCode == ScanCode_RETURN)
		{
			handle_pressed();
			return true;
		}
		return false;
	}

	point menu_item_widget::sub_menu_position() const
	{
		if (iMenu.type() == i_menu::MenuBar)
			return window_rect().bottom_left() + surface().surface_position();
		else
			return window_rect().top_right() + surface().surface_position();
	}

	void menu_item_widget::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{ iGap, 0.0, iGap * (iMenu.type() == i_menu::Popup ? 2.0 : 1.0), 0.0 });
		iLayout.set_spacing(size{ iGap, 0.0 });
		if (iMenu.type() == i_menu::Popup)
			iIcon.set_fixed_size(size{ iIconSize, iIconSize });
		else
			iIcon.set_fixed_size(size{});
		iSpacer.set_minimum_size(size{ 0.0, 0.0 });
		if (iMenuItem.type() == i_menu_item::Action)
		{
			auto action_changed = [this]()
			{
				iIcon.set_image(iMenuItem.action().is_unchecked() ? iMenuItem.action().image() : iMenuItem.action().checked_image());
				if (!iIcon.image().is_empty())
					iIcon.set_fixed_size(size{ iIconSize, iIconSize });
				else if (iMenu.type() == i_menu::MenuBar)
					iIcon.set_fixed_size(size{});
				iText.set_text(iMenuItem.action().menu_text());
				iShortcutText.set_text(iMenuItem.action().shortcut() != boost::none ? iMenuItem.action().shortcut()->as_text() : std::string());
				iSpacer.set_minimum_size(size{ iMenuItem.action().shortcut() != boost::none ?  iGap * 2.0 : 0.0, 0.0 });
				enable(iMenuItem.action().is_enabled());
			};
			iMenuItem.action().changed(action_changed, this);
			iMenuItem.action().checked(action_changed, this);
			iMenuItem.action().unchecked(action_changed, this);
			iMenuItem.action().enabled(action_changed, this);
			iMenuItem.action().disabled(action_changed, this);
			action_changed();
		}
		else
		{
			iMenuItem.sub_menu().opened([this]() {update(); }, this);
			iMenuItem.sub_menu().closed([this]() {update(); }, this);
			auto menu_changed = [this]() 
			{ 
				iIcon.set_image(iMenuItem.sub_menu().image());
				iText.set_text(iMenuItem.sub_menu().title());
			};
			iMenuItem.sub_menu().menu_changed(menu_changed, this);
			menu_changed();
		}
		iMenuItem.selected([this]()
		{
			if (iMenuItem.type() == i_menu_item::SubMenu && iMenu.type() == i_menu::Popup)
			{
				iSubMenuOpener = std::make_unique<neolib::callback_timer>(app::instance(), [this](neolib::callback_timer&)
				{
					if (!iMenuItem.sub_menu().is_open())
					{
						destroyed_flag destroyed(*this);
						iMenu.open_sub_menu.trigger(iMenuItem.sub_menu());
						if (!destroyed)
							update();
					}
				}, 250);
			}
		}, this);
	}

	void menu_item_widget::handle_pressed()
	{
		if (!iMenuItem.availabie())
			return;
		if (iMenuItem.type() == i_menu_item::Action)
		{
			iMenuItem.action().triggered.trigger();
			if (iMenuItem.action().is_checkable())
				iMenuItem.action().toggle();
			i_menu* menuToClose = &iMenu;
			while (menuToClose->has_parent() && menuToClose->parent().type() == i_menu::Popup)
				menuToClose = &menuToClose->parent();
			if (menuToClose->type() == i_menu::Popup)
				menuToClose->close();
			iMenu.clear_selection();
		}
		else
		{
			if (!iMenuItem.sub_menu().is_open())
			{
				iMenu.open_sub_menu.trigger(iMenuItem.sub_menu());
				update();
			}
			else if (iMenu.type() == i_menu::MenuBar)
			{
				iMenuItem.sub_menu().close();
				update();
			}
		}
	}
}