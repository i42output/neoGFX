// text_edit.cpp
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
#include "text_edit.hpp"

namespace neogfx
{
	text_edit::text_edit() : 
		scrollable_widget()
	{
		init();
	}

	text_edit::text_edit(i_widget& aParent) :
		scrollable_widget(aParent)
	{
		init();
	}

	text_edit::text_edit(i_layout& aLayout) :
		scrollable_widget(aLayout)
	{
		init();
	}

	text_edit::~text_edit()
	{
	}

	size text_edit::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return scrollable_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		auto result = scrollable_widget::minimum_size() + size{ font().height() } + margins().size();
		return convert_units(*this, su.saved_units(), result);
	}

	void text_edit::move_cursor(cursor::move_operation_e aMoveOperation) const
	{
		switch (aMoveOperation)
		{
		case cursor::StartOfDocument:
			break;
		case cursor::StartOfParagraph:
			break;
		case cursor::StartOfLine:
			break;
		case cursor::StartOfWord:
			break;
		case cursor::EndOfDocument:
			break;
		case cursor::EndOfParagraph:
			break;
		case cursor::EndOfLine:
			break;
		case cursor::EndOfWord:
			break;
		case cursor::PreviousParagraph:
			break;
		case cursor::PreviousLine:
			break;
		case cursor::PreviousWord:
			break;
		case cursor::PreviousCharacter:
			break;
		case cursor::NextParagraph:
			break;
		case cursor::NextLine:
			break;
		case cursor::NextWord:
			break;
		case cursor::NextCharacter:
			break;
		case cursor::Up:
			break;
		case cursor::Down:
			break;
		case cursor::Left:
			break;
		case cursor::Right:
			break;
		default:
			break;
		}
	}

	neogfx::cursor& text_edit::cursor() const
	{
		return iCursor;
	}

	std::string text_edit::text() const
	{
		return std::string(iDocument.begin(), iDocument.end());
	}

	void text_edit::set_text(const std::string& aText)
	{
		iCursor.set_position(0);
		iDocument.clear();
		iGlyphs.clear();
		insert_text(aText);
	}

	void text_edit::insert_text(const std::string& aText)
	{
		iDocument.insert(iDocument.begin(), aText.begin(), aText.end());
		graphics_context gc(*this);
		auto gt = gc.to_glyph_text(aText, font());
		iGlyphs.insert(iGlyphs.begin(), gt.cbegin(), gt.cend());
		update();
	}

	void text_edit::init()
	{
		iCursor.position_changed([this]()
		{
			update();
		}, this);
		iCursor.anchor_changed([this]()
		{
			update();
		}, this);
	}
}