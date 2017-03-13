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

#include <neogfx/neogfx.hpp>
#include <boost/optional.hpp>
#include <neolib/string_utils.hpp>
#include <neogfx/core/geometry.hpp>
#include "i_glyph_texture.hpp"
#include "font.hpp"

namespace neogfx
{
	typedef std::string string;
	typedef string::value_type character;

	// case insensitive text
	typedef std::basic_string<character, neolib::ci_char_traits<std::char_traits<character> > > ci_string;

	enum class text_category : uint8_t
	{
		Unknown = 0x00,
		None = 0x01,
		Whitespace = 0x02,
		Digit = 0x04,
		LTR = 0x08,
		RTL = 0x10,
		Mnemonic = 0x20
	};

	enum class text_direction : uint8_t
	{
		Invalid = 0x00,
		LTR = 0x01,
		RTL = 0x02,
		Digits_LTR = 0x04, // unused
		Digits_RTL = 0x08
	};

	class character_type
	{
	public:
		text_category category;
		text_direction direction;
	public:
		character_type() :
			category(text_category::Unknown), direction(text_direction::LTR) {}
		character_type(text_category aCategory, text_direction aDirection) :
			category(aCategory), direction(aDirection) {}
		character_type(text_category aCategory) :
			category(aCategory), direction(text_direction::LTR) {}
	public:
		bool operator==(const character_type& aRhs) const { return category == aRhs.category && direction == aRhs.direction; }
		bool operator!=(const character_type& aRhs) const { return !(*this == aRhs); }
	};

	class glyph
	{
	public:
		enum flags_e : uint8_t
		{
			Underline = 0x01,
			Subpixel = 0x02,
			Mnemonic = 0x04,
			Emoji = 0x08,
			UseFallback = 0x80
		};
	public:
		typedef uint32_t value_type;
		typedef std::pair<uint32_t, uint32_t> source_type;
	public:
		glyph() :
			iType{}, iFlags{}, iFallbackIndex{}, iSource{}, iAdvance{}, iOffset{} {}
		glyph(const character_type& aType, value_type aValue, source_type aSource, size aAdvance, size aOffset) :
			iType{ aType.category, aType.direction }, iValue{ aValue }, iFlags{}, iFallbackIndex{}, iSource{ aSource }, iAdvance{ aAdvance }, iOffset{ aOffset } {}
		glyph(const character_type& aType, value_type aValue) :
			iType{ aType.category, aType.direction }, iValue{ aValue }, iFlags{}, iFallbackIndex{}, iSource{}, iAdvance{}, iOffset{} {}
	public:
		bool operator==(const glyph& aRhs) const { return iType.category == aRhs.iType.category && iValue == aRhs.iValue; }
	public:
		bool is_whitespace() const { return category() == text_category::Whitespace; }
		bool is_digit() const { return category() == text_category::Digit; }
		text_category category() const { return iType.category; }
		text_direction direction() const { return iType.direction; }
	public:
		bool left_to_right() const { return direction() == text_direction::LTR; }
		bool right_to_left() const { return direction() == text_direction::RTL; }
		bool category_has_no_direction() const { return iType.category != text_category::LTR && iType.category != text_category::RTL; }
		void set_category(text_category aCategory) { iType.category = aCategory; }
		void set_direction(text_direction aDirection) { iType.direction = aDirection; }
		value_type value() const { return iValue; }
		void set_value(value_type aValue) { iValue = aValue; }
		const source_type& source() const { return iSource; }
		void set_source(const source_type aSource) { iSource = aSource; }
		size advance(bool aRoundUp = true) const { return aRoundUp ? (iAdvance + basic_size<float>{0.5f, 0.5f}).floor() : iAdvance; }
		void set_advance(const size& aAdvance) { iAdvance = aAdvance; }
		size offset() const { return (iOffset + basic_size<float>{0.5f, 0.5f}).floor(); }
		void set_offset(const size& aOffset) { iOffset = aOffset; }
		flags_e flags() const { return iFlags; }
		void set_flags(flags_e aFlags) { iFlags = aFlags; }
		bool underline() const { return (iFlags & Underline) == Underline; }
		void set_underline(bool aUnderline) { iFlags = static_cast<flags_e>(aUnderline ? iFlags | Underline : iFlags & ~Underline); }
		bool subpixel() const { return (iFlags & Subpixel) == Subpixel; }
		void set_subpixel(bool aSubpixel) { iFlags = static_cast<flags_e>(aSubpixel ? iFlags | Subpixel : iFlags & ~Subpixel); }
		bool mnemonic() const { return (iFlags & Mnemonic) == Mnemonic; }
		void set_mnemonic(bool aMnemonic) { iFlags = static_cast<flags_e>(aMnemonic ? iFlags | Mnemonic : iFlags & ~Mnemonic); }
		bool use_fallback() const { return (iFlags & UseFallback) == UseFallback; }
		void set_use_fallback(bool aUseFallback, uint32_t aFallbackIndex = 0) { iFlags = static_cast<flags_e>(aUseFallback ? iFlags | UseFallback : iFlags & ~UseFallback); iFallbackIndex = static_cast<uint8_t>(aFallbackIndex); }
		font fallback_font(font aFont) const
		{
			if (!use_fallback())
				return aFont;
			else
			{
				font fallbackFont = aFont.fallback();
				for (uint8_t i = 0; i < iFallbackIndex; ++i)
					fallbackFont = fallbackFont.fallback();
				return fallbackFont;
			}
		}
		uint8_t fallback_font_index() const { return iFallbackIndex; }
		void kerning_adjust(float aAdjust) { iAdvance.cx += aAdjust; }
	private:
		character_type iType;
		value_type iValue;
		flags_e iFlags;
		uint8_t iFallbackIndex;
		source_type iSource;
		basic_size<float> iAdvance;
		basic_size<float> iOffset;
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
				result.cx += i->advance().cx;
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

	template <typename Iter>
	inline text_direction glyph_text_direction(Iter aBegin, Iter aEnd)
	{
		text_direction result = text_direction::LTR;
		bool gotOne = false;
		for (Iter i = aBegin; i != aEnd; ++i)
		{
			if (!i->is_whitespace() && !i->category_has_no_direction())
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