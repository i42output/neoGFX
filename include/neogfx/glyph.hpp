// glyph.hpp
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
#include <boost/optional.hpp>
#include <neolib/string_utils.hpp>
#include "geometry.hpp"
#include "i_font_texture.hpp"
#include "font.hpp"

namespace neogfx
{
	typedef std::string string;
	typedef string::value_type character;

	// case insensitive text
	typedef std::basic_string<character, neolib::ci_char_traits<std::char_traits<character> > > ci_string;

	enum class text_direction
	{
		Unknown,
		None,
		Whitespace,
		LTR,
		RTL
	};

	class glyph
	{
	public:
		typedef uint32_t value_type;
		typedef std::pair<string::size_type, string::size_type> source_type;
	public:
		glyph(text_direction aDirection, value_type aValue, source_type aSource, size aExtents, size aOffset) :
			iDirection(aDirection), iValue(aValue), iUseFallback(false), iSource(aSource), iExtents(aExtents), iOffset(aOffset) {}
		glyph(text_direction aDirection, value_type aValue) :
			iDirection(aDirection), iValue(aValue), iUseFallback(), iSource(), iExtents(), iOffset() {}
	public:
		bool operator==(const glyph& aRhs) const { return iDirection == aRhs.iDirection && iValue == aRhs.iValue; }
	public:
		bool is_whitespace() const { return iDirection == text_direction::Whitespace; }
		text_direction direction() const { return iDirection; }
		bool no_direction() const { return iDirection != text_direction::LTR && iDirection != text_direction::RTL; }
		bool left_to_right() const { return iDirection == text_direction::LTR; }
		bool right_to_left() const { return iDirection == text_direction::RTL; }
		void set_direction(text_direction aDirection) { iDirection = aDirection; }
		value_type value() const { return iValue; }
		void set_value(value_type aValue) { iValue = aValue; }
		const source_type& source() const { return iSource; }
		void set_source(const source_type aSource) { iSource = aSource; }
		size extents() const { return iExtents; }
		void set_extents(const size& aExtents) { iExtents = aExtents; }
		size offset() const { return iOffset; }
		void set_offset(const size& aOffset) { iOffset = aOffset; }
		bool use_fallback() const { return iUseFallback; }
		void set_use_fallback(bool aUseFallback) { iUseFallback = aUseFallback; }
	private:
		text_direction iDirection;
		value_type iValue;
		bool iUseFallback;
		source_type iSource;
		size iExtents;
		size iOffset;
	};

	class glyph_text : private std::vector<glyph>
	{
	public:
		typedef std::vector<glyph> container;
		using container::const_iterator;
	public:
		glyph_text(const font& aFont) : 
			iFont(aFont)
		{
		}
		template <typename Iter>
		glyph_text(const font& aFont, Iter aBegin, Iter aEnd) : 
			container(aBegin, aEnd),
			iFont(aFont),
			iExtents(extents(iFont, begin(), end()))
		{
		}
		glyph_text(const font& aFont, container&& aGlyphs) :
			container(aGlyphs),
			iFont(aFont),
			iExtents(extents(iFont, begin(), end()))
		{
		}
	public:
		using container::cbegin;
		using container::cend;
		using container::empty;
	public:
		bool operator==(const glyph_text& aOther) const
		{
			return font() == aOther.font() && static_cast<const container&>(*this) == static_cast<const container&>(aOther);
		}
	public:
		static neogfx::size extents(const font& aFont, const_iterator aBegin, const_iterator aEnd)
		{
			neogfx::size result;
			bool usingNormal = false;
			bool usingFallback = false;
			for (glyph_text::const_iterator i = aBegin; i != aEnd; ++i)
			{
				result.cx += i->extents().cx;
				if (!i->use_fallback())
					usingNormal = true;
				else
					usingFallback = true;
			}
			if (usingNormal || !usingFallback)
				result.cy = aFont.height();
			if (usingFallback)
				result.cy = std::max(result.cy, aFont.fallback().height());
			return neogfx::size(std::ceil(result.cx), std::ceil(result.cy));
		}
	public:
		const neogfx::font& font() const
		{
			return iFont;
		}
		const neogfx::size& extents() const
		{
			return iExtents;
		}
		std::pair<const_iterator, const_iterator> word_break(const_iterator aBegin, const_iterator aFrom) const
		{
			std::pair<const_iterator, const_iterator> result(aFrom, aFrom);
			if (!aFrom->is_whitespace())
			{
				while(result.first != aBegin && !result.first->is_whitespace())
					--result.first;
				if (!result.first->is_whitespace())
				{
					result.first = aFrom;
					while(result.first != aBegin && (result.first - 1)->source() == aFrom->source())
						--result.first;
					result.second = result.first;
					return result;
				}
				result.second = result.first;
			}
			while(result.first != aBegin && (result.first - 1)->is_whitespace())
				--result.first;
			while(result.second->is_whitespace() && result.second != end())
				++result.second;
			return result;
		}
	private:
		neogfx::font iFont;
		neogfx::size iExtents;
	};

	inline text_direction glyph_text_direction(glyph_text::const_iterator aBegin, glyph_text::const_iterator aEnd)
	{
		text_direction result = text_direction::LTR;
		bool gotOne = false;
		for (glyph_text::const_iterator i = aBegin; i != aEnd; ++i)
		{
			if (!i->is_whitespace() && !i->no_direction())
			{
				if (!gotOne)
				{
					gotOne = true;
					result = i->direction();
				}
				else
				{
					if (result != i->direction())
						result = text_direction::LTR;
				}
			}
		}
		return result;
	}

	typedef boost::optional<glyph_text> optional_glyph_text;
}