// check_box.cpp
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
#include "check_box.hpp"

namespace neogfx
{
	check_box::box::box(check_box& aParent)
	{
		aParent.layout().add_widget(0, *this);
		set_margins(neogfx::margins(0.0));
		set_ignore_mouse_events(true);
	}

	size check_box::box::minimum_size() const
	{
		if (has_minimum_size())
			return widget::minimum_size();
		scoped_units su(*this, UnitsPixels);
		dimension length = std::ceil(units_converter(*this).from_device_units(static_cast<const check_box&>(parent()).text().font().height() * (2.0 / 3.0)));
		length = std::max(length, std::ceil(as_units(*this, UnitsMillimetres, 3.0)));
		return convert_units(*this, su.saved_units(), size(length, length));
	}

	size check_box::box::maximum_size() const
	{
		return minimum_size();
	}
		
	void check_box::box::paint(graphics_context& aGraphicsContext) const
	{
		scoped_units su(*this, UnitsPixels);
		rect boxRect = client_rect();
		aGraphicsContext.fill_solid_rect(boxRect, background_colour());
		colour borderColour1 = container_background_colour().mid(container_background_colour().mid(background_colour()));
		if (borderColour1.similar_intensity(container_background_colour(), 0.03125))
			borderColour1.dark() ? borderColour1.lighten(0x40) : borderColour1.darken(0x40);
		aGraphicsContext.draw_rect(boxRect, pen(borderColour1, 1.0));
		boxRect.deflate(1.0, 1.0);
		aGraphicsContext.draw_rect(boxRect, pen(borderColour1.mid(background_colour()), 1.0));
		boxRect.deflate(2.0, 2.0);
		if (static_cast<const check_box&>(parent()).is_checked())
		{
			/* todo: draw tick image eye candy */
			aGraphicsContext.draw_line(boxRect.top_left(), boxRect.bottom_right(), pen(app::instance().current_style().widget_detail_primary_colour(), 2.0));
			aGraphicsContext.draw_line(boxRect.bottom_left(), boxRect.top_right(), pen(app::instance().current_style().widget_detail_primary_colour(), 2.0));
		}
		else if (static_cast<const check_box&>(parent()).is_tristate())
		{
			aGraphicsContext.fill_solid_rect(boxRect, app::instance().current_style().widget_detail_primary_colour());
		}
	}

	check_box::check_box(const std::string& aText, style_e aStyle) :
		button(aText), iStyle(aStyle), iBox(*this), iChecked(false)
	{
		layout().set_margins(neogfx::margins(0.0));
		layout().add_spacer();
	}

	check_box::check_box(i_widget& aParent, const std::string& aText, style_e aStyle) :
		button(aParent, aText), iStyle(aStyle), iBox(*this), iChecked(false)
	{
		layout().set_margins(neogfx::margins(0.0));
		layout().add_spacer();
	}

	check_box::check_box(i_layout& aLayout, const std::string& aText, style_e aStyle) :
		button(aLayout, aText), iStyle(aStyle), iBox(*this), iChecked(false)
	{
		layout().set_margins(neogfx::margins(0.0));
		layout().add_spacer();
	}

	bool check_box::is_checked() const
	{
		return iChecked != boost::none && *iChecked == true;
	}

	void check_box::set_checked(bool aChecked)
	{
		if (iChecked == boost::none || *iChecked != aChecked)
		{
			iChecked = aChecked;
			update();
			if (*iChecked)
				checked.trigger();
			else
				unchecked.trigger();
		}
	}

	bool check_box::is_tristate() const
	{
		return iChecked == boost::none;
	}

	void check_box::set_tristate()
	{
		if (iStyle != TriState)
			throw not_tri_state();
		if (iChecked != boost::none)
		{
			iChecked = boost::none;
			update();
			tristate.trigger();
		}
	}

	void check_box::toggle()
	{
		if (is_checked())
			set_checked(false);
		else
			set_checked(true);
	}

	void check_box::handle_pressed()
	{
		button::handle_pressed();
		toggle();
	}
}