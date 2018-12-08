// glyph.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <optional>
#include <neolib/allocator.hpp>
#include <neolib/vecarray.hpp>
#include <neolib/string_utils.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/text/i_glyph_texture.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>
#include <neogfx/gfx/text/font.hpp>

namespace neogfx
{
	typedef std::string string;
	typedef string::value_type character;

	// case insensitive text
	typedef std::basic_string<character, neolib::ci_char_traits<std::char_traits<character> > > ci_string;

	enum class text_category : uint8_t
	{
		Unknown		= 0x00,
		None		= 0x01,
		Whitespace	= 0x02,
		Digit		= 0x03,
		LTR			= 0x04,
		RTL			= 0x05,
		Mark		= 0x06,
		Mnemonic	= 0x07,
		Emoji		= 0x08,
		Control		= 0x09
	};

	enum class text_direction : uint8_t
	{
		Invalid =		0x00,
		LTR =			0x01,
		RTL =			0x02,
		None_LTR =		0x04 | 0x01,
		None_RTL =		0x08 | 0x02,
		Digit_LTR =		0x10 | 0x01, // unused
		Digit_RTL =		0x20 | 0x02,
		Emoji_LTR =		0x40 | 0x01,
		Emoji_RTL =		0x80 | 0x02,
		BaseMask =		0x01 | 0x02
	};

	inline text_direction base_text_direction(text_direction aDirection)
	{
		return static_cast<text_direction>(static_cast<uint8_t>(aDirection) & static_cast<uint8_t>(text_direction::BaseMask));
	}

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

	class glyph;

	class i_glyph_font_cache
	{
	public:
		virtual const font& glyph_font(const glyph& aGlyph) const = 0;
		virtual void cache_glyph_font(font_id aFontId) = 0;
		virtual void cache_glyph_font(const font& aFont) = 0;
	};

	class glyph
	{
	public:
		struct no_font_specified : std::logic_error { no_font_specified() : std::logic_error("neogfx::glyph::no_font_specified") {} };
	public:
		enum flags_e : uint8_t
		{
			Underline = 0x01,
			Subpixel = 0x02,
			Mnemonic = 0x04
		};
	public:
		typedef uint32_t value_type;
		typedef std::pair<uint32_t, uint32_t> source_type;
	public:
		glyph() :
			iType{},
			iValue{},
			iFlags{},
			iSource{}, 
			iFont{},
			iAdvance{}, 
			iOffset{}
		{
		}
		glyph(const character_type& aType, value_type aValue, source_type aSource, i_glyph_font_cache& aFontCache, neogfx::font& aFont, size aAdvance, size aOffset) :
			iType{ aType.category, aType.direction }, 
			iValue{ aValue }, 
			iFlags{}, 
			iSource{ aSource }, 
			iFont{ aFont.id() },
			iAdvance{ aAdvance },
			iOffset{ aOffset }
		{
			aFontCache.cache_glyph_font(aFont);
		}
		glyph(const character_type& aType, value_type aValue) :
			iType{ aType.category, aType.direction }, 
			iValue{ aValue }, 
			iFlags{}, 
			iSource{}, 
			iFont{},
			iAdvance{}, 
			iOffset{}
		{
		}
		glyph(const glyph& aOther) :
			iType{ aOther.iType },
			iValue{ aOther.iValue },
			iFlags{ aOther.iFlags },
			iSource{ aOther.iSource },
			iFont{ aOther.iFont },
			iAdvance{ aOther.iAdvance },
			iOffset{ aOther.iOffset }
		{
		}
	public:
		bool operator==(const glyph& aRhs) const 
		{ 
			return iType.category == aRhs.iType.category && iValue == aRhs.iValue; 
		}
	public:
		bool has_font() const
		{
			return font_specified();
		}
		bool has_font_glyph() const
		{
			return has_font() && !is_whitespace() && !is_emoji();
		}
		bool is_whitespace() const 
		{ 
			return category() == text_category::Whitespace; 
		};
		bool is_non_line_breaking_whitespace() const 
		{ 
			return is_whitespace() && value() != U'\n'; 
		}
		bool is_line_breaking_whitespace() const 
		{ 
			return is_whitespace() && value() == U'\n'; 
		}
		bool is_digit() const 
		{ 
			return category() == text_category::Digit; 
		}
		bool is_emoji() const 
		{ 
			return category() == text_category::Emoji; 
		}
		text_category category() const 
		{ 
			return iType.category; 
		}
		text_direction direction() const 
		{ 
			return iType.direction; 
		}
	public:
		bool left_to_right() const 
		{ 
			return direction() == text_direction::LTR; 
		}
		bool right_to_left() const 
		{ 
			return direction() == text_direction::RTL; 
		}
		bool category_has_no_direction() const 
		{ 
			return iType.category != text_category::LTR && iType.category != text_category::RTL; 
		}
		void set_category(text_category aCategory) 
		{ 
			iType.category = aCategory; 
		}
		void set_direction(text_direction aDirection) 
		{ 
			iType.direction = aDirection; 
		}
		value_type value() const 
		{ 
			return iValue; 
		}
		void set_value(value_type aValue) 
		{ 
			iValue = aValue; 
		}
		const source_type& source() const 
		{ 
			return iSource; 
		}
		void set_source(const source_type aSource) 
		{ 
			iSource = aSource; 
		}
		bool font_specified() const
		{
			return font_id() != neogfx::font_id{};
		}
		neogfx::font_id font_id() const
		{
			return iFont;
		}
		neogfx::font font() const;
		const neogfx::font& font(const i_glyph_font_cache& aFontCache) const
		{
			if (font_specified())
				return aFontCache.glyph_font(*this);
			throw no_font_specified();
		}
		void set_font(neogfx::font_id aFont)
		{
			iFont = aFont;
		}
		void set_font(const neogfx::font& aFont, i_glyph_font_cache& aFontCache)
		{
			set_font(aFont.id());
			aFontCache.cache_glyph_font(aFont);
		}
		size advance(bool aRoundUp = true) const 
		{ 
			return aRoundUp ? (iAdvance + basic_size<float>{0.5f, 0.5f}).floor() : iAdvance; 
		}
		void set_advance(const size& aAdvance) 
		{ 
			iAdvance = aAdvance; 
		}
		size offset() const 
		{ 
			return (iOffset + basic_size<float>{0.5f, 0.5f}).floor(); 
		}
		void set_offset(const size& aOffset) 
		{ 
			iOffset = aOffset; 
		}
		size extents(const i_glyph_font_cache& aFontCache) const
		{
			return extents(font(aFontCache));
		}
		size extents(const neogfx::font& aFont) const
		{
			if (iExtents == basic_size<float>{})
			{
				iExtents = size{ advance().cx, !is_emoji() ? aFont.height() : advance().cx };
				try
				{
					if (has_font_glyph())
						iExtents.cx = static_cast<float>(offset().cx + glyph_texture(aFont).placement().x + glyph_texture(aFont).texture().extents().cx);
				}
				catch (...)
				{
					// silently ignore freetype exceptions.
				}
			}
			return iExtents;
		}
		void set_extents(size& aExtents)
		{
			iExtents = aExtents;
		}
		flags_e flags() const
		{ 
			return iFlags; 
		}
		void set_flags(flags_e aFlags) 
		{ 
			iFlags = aFlags; 
		}
		bool underline() const 
		{ 
			return (iFlags & Underline) == Underline; 
		}
		void set_underline(bool aUnderline) 
		{ 
			iFlags = static_cast<flags_e>(aUnderline ? iFlags | Underline : iFlags & ~Underline); 
		}
		bool subpixel() const 
		{ 
			return (iFlags & Subpixel) == Subpixel; 
		}
		void set_subpixel(bool aSubpixel) 
		{ 
			iFlags = static_cast<flags_e>(aSubpixel ? iFlags | Subpixel : iFlags & ~Subpixel); 
		}
		bool mnemonic() const 
		{ 
			return (iFlags & Mnemonic) == Mnemonic; 
		}
		void set_mnemonic(bool aMnemonic) 
		{ 
			iFlags = static_cast<flags_e>(aMnemonic ? iFlags | Mnemonic : iFlags & ~Mnemonic); 
		}
		void kerning_adjust(float aAdjust) 
		{ 
			iAdvance.cx += aAdjust; 
		}
		const i_glyph_texture& glyph_texture(const i_glyph_font_cache& aFontCache) const
		{
			return glyph_texture(font(aFontCache));
		}
		const i_glyph_texture& glyph_texture(const neogfx::font& aFont) const
		{
			return aFont.glyph_texture(*this);
		}
	private:
		character_type iType;
		value_type iValue;
		flags_e iFlags;
		source_type iSource;
		neogfx::font_id iFont;
		basic_size<float> iAdvance;
		basic_size<float> iOffset;
		mutable basic_size<float> iExtents;
	};

	constexpr std::size_t SMALL_OPTIMIZATION_GLYPH_TEXT_GLYPH_COUNT = 16;

	class glyph_font_cache : public i_glyph_font_cache
	{
	private:
		typedef std::pair<neolib::small_cookie_auto_ref, font> cache_entry;
		typedef std::unordered_map<font_id, cache_entry, std::hash<font_id>, std::equal_to<font_id>, neolib::fast_pool_allocator<std::pair<const font_id, cache_entry>>> font_cache;
	public:
		struct cached_font_not_found : std::logic_error { cached_font_not_found() : std::logic_error("neogfx::glyph_font_cache::cached_font_not_found") {} };
	public:
		const font& glyph_font(const glyph& aGlyph) const override;
		void cache_glyph_font(font_id aFontId) override;
		void cache_glyph_font(const font& aFont) override;
	public:
		void clear();
	private:
		const font_cache& cache() const;
		font_cache& cache();
	private:
		font_cache iCache;
	};

	class glyph_text : public glyph_font_cache, private neolib::vecarray<glyph, SMALL_OPTIMIZATION_GLYPH_TEXT_GLYPH_COUNT, -1>
	{
	private:
		static constexpr std::size_t SMALL_OPTIMIZATION_FONT_COUNT = 4;
		typedef neolib::vecarray<glyph, SMALL_OPTIMIZATION_GLYPH_TEXT_GLYPH_COUNT, -1> container;
	public:
		using container::const_iterator;
	public:
		glyph_text();
		template <typename Iter>
		glyph_text(Iter aBegin, Iter aEnd) :
			container{ aBegin, aEnd }
		{
		}
		glyph_text(const glyph_text& aOther);
		glyph_text(glyph_text&& aOther);
	public:
		glyph_text& operator=(const glyph_text& aOther);
		glyph_text& operator=(glyph_text&& aOther);
	public:
		using container::cbegin;
		using container::cend;
		using container::empty;
		using container::begin;
		using container::end;
		iterator begin();
		iterator end();
	public:
		using container::back;
		reference back();
	public:
		template< class... Args >
		reference emplace_back(Args&&... args)
		{
			auto& result  = container::emplace_back(std::forward<Args>(args)...);
			iExtents = std::nullopt;
			return result;
		}
		void push_back(const glyph& aGlyph);
		void clear();
	public:
		bool operator==(const glyph_text& aOther) const;
	public:
		neogfx::size extents(const_iterator aBegin, const_iterator aEnd, bool aEndIsLineEnd = true) const;
	public:
		const neogfx::size& extents() const;
		std::pair<const_iterator, const_iterator> word_break(const_iterator aBegin, const_iterator aFrom) const;
	private:
		mutable std::optional<neogfx::size> iExtents;
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

	typedef std::optional<glyph_text> optional_glyph_text;
}