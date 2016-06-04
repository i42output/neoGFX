// font.hpp
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
#include <neolib/variant.hpp>
#include "geometry.hpp"

namespace neogfx
{
	class i_native_font_face;

	class font_info
	{
		// exceptions
	public:
		struct unknown_style : std::logic_error { unknown_style() : std::logic_error("neogfx::font_info::unknown_style") {} };
		struct unknown_style_name : std::logic_error { unknown_style_name() : std::logic_error("neogfx::font_info::unknown_style_name") {} };
		struct password_not_set : std::logic_error { password_not_set() : std::logic_error("neogfx::font_info::password_not_set") {} };
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
		typedef boost::optional<std::string> optional_password;
	private:
		typedef boost::optional<style_e> optional_style;
		typedef boost::optional<std::string> optional_style_name;
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
		virtual bool password() const;
		virtual const std::string& password_mask() const;
		virtual void set_password(bool aPassword, const std::string& aMask = "\xE2\x97\x8F");
		virtual weight_e weight() const;
		virtual point_size size() const;
	public:
		font_info with_size(point_size aSize) const;
	public:
		bool operator==(const font_info& aRhs) const;
		bool operator!=(const font_info& aRhs) const;
		bool operator<(const font_info& aRhs) const;
	public:
		static weight_e weight_from_style(font_info::style_e aStyle);
		static weight_e weight_from_style_name(std::string aStyleName);
	private:
		std::string iFamilyName;
		optional_style iStyle;
		optional_style_name iStyleName;
		bool iUnderline;
		mutable optional_password iPassword;
		weight_e iWeight;
		point_size iSize;
	};

	class font : public font_info
	{
		friend class graphics_context;
		// construction
	public:
		font();
		font(const std::string& aFamilyName, style_e aStyle, point_size aSize);
		font(const std::string& aFamilyName, const std::string& aStyleName, point_size aSize);
		font(const font_info& aFontInfo);
		font(const font& aOther);
		font(const font& aOther, style_e aStyle, point_size aSize);
		font(const font& aOther, const std::string& aStyleName, point_size aSize);
		static font load_from_file(const std::string aFileName);
		static font load_from_file(const std::string aFileName, style_e aStyle, point_size aSize);
		static font load_from_file(const std::string aFileName, const std::string& aStyleName, point_size aSize);
		static font load_from_memory(const void* aData, std::size_t aSizeInBytes);
		static font load_from_memory(const void* aData, std::size_t aSizeInBytes, style_e aStyle, point_size aSize);
		static font load_from_memory(const void* aData, std::size_t aSizeInBytes, const std::string& aStyleName, point_size aSize);
		virtual ~font();
		font& operator=(const font& aOther);
	private:
		font(std::unique_ptr<i_native_font_face> aNativeFontFace);
		font(std::unique_ptr<i_native_font_face> aNativeFontFace, style_e aStyle);
	public:
		font fallback() const;
		// operations
	public:
		virtual const std::string& family_name() const;
		virtual style_e style() const;
		virtual const std::string& style_name() const;
		virtual point_size size() const;
		dimension height() const;
		dimension descender() const;
		dimension line_spacing() const;
		dimension kerning(uint32_t aLeftGlyphIndex, uint32_t aRightGlyphIndex) const;
	public:
		i_native_font_face& native_font_face() const;
	private:
		
		// attributes
	private:
		std::shared_ptr<i_native_font_face> iNativeFontFace;
	};

	typedef boost::optional<font> optional_font;
}