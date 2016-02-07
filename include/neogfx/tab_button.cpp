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
		set_margins(neogfx::margins(0.0));
		layout().set_margins(neogfx::margins(0.0));
		iContainer.adding_tab(*this);
	}

	tab_button::tab_button(i_widget& aParent, i_tab_container& aContainer, const std::string& aText) :
		push_button(aParent, aText, ButtonStyleTab), iContainer(aContainer), iSelectedState(false)
	{
		set_margins(neogfx::margins(0.0));
		layout().set_margins(neogfx::margins(0.0));
		iContainer.adding_tab(*this);
	}

	tab_button::tab_button(i_layout& aLayout, i_tab_container& aContainer, const std::string& aText) :
		push_button(aLayout, aText, ButtonStyleTab), iContainer(aContainer), iSelectedState(false)
	{
		set_margins(neogfx::margins(0.0));
		layout().set_margins(neogfx::margins(0.0));
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

	size tab_button::minimum_size() const
	{
		if (is_selected())
		{
			scoped_units su(*this, UnitsPixels);
			return convert_units(*this, su.saved_units(), push_button::minimum_size() + as_units(*this, UnitsMillimetres, size(1.0, 1.0)).ceil());
		}
		return push_button::minimum_size();
	}

	void tab_button::handle_pressed()
	{
		push_button::handle_pressed();
		select();
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