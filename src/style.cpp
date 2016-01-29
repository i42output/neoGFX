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
		iDefaultFontInfo(app::instance().rendering_engine().font_manager().default_system_font_info()),
		iFallbackFontInfo(app::instance().rendering_engine().font_manager().default_fallback_font_info())
	{
	}

	style::style(const std::string& aName, const i_style& aOther) :
		iName(aName),
		iDefaultMargins(aOther.default_margins()),
		iDefaultSpacing(aOther.default_spacing()),
		iDefaultColour(aOther.has_default_colour() ? aOther.default_colour() : optional_colour()),
		iDefaultBackgroundColour(aOther.has_default_background_colour() ? aOther.default_background_colour() : optional_colour()),
		iDefaultForegroundColour(aOther.has_default_foreground_colour() ? aOther.default_foreground_colour() : optional_colour()),
		iDefaultTextColour(aOther.has_default_text_colour() ? aOther.default_text_colour() : optional_colour()),
		iDefaultSelectionColour(aOther.has_default_selection_colour() ? aOther.default_selection_colour() : optional_colour()),
		iDefaultHoverColour(aOther.has_default_hover_colour() ? aOther.default_hover_colour() : optional_colour()),
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

	bool style::has_default_colour() const
	{
		return iDefaultColour != boost::none;
	}

	colour style::default_colour() const
	{
		if (has_default_colour())
			return *iDefaultColour;
		return colour(0xEF, 0xEB, 0xE7);
	}

	void style::set_default_colour(const optional_colour& aDefaultColour)
	{
		if (iDefaultColour != aDefaultColour)
		{
			iDefaultColour = aDefaultColour;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	bool style::has_default_background_colour() const
	{
		return iDefaultBackgroundColour != boost::none;
	}

	colour style::default_background_colour() const
	{
		if (has_default_background_colour())
			return *iDefaultBackgroundColour;
		else
			return default_colour().light() ? default_colour().lighter(0x20) : default_colour().darker(0x20);
	}

	void style::set_default_background_colour(const optional_colour& aBackgroundColour)
	{
		if (iDefaultBackgroundColour != aBackgroundColour)
		{
			iDefaultBackgroundColour = aBackgroundColour;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	bool style::has_default_foreground_colour() const
	{
		return iDefaultForegroundColour != boost::none;
	}

	colour style::default_foreground_colour() const
	{
		if (has_default_foreground_colour())
			return *iDefaultForegroundColour;
		else
			return default_colour().light() ? default_colour().darker(0x20) : default_colour().lighter(0x20);
	}

	void style::set_default_foreground_colour(const optional_colour& aForegroundColour)
	{
		if (iDefaultForegroundColour != aForegroundColour)
		{
			iDefaultForegroundColour = aForegroundColour;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	bool style::has_default_text_colour() const
	{
		return iDefaultTextColour != boost::none;
	}

	colour style::default_text_colour() const
	{
		if (has_default_text_colour())
			return *iDefaultTextColour;
		else
			return default_colour().dark() ? colour::White : colour::Black;
	}

	void style::set_default_text_colour(const optional_colour& aTextColour)
	{
		if (iDefaultTextColour != aTextColour)
		{
			iDefaultTextColour = aTextColour;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	bool style::has_default_selection_colour() const
	{
		return iDefaultTextColour != boost::none;
	}

	colour style::default_selection_colour() const
	{
		if (has_default_selection_colour())
			return *iDefaultTextColour;
		else
			return colour(0x2A, 0x82, 0xDA);
	}

	void style::set_default_selection_colour(const optional_colour& aSelectionColour)
	{
		if (iDefaultSelectionColour != aSelectionColour)
		{
			iDefaultSelectionColour = aSelectionColour;
			if (&app::instance().current_style() == this)
				app::instance().surface_manager().invalidate_surfaces();
		}
	}

	bool style::has_default_hover_colour() const
	{
		return iDefaultHoverColour != boost::none;
	}

	colour style::default_hover_colour() const
	{
		if (has_default_hover_colour())
			return *iDefaultHoverColour;
		else
			return default_selection_colour().lighter(0x40);
	}

	void style::set_default_hover_colour(const optional_colour& aHoverColour)
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