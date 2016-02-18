// graphics_context.cpp
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

#include "graphics_context.hpp"
#include "i_surface.hpp"
#include "i_native_surface.hpp"
#include "i_native_graphics_context.hpp"
#include "i_widget.hpp"

namespace neogfx
{
	struct graphics_context::glyph_drawing
	{
		const graphics_context& iParent;
		glyph_drawing(const graphics_context& aParent) : iParent(aParent)
		{
			if (++iParent.iDrawingGlyphs == 1)
				iParent.iNativeGraphicsContext->begin_drawing_glyphs();
		}
		~glyph_drawing()
		{
			if (--iParent.iDrawingGlyphs == 0)
				iParent.iNativeGraphicsContext->end_drawing_glyphs();
		}
	};

	graphics_context::graphics_context(const i_surface& aSurface) :
		iSurface(aSurface),
		iNativeGraphicsContext(aSurface.native_surface().create_graphics_context()),
		iUnitsContext(*this),
		iDefaultFont(),
		iOrigin(0, 0),
		iExtents(aSurface.extents()),
		iDrawingGlyphs(0)
	{
	}

	graphics_context::graphics_context(const i_surface& aSurface, const font& aDefaultFont) :
		iSurface(aSurface), 
		iNativeGraphicsContext(aSurface.native_surface().create_graphics_context()), 
		iUnitsContext(*this),
		iDefaultFont(aDefaultFont),
		iOrigin(0, 0),
		iExtents(aSurface.extents()),
		iDrawingGlyphs(0)
	{
	}

	graphics_context::graphics_context(const i_widget& aWidget) :
		iSurface(aWidget.surface()),
		iNativeGraphicsContext(aWidget.surface().native_surface().create_graphics_context()),
		iUnitsContext(*this),
		iDefaultFont(aWidget.font()),
		iOrigin(aWidget.origin()),
		iExtents(aWidget.extents()),
		iDrawingGlyphs(0)
	{
	}

	graphics_context::graphics_context(const graphics_context& aOther) :
		iSurface(aOther.iSurface), 
		iNativeGraphicsContext(aOther.iNativeGraphicsContext->clone()), 
		iUnitsContext(*this), 
		iDefaultFont(aOther.iDefaultFont), 
		iOrigin(aOther.origin()),
		iExtents(aOther.extents()),
		iDrawingGlyphs(0)
	{
	}

	graphics_context::~graphics_context()
	{
	}

	delta graphics_context::to_device_units(const delta& aValue) const
	{
		return units_converter(*this).to_device_units(aValue);
	}

	size graphics_context::to_device_units(const size& aValue) const
	{
		return units_converter(*this).to_device_units(aValue);
	}

	point graphics_context::to_device_units(const point& aValue) const
	{
		return units_converter(*this).to_device_units(aValue);
	}

	rect graphics_context::to_device_units(const rect& aValue) const
	{
		return units_converter(*this).to_device_units(aValue);
	}

	path graphics_context::to_device_units(const path& aValue) const
	{
		path result = aValue;
		result.set_position(to_device_units(result.position()));
		for (std::size_t i = 0; i < result.paths().size(); ++i)
			for (std::size_t j = 0; j < result.paths()[i].size(); ++j)
				result.paths()[i][j] = to_device_units(result.paths()[i][j]);
		return result;
	}

	delta graphics_context::from_device_units(const delta& aValue) const
	{
		return units_converter(*this).from_device_units(aValue);
	}

	size graphics_context::from_device_units(const size& aValue) const
	{
		return units_converter(*this).from_device_units(aValue);
	}

	point graphics_context::from_device_units(const point& aValue) const
	{
		return units_converter(*this).from_device_units(aValue);
	}

	rect graphics_context::from_device_units(const rect& aValue) const
	{
		return units_converter(*this).from_device_units(aValue);
	}

	path graphics_context::from_device_units(const path& aValue) const
	{
		path result = aValue;
		result.set_position(from_device_units(result.position()));
		for (std::size_t i = 0; i < result.paths().size(); ++i)
			for (std::size_t j = 0; j < result.paths()[i].size(); ++j)
				result.paths()[i][j] = from_device_units(result.paths()[i][j]);
		return result;
	}

	void graphics_context::set_default_font(const font& aDefaultFont) const
	{
		iDefaultFont = aDefaultFont;
	}

	void graphics_context::set_extents(const size& aExtents) const
	{
		iExtents = aExtents;
	}

	void graphics_context::set_origin(const point& aOrigin) const
	{
		iOrigin = to_device_units(aOrigin);
	}

	const point& graphics_context::origin() const
	{
		return from_device_units(iOrigin);
	}
	
	void graphics_context::set_pixel(const point& aPoint, const colour& aColour) const
	{
	}

	void graphics_context::draw_pixel(const point& aPoint, const colour& aColour) const
	{
	}

	void graphics_context::draw_line(const point& aFrom, const point& aTo, const pen& aPen) const
	{
		iNativeGraphicsContext->draw_line(to_device_units(aFrom) + iOrigin, to_device_units(aTo) + iOrigin, aPen);
	}

	void graphics_context::draw_rect(const rect& aRect, const pen& aPen) const
	{
		iNativeGraphicsContext->draw_rect(to_device_units(aRect) + iOrigin, aPen);
	}

	void graphics_context::draw_circle(const point& aCentre, dimension aRadius, const pen& aPen) const
	{
		iNativeGraphicsContext->draw_circle(to_device_units(aCentre) + iOrigin, aRadius, aPen);
	}

	void graphics_context::draw_path(const path& aPath, const pen& aPen) const
	{
		path path = to_device_units(aPath);
		path.set_position(path.position() + iOrigin);
		iNativeGraphicsContext->draw_path(path, aPen);
	}

	void graphics_context::draw_focus_rect(const rect& aRect) const
	{
		push_logical_operation(LogicalXor);
		line_stipple_on(1, 0xAAAA);
		draw_rect(aRect, pen(colour::White, 1.0));
		line_stipple_off();
		pop_logical_operation();
	}

	void graphics_context::fill_solid_rect(const rect& aRect, const colour& aColour) const
	{
		iNativeGraphicsContext->fill_solid_rect(to_device_units(aRect) + iOrigin, aColour);
	}

	void graphics_context::fill_gradient_rect(const rect& aRect, const gradient& aGradient) const
	{
		iNativeGraphicsContext->fill_gradient_rect(to_device_units(aRect) + iOrigin, aGradient);
	}

	void graphics_context::fill_solid_circle(const point& aCentre, dimension aRadius, const colour& aColour) const
	{
		iNativeGraphicsContext->fill_solid_circle(to_device_units(aCentre) + iOrigin, aRadius, aColour);
	}

	void graphics_context::fill_and_draw_path(const path& aPath, const colour& aFillColour, const pen& aOutlinePen) const
	{
		path path = to_device_units(aPath);
		path.set_position(path.position() + iOrigin);
		iNativeGraphicsContext->fill_and_draw_path(path, aFillColour, aOutlinePen);
	}

	size graphics_context::text_extent(const text& aText, const font& aFont, bool aUseCache) const
	{
		return text_extent(aText.begin(), aText.end(), aFont, aUseCache);
	}

	size graphics_context::text_extent(text::const_iterator aTextBegin, text::const_iterator aTextEnd, const font& aFont, bool aUseCache) const
	{
		const auto& glyphText = aUseCache && !iGlyphTextCache->empty() ? *iGlyphTextCache : to_glyph_text(aTextBegin, aTextEnd, aFont);
		if (aUseCache && iGlyphTextCache->empty())
			*iGlyphTextCache = glyphText;
		return from_device_units(size(glyphText.extents().cx, glyphText.extents().cy));
	}

	size graphics_context::multiline_text_extent(const text& aText, const font& aFont, bool aUseCache) const
	{
		return multiline_text_extent(aText, aFont, 0, aUseCache);
	}

	size graphics_context::multiline_text_extent(const text& aText, const font& aFont, dimension aMaxWidth, bool aUseCache) const
	{
		const auto& glyphText = aUseCache && !iGlyphTextCache->empty() ? *iGlyphTextCache : to_glyph_text(aText.begin(), aText.end(), aFont);
		if (aUseCache && iGlyphTextCache->empty())
			*iGlyphTextCache = glyphText;
		typedef std::pair<glyph_text::const_iterator, glyph_text::const_iterator> line_t;
		typedef std::vector<line_t> lines_t;
		lines_t lines;
		std::array<glyph, 2> delimeters = { glyph(text_direction::Whitespace, '\r'), glyph(text_direction::Whitespace, '\n') };
		neolib::tokens(glyphText.cbegin(), glyphText.cend(), delimeters.begin(), delimeters.end(), lines, 0, false);
		size result;
		for (lines_t::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			if (aMaxWidth == 0)
			{
				size lineExtent = from_device_units(glyph_text::extents(aFont, i->first, i->second));
				result.cx = std::max(result.cx, lineExtent.cx);
				result.cy += lineExtent.cy;
			}
			else
			{
				glyph_text::const_iterator next = i->first;
				glyph_text::const_iterator lineStart = next;
				glyph_text::const_iterator lineEnd = i->second;
				dimension maxWidth = to_device_units(size(aMaxWidth, 0)).cx;
				dimension lineWidth = 0;
				bool gotLine = false;
				while(next != i->second)
				{
					if (lineWidth + next->extents().cx > maxWidth)
					{
						std::pair<glyph_text::const_iterator, glyph_text::const_iterator> wordBreak = glyphText.word_break(lineStart, next);
						lineWidth -= glyph_text::extents(aFont, wordBreak.first, next).cx;
						lineEnd = wordBreak.first;
						next = wordBreak.second;
						if (lineEnd == next)
						{
							while(lineEnd != i->second && (lineEnd + 1)->source() == wordBreak.first->source())
								++lineEnd;
							next = lineEnd;
						}
						gotLine = true;
					}
					else
					{	
						lineWidth += next->extents().cx;
						++next;
					}
					if (gotLine || next == i->second)
					{
						result.cx = std::max(result.cx, from_device_units(size(lineWidth, 0)).cx);
						result.cy += from_device_units(glyph_text::extents(aFont, i->first, i->second)).cy;
						lineStart = next;
						lineEnd = i->second;
						lineWidth = 0;
						gotLine = false;
					}
				}
			}
		}
		if (result.cy == 0)
			result.cy = from_device_units(size(0, aFont.height())).cy;
		return result;
	}

	glyph_text graphics_context::to_glyph_text(const text& aText, const font& aFont) const
	{
		return to_glyph_text(aText.begin(), aText.end(), aFont);
	}

	bool graphics_context::is_text_left_to_right(const text& aText, const font& aFont, bool aUseCache) const
	{
		const auto& glyphText = aUseCache && !iGlyphTextCache->empty() ? *iGlyphTextCache : to_glyph_text(aText.begin(), aText.end(), aFont);
		if (aUseCache && iGlyphTextCache->empty())
			*iGlyphTextCache = glyphText;
		return glyph_text_direction(glyphText.cbegin(), glyphText.cend()) == text_direction::LTR;
	}

	bool graphics_context::is_text_right_to_left(const text& aText, const font& aFont, bool aUseCache) const
	{
		return !is_text_left_to_right(aText, aFont, aUseCache);
	}

	void graphics_context::draw_text(const point& aPoint, const text& aText, const font& aFont, const colour& aColour, bool aUseCache) const
	{
		glyph_drawing gd(*this);
		draw_text(aPoint, aText.begin(), aText.end(), aFont, aColour, aUseCache);
	}

	void graphics_context::draw_text(const point& aPoint, text::const_iterator aTextBegin, text::const_iterator aTextEnd, const font& aFont, const colour& aColour, bool aUseCache) const
	{
		glyph_drawing gd(*this);
		const auto& glyphText = aUseCache && !iGlyphTextCache->empty() ? *iGlyphTextCache : to_glyph_text(aTextBegin, aTextEnd, aFont);
		if (aUseCache && iGlyphTextCache->empty())
			*iGlyphTextCache = glyphText;
		point pt = aPoint;
		for (glyph_text::const_iterator i = glyphText.cbegin(); i != glyphText.cend(); ++i)
		{
			draw_glyph(pt, *i, aFont, aColour);
			pt.x += from_device_units(size(i->extents().cx, 0.0)).cx;
		}
	}

	void graphics_context::draw_multiline_text(const point& aPoint, const text& aText, const font& aFont, const colour& aColour, alignment aAlignment, bool aUseCache) const
	{
		glyph_drawing gd(*this);
		draw_multiline_text(aPoint, aText, aFont, 0, aColour, aAlignment, aUseCache);
	}

	void graphics_context::draw_multiline_text(const point& aPoint, const text& aText, const font& aFont, dimension aMaxWidth, const colour& aColour, alignment aAlignment, bool aUseCache) const
	{
		glyph_drawing gd(*this);
		const auto& glyphText = aUseCache && !iGlyphTextCache->empty() ? *iGlyphTextCache : to_glyph_text(aText.begin(), aText.end(), aFont);
		if (aUseCache && iGlyphTextCache->empty())
			*iGlyphTextCache = glyphText;
		typedef std::pair<glyph_text::const_iterator, glyph_text::const_iterator> line_t;
		typedef std::vector<line_t> lines_t;
		lines_t lines;
		std::array<glyph, 2> delimeters = { glyph(text_direction::Whitespace, '\r'), glyph(text_direction::Whitespace, '\n') };
		neolib::tokens(glyphText.cbegin(), glyphText.cend(), delimeters.begin(), delimeters.end(), lines, 0, false);
		point pos = aPoint;
		size textExtent = multiline_text_extent(aText, aFont, aMaxWidth, aUseCache);
		for (lines_t::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			if (aMaxWidth == 0)
			{
				point linePos = pos;
				size lineExtent = from_device_units(glyph_text::extents(aFont, i->first, i->second));
				if (glyph_text_direction(i->first, i->second) == text_direction::RTL)
					linePos.x += textExtent.cx - lineExtent.cx;
				draw_glyph_text(linePos, glyphText, aFont, aColour);
				pos.y += lineExtent.cy;
			}
			else
			{
				glyph_text::const_iterator next = i->first;
				glyph_text::const_iterator lineStart = next;
				glyph_text::const_iterator lineEnd = i->second;
				dimension maxWidth = to_device_units(size(aMaxWidth, 0)).cx;
				dimension lineWidth = 0;
				bool gotLine = false;
				while(next != i->second)
				{
					if (lineWidth + next->extents().cx > maxWidth)
					{
						std::pair<glyph_text::const_iterator, glyph_text::const_iterator> wordBreak = glyphText.word_break(lineStart, next);
						lineWidth -= glyph_text::extents(aFont, wordBreak.first, next).cx;
						lineEnd = wordBreak.first;
						next = wordBreak.second;
						if (lineEnd == next)
						{
							while(lineEnd != i->second && (lineEnd + 1)->source() == wordBreak.first->source())
								++lineEnd;
							next = lineEnd;
						}
						gotLine = true;
					}
					else
					{
						lineWidth += next->extents().cx;
						++next;
					}
					if (gotLine || next == i->second)
					{
						point linePos = pos;
						if (aAlignment == alignment::Left && glyph_text_direction(lineStart, next) == text_direction::RTL ||
							aAlignment == alignment::Right && glyph_text_direction(lineStart, next) == text_direction::LTR)
							linePos.x += textExtent.cx - from_device_units(size(lineWidth, 0)).cx;
						else if (aAlignment == alignment::Centre)
							linePos.x += (textExtent.cx - from_device_units(size(lineWidth, 0)).cx) / 2;
						draw_glyph_text(linePos, lineStart, lineEnd, aFont, aColour);
						pos.y += glyph_text::extents(aFont, lineStart, lineEnd).cy;
						lineStart = next;
						lineEnd = i->second;
						lineWidth = 0;
						gotLine = false;
					}
				}
			}
		}
	}

	void graphics_context::draw_glyph_text(const point& aPoint, const glyph_text& aText, const font& aFont, const colour& aColour) const
	{
		glyph_drawing gd(*this);
		draw_glyph_text(aPoint, aText.cbegin(), aText.cend(), aFont, aColour);
	}

	void graphics_context::draw_glyph_text(const point& aPoint, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const font& aFont, const colour& aColour) const
	{
		glyph_drawing gd(*this);
		point pos = aPoint;
		for (glyph_text::const_iterator i = aTextBegin; i != aTextEnd; ++i)
		{
			draw_glyph(pos + i->offset(), *i, aFont, aColour);
			pos.x += i->extents().cx;
		}
	}

	size graphics_context::extents() const
	{
		return iExtents;
	}

	dimension graphics_context::horizontal_dpi() const
	{
		return iSurface.horizontal_dpi();
	}
	
	dimension graphics_context::vertical_dpi() const
	{
		return iSurface.vertical_dpi();
	}

	dimension graphics_context::em_size() const
	{
		return static_cast<dimension>(iDefaultFont.size() / 72.0 * horizontal_dpi());
	}

	const i_device_metrics& graphics_context::device_metrics() const
	{
		return *this;
	}

	units_e graphics_context::units() const
	{
		return iUnitsContext.units();
	}

	units_e graphics_context::set_units(units_e aUnits) const
	{
		return iUnitsContext.set_units(aUnits);
	}

	i_native_font_face& graphics_context::to_native_font_face(const font& aFont)
	{
		return aFont.native_font_face();
	}

	// Forward on to native..

	void graphics_context::flush() const
	{
	}

	void graphics_context::scissor_on(const rect& aRect) const
	{
		iNativeGraphicsContext->scissor_on(to_device_units(aRect) + iOrigin);
	}

	void graphics_context::scissor_off() const
	{
		iNativeGraphicsContext->scissor_off();
	}

	void graphics_context::clip_to(const rect& aRect) const
	{
		iNativeGraphicsContext->clip_to(to_device_units(aRect) + iOrigin);
	}

	void graphics_context::clip_to(const path& aPath, dimension aPathOutline) const
	{
		path path = to_device_units(aPath);
		path.set_shape(path::ConvexPolygon);
		path.set_position(path.position() + iOrigin);
		iNativeGraphicsContext->clip_to(path, aPathOutline);
	}

	void graphics_context::reset_clip() const
	{
		iNativeGraphicsContext->reset_clip();
	}

	smoothing_mode_e graphics_context::smoothing_mode() const
	{
		return iNativeGraphicsContext->smoothing_mode();
	}

	smoothing_mode_e graphics_context::set_smoothing_mode(smoothing_mode_e aSmoothingMode) const
	{
		return iNativeGraphicsContext->set_smoothing_mode(aSmoothingMode);
	}

	void graphics_context::push_logical_operation(logical_operation_e aLogicalOperation) const
	{
		iNativeGraphicsContext->push_logical_operation(aLogicalOperation);
	}

	void graphics_context::pop_logical_operation() const
	{
		iNativeGraphicsContext->pop_logical_operation();
	}

	void graphics_context::line_stipple_on(uint32_t aFactor, uint16_t aPattern) const
	{
		iNativeGraphicsContext->line_stipple_on(aFactor, aPattern);
	}

	void graphics_context::line_stipple_off() const
	{
		iNativeGraphicsContext->line_stipple_off();
	}

	void graphics_context::clear(const colour& aColour) const
	{
		if (origin() == point{} && extents() == iSurface.extents())
			iNativeGraphicsContext->clear(aColour);
		else
			fill_solid_rect(rect{origin(), extents()}, aColour);
	}

	glyph_text graphics_context::to_glyph_text(text::const_iterator aTextBegin, text::const_iterator aTextEnd, const font& aFont) const
	{
		return iNativeGraphicsContext->to_glyph_text(aTextBegin, aTextEnd, aFont);
	}

	void graphics_context::draw_glyph(const point& aPoint, const glyph& aGlyph, const font& aFont, const colour& aColour) const
	{
		glyph_drawing gd(*this);
		iNativeGraphicsContext->draw_glyph(to_device_units(aPoint) + iOrigin, aGlyph, aFont, aColour);
	}

	void graphics_context::set_glyph_text_cache(glyph_text& aGlyphTextCache) const
	{
		iGlyphTextCache = &aGlyphTextCache;
	}

	void graphics_context::reset_glyph_text_cache() const
	{
		iGlyphTextCache = 0;
	}

	void graphics_context::draw_texture(const point& aPoint, const i_texture& aTexture) const
	{
		iNativeGraphicsContext->draw_texture(to_device_units(aPoint) + iOrigin, aTexture, rect(point(0.0, 0.0), aTexture.extents()));
	}

	void graphics_context::draw_texture(const point& aPoint, const i_texture& aTexture, const rect& aTextureRect) const
	{
		iNativeGraphicsContext->draw_texture(to_device_units(aPoint) + iOrigin, aTexture, aTextureRect);
	}
}