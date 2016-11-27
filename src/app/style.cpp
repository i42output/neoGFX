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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/app/style.hpp>

namespace neogfx
{
	style::style(const std::string& aName) :
		iName(aName),
		iMargins(2.0),
		iSpacing(2.0, 2.0),
		iFontInfo(app::instance().rendering_engine().font_manager().default_system_font_info())
	{
	}

	style::style(const std::string& aName, const i_style& aOther) :
		iName(aName),
		iMargins(aOther.margins()),
		iSpacing(aOther.spacing()),
		iColour(aOther.has_colour() ? aOther.colour() : optional_colour()),
		iBackgroundColour(aOther.has_background_colour() ? aOther.background_colour() : optional_colour()),
		iForegroundColour(aOther.has_foreground_colour() ? aOther.foreground_colour() : optional_colour()),
		iTextColour(aOther.has_text_colour() ? aOther.text_colour() : optional_colour()),
		iSelectionColour(aOther.has_selection_colour() ? aOther.selection_colour() : optional_colour()),
		iHoverColour(aOther.has_hover_colour() ? aOther.hover_colour() : optional_colour()),
		iFontInfo(aOther.font_info())
	{
	}

	style::~style()
	{
	}

	const std::string& style::name() const
	{
		return iName;
	}

	const neogfx::margins& style::margins() const
	{
		return iMargins;
	}

	void style::set_margins(const neogfx::margins& aMargins)
	{
		if (iMargins != aMargins)
		{
			iMargins = aMargins;
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().layout_surfaces();
			}
		}
	}

	const size& style::spacing() const
	{
		return iSpacing;
	}

	void style::set_spacing(const size& aSpacing)
	{
		if (iSpacing != aSpacing)
		{
			iSpacing = aSpacing;
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().layout_surfaces();
			}
		}
	}

	bool style::has_colour() const
	{
		return iColour != boost::none;
	}

	colour style::colour() const
	{
		if (has_colour())
			return *iColour;
		return neogfx::colour(0xEF, 0xEB, 0xE7);
	}

	void style::set_colour(const optional_colour& aColour)
	{
		if (iColour != aColour)
		{
			iColour = aColour;
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().invalidate_surfaces();
			}
		}
	}

	bool style::has_background_colour() const
	{
		return iBackgroundColour != boost::none;
	}

	colour style::background_colour() const
	{
		if (has_background_colour())
			return *iBackgroundColour;
		else
			return colour().light() ? colour().lighter(0x20) : colour().darker(0x20);
	}

	void style::set_background_colour(const optional_colour& aBackgroundColour)
	{
		if (iBackgroundColour != aBackgroundColour)
		{
			iBackgroundColour = aBackgroundColour;
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().invalidate_surfaces();
			}
		}
	}

	bool style::has_foreground_colour() const
	{
		return iForegroundColour != boost::none;
	}

	colour style::foreground_colour() const
	{
		if (has_foreground_colour())
			return *iForegroundColour;
		else
			return colour().light() ? colour().darker(0x20) : colour().lighter(0x20);
	}

	void style::set_foreground_colour(const optional_colour& aForegroundColour)
	{
		if (iForegroundColour != aForegroundColour)
		{
			iForegroundColour = aForegroundColour;
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().invalidate_surfaces();
			}
		}
	}

	bool style::has_text_colour() const
	{
		return iTextColour != boost::none;
	}

	colour style::text_colour() const
	{
		if (has_text_colour())
			return *iTextColour;
		else
			return colour().dark() ? colour::White : colour::Black;
	}

	void style::set_text_colour(const optional_colour& aTextColour)
	{
		if (iTextColour != aTextColour)
		{
			iTextColour = aTextColour;
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().invalidate_surfaces();
			}
		}
	}

	bool style::has_selection_colour() const
	{
		return iTextColour != boost::none;
	}

	colour style::selection_colour() const
	{
		if (has_selection_colour())
			return *iTextColour;
		else
			return neogfx::colour(0x2A, 0x82, 0xDA);
	}

	void style::set_selection_colour(const optional_colour& aSelectionColour)
	{
		if (iSelectionColour != aSelectionColour)
		{
			iSelectionColour = aSelectionColour;
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().invalidate_surfaces();
			}
		}
	}

	bool style::has_hover_colour() const
	{
		return iHoverColour != boost::none;
	}

	colour style::hover_colour() const
	{
		if (has_hover_colour())
			return *iHoverColour;
		else
			return selection_colour().lighter(0x40);
	}

	void style::set_hover_colour(const optional_colour& aHoverColour)
	{
		if (iHoverColour != aHoverColour)
		{
			iHoverColour = aHoverColour;
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().invalidate_surfaces();
			}
		}
	}

	bool style::has_widget_detail_primary_colour() const
	{
		return iWidgetDetailPrimaryColour != boost::none;
	}

	colour style::widget_detail_primary_colour() const
	{
		if (has_widget_detail_primary_colour())
			return *iWidgetDetailPrimaryColour;
		else
			return colour().same_lightness_as(colour().light() ? neogfx::colour{32, 32, 32} : neogfx::colour{224, 224, 224});
	}

	void style::set_widget_detail_primary_colour(const optional_colour& aWidgetDetailPrimaryColour)
	{
		if (iWidgetDetailPrimaryColour != aWidgetDetailPrimaryColour)
		{
			iWidgetDetailPrimaryColour = aWidgetDetailPrimaryColour;
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().invalidate_surfaces();
			}
		}
	}

	bool style::has_widget_detail_secondary_colour() const
	{
		return iWidgetDetailSecondaryColour != boost::none;
	}

	colour style::widget_detail_secondary_colour() const
	{
		if (has_widget_detail_secondary_colour())
			return *iWidgetDetailSecondaryColour;
		else
			return colour().same_lightness_as(colour().light() ? neogfx::colour{ 64, 64, 64 } : neogfx::colour{ 192, 192, 192 });
	}

	void style::set_widget_detail_secondary_colour(const optional_colour& aWidgetDetailSecondaryColour)
	{
		if (iWidgetDetailSecondaryColour != aWidgetDetailSecondaryColour)
		{
			iWidgetDetailSecondaryColour = aWidgetDetailSecondaryColour;
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().invalidate_surfaces();
			}
		}
	}

	const font_info& style::font_info() const
	{
		return iFontInfo;
	}

	void style::set_font_info(const neogfx::font_info& aFontInfo)
	{
		if (iFontInfo != aFontInfo)
		{
			iFontInfo = aFontInfo;
			iFont.reset();
			if (&app::instance().current_style() == this)
			{
				app::instance().current_style_changed.trigger();
				app::instance().surface_manager().layout_surfaces();
			}
		}
	}

	const font& style::font() const
	{
		if (iFont == boost::none)
		{
			iFont = neogfx::font(iFontInfo);
		}
		return *iFont;
	}
}