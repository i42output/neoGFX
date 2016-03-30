// tab_button.cpp
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
#include "tab_button.hpp"

namespace neogfx
{
	tab_button::tab_button(i_tab_container& aContainer, const std::string& aText) :
		push_button(aText, ButtonStyleTab), iContainer(aContainer), iSelectedState(false)
	{
		iContainer.adding_tab(*this);
	}

	tab_button::tab_button(i_widget& aParent, i_tab_container& aContainer, const std::string& aText) :
		push_button(aParent, aText, ButtonStyleTab), iContainer(aContainer), iSelectedState(false)
	{
		iContainer.adding_tab(*this);
	}

	tab_button::tab_button(i_layout& aLayout, i_tab_container& aContainer, const std::string& aText) :
		push_button(aLayout, aText, ButtonStyleTab), iContainer(aContainer), iSelectedState(false)
	{
		iContainer.adding_tab(*this);
	}

	tab_button::~tab_button()
	{
		iContainer.removing_tab(*this);
	}

	bool tab_button::is_selected() const
	{
		return iSelectedState == true;
	}

	bool tab_button::is_deselected() const
	{
		return iSelectedState == false;
	}

	void tab_button::select()
	{
		set_selected_state(true);
	}

	const i_tab_container& tab_button::container() const
	{
		return iContainer;
	}

	i_tab_container& tab_button::container()
	{
		return iContainer;
	}

	const std::string& tab_button::text() const
	{
		return push_button::text().text();
	}

	void tab_button::set_text(const std::string& aText)
	{
		push_button::text().set_text(aText);
	}

	void tab_button::set_image(const i_texture& aTexture)
	{
		push_button::image().set_image(aTexture);
	}

	void tab_button::set_image(const i_image& aImage)
	{
		push_button::image().set_image(aImage);
	}

	rect tab_button::path_bounding_rect() const
	{
		scoped_units su(*this, UnitsPixels);
		rect result = push_button::path_bounding_rect();
		if (is_deselected())
			result.deflate(as_units(*this, UnitsMillimetres, delta(0.0, 25.4/96.0)).ceil() * delta(0.0, 2.0));
		else
			result.extents() += size(0.0, 5.0);
		return convert_units(*this, su.saved_units(), result);
	}

	bool tab_button::spot_colour() const
	{
		return is_selected();
	}

	colour tab_button::border_mid_colour() const
	{
		colour result = push_button::border_mid_colour();
		if (!is_selected())
			result.darken(0x20);
		return result;
	}
	
	size tab_button::minimum_size() const
	{
		if (has_minimum_size())
			return push_button::minimum_size();
		scoped_units su(*this, UnitsPixels);
		return convert_units(*this, su.saved_units(), push_button::minimum_size() + as_units(*this, UnitsMillimetres, size(25.4/96.0, 25.4/96.0)).ceil() * size(4.0, 4.0));
	}

	void tab_button::handle_pressed()
	{
		push_button::handle_pressed();
		select();
	}

	colour tab_button::foreground_colour() const
	{
		if (has_foreground_colour() || is_deselected())
			return push_button::foreground_colour();
		return container_background_colour();
	}

	void tab_button::paint(graphics_context& aGraphicsContext) const
	{
		push_button::paint(aGraphicsContext);
		{
			scoped_units su1(*this, UnitsPixels);
			scoped_units su2(aGraphicsContext, UnitsPixels);
			rect clipRect = default_clip_rect();
			clipRect.cy += 2.0;
			clipRect.x += 1.0;
			clipRect.cx -= 2.0;
			aGraphicsContext.scissor_off();
			aGraphicsContext.scissor_on(clipRect);
			push_button::paint(aGraphicsContext);
		}
	}

	void tab_button::set_selected_state(bool aSelectedState)
	{
		if (iSelectedState != aSelectedState)
		{
			if (aSelectedState)
				iContainer.selecting_tab(*this);
			iSelectedState = aSelectedState;
			update();
			if (is_selected())
				selected.trigger();
			else if (is_deselected())
				deselected.trigger();
		}
	}
}