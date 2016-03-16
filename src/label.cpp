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
	label::label(const std::string& aText, bool aMultiLine, alignment aAlignment, label_placement aPlacement) :
		widget(), iAlignment(aAlignment), iPlacement(aPlacement), iLayout(*this), iImage(iLayout, neogfx::texture()), iText(iLayout, aText, aMultiLine)
	{
		iLayout.set_size_policy(neogfx::size_policy::Minimum);
		iLayout.set_margins(neogfx::margins{});
		iText.set_margins(neogfx::margins{});
		iImage.set_margins(neogfx::margins{});
		handle_placement_change();
		set_ignore_mouse_events(true);
		iText.set_ignore_mouse_events(true);
		iImage.set_ignore_mouse_events(true);
	}

	label::label(i_widget& aParent, const std::string& aText, bool aMultiLine, alignment aAlignment, label_placement aPlacement) :
		widget(aParent), iAlignment(aAlignment), iPlacement(aPlacement), iLayout(*this), iImage(iLayout, neogfx::texture()), iText(iLayout, aText, aMultiLine)
	{
		iLayout.set_size_policy(neogfx::size_policy::Minimum);
		iLayout.set_margins(neogfx::margins{});
		iText.set_margins(neogfx::margins{});
		iImage.set_margins(neogfx::margins{});
		handle_placement_change();
		set_ignore_mouse_events(true);
		iText.set_ignore_mouse_events(true);
		iImage.set_ignore_mouse_events(true);
	}

	label::label(i_layout& aLayout, const std::string& aText, bool aMultiLine, alignment aAlignment, label_placement aPlacement) :
		widget(aLayout), iAlignment(aAlignment), iPlacement(aPlacement), iLayout(*this), iImage(iLayout, neogfx::texture()), iText(iLayout, aText, aMultiLine)
	{
		iLayout.set_size_policy(neogfx::size_policy::Minimum);
		iLayout.set_margins(neogfx::margins{});
		iText.set_margins(neogfx::margins{});
		iImage.set_margins(neogfx::margins{});
		handle_placement_change();
		set_ignore_mouse_events(true);
		iText.set_ignore_mouse_events(true);
		iImage.set_ignore_mouse_events(true);
	}

	label_placement label::placement() const
	{
		return iPlacement;
	}

	void label::set_placement(label_placement aPlacement)
	{
		if (iPlacement == aPlacement)
			return;
		iPlacement = aPlacement;
		handle_placement_change();
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

	void label::handle_placement_change()
	{
		iLayout.remove_items();

		grid_layout::cell_coordinates start{};

		bool padLeft = ((iAlignment & alignment::Left) != alignment::Left);
		bool padTop = ((iAlignment & alignment::Top) != alignment::Top);
		if (padLeft || padTop)
		{
			iLayout.add_spacer(0, 0);
			if (padLeft)
				start.x = 1;
			if (padTop)
				start.y = 1;
		}

		switch (iPlacement)
		{
		case label_placement::ImageTextHorizontal:
			iLayout.add_item(start.y + 0, start.x + 0, iImage);
			iLayout.add_item(start.y + 0, start.x + 1, iText);
			break;
		case label_placement::ImageTextVertical:
			iLayout.add_item(start.y + 0, start.x + 0, iImage);
			iLayout.add_item(start.y + 1, start.x + 0, iText);
			break;
		case label_placement::TextImageHorizontal:
			iLayout.add_item(start.y + 0, start.x + 0, iText);
			iLayout.add_item(start.y + 0, start.x + 1, iImage);
			break;
		case label_placement::TextImageVertical:
			iLayout.add_item(start.y + 0, start.x + 0, iText);
			iLayout.add_item(start.y + 1, start.x + 0, iImage);
			break;
		}

		bool padRight = ((iAlignment & alignment::Right) != alignment::Right);
		bool padBottom = ((iAlignment & alignment::Bottom) != alignment::Bottom);
		if (padRight || padBottom)
		{
			iLayout.add_spacer(padBottom ? iLayout.rows() : iLayout.rows() - 1, padRight ? iLayout.columns() : iLayout.columns() - 1);
		}
	}
}