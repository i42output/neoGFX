// button.cpp
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
#include "button.hpp"
#include "label.hpp"
#include "app.hpp"

namespace neogfx
{
	button::our_layout::our_layout(button& aParent) :
		horizontal_layout(aParent)
	{
	}

	neogfx::margins button::our_layout::margins() const
	{
		return has_margins() ? 
			horizontal_layout::margins() :
			owner()->has_margins() ?
				owner()->margins() :
				neogfx::margins(owner()->margins().left * 2.0, owner()->margins().top, owner()->margins().right * 2.0, owner()->margins().bottom);
	}

	button::button(const std::string& aText, alignment aAlignment) :
		widget(), iLayout(*this), iLabel(new neogfx::label(iLayout, aText, true, aAlignment))
	{
		set_focus_policy(focus_policy::TabFocus);
	}
	
	button::button(i_widget& aParent, const std::string& aText, alignment aAlignment) :
		widget(aParent), iLayout(*this), iLabel(new neogfx::label(iLayout, aText, true, aAlignment))
	{
		set_focus_policy(focus_policy::TabFocus);
	}


	button::button(i_layout& aLayout, const std::string& aText, alignment aAlignment) :
		widget(aLayout), iLayout(*this), iLabel(new neogfx::label(iLayout, aText, true, aAlignment))
	{
		set_focus_policy(focus_policy::TabFocus);
	}

	const neogfx::label& button::label() const
	{
		return static_cast<const neogfx::label&>(*iLabel);
	}

	neogfx::label& button::label()
	{
		return static_cast<neogfx::label&>(*iLabel);
	}

	const text_widget& button::text() const
	{
		return label().text();
	}

	text_widget& button::text()
	{
		return label().text();
	}

	void button::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		bool wasCapturing = capturing();
		widget::mouse_button_released(aButton, aPosition);
		if (wasCapturing && client_rect().contains(aPosition))
		{
			if (aButton == mouse_button::Left)
				pressed.trigger();
			else if (aButton == mouse_button::Right)
				hide();
		}
	}
}

