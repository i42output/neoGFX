// style.hpp
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
#include "i_style.hpp"

namespace neogfx
{
	class style : public i_style
	{
	public:
		style(const std::string& aName);
		style(const std::string& aName, const i_style& aOther);
		virtual ~style();
	public:
		virtual const std::string& name() const;
		virtual const neogfx::margins& margins() const;
		virtual void set_margins(const neogfx::margins& aMargins);
		virtual const size& spacing() const;
		virtual void set_spacing(const size& aSpacing);
		virtual bool has_colour() const;
		virtual neogfx::colour colour() const;
		virtual void set_colour(const optional_colour& aColour);
		virtual bool has_background_colour() const;
		virtual neogfx::colour background_colour() const;
		virtual void set_background_colour(const optional_colour& aBackgroundColour);
		virtual bool has_foreground_colour() const;
		virtual neogfx::colour foreground_colour() const;
		virtual void set_foreground_colour(const optional_colour& aForegroundColour);
		virtual bool has_text_colour() const;
		virtual neogfx::colour text_colour() const;
		virtual void set_text_colour(const optional_colour& aTextColour);
		virtual bool has_selection_colour() const;
		virtual neogfx::colour selection_colour() const;
		virtual void set_selection_colour(const optional_colour& aSelectionColour);
		virtual bool has_hover_colour() const;
		virtual neogfx::colour hover_colour() const;
		virtual void set_hover_colour(const optional_colour& aHoverColour);
		virtual bool has_widget_detail_primary_colour() const;
		virtual neogfx::colour widget_detail_primary_colour() const;
		virtual void set_widget_detail_primary_colour(const optional_colour& aWidgetDetailPrimaryColour);
		virtual bool has_widget_detail_secondary_colour() const;
		virtual neogfx::colour widget_detail_secondary_colour() const;
		virtual void set_widget_detail_secondary_colour(const optional_colour& aWidgetDetailSecondaryColour);
		virtual const neogfx::font_info& font_info() const;
		virtual void set_font_info(const neogfx::font_info& aFontInfo);
		virtual const neogfx::font& font() const;
	private:
		std::string iName;
		neogfx::margins iMargins;
		size iSpacing;
		optional_colour iColour;
		optional_colour iBackgroundColour;
		optional_colour iForegroundColour;
		optional_colour iTextColour;
		optional_colour iSelectionColour;
		optional_colour iHoverColour;
		optional_colour iWidgetDetailPrimaryColour;
		optional_colour iWidgetDetailSecondaryColour;
		neogfx::font_info iFontInfo;
		mutable optional_font iFont;
	};
}