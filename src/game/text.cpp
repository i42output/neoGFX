// text.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include <neogfx/game/rectangle.hpp>
#include <neogfx/game/text.hpp>

namespace neogfx
{
	text::text(i_shape_container& aContainer, const vec3& aPosition, const std::string& aText, const neogfx::font& aFont, const neogfx::text_appearance& aAppearance, neogfx::alignment aAlignment) :
		shape{ aContainer }, iText{ aText }, iFont{ aFont }, iAppearance{ aAppearance }, iAlignment { aAlignment }, iGlyphTextCache(aFont)
	{
		set_position(aPosition);
		add_frame(std::make_shared<neogfx::shape_frame>(aAppearance.ink()));
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

	const neogfx::text_appearance& text::appearance() const
	{
		return iAppearance;
	}

	void text::set_appearance(const neogfx::text_appearance& aAppearance)
	{
		iAppearance = aAppearance;
		for (frame_index i = 0; i < frame_count(); ++i)
			shape_frame(i).set_colour(aAppearance.ink());
	}

	neogfx::alignment text::alignment() const
	{
		return iAlignment;
	}
	
	void text::set_alignment(neogfx::alignment aAlignment)
	{
		iAlignment = aAlignment;
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

	rect text::bounding_box_2d(bool aWithPosition) const
	{
		return rect{ point{ origin() + (aWithPosition ? position() : vec3{}) }, size{ extents() } };
	}

	void text::paint(graphics_context& aGraphicsContext) const
	{
		if (iGlyphTextCache.font() != font())
		{
			iTextExtent = boost::none;
			iGlyphTextCache = glyph_text(font());
		}
		aGraphicsContext.set_glyph_text_cache(iGlyphTextCache);
		auto bb2d = bounding_box_2d();
		bb2d.position() = bb2d.position().ceil();
		rectangle bb3d{ vec3{ bb2d.x, bb2d.y, position().y }, bb2d.extents().to_vec2() };
		if (appearance().has_paper())
			aGraphicsContext.fill_shape(bb3d, to_brush(appearance().paper()));
		if (iBorder != boost::none)
		{
			aGraphicsContext.draw_shape(bb3d, pen{ appearance().ink(), *iBorder });
			bb2d.deflate(size{ *iBorder });
		}
		if (iMargins != boost::none)
		{
			bb2d.position() += point{iMargins->left, iMargins->right};
			bb2d.extents() -= size{iMargins->left + iMargins->right, iMargins->bottom + iMargins->top};
		}
		auto pos = aGraphicsContext.logical_coordinates().second.y < aGraphicsContext.logical_coordinates().first.y ? bb2d.bottom_left() : bb2d.top_left();
		aGraphicsContext.draw_multiline_text(vec3{pos.x, pos.y, position().z}, iText, font(), bb2d.extents().cx, appearance(), iAlignment, UseGlyphTextCache);
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
		graphics_context gc{ container().as_widget(), graphics_context::type::Unattached };
		gc.set_glyph_text_cache(iGlyphTextCache);
		return *(iTextExtent = gc.multiline_text_extent(iText, font(), UseGlyphTextCache));
	}
}