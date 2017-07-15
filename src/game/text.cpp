// text.cpp
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
#include <neogfx/game/text.hpp>

namespace neogfx
{
	text::text(i_shape_container& aContainer, const vec3& aPosition, const std::string& aText, const neogfx::font& aFont, const colour& aTextColour, neogfx::alignment aAlignment, const optional_colour& aBackgroundColour) :
		shape{ aContainer }, iText{ aText }, iFont{ aFont }, iAlignment{ aAlignment }, iGlyphTextCache(aFont), iBackgroundColour{ aBackgroundColour }
	{
		set_position(aPosition);
		add_frame(std::make_shared<neogfx::frame>(aTextColour));
	}

	const colour& text::text_colour() const
	{
		if (frame(0).colour() != boost::none)
			return *frame(0).colour();
		static const colour white = colour::White;
		return white;
	}

	void text::set_text_colour(const colour& aTextColour)
	{
		for (frame_index i = 0; i < frame_count(); ++i)
			frame(i).set_colour(aTextColour);
	}

	const std::string& text::value() const
	{
		return iText;
	}

	void text::set_value(const std::string& aText)
	{
		iText = aText;
		iTextExtent = boost::none;
		iGlyphTextCache = glyph_text(font());
		clear_vertices_cache();
	}

	const neogfx::font& text::font() const
	{
		return iFont;
	}

	void text::set_font(const neogfx::font& aFont)
	{
		iFont = aFont;
		iTextExtent = boost::none;
		iGlyphTextCache = glyph_text(font());
		clear_vertices_cache();
	}

	neogfx::alignment text::alignment() const
	{
		return iAlignment;
	}
	
	void text::set_alignment(neogfx::alignment aAlignment)
	{
		iAlignment = aAlignment;
	}

	const optional_colour& text::background_colour() const
	{
		return iBackgroundColour;
	}

	void text::set_background_colour(const optional_colour& aBackgroundColour)
	{
		iBackgroundColour = aBackgroundColour;
	}

	const optional_dimension& text::border() const
	{
		return iBorder;
	}

	void text::set_border(const optional_dimension& aBorder)
	{
		iBorder = aBorder;
		clear_vertices_cache();
	}

	const optional_margins& text::margins() const
	{
		return iMargins;
	}

	void text::set_margins(const optional_margins& aMargins)
	{
		iMargins = aMargins;
		clear_vertices_cache();
	}

	vec3 text::extents() const
	{
		auto e = text_extent();
		if (iBorder != boost::none)
		{
			e.cx += (*iBorder * 2.0);
			e.cy += (*iBorder * 2.0);
		}
		if (iMargins != boost::none)
		{
			e.cx += (iMargins->left + iMargins->right);
			e.cy += (iMargins->top + iMargins->bottom);
		}
		return vec3{ e.cx, e.cy, 0.0 };
	}

	rect text::bounding_box_2d() const
	{
		return rect{ point{ origin() + position() }, size{ extents() } };
	}

	void text::paint(graphics_context& aGraphicsContext) const
	{
		if (iGlyphTextCache.font() != font())
		{
			iTextExtent = boost::none;
			iGlyphTextCache = glyph_text(font());
		}
		aGraphicsContext.set_glyph_text_cache(iGlyphTextCache);
		auto bb = bounding_box_2d();
		bb.position() = bb.position().ceil();
		if (iBackgroundColour != boost::none)
			aGraphicsContext.fill_rect(bb, *iBackgroundColour);
		if (iBorder != boost::none)
		{
			aGraphicsContext.draw_rect(bb, pen(text_colour(), *iBorder));
			bb.deflate(size{*iBorder});
		}
		if (iMargins != boost::none)
		{
			bb.position() += point{iMargins->left, iMargins->right};
			bb.extents() -= size{iMargins->left + iMargins->right, iMargins->bottom + iMargins->top};
		}
		aGraphicsContext.draw_multiline_text(
			aGraphicsContext.logical_coordinates().second.y < aGraphicsContext.logical_coordinates().first.y ? bb.bottom_left() : bb.top_left(), 
			iText, font(), bb.extents().cx, text_colour(), iAlignment, true);
	}

	size text::text_extent() const
	{
		if (iGlyphTextCache.font() != font())
		{
			iTextExtent = boost::none;
			iGlyphTextCache = glyph_text(font());
		}
		if (iTextExtent != boost::none)
			return *iTextExtent;
		graphics_context gc(container().as_widget());
		gc.set_glyph_text_cache(iGlyphTextCache);
		return *(iTextExtent = gc.multiline_text_extent(iText, font(), true));
	}
}