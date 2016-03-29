// toolbar_button.cpp
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
#include "toolbar_button.hpp"

namespace neogfx
{
	toolbar_button::toolbar_button(i_action& aAction) : 
		push_button(aAction.button_text(), ButtonStyleToolbar), iAction(aAction)
	{
		init();
	}

	toolbar_button::toolbar_button(i_widget& aParent, i_action& aAction) :
		push_button(aParent, aAction.button_text(), ButtonStyleToolbar), iAction(aAction)
	{
		init();
	}

	toolbar_button::toolbar_button(i_layout& aLayout, i_action& aAction) :
		push_button(aLayout, aAction.button_text(), ButtonStyleToolbar), iAction(aAction)
	{
		init();
	}

	toolbar_button::~toolbar_button()
	{
	}
	
	void toolbar_button::layout_items_completed()
	{
		push_button::layout_items_completed();
		if (capturing())
		{
			point pt = label().position();
			pt += point{ 1.0, 0.0 };
			label().move(pt);
		}
	}

	colour toolbar_button::foreground_colour() const
	{
		return colour{};
	}

	void toolbar_button::mouse_button_pressed(mouse_button aButton, const point& aPosition)
	{
		push_button::mouse_button_pressed(aButton, aPosition);
		layout_items(false);
	}

	void toolbar_button::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		push_button::mouse_button_released(aButton, aPosition);
		layout_items(false);
	}

	void toolbar_button::handle_pressed()
	{
		push_button::handle_pressed();
		if (iAction.is_enabled() && !iAction.is_separator())
		{
			if (!iAction.is_checkable())
				iAction.triggered.trigger();
			else if (is_checked())
				iAction.set_checked();
			else
				iAction.set_unchecked();
		}
	}

	void toolbar_button::init()
	{
		label().set_placement(label_placement::ImageVertical);
		text().set_text(iAction.button_text());
		image().set_image(iAction.image());
		auto update_enabled = [this]()
		{
			enable(iAction.is_enabled());
		};
		iAction.enabled(update_enabled);
		iAction.disabled(update_enabled);
		update_enabled();
	}
}