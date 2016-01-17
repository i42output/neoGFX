// text_widget.cpp
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
#include "text_widget.hpp"
#include "graphics_context.hpp"

namespace neogfx
{
	text_widget::text_widget(const std::string& aText, bool aMultiLine) : 
		widget(), iText(aText), iGlyphTextCache(font()), iMultiLine(aMultiLine)
	{
	}

	text_widget::text_widget(i_widget& aParent, const std::string& aText, bool aMultiLine) :
		widget(aParent), iText(aText), iGlyphTextCache(font()), iMultiLine(aMultiLine)
	{
	}

	text_widget::text_widget(i_layout& aLayout, const std::string& aText, bool aMultiLine) :
		widget(aLayout), iText(aText), iGlyphTextCache(font()), iMultiLine(aMultiLine)
	{
	}

	size text_widget::minimum_size() const
	{
		if (widget::has_minimum_size())
			return widget::minimum_size();
		else
		{
			size result = units_converter(*this).to_device_units(text_extent() + margins().size());
			result.cx = std::ceil(result.cx);
			result.cy = std::ceil(result.cy);
			return units_converter(*this).from_device_units(result);
		}
	}

	void text_widget::paint(graphics_context& aGraphicsContext) const
	{
		if (iGlyphTextCache.font() != font())
		{
			iTextExtent = boost::none;
			iGlyphTextCache = glyph_text(font());
		}
		aGraphicsContext.set_glyph_text_cache(iGlyphTextCache);
		size textSize = text_extent();
		point textPosition(std::floor((client_rect().width() - textSize.cx) / 2.0), std::floor((client_rect().height() - textSize.cy) / 2.0));
		if (multi_line())
			aGraphicsContext.draw_multiline_text(textPosition, text(), font(), textSize.cx, text_colour(), alignment::Centre, true);
		else
			aGraphicsContext.draw_text(textPosition, text(), font(), text_colour(), true);
	}

	void text_widget::set_font(const optional_font& aFont)
	{
		widget::set_font(aFont);
		iTextExtent = boost::none;
		iGlyphTextCache = glyph_text(font());
	}

	const std::string& text_widget::text() const
	{
		return iText;
	}

	void text_widget::set_text(const std::string& aText)
	{
		iText = aText;
		iTextExtent = boost::none;
		iGlyphTextCache = glyph_text(font());
		update();
	}

	bool text_widget::multi_line() const
	{
		return iMultiLine;
	}

	bool text_widget::has_text_colour() const
	{
		return iTextColour != boost::none;
	}

	colour text_widget::text_colour() const
	{
		if (has_text_colour())
			return *iTextColour;
		optional_colour textColour;
		const i_widget* w = 0;
		do
		{
			if (w == 0)
				w = this;
			else
				w = &w->parent();
			if (w->has_background_colour())
			{
				textColour = w->background_colour().to_hsl().lightness() >= 0.5f ? colour::Black : colour::White;
				break; 
			}
			else if (w->has_foreground_colour())
			{
				textColour = w->foreground_colour().to_hsl().lightness() >= 0.5f ? colour::Black : colour::White;
				break;
			}
		} while (w->has_parent());
		colour defaultTextColour = app::instance().current_style().default_text_colour();
		if (textColour == boost::none || textColour->similar_intensity(defaultTextColour))
			return defaultTextColour;
		else
			return *textColour;
	}

	void text_widget::set_text_colour(const optional_colour& aTextColour)
	{
		iTextColour = aTextColour;
		update();
	}

	size text_widget::text_extent() const
	{
		if (iGlyphTextCache.font() != font())
		{
			iTextExtent = boost::none;
			iGlyphTextCache = glyph_text(font());
		}
		if (iTextExtent != boost::none)
			return *iTextExtent;
		graphics_context gc(*this);
		gc.set_glyph_text_cache(iGlyphTextCache);
		if (iMultiLine)
		{
			if (has_minimum_size() && widget::minimum_size().cx != 0 && widget::minimum_size().cy == 0)
				return *(iTextExtent = gc.multiline_text_extent(iText, font(), widget::minimum_size().cx - margins().size().cx, true));
			else
				return *(iTextExtent = gc.multiline_text_extent(iText, font(), true));
		}
		else
			return *(iTextExtent = gc.text_extent(iText, font(), true));
	}
}