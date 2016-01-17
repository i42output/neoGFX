// style.cpp
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
#include "style.hpp"

namespace neogfx
{
	style::style(const std::string& aName) :
		iName(aName),
		iDefaultMargins(2.0),
		iDefaultSpacing(2.0, 2.0),
		iDefaultBackgroundColour(colour::White),
		iDefaultForegroundColour(colour::LightGray),
		iDefaultTextColour(colour::Black),
		iDefaultSelectionColour(51, 153, 255),
		iDefaultHoverColour(iDefaultSelectionColour.lighter(0x40)),
		iDefaultFontInfo(app::instance().rendering_engine().font_manager().default_system_font_info()),
		iFallbackFontInfo(app::instance().rendering_engine().font_manager().default_fallback_font_info())
	{
	}

	style::style(const std::string& aName, const i_style& aOther) :
		iName(aName),
		iDefaultMargins(aOther.default_margins()),
		iDefaultSpacing(aOther.default_spacing()),
		iDefaultBackgroundColour(aOther.default_background_colour()),
		iDefaultForegroundColour(aOther.default_foreground_colour()),
		iDefaultTextColour(aOther.default_text_colour()),
		iDefaultSelectionColour(aOther.default_selection_colour()),
		iDefaultHoverColour(aOther.default_hover_colour()),
		iDefaultFontInfo(aOther.default_font_info()),
		iFallbackFontInfo(aOther.fallback_font_info())
	{
	}

	style::~style()
	{
	}

	const std::string& style::name() const
	{
		return iName;
	}

	const margins& style::default_margins() const
	{
		return iDefaultMargins;
	}

	void style::set_default_margins(const margins& aMargins)
	{
		if (iDefaultMargins != aMargins)
		{
			iDefaultMargins = aMargins;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	const size& style::default_spacing() const
	{
		return iDefaultSpacing;
	}

	void style::set_default_spacing(const size& aSpacing)
	{
		if (iDefaultSpacing != aSpacing)
		{
			iDefaultSpacing = aSpacing;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	const colour& style::default_background_colour() const
	{
		return iDefaultBackgroundColour;
	}

	void style::set_default_background_colour(const colour& aBackgroundColour)
	{
		if (iDefaultBackgroundColour != aBackgroundColour)
		{
			iDefaultBackgroundColour = aBackgroundColour;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	const colour& style::default_foreground_colour() const
	{
		return iDefaultForegroundColour;
	}

	void style::set_default_foreground_colour(const colour& aForegroundColour)
	{
		if (iDefaultForegroundColour != aForegroundColour)
		{
			iDefaultForegroundColour = aForegroundColour;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	const colour& style::default_text_colour() const
	{
		return iDefaultTextColour;
	}

	void style::set_default_text_colour(const colour& aTextColour)
	{
		if (iDefaultTextColour != aTextColour)
		{
			iDefaultTextColour = aTextColour;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	const colour& style::default_selection_colour() const
	{
		return iDefaultSelectionColour;
	}

	void style::set_default_selection_colour(const colour& aSelectionColour)
	{
		if (iDefaultSelectionColour != aSelectionColour)
		{
			iDefaultSelectionColour = aSelectionColour;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	const colour& style::default_hover_colour() const
	{
		return iDefaultHoverColour;
	}

	void style::set_default_hover_colour(const colour& aHoverColour)
	{
		if (iDefaultHoverColour != aHoverColour)
		{
			iDefaultHoverColour = aHoverColour;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	const font_info& style::default_font_info() const
	{
		return iDefaultFontInfo;
	}

	void style::set_default_font_info(const font_info& aFontInfo)
	{
		if (iDefaultFontInfo != aFontInfo)
		{
			iDefaultFontInfo = aFontInfo;
			iDefaultFont.reset();
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	const font& style::default_font() const
	{
		if (iDefaultFont == boost::none)
		{
			iDefaultFont = font();
		}
		return *iDefaultFont;
	}

	const font_info& style::fallback_font_info() const
	{
		return iFallbackFontInfo;
	}

	void style::set_fallback_font_info(const font_info& aFontInfo)
	{
		if (iFallbackFontInfo != aFontInfo)
		{
			iFallbackFontInfo = aFontInfo;
			iFallbackFont.reset();
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	const font& style::fallback_font() const
	{
		if (iFallbackFont == boost::none)
		{
			iFallbackFont = font(iFallbackFontInfo);
		}
		return *iFallbackFont;
	}
}