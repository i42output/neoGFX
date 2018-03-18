// label.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/text_widget.hpp>
#include <neogfx/gui/layout/i_spacer.hpp>

namespace neogfx
{
	label::label(label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{}, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, neogfx::texture{} }, 
		iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(const std::string& aText, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{}, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, neogfx::texture{} }, 
		iText{ iLayout, aText, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(const i_texture& aTexture, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{}, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, aTexture }, 
		iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(const i_image& aImage, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{}, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, aImage }, 
		iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(i_widget& aParent, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{ aParent }, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, neogfx::texture{} }, 
		iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(i_widget& aParent, const std::string& aText, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{ aParent }, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, neogfx::texture{} }, 
		iText{ iLayout, aText, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(i_widget& aParent, const i_texture& aTexture, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{ aParent }, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, aTexture }, 
		iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(i_widget& aParent, const i_image& aImage, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{ aParent }, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, aImage }, 
		iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(i_layout& aLayout, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{ aLayout }, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, neogfx::texture{} }, 
		iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(i_layout& aLayout, const std::string& aText, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{ aLayout }, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, neogfx::texture{} }, 
		iText{ iLayout, aText, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(i_layout& aLayout, const i_texture& aTexture, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{ aLayout }, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, aTexture }, 
		iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::label(i_layout& aLayout, const i_image& aImage, label_type aType, alignment aAlignment, label_placement aPlacement) :
		widget{ aLayout }, 
		iAlignment{ aAlignment }, 
		iPlacement{ aPlacement }, 
		iLayout{ *this }, 
		iImage{ iLayout, aImage }, 
		iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty }, 
		iCentreSpacer{ nullptr }
	{
		iLayout.set_alignment(aAlignment);
		iText.set_alignment(aAlignment);
		init();
	}

	label::~label()
	{
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

	const i_spacer& label::centre_spacer() const
	{
		if (iCentreSpacer != nullptr)
			return *iCentreSpacer;
		throw no_centre_spacer();
	}

	i_spacer& label::centre_spacer()
	{
		if (iCentreSpacer != nullptr)
			return *iCentreSpacer;
		throw no_centre_spacer();
	}

	bool label::has_buddy() const
	{
		return iBuddy != nullptr;
	}

	i_widget& label::buddy() const
	{
		if (has_buddy())
			return *iBuddy;
		throw no_buddy();
	}

	void label::set_buddy(i_widget& aBuddy)
	{
		iBuddy = std::shared_ptr<i_widget>(std::shared_ptr<i_widget>(), &aBuddy);
	}

	void label::set_buddy(std::shared_ptr<i_widget> aBuddy)
	{
		iBuddy = aBuddy;
	}

	void label::unset_buddy()
	{
		iBuddy.reset();
	}

	void label::init()
	{
		iImage.set_dpi_auto_scale(true);
		iLayout.set_margins(neogfx::margins{});
		iText.set_margins(neogfx::margins{});
		iImage.set_margins(neogfx::margins{});
		set_ignore_mouse_events(true);
		iText.set_ignore_mouse_events(true);
		iImage.set_ignore_mouse_events(true);
		handle_placement_change();
		iSink += iLayout.alignment_changed([this]() { handle_placement_change(); });
		iSink += iText.visibility_changed([this](){ handle_placement_change(); });
		iSink += iText.text_changed([this]() { handle_placement_change(); });
		iSink += iImage.visibility_changed([this]() { handle_placement_change(); });
		iSink += iImage.image_changed([this]() { handle_placement_change(); });
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
		iLayout.remove_all();
		iCentreSpacer = nullptr;

		switch (effective_placement())
		{
		case label_placement::TextHorizontal:
			iLayout.add_item_at_position(0, 0, iText);
			break;
		case label_placement::TextVertical:
			iLayout.add_item_at_position(0, 0, iText);
			break;
		case label_placement::ImageHorizontal:
			iLayout.add_item_at_position(0, 0, iImage);
			break;
		case label_placement::ImageVertical:
			iLayout.add_item_at_position(0, 0, iImage);
			break;
		case label_placement::TextImageHorizontal:
			switch (iLayout.alignment() & neogfx::alignment::Horizontal)
			{
			case neogfx::alignment::Left:
				iLayout.add_item_at_position(0, 0, iText);
				iLayout.add_item_at_position(0, 1, iImage);
				iLayout.add_spacer_at_position(0, 2);
				break;
			case neogfx::alignment::Centre:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(0, 1, iText);
				iLayout.add_item_at_position(0, 2, iImage);
				iLayout.add_spacer_at_position(0, 3);
				break;
			case neogfx::alignment::Right:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(0, 1, iText);
				iLayout.add_item_at_position(0, 2, iImage);
				break;
			}
			break;
		case label_placement::TextImageVertical:
			switch (iLayout.alignment() & neogfx::alignment::Vertical)
			{
			case neogfx::alignment::Top:
				iLayout.add_item_at_position(0, 0, iText);
				iLayout.add_item_at_position(1, 0, iImage);
				iLayout.add_spacer_at_position(2, 0);
				break;
			case neogfx::alignment::VCentre:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(1, 0, iText);
				iLayout.add_item_at_position(2, 0, iImage);
				iLayout.add_spacer_at_position(3, 0);
				break;
			case neogfx::alignment::Bottom:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(1, 0, iText);
				iLayout.add_item_at_position(2, 0, iImage);
				break;
			}
			break;
		case label_placement::ImageTextHorizontal:
			switch (iLayout.alignment() & neogfx::alignment::Horizontal)
			{
			case neogfx::alignment::Left:
				iLayout.add_item_at_position(0, 0, iImage);
				iLayout.add_item_at_position(0, 1, iText);
				iLayout.add_spacer_at_position(0, 2);
				break;
			case neogfx::alignment::Centre:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(0, 1, iImage);
				iLayout.add_item_at_position(0, 2, iText);
				iLayout.add_spacer_at_position(0, 3);
				break;
			case neogfx::alignment::Right:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(0, 1, iImage);
				iLayout.add_item_at_position(0, 2, iText);
				break;
			}
			break;
		case label_placement::ImageTextVertical:
			switch (iLayout.alignment() & neogfx::alignment::Vertical)
			{
			case neogfx::alignment::Top:
				iLayout.add_item_at_position(0, 0, iImage);
				iLayout.add_item_at_position(1, 0, iText);
				iLayout.add_spacer_at_position(2, 0);
				break;
			case neogfx::alignment::VCentre:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(1, 0, iImage);
				iLayout.add_item_at_position(2, 0, iText);
				iLayout.add_spacer_at_position(3, 0);
				break;
			case neogfx::alignment::Bottom:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(1, 0, iImage);
				iLayout.add_item_at_position(2, 0, iText);
				break;
			}
			break;
		case label_placement::TextSpacerImageHorizontal:
			switch (iLayout.alignment() & neogfx::alignment::Horizontal)
			{
			case neogfx::alignment::Left:
				iLayout.add_item_at_position(0, 0, iText);
				iCentreSpacer = &iLayout.add_spacer_at_position(0, 1);
				iLayout.add_item_at_position(0, 2, iImage);
				iLayout.add_spacer_at_position(0, 3);
				break;
			case neogfx::alignment::Centre:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(0, 1, iText);
				iCentreSpacer = &iLayout.add_spacer_at_position(0, 2);
				iLayout.add_item_at_position(0, 3, iImage);
				iLayout.add_spacer_at_position(0, 3);
				break;
			case neogfx::alignment::Right:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(0, 1, iText);
				iCentreSpacer = &iLayout.add_spacer_at_position(0, 2);
				iLayout.add_item_at_position(0, 3, iImage);
				break;
			}
			break;
		case label_placement::TextSpacerImageVertical:
			switch (iLayout.alignment() & neogfx::alignment::Vertical)
			{
			case neogfx::alignment::Top:
				iLayout.add_item_at_position(0, 0, iText);
				iCentreSpacer = &iLayout.add_spacer_at_position(1, 0);
				iLayout.add_item_at_position(2, 0, iImage);
				iLayout.add_spacer_at_position(3, 0);
				break;
			case neogfx::alignment::VCentre:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(1, 0, iText);
				iCentreSpacer = &iLayout.add_spacer_at_position(2, 0);
				iLayout.add_item_at_position(3, 0, iImage);
				iLayout.add_spacer_at_position(4, 0);
				break;
			case neogfx::alignment::Bottom:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(1, 0, iText);
				iCentreSpacer = &iLayout.add_spacer_at_position(2, 0);
				iLayout.add_item_at_position(3, 0, iImage);
				break;
			}
			break;
		case label_placement::ImageSpacerTextHorizontal:
			switch (iLayout.alignment() & neogfx::alignment::Horizontal)
			{
			case neogfx::alignment::Left:
				iLayout.add_item_at_position(0, 0, iImage);
				iCentreSpacer = &iLayout.add_spacer_at_position(0, 1);
				iLayout.add_item_at_position(0, 2, iText);
				iLayout.add_spacer_at_position(0, 3);
				break;
			case neogfx::alignment::Centre:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(0, 1, iImage);
				iCentreSpacer = &iLayout.add_spacer_at_position(0, 2);
				iLayout.add_item_at_position(0, 3, iText);
				iLayout.add_spacer_at_position(0, 4);
				break;
			case neogfx::alignment::Right:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(0, 1, iImage);
				iCentreSpacer = &iLayout.add_spacer_at_position(0, 2);
				iLayout.add_item_at_position(0, 3, iText);
				break;
			}
			break;
		case label_placement::ImageSpacerTextVertical:
			switch (iLayout.alignment() & neogfx::alignment::Vertical)
			{
			case neogfx::alignment::Top:
				iLayout.add_item_at_position(0, 0, iImage);
				iCentreSpacer = &iLayout.add_spacer_at_position(1, 0);
				iLayout.add_item_at_position(2, 0, iText);
				iLayout.add_spacer_at_position(3, 0);
				break;
			case neogfx::alignment::VCentre:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(1, 0, iImage);
				iCentreSpacer = &iLayout.add_spacer_at_position(2, 0);
				iLayout.add_item_at_position(3, 0, iText);
				iLayout.add_spacer_at_position(4, 0);
				break;
			case neogfx::alignment::Bottom:
				iLayout.add_spacer_at_position(0, 0);
				iLayout.add_item_at_position(1, 0, iImage);
				iCentreSpacer = &iLayout.add_spacer_at_position(2, 0);
				iLayout.add_item_at_position(3, 0, iText);
				break;
			}
			break;
		}
	}
}