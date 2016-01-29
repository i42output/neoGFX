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

#include "neogfx.hpp"
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
		virtual const margins& default_margins() const;
		virtual void set_default_margins(const margins& aMargins);
		virtual const size& default_spacing() const;
		virtual void set_default_spacing(const size& aSpacing);
		virtual bool has_default_colour() const;
		virtual colour default_colour() const;
		virtual void set_default_colour(const optional_colour& aDefaultColour);
		virtual bool has_default_background_colour() const;
		virtual colour default_background_colour() const;
		virtual void set_default_background_colour(const optional_colour& aBackgroundColour);
		virtual bool has_default_foreground_colour() const;
		virtual colour default_foreground_colour() const;
		virtual void set_default_foreground_colour(const optional_colour& aForegroundColour);
		virtual bool has_default_text_colour() const;
		virtual colour default_text_colour() const;
		virtual void set_default_text_colour(const optional_colour& aTextColour);
		virtual bool has_default_selection_colour() const;
		virtual colour default_selection_colour() const;
		virtual void set_default_selection_colour(const optional_colour& aSelectionColour);
		virtual bool has_default_hover_colour() const;
		virtual colour default_hover_colour() const;
		virtual void set_default_hover_colour(const optional_colour& aHoverColour);
		virtual const font_info& default_font_info() const;
		virtual void set_default_font_info(const font_info& aFontInfo);
		virtual const font& default_font() const;
		virtual const font_info& fallback_font_info() const;
		virtual void set_fallback_font_info(const font_info& aFontInfo);
		virtual const font& fallback_font() const;
	private:
		std::string iName;
		margins iDefaultMargins;
		size iDefaultSpacing;
		optional_colour iDefaultColour;
		optional_colour iDefaultBackgroundColour;
		optional_colour iDefaultForegroundColour;
		optional_colour iDefaultTextColour;
		optional_colour iDefaultSelectionColour;
		optional_colour iDefaultHoverColour;
		font_info iDefaultFontInfo;
		font_info iFallbackFontInfo;
		mutable optional_font iDefaultFont;
		mutable optional_font iFallbackFont;
	};
}