// font.cpp
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
#include "app.hpp"
#include "font.hpp"
#include "i_native_font.hpp"

namespace neogfx
{
	font_info::font_info() :
		iSize{}
	{
	}

	font_info::font_info(const std::string& aFamilyName, style_e aStyle, point_size aSize) :
		iFamilyName{aFamilyName}, iStyle{aStyle}, iSize{aSize}
	{
	}

	font_info::font_info(const std::string& aFamilyName, const std::string& aStyleName, point_size aSize) :
		iFamilyName{aFamilyName}, iStyleName{aStyleName}, iSize{aSize}
	{

	}

	font_info::font_info(const std::string& aFamilyName, style_e aStyle, const std::string& aStyleName, point_size aSize) :
		iFamilyName{aFamilyName}, iStyle{aStyle}, iStyleName{aStyleName}, iSize{aSize}
	{

	}

	font_info::font_info(const font_info& aOther) :
		iFamilyName{aOther.iFamilyName}, iStyle{aOther.iStyle}, iStyleName{aOther.iStyleName}, iSize{aOther.iSize}
	{
	}

	font_info::~font_info()
	{
	}

	font_info& font_info::operator=(const font_info& aOther)
	{
		iFamilyName = aOther.iFamilyName;
		iStyle = aOther.iStyle;
		iStyleName = aOther.iStyleName;
		iSize = aOther.iSize;
		return *this;
	}

	const std::string& font_info::family_name() const
	{
		return iFamilyName;
	}

	bool font_info::style_available() const
	{
		return iStyle != boost::none;
	}

	font_info::style_e font_info::style() const
	{
		if (style_available())
			return *iStyle;
		else
			throw unknown_style();
	}

	bool font_info::style_name_available() const
	{
		return iStyleName != boost::none;
	}

	const std::string& font_info::style_name() const
	{
		if (style_name_available())
			return *iStyleName;
		else
			throw unknown_style_name();
	}

	font::point_size font_info::size() const
	{
		return iSize;
	}

	font_info font_info::with_size(point_size aSize) const
	{
		return font_info(iFamilyName, iStyle, iStyleName, aSize);
	}

	bool font_info::operator==(const font_info& aRhs) const
	{
		return iFamilyName == aRhs.iFamilyName && iStyle == aRhs.iStyle && iSize == aRhs.iSize;
	}

	font_info::font_info(const std::string& aFamilyName, const optional_style& aStyle, const optional_style_name& aStyleName, point_size aSize) :
		iFamilyName{aFamilyName}, iStyle{aStyle}, iStyleName{aStyleName}, iSize{aSize}
	{
	}

	bool font_info::operator!=(const font_info& aRhs) const
	{
		return !operator==(aRhs);
	}

	bool font_info::operator<(const font_info& aRhs) const
	{
		return std::tie(iFamilyName, iStyle, iSize) < std::tie(aRhs.iFamilyName, aRhs.iStyle, aRhs.iSize);
	}

	font::font() :
		font_info(app::instance().current_style().font_info()), iNativeFontFace(app::instance().current_style().font().iNativeFontFace)
	{
	}

	font::font(const std::string& aFamilyName, style_e aStyle, point_size aSize) :
		font_info(aFamilyName, aStyle, aSize), iNativeFontFace(app::instance().rendering_engine().font_manager().create_font(aFamilyName, aStyle, aSize, app::instance().rendering_engine().screen_metrics()))
	{
	}

	font::font(const std::string& aFamilyName, const std::string& aStyleName, point_size aSize) :
		font_info(aFamilyName, aStyleName, aSize), iNativeFontFace(app::instance().rendering_engine().font_manager().create_font(aFamilyName, aStyleName, aSize, app::instance().rendering_engine().screen_metrics()))
	{
	}

	font::font(const font_info& aFontInfo) :
		font_info(aFontInfo), iNativeFontFace(app::instance().rendering_engine().font_manager().create_font(static_cast<font_info>(*this), app::instance().rendering_engine().screen_metrics()))
	{
	}

	font::font(const font& aOther) :
		font_info(aOther), iNativeFontFace(aOther.iNativeFontFace)
	{
	}
	
	font::font(const font& aOther, style_e aStyle, point_size aSize) :
		font_info(aOther.native_font_face().family_name(), aStyle, aSize), iNativeFontFace(app::instance().rendering_engine().font_manager().create_font(aOther.iNativeFontFace->native_font(), aStyle, aSize, app::instance().rendering_engine().screen_metrics()))
	{
	}

	font::font(const font& aOther, const std::string& aStyleName, point_size aSize) :
		font_info(aOther.native_font_face().family_name(), aStyleName, aSize), iNativeFontFace(app::instance().rendering_engine().font_manager().create_font(aOther.iNativeFontFace->native_font(), aStyleName, aSize, app::instance().rendering_engine().screen_metrics()))
	{
	}

	font::font(std::unique_ptr<i_native_font_face> aNativeFontFace) :
		font_info(aNativeFontFace->family_name(), aNativeFontFace->style_name(), aNativeFontFace->size()), iNativeFontFace(std::move(aNativeFontFace))
	{
	}

	font::font(std::unique_ptr<i_native_font_face> aNativeFontFace, style_e aStyle) :
		font_info(aNativeFontFace->family_name(), aStyle, aNativeFontFace->style_name(), aNativeFontFace->size()), iNativeFontFace(std::move(aNativeFontFace))
	{
	}

	font font::load_from_file(const std::string aFileName)
	{
		return font(app::instance().rendering_engine().font_manager().load_font_from_file(aFileName, app::instance().rendering_engine().screen_metrics()));
	}

	font font::load_from_file(const std::string aFileName, style_e aStyle, point_size aSize)
	{
		return font(app::instance().rendering_engine().font_manager().load_font_from_file(aFileName, aStyle, aSize, app::instance().rendering_engine().screen_metrics()));
	}
	
	font font::load_from_file(const std::string aFileName, const std::string& aStyleName, point_size aSize)
	{
		return font(app::instance().rendering_engine().font_manager().load_font_from_file(aFileName, aStyleName, aSize, app::instance().rendering_engine().screen_metrics()));
	}

	font font::load_from_memory(const void* aData, std::size_t aSizeInBytes)
	{
		return font(app::instance().rendering_engine().font_manager().load_font_from_memory(aData, aSizeInBytes, app::instance().rendering_engine().screen_metrics()));
	}

	font font::load_from_memory(const void* aData, std::size_t aSizeInBytes, style_e aStyle, point_size aSize)
	{
		return font(app::instance().rendering_engine().font_manager().load_font_from_memory(aData, aSizeInBytes, aStyle, aSize, app::instance().rendering_engine().screen_metrics()));
	}

	font font::load_from_memory(const void* aData, std::size_t aSizeInBytes, const std::string& aStyleName, point_size aSize)
	{
		return font(app::instance().rendering_engine().font_manager().load_font_from_memory(aData, aSizeInBytes, aStyleName, aSize, app::instance().rendering_engine().screen_metrics()));
	}

	font::~font()
	{
	}

	font& font::operator=(const font& aOther)
	{
		font_info::operator=(aOther);
		iNativeFontFace = aOther.iNativeFontFace;
		return *this;
	}

	font font::fallback() const
	{
		return font{app::instance().rendering_engine().font_manager().create_fallback_font(*iNativeFontFace), style()};
	}

	const std::string& font::family_name() const
	{
		return iNativeFontFace->family_name();
	}

	font::style_e font::style() const
	{
		return iNativeFontFace->style();
	}

	const std::string& font::style_name() const
	{
		return iNativeFontFace->style_name();
	}

	font::point_size font::size() const
	{
		return iNativeFontFace->size();
	}

	dimension font::height() const
	{
		return iNativeFontFace->height();
	}

	dimension font::descender() const
	{
		return iNativeFontFace->descender();
	}

	dimension font::line_spacing() const
	{
		/* todo */
		return 0;
	}

	dimension font::kerning(uint32_t aLeftGlyphIndex, uint32_t aRightGlyphIndex) const
	{
		return iNativeFontFace->kerning(aLeftGlyphIndex, aRightGlyphIndex);
	}

	i_native_font_face& font::native_font_face() const
	{
		return *iNativeFontFace;
	}
}