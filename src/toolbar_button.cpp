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
#include "app.hpp"
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

	neogfx::size_policy toolbar_button::size_policy() const
	{
		if (push_button::has_size_policy() || !iAction.is_separator())
			return push_button::size_policy();
		return neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding };
	}

	size toolbar_button::minimum_size() const
	{
		if (push_button::has_minimum_size() || !iAction.is_separator())
			return push_button::minimum_size();
		return units_converter(*this).from_device_units(size{ 2.0, 2.0 });
	}	

	size toolbar_button::maximum_size() const
	{
		if (push_button::has_maximum_size() || !iAction.is_separator())
			return push_button::maximum_size();
		return size(std::numeric_limits<size::dimension_type>::max(), std::numeric_limits<size::dimension_type>::max());
	}

	void toolbar_button::paint(graphics_context& aGraphicsContext) const
	{
		if (!iAction.is_separator())
			push_button::paint(aGraphicsContext);
		else
		{
			scoped_units su(*this, aGraphicsContext, UnitsPixels);
			rect line = client_rect();
			line.deflate(0, std::floor(client_rect().height() / 6.0));
			line.cx = 1.0;
			colour ink = (has_foreground_colour() ? foreground_colour() : app::instance().current_style().foreground_colour());
			aGraphicsContext.fill_solid_rect(line, ink.darker(0x40));
			++line.x;
			aGraphicsContext.fill_solid_rect(line, ink.lighter(0x40));
		}
	}

	colour toolbar_button::foreground_colour() const
	{
		if (push_button::has_foreground_colour())
			return push_button::foreground_colour();
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
		if (iAction.is_checkable())
			set_checkable();
		label().set_placement(label_placement::ImageVertical);
		text().set_text(iAction.button_text());
		image().set_image(iAction.image());
		auto update_enabled = [this]()
		{
			enable(iAction.is_enabled());
		};
		iAction.enabled(update_enabled);
		iAction.disabled(update_enabled);
		auto update_checked = [this]()
		{
			if (is_checked())
			{
				iAction.set_checked();
				image().set_image(iAction.checked_image());
			}
			else
			{
				iAction.set_unchecked();
				image().set_image(iAction.image());
			}
		};
		checked(update_checked);
		unchecked(update_checked);
		iAction.checked([this]() {set_checked(true); });
		iAction.unchecked([this]() {set_checked(false); });
		if (is_checked() || iAction.is_checked())
		{
			set_checked(true);
			update_checked();
		}
	}
}