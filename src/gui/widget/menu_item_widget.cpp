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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/menu_item_widget.hpp>
#include <neogfx/gui/widget/i_menu.hpp>
#include <neogfx/gui/window/popup_menu.hpp>
#include <neogfx/app/app.hpp>

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
		app::instance().remove_mnemonic(*this);
		iSubMenuOpener.reset();
	}

	i_menu& menu_item_widget::menu() const
	{
		return iMenu;
	}

	i_menu_item& menu_item_widget::menu_item() const
	{
		return iMenuItem;
	}

	neogfx::size_policy menu_item_widget::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy{ menu().type() == i_menu::type_e::Popup ? neogfx::size_policy::Expanding : neogfx::size_policy::Minimum, neogfx::size_policy::Minimum };
	}

	size menu_item_widget::minimum_size(const optional_size&) const
	{
		size result = widget::minimum_size();
		if (menu_item().type() == i_menu_item::Action && menu_item().action().is_separator())
			result.cy = units_converter(*this).from_device_units(3.0);
		return result;
	}

	void menu_item_widget::paint_non_client(graphics_context& aGraphicsContext) const
	{
		if (menu().has_selected_item() && menu().selected_item() == (menu().find_item(menu_item())))
		{
			bool openSubMenu = (menu_item().type() == i_menu_item::SubMenu && menu_item().sub_menu().is_open());
			colour background;
			if (openSubMenu && menu().type() == i_menu::MenuBar)
			{
				background = app::instance().current_style().palette().colour().dark() ?
					app::instance().current_style().palette().colour().darker(0x40) :
					app::instance().current_style().palette().colour().lighter(0x40);
				if (background.similar_intensity(app::instance().current_style().palette().colour(), 0.05))
				{
					background = app::instance().current_style().palette().selection_colour();
					background.set_alpha(0xC0);
				}
			}
			else
			{
				background = background_colour().light() ? background_colour().darker(0x40) : background_colour().lighter(0x40);
				background.set_alpha(0xC0);
			}
			aGraphicsContext.fill_rect(client_rect(), background);
		}
	}

	void menu_item_widget::paint(graphics_context& aGraphicsContext) const
	{
		if (menu_item().type() != i_menu_item::Action || !menu_item().action().is_separator())
		{
			widget::paint(aGraphicsContext);
			if (menu_item().type() == i_menu_item::SubMenu && menu().type() == i_menu::Popup)
			{
				bool openSubMenu = (menu_item().type() == i_menu_item::SubMenu && menu_item().sub_menu().is_open());
				colour ink = openSubMenu ? app::instance().current_style().palette().selection_colour()
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
					iSubMenuArrow = std::make_pair(ink, image{ "neogfx::menu_item_widget::" + ink.to_string(), sArrowImagePattern, { { 0, colour{} },{ 1, ink } } });
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

	bool menu_item_widget::can_capture() const
	{
		return false;
	}

	void menu_item_widget::mouse_entered()
	{
		widget::mouse_entered();
		update();
		if (menu_item().available())
			menu().select_item_at(menu().find_item(menu_item()));
	}

	void menu_item_widget::mouse_left()
	{
		widget::mouse_left();
		update();
		if (menu().has_selected_item() && menu().selected_item() == (menu().find_item(menu_item())) &&
			(menu_item().type() == i_menu_item::Action || (!menu_item().sub_menu().is_open() && !iSubMenuOpener)))
			menu().clear_selection();
	}

	void menu_item_widget::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left && menu_item().type() == i_menu_item::SubMenu)
			select_item();
	}

	void menu_item_widget::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		widget::mouse_button_released(aButton, aPosition);
		if (aButton == mouse_button::Left && menu_item().type() == i_menu_item::Action)
			select_item();
	}

	bool menu_item_widget::key_pressed(scan_code_e aScanCode, key_code_e, key_modifiers_e)
	{
		if (aScanCode == ScanCode_RETURN)
		{
			select_item(true);
			return true;
		}
		return false;
	}

	std::string menu_item_widget::mnemonic() const
	{
		return mnemonic_from_text(iText.text());
	}

	void menu_item_widget::mnemonic_execute()
	{
		select_item(true);
	}

	i_widget& menu_item_widget::mnemonic_widget()
	{
		return iText;
	}

	point menu_item_widget::sub_menu_position() const
	{
		if (menu().type() == i_menu::MenuBar)
			return window_rect().bottom_left() + surface().surface_position();
		else
			return window_rect().top_right() + surface().surface_position();
	}

	void menu_item_widget::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{ iGap, 2.0, iGap * (menu().type() == i_menu::Popup ? 2.0 : 1.0), 2.0 });
		iLayout.set_spacing(size{ iGap, 0.0 });
		if (menu().type() == i_menu::Popup)
			iIcon.set_fixed_size(size{ iIconSize, iIconSize });
		else
			iIcon.set_fixed_size(size{});
		iSpacer.set_minimum_size(size{ 0.0, 0.0 });
		auto text_updated = [this]()
		{
			auto m = mnemonic_from_text(iText.text());
			if (!m.empty())
				app::instance().add_mnemonic(*this);
			else
				app::instance().remove_mnemonic(*this);
		};
		iSink += iText.text_changed(text_updated);
		text_updated();
		if (menu_item().type() == i_menu_item::Action)
		{
			auto action_changed = [this]()
			{
				iIcon.set_image(menu_item().action().is_unchecked() ? menu_item().action().image() : menu_item().action().checked_image());
				if (!iIcon.image().is_empty())
					iIcon.set_fixed_size(size{ iIconSize, iIconSize });
				else if (menu().type() == i_menu::MenuBar)
					iIcon.set_fixed_size(size{});
				iText.set_text(menu_item().action().menu_text());
				if (menu().type() != i_menu::MenuBar)
					iShortcutText.set_text(menu_item().action().shortcut() != boost::none ? menu_item().action().shortcut()->as_text() : std::string());
				iSpacer.set_minimum_size(size{ menu_item().action().shortcut() != boost::none && menu().type() != i_menu::MenuBar ? iGap * 2.0 : 0.0, 0.0 });
				enable(menu_item().action().is_enabled());
			};
			iSink += menu_item().action().changed(action_changed);
			iSink += menu_item().action().checked(action_changed);
			iSink += menu_item().action().unchecked(action_changed);
			iSink += menu_item().action().enabled(action_changed);
			iSink += menu_item().action().disabled(action_changed);
			action_changed();
		}
		else
		{
			iSink += menu_item().sub_menu().opened([this]() {update(); });
			iSink += menu_item().sub_menu().closed([this]() {update(); });
			auto menu_changed = [this]() 
			{ 
				iIcon.set_image(menu_item().sub_menu().image());
				iText.set_text(menu_item().sub_menu().title());
			};
			iSink += menu_item().sub_menu().menu_changed(menu_changed);
			menu_changed();
		}
		iSink += menu_item().selected([this]()
		{
			if (menu_item().type() == i_menu_item::SubMenu && menu_item().select_any_sub_menu_item() && menu().type() == i_menu::Popup)
			{
				if (!iSubMenuOpener)
				{
					iSubMenuOpener = std::make_unique<neolib::callback_timer>(app::instance(), [this](neolib::callback_timer&)
					{
						destroyed_flag destroyed(*this);
						if (!menu_item().sub_menu().is_open())
							menu().open_sub_menu.trigger(menu_item().sub_menu());
						if (!destroyed)
							update();
						iSubMenuOpener.reset();
					}, 250);
				}
			}
		});
		iSink += menu_item().deselected([this]()
		{
			iSubMenuOpener.reset();
		});
	}

	void menu_item_widget::select_item(bool aSelectAnySubMenuItem)
	{
		destroyed_flag destroyed(*this);
		if (!menu_item().available())
			return;
		if (menu_item().type() == i_menu_item::Action)
		{
			menu().clear_selection();
			if (destroyed)
				return;
			menu_item().action().triggered.async_trigger();
			if (destroyed)
				return;
			if (menu_item().action().is_checkable())
				menu_item().action().toggle();
			if (destroyed)
				return;
			i_menu* menuToClose = &menu();
			while (menuToClose->has_parent())
				menuToClose = &menuToClose->parent();
			if (menuToClose->type() == i_menu::MenuBar)
				menuToClose->clear_selection();
			else if (menuToClose->type() == i_menu::Popup && menuToClose->is_open())
				menuToClose->close();
			if (destroyed)
				return;
		}
		else
		{
			if (!menu_item().sub_menu().is_open())
			{
				menu().select_item_at(menu().find_item(menu_item()), aSelectAnySubMenuItem);
				if (destroyed)
					return;
				menu().open_sub_menu.trigger(menu_item().sub_menu());
				if (destroyed)
					return;
				update();
			}
			else if (menu().type() == i_menu::MenuBar)
			{
				menu_item().sub_menu().close();
				if (destroyed)
					return;
				update();
			}
			else
			{
				auto& subMenu = menu_item().sub_menu();
				if (!subMenu.has_selected_item() && subMenu.has_available_items())
					subMenu.select_item_at(subMenu.first_available_item(), false);
			}
		}
	}
}