// slider.cpp
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
#include "slider.hpp"
#include "app.hpp"

namespace neogfx
{
	slider_impl::slider_impl(type_e aType) :
		iType(aType),
		iNormalizedValue{ 0.0 }
	{
		init();
	}

	slider_impl::slider_impl(i_widget& aParent, type_e aType) :
		widget(aParent),
		iType(aType),
		iNormalizedValue{ 0.0 }
	{
		init();
	}

	slider_impl::slider_impl(i_layout& aLayout, type_e aType) :
		widget(aLayout),
		iType(aType),
		iNormalizedValue{ 0.0 }
	{
		init();
	}

	slider_impl::~slider_impl()
	{
	}

	size slider_impl::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		return convert_units(*this, su.saved_units(), size{ 16, 16 });
	}

	void slider_impl::paint(graphics_context& aGraphicsContext) const
	{
		scoped_units su(*this, UnitsPixels);
		rect rectBarBox = bar_box();
		colour ink = background_colour().light(0x80) ? background_colour().darker(0x80) : background_colour().lighter(0x80);
		aGraphicsContext.fill_rounded_rect(rectBarBox, 2.0, ink);
		rectBarBox.deflate(size{1.0, 1.0});
		aGraphicsContext.fill_rounded_rect(rectBarBox, 2.0, ink.mid(background_colour()));
		rect rectValue = rectBarBox;
		rectValue.cx = rectValue.width() * normalized_value();
		if (normalized_value() > 0.0)
			aGraphicsContext.fill_rounded_rect(rectValue, 2.0, app::instance().current_style().selection_colour());
		rect rectIndicator = indicator_box();
		colour indicatorColour = foreground_colour();
		if (capturing())
		{
			if (indicatorColour.light(0x40))
				indicatorColour.darken(0x40);
			else
				indicatorColour.lighten(0x40);
		}
		colour indicatorBorderColour = indicatorColour.darker(0x40);
		indicatorColour.lighten(0x40);
		aGraphicsContext.fill_circle(rectIndicator.centre(), rectIndicator.width() / 2.0, indicatorBorderColour);
		aGraphicsContext.fill_circle(rectIndicator.centre(), rectIndicator.width() / 2.0 - 1.0, indicatorColour);
	}

	void slider_impl::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			if (indicator_box().contains(aPosition))
			{
				iClickOffset = aPosition - indicator_box().centre();
				set_normalized_value(std::max(0.0, std::min(aPosition.x - bar_box().x - iClickOffset->x, bar_box().right())) / bar_box().cx);
			}
			else if (bar_box().contains(aPosition))
			{
				set_normalized_value(std::max(0.0, std::min(aPosition.x - bar_box().x, bar_box().right())) / bar_box().cx);
			}
		}
	}

	void slider_impl::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		widget::mouse_button_released(aButton, aPosition);
		if (aButton == mouse_button::Left)
			iClickOffset = boost::none;
	}

	void slider_impl::mouse_moved(const point& aPosition)
	{
		widget::mouse_moved(aPosition);
		if (iClickOffset != boost::none)
			set_normalized_value(std::max(0.0, std::min(aPosition.x - bar_box().x - iClickOffset->x, bar_box().right())) / bar_box().cx);
	}

	void slider_impl::set_normalized_value(double aValue)
	{
		aValue = std::max(0.0, std::min(1.0, aValue));
		if (iNormalizedValue != aValue)
		{
			iNormalizedValue = aValue;
			update();
		}
	}

	void slider_impl::init()
	{
		set_margins(neogfx::margins{});
		if (iType == Horizontal)
			set_size_policy(neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum });
		else
			set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding });

		auto step_up = [this]()
		{
			set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() + normalized_step_value())));
		};
		auto step_down = [this]()
		{
			set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() - normalized_step_value())));
		};
	}

	rect slider_impl::bar_box() const
	{
		rect result = client_rect(false);
		result.deflate(size{ result.height() / 2.5 });
		return result;
	}

	rect slider_impl::indicator_box() const
	{
		rect rectBarBox = bar_box();
		rect result{ point{ rectBarBox.x + rectBarBox.cx * normalized_value(), rectBarBox.centre().y}, size{} };
		result.inflate(size{ rectBarBox.height() * 1.5 });
		return result;
	}
}

