// toolbar_button.cpp
/*
neogfx C++ GUI Library
Copyright (c) 2015-present, Leigh Johnston.

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
#include <neogfx/gui/widget/toolbar_button.hpp>

namespace neogfx
{
	toolbar_button::toolbar_button(i_action& aAction) : 
		push_button(aAction.button_text(), push_button_style::Toolbar), iAction(aAction)
	{
		init();
	}

	toolbar_button::toolbar_button(i_widget& aParent, i_action& aAction) :
		push_button(aParent, aAction.button_text(), push_button_style::Toolbar), iAction(aAction)
	{
		init();
	}

	toolbar_button::toolbar_button(i_layout& aLayout, i_action& aAction) :
		push_button(aLayout, aAction.button_text(), push_button_style::Toolbar), iAction(aAction)
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

	size toolbar_button::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (push_button::has_minimum_size() || !iAction.is_separator())
			return push_button::minimum_size(aAvailableSpace);
		return units_converter(*this).from_device_units(size{ 2.0, 2.0 });
	}	

	size toolbar_button::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (push_button::has_maximum_size() || !iAction.is_separator())
			return push_button::maximum_size(aAvailableSpace);
		return size(std::numeric_limits<size::dimension_type>::max(), std::numeric_limits<size::dimension_type>::max());
	}

	void toolbar_button::paint(graphics_context& aGraphicsContext) const
	{
		if (!iAction.is_separator())
			push_button::paint(aGraphicsContext);
		else
		{
			scoped_units su(*this, aGraphicsContext, units::Pixels);
			rect line = client_rect();
			line.deflate(0, std::floor(client_rect().height() / 6.0));
			line.cx = 1.0;
			colour ink = (has_foreground_colour() ? foreground_colour() : app::instance().current_style().palette().foreground_colour());
			aGraphicsContext.fill_rect(line, ink.darker(0x40));
			++line.x;
			aGraphicsContext.fill_rect(line, ink.lighter(0x40));
		}
	}

	colour toolbar_button::foreground_colour() const
	{
		if (push_button::has_foreground_colour())
			return push_button::foreground_colour();
		return colour{};
	}

	focus_policy toolbar_button::focus_policy() const
	{
		return neogfx::focus_policy::NoFocus;
	}

	void toolbar_button::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		push_button::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		layout_items(false);
	}

	void toolbar_button::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		push_button::mouse_button_released(aButton, aPosition);
		layout_items(false);
	}

	void toolbar_button::handle_clicked()
	{
		push_button::handle_clicked();
		if (iAction.is_enabled() && !iAction.is_separator())
		{
			iAction.triggered.async_trigger();
			if (iAction.is_checkable())
			{
				if (is_checked())
					iAction.check();
				else
					iAction.uncheck();
			}
		}
	}

	void toolbar_button::init()
	{
		if (iAction.is_checkable())
			set_checkable();
		label().set_placement(label_placement::ImageVertical);
		text().set_text(iAction.button_text());
		image().set_image(iAction.image());
		iSink += iAction.enabled([this]() { enable(); });
		iSink += iAction.disabled([this]() { disable(); });
		enable(iAction.is_enabled());
		auto update_checked = [this]()
		{
			if (is_checked())
			{
				iAction.check();
				image().set_image(iAction.checked_image());
			}
			else
			{
				iAction.uncheck();
				image().set_image(iAction.image());
			}
		};
		iSink += checked(update_checked);
		iSink += unchecked(update_checked);
		iSink += iAction.checked([this]() {set_checked(true); });
		iSink += iAction.unchecked([this]() {set_checked(false); });
		set_checked(iAction.is_checked());
	}
}