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
	public:
		text_edit();
		text_edit(i_widget& aParent);
		text_edit(i_layout& aLayout);
		~text_edit();
	public:
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
	public:
		virtual void move_cursor(cursor::move_operation_e aMoveOperation) const;
	public:
		neogfx::cursor& cursor() const;
		std::string text() const;
		void set_text(const std::string& aText);
		void insert_text(const std::string& aText);
	private:
		void init();
	private:
		mutable neogfx::cursor iCursor;
		neolib::segmented_array<char> iDocument;
		neolib::segmented_array<glyph> iGlyphs;
	};
}