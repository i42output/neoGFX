// i_style.hpp
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
#include "geometry.hpp"
#include "colour.hpp"
#include "font.hpp"

namespace neogfx
{
	class i_style
	{
	public:
		virtual ~i_style() {}
	public:
		virtual const std::string& name() const = 0;
		virtual const margins& default_margins() const = 0;
		virtual void set_default_margins(const margins& aMargins) = 0;
		virtual const size& default_spacing() const = 0;
		virtual void set_default_spacing(const size& aSpacing) = 0;
		virtual const colour& default_background_colour() const = 0;
		virtual void set_default_background_colour(const colour& aBackgroundColour) = 0;
		virtual const colour& default_foreground_colour() const = 0;
		virtual void set_default_foreground_colour(const colour& aForegroundColour) = 0;
		virtual const colour& default_text_colour() const = 0;
		virtual void set_default_text_colour(const colour& aTextColour) = 0;
		virtual const colour& default_selection_colour() const = 0;
		virtual void set_default_selection_colour(const colour& aSelectionColour) = 0;
		virtual const colour& default_hover_colour() const = 0;
		virtual void set_default_hover_colour(const colour& aHoverColour) = 0;
		virtual const font_info& default_font_info() const = 0;
		virtual void set_default_font_info(const font_info& aFontInfo) = 0;
		virtual const font& default_font() const = 0;
		virtual const font_info& fallback_font_info() const = 0;
		virtual void set_fallback_font_info(const font_info& aFontInfo) = 0;
		virtual const font& fallback_font() const = 0;
	};
}