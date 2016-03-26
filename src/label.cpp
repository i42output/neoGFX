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
		iLayout.set_alignment(aAlignment);
		init();
	}

	label::label(i_widget& aParent, const std::string& aText, bool aMultiLine, alignment aAlignment, label_placement aPlacement) :
		widget(aParent), iAlignment(aAlignment), iPlacement(aPlacement), iLayout(*this), iImage(iLayout, neogfx::texture()), iText(iLayout, aText, aMultiLine)
	{
		iLayout.set_alignment(aAlignment);
		init();
	}

	label::label(i_layout& aLayout, const std::string& aText, bool aMultiLine, alignment aAlignment, label_placement aPlacement) :
		widget(aLayout), iAlignment(aAlignment), iPlacement(aPlacement), iLayout(*this), iImage(iLayout, neogfx::texture()), iText(iLayout, aText, aMultiLine)
	{
		iLayout.set_alignment(aAlignment);
		init();
	}

	neogfx::size_policy label::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy::Minimum;
	}

	void label::set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout)
	{
		widget::set_size_policy(aSizePolicy, aUpdateLayout);
		text().set_size_policy(aSizePolicy, aUpdateLayout);
		image().set_size_policy(aSizePolicy, aUpdateLayout);
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

	void label::init()
	{
		iLayout.set_margins(neogfx::margins{});
		iText.set_margins(neogfx::margins{});
		iImage.set_margins(neogfx::margins{});
		set_ignore_mouse_events(true);
		iText.set_ignore_mouse_events(true);
		iImage.set_ignore_mouse_events(true);
		handle_placement_change();
		iLayout.alignment_changed([this]() { handle_placement_change(); });
		iText.visibility_changed([this](){ handle_placement_change(); });
		iText.text_changed([this]() { handle_placement_change(); });
		iImage.visibility_changed([this]() { handle_placement_change(); });
		iImage.image_changed([this]() { handle_placement_change(); });
	}

	label_placement label::effective_placement() const
	{
		switch (iPlacement)
		{
		case label_placement::TextImageHorizontal:
			if (iImage.image().is_empty() || iImage.hidden())
				return label_placement::TextHorizontal;
			else if (iText.text().empty() || iText.hidden())
				return label_placement::ImageHorizontal;
			break;
		case label_placement::TextImageVertical:
			if (iImage.image().is_empty() || iImage.hidden())
				return label_placement::TextVertical;
			else if (iText.text().empty() || iText.hidden())
				return label_placement::ImageVertical;
			break;
		case label_placement::ImageTextHorizontal:
			if (iImage.image().is_empty() || iImage.hidden())
				return label_placement::TextHorizontal;
			else if (iText.text().empty() || iText.hidden())
				return label_placement::ImageHorizontal;
			break;
		case label_placement::ImageTextVertical:
			if (iImage.image().is_empty() || iImage.hidden())
				return label_placement::TextVertical;
			else if (iText.text().empty() || iText.hidden())
				return label_placement::ImageVertical;
			break;
		default:
			break;
		}
		return iPlacement;
	}

	void label::handle_placement_change()
	{
		iLayout.remove_items();

		grid_layout::cell_coordinates start{};

		switch (effective_placement())
		{
		case label_placement::TextHorizontal:
			iLayout.add_item(0, 0, iText);
			break;
		case label_placement::TextVertical:
			iLayout.add_item(0, 0, iText);
			break;
		case label_placement::ImageHorizontal:
			iLayout.add_item(0, 0, iImage);
			break;
		case label_placement::ImageVertical:
			iLayout.add_item(0, 0, iImage);
			break;
		case label_placement::TextImageHorizontal:
			switch (iLayout.alignment() & neogfx::alignment::Horizontal)
			{
			case neogfx::alignment::Left:
				iLayout.add_item(0, 0, iText);
				iLayout.add_item(0, 1, iImage);
				iLayout.add_spacer(0, 2);
				break;
			case neogfx::alignment::Centre:
				iLayout.add_spacer(0, 0);
				iLayout.add_item(0, 1, iText);
				iLayout.add_item(0, 2, iImage);
				iLayout.add_spacer(0, 3);
				break;
			case neogfx::alignment::Right:
				iLayout.add_spacer(0, 0);
				iLayout.add_item(0, 1, iText);
				iLayout.add_item(0, 2, iImage);
				break;
			}
			break;
		case label_placement::TextImageVertical:
			switch (iLayout.alignment() & neogfx::alignment::Vertical)
			{
			case neogfx::alignment::Top:
				iLayout.add_item(0, 0, iText);
				iLayout.add_item(1, 0, iImage);
				iLayout.add_spacer(2, 0);
				break;
			case neogfx::alignment::VCentre:
				iLayout.add_spacer(0, 0);
				iLayout.add_item(1, 0, iText);
				iLayout.add_item(2, 0, iImage);
				iLayout.add_spacer(3, 0);
				break;
			case neogfx::alignment::Bottom:
				iLayout.add_spacer(0, 0);
				iLayout.add_item(1, 0, iText);
				iLayout.add_item(2, 0, iImage);
				break;
			}
			break;
		case label_placement::ImageTextHorizontal:
			switch (iLayout.alignment() & neogfx::alignment::Horizontal)
			{
			case neogfx::alignment::Left:
				iLayout.add_item(0, 0, iImage);
				iLayout.add_item(0, 1, iText);
				iLayout.add_spacer(0, 2);
				break;
			case neogfx::alignment::Centre:
				iLayout.add_spacer(0, 0);
				iLayout.add_item(0, 1, iImage);
				iLayout.add_item(0, 2, iText);
				iLayout.add_spacer(0, 3);
				break;
			case neogfx::alignment::Right:
				iLayout.add_spacer(0, 0);
				iLayout.add_item(0, 1, iImage);
				iLayout.add_item(0, 2, iText);
				break;
			}
			break;
		case label_placement::ImageTextVertical:
			switch (iLayout.alignment() & neogfx::alignment::Vertical)
			{
			case neogfx::alignment::Top:
				iLayout.add_item(0, 0, iImage);
				iLayout.add_item(1, 0, iText);
				iLayout.add_spacer(2, 0);
				break;
			case neogfx::alignment::VCentre:
				iLayout.add_spacer(0, 0);
				iLayout.add_item(1, 0, iImage);
				iLayout.add_item(2, 0, iText);
				iLayout.add_spacer(3, 0);
				break;
			case neogfx::alignment::Bottom:
				iLayout.add_spacer(0, 0);
				iLayout.add_item(1, 0, iImage);
				iLayout.add_item(2, 0, iText);
				break;
			}
			break;
		}
	}
}