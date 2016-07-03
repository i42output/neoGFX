// i_native_font_face.hpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/core/geometry.hpp>
#include <neogfx/gfx/text/font.hpp>

namespace neogfx
{
	class i_native_font;

	class glyph;
	class i_glyph_texture;

	class i_native_font_face
	{
	public:
		virtual ~i_native_font_face() {}
	public:
		virtual i_native_font& native_font() = 0;
		virtual const std::string& family_name() const = 0;
		virtual font::style_e style() const = 0;
		virtual const std::string& style_name() const = 0;
		virtual font::point_size size() const = 0;
		virtual dimension horizontal_dpi() const = 0;
		virtual dimension vertical_dpi() const = 0;
		virtual dimension height() const = 0;
		virtual dimension descender() const = 0;
		virtual dimension underline_position() const = 0;
		virtual dimension underline_thickness() const = 0;
		virtual dimension line_spacing() const = 0;
		virtual dimension kerning(uint32_t aLeftGlyphIndex, uint32_t aRightGlyphIndex) const = 0;
		virtual i_native_font_face& fallback() const = 0;
		virtual void* handle() const = 0;
		virtual void* aux_handle() const = 0;
		virtual uint32_t glyph_index(char32_t aCodePoint) const = 0;
		virtual i_glyph_texture& glyph_texture(const glyph& aGlyph) const = 0;
	};
}