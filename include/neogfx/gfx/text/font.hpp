// font.hpp
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
#include <neolib/variant.hpp>
#include <neogfx/core/geometrical.hpp>

namespace neogfx
{
	class i_native_font_face;
	class glyph;
	class i_glyph_texture;

	class font_info
	{
		// exceptions
	public:
		struct unknown_style : std::logic_error { unknown_style() : std::logic_error("neogfx::font_info::unknown_style") {} };
		struct unknown_style_name : std::logic_error { unknown_style_name() : std::logic_error("neogfx::font_info::unknown_style_name") {} };
		// types
	public:
		enum style_e
		{
			Invalid = 0x00,
			Normal = 0x01,
			Bold = 0x02,
			Italic = 0x04,
			Underline = 0x08,
			BoldItalic = Bold | Italic,
			BoldItalicUnderline = Bold | Italic | Underline,
			BoldUnderline = Bold | Underline,
			ItalicUnderline = Italic | Underline
		};
		enum weight_e
		{
			WeightThin			= 100,
			WeightExtralight	= 200,
			WeightUltralight	= 200,
			WeightLight			= 300,
			WeightNormal		= 400,
			WeightRegular		= 400,
			WeightMedium		= 500,
			WeightSemibold		= 600,
			WeightDemibold		= 600,
			WeightBold			= 700,
			WeightExtrabold		= 800,
			WeightUltrabold		= 800,
			WeightHeavy			= 900,
			WeightBlack			= 900
		};
		typedef double point_size;
	private:
		typedef std::optional<style_e> optional_style;
		typedef std::optional<std::string> optional_style_name;
	private:
		class instance;
	public:
		font_info();
		font_info(const std::string& aFamilyName, style_e aStyle, point_size aSize);
		font_info(const std::string& aFamilyName, const std::string& aStyleName, point_size aSize);
		font_info(const std::string& aFamilyName, style_e aStyle, const std::string& aStyleName, point_size aSize);
		font_info(const font_info& aOther);
		virtual ~font_info();
		font_info& operator=(const font_info& aOther);
	private:
		font_info(const std::string& aFamilyName, const optional_style& aStyle, const optional_style_name& aStyleName, point_size aSize);
	public:
		virtual const std::string& family_name() const;
		virtual bool style_available() const;
		virtual style_e style() const;
		virtual bool style_name_available() const;
		virtual const std::string& style_name() const;
		virtual bool underline() const;
		virtual void set_underline(bool aUnderline);
		virtual weight_e weight() const;
		virtual point_size size() const;
		virtual bool kerning() const;
		virtual void enable_kerning();
		virtual void disable_kerning();
	public:
		font_info with_style(style_e aStyle) const;
		font_info with_size(point_size aSize) const;
	public:
		bool operator==(const font_info& aRhs) const;
		bool operator!=(const font_info& aRhs) const;
		bool operator<(const font_info& aRhs) const;
	public:
		static weight_e weight_from_style(font_info::style_e aStyle);
		static weight_e weight_from_style_name(std::string aStyleName);
	private:
		mutable std::shared_ptr<instance> iInstance;
	};

	class font : public font_info
	{
		friend class graphics_context;
		// exceptions
	public:
		struct no_fallback_font : std::logic_error { no_fallback_font() : std::logic_error("neogfx::font::no_fallback_font") {} };
		// types
	public:
		typedef uint16_t token;
		class scoped_token
		{
		public:
			scoped_token();
			scoped_token(token aToken);
			scoped_token(const font& aFont);
			scoped_token(const scoped_token& aOther);
			~scoped_token();
		public:
			scoped_token& operator=(const scoped_token& aOther);
		public:
			token operator*() const
			{
				return get();
			}
			bool operator==(const scoped_token& aOther) const
			{
				return get() == aOther.get();
			}
		public:
			token get() const
			{
				return iToken;
			}
		private:
			void add_ref();
			void release();
		private:
			token iToken;
		};
	private:
		class instance;
		// construction
	public:
		font();
		font(const std::string& aFamilyName, style_e aStyle, point_size aSize);
		font(const std::string& aFamilyName, const std::string& aStyleName, point_size aSize);
		font(const font_info& aFontInfo);
		font(const font& aOther);
		font(const font& aOther, style_e aStyle, point_size aSize);
		font(const font& aOther, const std::string& aStyleName, point_size aSize);
		static font load_from_file(const std::string& aFileName);
		static font load_from_file(const std::string& aFileName, style_e aStyle, point_size aSize);
		static font load_from_file(const std::string& aFileName, const std::string& aStyleName, point_size aSize);
		static font load_from_memory(const void* aData, std::size_t aSizeInBytes);
		static font load_from_memory(const void* aData, std::size_t aSizeInBytes, style_e aStyle, point_size aSize);
		static font load_from_memory(const void* aData, std::size_t aSizeInBytes, const std::string& aStyleName, point_size aSize);
		~font();
		font& operator=(const font& aOther);
	private:
		font(std::unique_ptr<i_native_font_face> aNativeFontFace);
		font(std::unique_ptr<i_native_font_face> aNativeFontFace, style_e aStyle);
	public:
		bool has_fallback() const;
		font fallback() const;
		// operations
	public:
		const std::string& family_name() const override;
		style_e style() const override;
		const std::string& style_name() const override;
		point_size size() const override;
		dimension height() const;
		dimension descender() const;
		dimension line_spacing() const;
		using font_info::kerning;
		dimension kerning(uint32_t aLeftGlyphIndex, uint32_t aRightGlyphIndex) const;
		bool is_bitmap_font() const;
		uint32_t num_fixed_sizes() const;
		point_size fixed_size(uint32_t aFixedSizeIndex) const;
	public:
		token get_token() const;
		void return_token(token aToken) const;
		static const font& from_token(token aToken);
	public:
		const i_glyph_texture& glyph_texture(const glyph& aGlyph) const;
	public:
		bool operator==(const font& aRhs) const;
		bool operator!=(const font& aRhs) const;
		bool operator<(const font& aRhs) const;
	public:
		i_native_font_face& native_font_face() const;
		// attributes
	private:
		mutable std::shared_ptr<instance> iInstance;
	};

	typedef std::optional<font> optional_font;
}