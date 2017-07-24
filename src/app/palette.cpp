// palette.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neogfx/app/palette.hpp>

namespace neogfx
{
	palette::palette()
	{
	}

	palette::palette(const i_palette& aOther) :
		iColour{ aOther.has_colour() ? aOther.colour() : optional_colour{} },
		iBackgroundColour{ aOther.has_background_colour() ? aOther.background_colour() : optional_colour{} },
		iForegroundColour{ aOther.has_foreground_colour() ? aOther.foreground_colour() : optional_colour{} },
		iTextColour{ aOther.has_text_colour() ? aOther.text_colour() : optional_colour{} },
		iSelectionColour{ aOther.has_selection_colour() ? aOther.selection_colour() : optional_colour{} },
		iHoverColour{ aOther.has_hover_colour() ? aOther.hover_colour() : optional_colour{} },
		iWidgetDetailPrimaryColour{ aOther.has_widget_detail_primary_colour() ? aOther.widget_detail_primary_colour() : optional_colour{} },
		iWidgetDetailSecondaryColour{ aOther.has_widget_detail_secondary_colour() ? aOther.widget_detail_secondary_colour() : optional_colour{} }
	{
	}

	palette::palette(const palette& aOther) :
		palette(static_cast<const i_palette&>(aOther))
	{
	}

	palette& palette::operator=(const i_palette& aOther)
	{
		if (*this != aOther)
		{
			iColour = aOther.has_colour() ? aOther.colour() : optional_colour{};
			iBackgroundColour = aOther.has_background_colour() ? aOther.background_colour() : optional_colour{};
			iForegroundColour = aOther.has_foreground_colour() ? aOther.foreground_colour() : optional_colour{};
			iTextColour = aOther.has_text_colour() ? aOther.text_colour() : optional_colour{};
			iSelectionColour = aOther.has_selection_colour() ? aOther.selection_colour() : optional_colour{};
			iHoverColour = aOther.has_hover_colour() ? aOther.hover_colour() : optional_colour{};
			iWidgetDetailPrimaryColour = aOther.has_widget_detail_primary_colour() ? aOther.widget_detail_primary_colour() : optional_colour{};
			iWidgetDetailSecondaryColour = aOther.has_widget_detail_secondary_colour() ? aOther.widget_detail_secondary_colour() : optional_colour{};
			changed.trigger();
		}
		return *this;
	}

	bool palette::operator==(const i_palette& aOther) const
	{
		return has_colour() == aOther.has_colour() &&
			colour() == aOther.colour() &&
			has_background_colour() == aOther.has_background_colour() &&
			background_colour() == aOther.background_colour() &&
			has_foreground_colour() == aOther.has_foreground_colour() &&
			foreground_colour() == aOther.foreground_colour() &&
			has_text_colour() == aOther.has_text_colour() &&
			text_colour() == aOther.text_colour() &&
			has_selection_colour() == aOther.has_selection_colour() &&
			selection_colour() == aOther.selection_colour() &&
			has_hover_colour() == aOther.has_hover_colour() &&
			hover_colour() == aOther.hover_colour() &&
			has_widget_detail_primary_colour() == aOther.has_widget_detail_primary_colour() &&
			widget_detail_primary_colour() == aOther.widget_detail_primary_colour() &&
			has_widget_detail_secondary_colour() == aOther.has_widget_detail_secondary_colour() &&
			widget_detail_secondary_colour() == aOther.widget_detail_secondary_colour();
	}

	bool palette::operator!=(const i_palette& aOther) const
	{
		return !(*this == aOther);
	}

	bool palette::has_colour() const
	{
		return iColour != boost::none;
	}

	colour palette::colour() const
	{
		if (has_colour())
			return iColour->with_alpha(0xFF);
		return neogfx::colour{ 0xEF, 0xEB, 0xE7 };
	}

	void palette::set_colour(const optional_colour& aColour)
	{
		if (iColour != aColour)
		{
			iColour = aColour;
			changed.trigger();
		}
	}

	bool palette::has_background_colour() const
	{
		return iBackgroundColour != boost::none;
	}

	colour palette::background_colour() const
	{
		if (has_background_colour())
			return *iBackgroundColour;
		else
			return colour().light() ? colour().lighter(0x20) : colour().darker(0x20);
	}

	void palette::set_background_colour(const optional_colour& aBackgroundColour)
	{
		if (iBackgroundColour != aBackgroundColour)
		{
			iBackgroundColour = aBackgroundColour;
			changed.trigger();
		}
	}

	bool palette::has_foreground_colour() const
	{
		return iForegroundColour != boost::none;
	}

	colour palette::foreground_colour() const
	{
		if (has_foreground_colour())
			return *iForegroundColour;
		else
			return colour().light() ? colour().darker(0x20) : colour().lighter(0x20);
	}

	void palette::set_foreground_colour(const optional_colour& aForegroundColour)
	{
		if (iForegroundColour != aForegroundColour)
		{
			iForegroundColour = aForegroundColour;
			changed.trigger();
		}
	}

	bool palette::has_text_colour() const
	{
		return iTextColour != boost::none;
	}

	colour palette::text_colour() const
	{
		if (has_text_colour())
			return *iTextColour;
		else
		{
			if (colour().to_hsl().lightness() < 0.5)
				return colour::White;
			else
				return colour::Black;
		}
	}

	void palette::set_text_colour(const optional_colour& aTextColour)
	{
		if (iTextColour != aTextColour)
		{
			iTextColour = aTextColour;
			changed.trigger();
		}
	}

	bool palette::has_selection_colour() const
	{
		return iTextColour != boost::none;
	}

	colour palette::selection_colour() const
	{
		if (has_selection_colour())
			return *iTextColour;
		else
			return neogfx::colour{ 0x2A, 0x82, 0xDA };
	}

	void palette::set_selection_colour(const optional_colour& aSelectionColour)
	{
		if (iSelectionColour != aSelectionColour)
		{
			iSelectionColour = aSelectionColour;
			changed.trigger();
		}
	}

	bool palette::has_hover_colour() const
	{
		return iHoverColour != boost::none;
	}

	colour palette::hover_colour() const
	{
		if (has_hover_colour())
			return *iHoverColour;
		else
			return selection_colour().lighter(0x40);
	}

	void palette::set_hover_colour(const optional_colour& aHoverColour)
	{
		if (iHoverColour != aHoverColour)
		{
			iHoverColour = aHoverColour;
			changed.trigger();
		}
	}

	bool palette::has_widget_detail_primary_colour() const
	{
		return iWidgetDetailPrimaryColour != boost::none;
	}

	colour palette::widget_detail_primary_colour() const
	{
		if (has_widget_detail_primary_colour())
			return *iWidgetDetailPrimaryColour;
		else
			return colour().same_lightness_as(colour().light() ? neogfx::colour{ 32, 32, 32 } : neogfx::colour{ 224, 224, 224 });
	}

	void palette::set_widget_detail_primary_colour(const optional_colour& aWidgetDetailPrimaryColour)
	{
		if (iWidgetDetailPrimaryColour != aWidgetDetailPrimaryColour)
		{
			iWidgetDetailPrimaryColour = aWidgetDetailPrimaryColour;
			changed.trigger();
		}
	}

	bool palette::has_widget_detail_secondary_colour() const
	{
		return iWidgetDetailSecondaryColour != boost::none;
	}

	colour palette::widget_detail_secondary_colour() const
	{
		if (has_widget_detail_secondary_colour())
			return *iWidgetDetailSecondaryColour;
		else
			return colour().same_lightness_as(colour().light() ? neogfx::colour{ 64, 64, 64 } : neogfx::colour{ 192, 192, 192 });
	}

	void palette::set_widget_detail_secondary_colour(const optional_colour& aWidgetDetailSecondaryColour)
	{
		if (iWidgetDetailSecondaryColour != aWidgetDetailSecondaryColour)
		{
			iWidgetDetailSecondaryColour = aWidgetDetailSecondaryColour;
			changed.trigger();
		}
	}
}