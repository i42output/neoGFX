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

#include <neogfx/neogfx.hpp>
#include <neogfx/core/geometry.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/text/font.hpp>

namespace neogfx
{
	class i_style
	{
	public:
		virtual ~i_style() {}
	public:
		virtual const std::string& name() const = 0;
		virtual const neogfx::margins& margins() const = 0;
		virtual void set_margins(const neogfx::margins& aMargins) = 0;
		virtual const size& spacing() const = 0;
		virtual void set_spacing(const size& aSpacing) = 0;
		virtual bool has_colour() const = 0;
		virtual neogfx::colour colour() const = 0;
		virtual void set_colour(const optional_colour& aDefaultColour) = 0;
		virtual bool has_background_colour() const = 0;
		virtual neogfx::colour background_colour() const = 0;
		virtual void set_background_colour(const optional_colour& aBackgroundColour) = 0;
		virtual bool has_foreground_colour() const = 0;
		virtual neogfx::colour foreground_colour() const = 0;
		virtual void set_foreground_colour(const optional_colour& aForegroundColour) = 0;
		virtual bool has_text_colour() const = 0;
		virtual neogfx::colour text_colour() const = 0;
		virtual void set_text_colour(const optional_colour& aTextColour) = 0;
		virtual bool has_selection_colour() const = 0;
		virtual neogfx::colour selection_colour() const = 0;
		virtual void set_selection_colour(const optional_colour& aSelectionColour) = 0;
		virtual bool has_hover_colour() const = 0;
		virtual neogfx::colour hover_colour() const = 0;
		virtual void set_hover_colour(const optional_colour& aHoverColour) = 0;
		virtual bool has_widget_detail_primary_colour() const = 0;
		virtual neogfx::colour widget_detail_primary_colour() const = 0;
		virtual void set_widget_detail_primary_colour(const optional_colour& aWidgetDetailPrimaryColour) = 0;
		virtual bool has_widget_detail_secondary_colour() const = 0;
		virtual neogfx::colour widget_detail_secondary_colour() const = 0;
		virtual void set_widget_detail_secondary_colour(const optional_colour& aWidgetDetailSecondaryColour) = 0;
		virtual const font_info& font_info() const = 0;
		virtual void set_font_info(const neogfx::font_info& aFontInfo) = 0;
		virtual const neogfx::font& font() const = 0;
		virtual const neogfx::font_info& fallback_font_info() const = 0;
		virtual void set_fallback_font_info(const neogfx::font_info& aFontInfo) = 0;
		virtual const neogfx::font& fallback_font() const = 0;
	};
}