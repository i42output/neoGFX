// text_edit.hpp
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

#pragma once

#include "neogfx.hpp"
#include <neolib/segmented_array.hpp>
#include "scrollable_widget.hpp"
#include "i_document.hpp"
#include "glyph.hpp"
#include "cursor.hpp"

namespace neogfx
{
	class text_edit : public scrollable_widget, public i_document
	{
	private:
		typedef neolib::segmented_array<char, 256> document_text;
		typedef neolib::segmented_array<glyph, 256> document_glyphs;
	public:
		typedef document_text::size_type position_type;
	public:
		text_edit();
		text_edit(i_widget& aParent);
		text_edit(i_layout& aLayout);
		~text_edit();
	public:
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool text_input(const std::string& aText);
	public:
		virtual void move_cursor(cursor::move_operation_e aMoveOperation) const;
	public:
		neogfx::alignment alignment() const;
		void set_alignment(neogfx::alignment aAlignment);
		bool has_text_colour() const;
		colour text_colour() const;
		void set_text_colour(const optional_colour& aTextColour);
	public:
		neogfx::cursor& cursor() const;
		point position(position_type aPosition) const;
		position_type hit_test(const point& aPoint) const;
		std::string text() const;
		void set_text(const std::string& aText);
		void insert_text(const std::string& aText);
	private:
		void init();
		void refresh_paragraph(document_text::const_iterator aWhere);
		size extents(const neogfx::font& aFont, document_glyphs::const_iterator aBegin, document_glyphs::const_iterator aEnd) const;
		std::pair<document_glyphs::const_iterator, document_glyphs::const_iterator> word_break(document_glyphs::const_iterator aBegin, document_glyphs::const_iterator aFrom) const;
	private:
		neogfx::alignment iAlignment;
		optional_colour iTextColour;
		mutable neogfx::cursor iCursor;
		document_text iText;
		document_glyphs iGlyphs;
	};
}