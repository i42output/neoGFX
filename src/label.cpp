// label.cpp
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
#include "label.hpp"
#include "text_widget.hpp"
#include "i_spacer.hpp"

namespace neogfx
{
	label::label(const std::string& aText, bool aMultiLine, alignment aAlignment) :
		widget(), iAlignment(aAlignment), iLayout(*this), iImage(iLayout, neogfx::texture()), iText(iLayout, aText, aMultiLine)
	{
		handle_alignment();
		iLayout.set_size_policy(neogfx::size_policy::Minimum);
		iLayout.set_margins(neogfx::margins(0.0));
		iText.set_margins(neogfx::margins(0.0));
		set_ignore_mouse_events(true);
		iText.set_ignore_mouse_events(true);
	}

	label::label(i_widget& aParent, const std::string& aText, bool aMultiLine, alignment aAlignment) :
		widget(aParent), iAlignment(aAlignment), iLayout(*this), iImage(iLayout, neogfx::texture()), iText(iLayout, aText, aMultiLine)
	{
		handle_alignment();
		iLayout.set_size_policy(neogfx::size_policy::Minimum);
		iLayout.set_margins(neogfx::margins(0.0));
		set_ignore_mouse_events(true);
	}

	label::label(i_layout& aLayout, const std::string& aText, bool aMultiLine, alignment aAlignment) :
		widget(aLayout), iAlignment(aAlignment), iLayout(*this), iImage(iLayout, neogfx::texture()), iText(iLayout, aText, aMultiLine)
	{
		handle_alignment();
		iLayout.set_size_policy(neogfx::size_policy::Minimum);
		iLayout.set_margins(neogfx::margins(0.0));
		set_ignore_mouse_events(true);
	}

	const image_widget& label::image() const
	{
		return iImage;
	}

	image_widget& label::image()
	{
		return iImage;
	}

	const text_widget& label::text() const
	{
		return iText;
	}

	text_widget& label::text()
	{
		return iText;
	}

	void label::handle_alignment()
	{
		if ((iAlignment & alignment::Left) == alignment::Left)
			iLayout.add_spacer(iLayout.item_count());
		else if ((iAlignment & alignment::Right) == alignment::Right)
			iLayout.add_spacer(0);
		else if (((iAlignment & alignment::Centre) == alignment::Centre) || ((iAlignment & alignment::Justify) == alignment::Justify))
		{
			iLayout.add_spacer(0);
			iLayout.add_spacer(iLayout.item_count());
		}
	}
}