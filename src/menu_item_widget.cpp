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
		if (iMenuItem.type() == i_menu_item::SubMenu)
		{
			iMenuItem.sub_menu().opened.unsubscribe(this);
			iMenuItem.sub_menu().closed.unsubscribe(this);
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
		return neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum };
	}

	void menu_item_widget::paint_non_client(graphics_context& aGraphicsContext) const
	{
		widget::paint_non_client(aGraphicsContext);
		bool openSubMenu = (iMenuItem.type() == i_menu_item::SubMenu && iMenuItem.sub_menu().is_open());
		if (enabled() && (entered() || capturing()) || openSubMenu)
		{
			colour background;
			if (openSubMenu)
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
			aGraphicsContext.fill_solid_rect(client_rect(), background);
		}
	}

	void menu_item_widget::paint(graphics_context& aGraphicsContext) const
	{
	}

	void menu_item_widget::mouse_entered()
	{
		widget::mouse_entered();
		update();
	}

	void menu_item_widget::mouse_left()
	{
		widget::mouse_left();
		update();
	}

	void menu_item_widget::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		bool wasCapturing = capturing();
		widget::mouse_button_released(aButton, aPosition);
		if (wasCapturing && client_rect().contains(aPosition))
		{
			if (aButton == mouse_button::Left)
				handle_pressed();
		}
	}

	void menu_item_widget::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		if (aScanCode == ScanCode_SPACE)
			handle_pressed();
	}

	point menu_item_widget::sub_menu_position() const
	{
		return window_rect().bottom_left() + surface().surface_position();
	}

	void menu_item_widget::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{ 9.0, 0.0 });
		iLayout.set_spacing(size{ 9.0, 0.0 });
		if (iMenu.type() == i_menu::Popup)
			iIcon.set_fixed_size(size{ 16.0, 16.0 });
		else
			iIcon.set_fixed_size(size{});
		iSpacer.set_minimum_size(size{ 0.0, 0.0 });
		if (iMenuItem.type() == i_menu_item::Action)
		{
			auto action_changed = [this]()
			{
				iIcon.set_image(iMenuItem.action().is_unchecked() ? iMenuItem.action().image() : iMenuItem.action().checked_image());
				if (!iIcon.image().is_empty())
					iIcon.set_fixed_size(size{ 16.0, 16.0 });
				else if (iMenu.type() == i_menu::MenuBar)
					iIcon.set_fixed_size(size{});
				iText.set_text(iMenuItem.action().menu_text());
				iShortcutText.set_text(iMenuItem.action().shortcut() != boost::none ? iMenuItem.action().shortcut()->as_text() : std::string());
				iSpacer.set_minimum_size(size{ iMenuItem.action().shortcut() != boost::none ? 18.0 : 0.0, 0.0 });
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
			iText.set_text(iMenuItem.sub_menu().title());
		}
	}

	void menu_item_widget::handle_pressed()
	{
		if (iMenuItem.type() == i_menu_item::Action)
		{
			if (iMenuItem.action().is_enabled())
			{
				iMenuItem.action().triggered.trigger();
				if (iMenuItem.action().is_checkable())
					iMenuItem.action().toggle();
			}
		}
		else
			iMenu.open_sub_menu.trigger(iMenuItem.sub_menu());
	}
}