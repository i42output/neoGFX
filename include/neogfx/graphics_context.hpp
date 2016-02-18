// graphics_context.hpp
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
#include <memory>
#include "primitives.hpp"
#include "geometry.hpp"
#include "path.hpp"
#include "pen.hpp"
#include "font.hpp"
#include "i_texture.hpp"

namespace neogfx
{
	enum colour_format_e
	{
		ColourFormatRGBA8
	};

	enum smoothing_mode_e
	{
		SmoothingModeNone,
		SmoothingModeAntiAlias
	};

	enum logical_operation_e
	{
		LogicalNone,
		LogicalXor
	};

	class i_surface;
	class i_widget;
	class i_native_graphics_context;

	class graphics_context : public i_device_metrics, public i_units_context
	{
		// types
	private:
		struct glyph_drawing;
		friend class generic_surface;
		// construction
	public:
		graphics_context(const i_surface& aSurface);
		graphics_context(const i_surface& aSurface, const font& aDefaultFont);
		graphics_context(const i_widget& aWidget);
		graphics_context(const graphics_context& aOther);
		virtual ~graphics_context();
		// operations
	public:
		delta to_device_units(const delta& aValue) const;
		size to_device_units(const size& aValue) const;
		point to_device_units(const point& aValue) const;
		rect to_device_units(const rect& aValue) const;
		path to_device_units(const path& aValue) const;
		delta from_device_units(const delta& aValue) const;
		size from_device_units(const size& aValue) const;
		point from_device_units(const point& aValue) const;
		rect from_device_units(const rect& aValue) const;
		path from_device_units(const path& aValue) const;
		void set_default_font(const font& aDefaultFont) const;
		void set_extents(const size& aExtents) const;
		void set_origin(const point& aOrigin) const;
		const point& origin() const;
		void flush() const;
		void scissor_on(const rect& aRect) const;
		void scissor_off() const;
		void clip_to(const rect& aRect) const;
		void clip_to(const path& aPath, dimension aPathOutline = 0) const;
		void reset_clip() const;
		smoothing_mode_e smoothing_mode() const;
		smoothing_mode_e set_smoothing_mode(smoothing_mode_e aSmoothingMode) const;
		void push_logical_operation(logical_operation_e aLogicalOperation) const;
		void pop_logical_operation() const;
		void line_stipple_on(uint32_t aFactor, uint16_t aPattern) const;
		void line_stipple_off() const;
		void clear(const colour& aColour) const;
		void set_pixel(const point& aPoint, const colour& aColour) const;
		void draw_pixel(const point& aPoint, const colour& aColour) const;
		void draw_line(const point& aFrom, const point& aTo, const pen& aPen) const;
		void draw_rect(const rect& aRect, const pen& aPen) const;
		void draw_circle(const point& aCentre, dimension aRadius, const pen& aPen) const;
		void draw_path(const path& aPath, const pen& aPen) const;
		void draw_focus_rect(const rect& aRect) const;
		void fill_solid_rect(const rect& aRect, const colour& aColour) const;
		void fill_gradient_rect(const rect& aRect, const gradient& aGradient) const;
		void fill_solid_circle(const point& aCentre, dimension aRadius, const colour& aColour) const;
		void fill_and_draw_path(const path& aPath, const colour& aFillColour, const pen& aOutlinePen) const;
		size text_extent(const text& aText, const font& aFont, bool aUseCache = false) const;
		size text_extent(text::const_iterator aTextBegin, text::const_iterator aTextEnd, const font& aFont, bool aUseCache = false) const;
		size multiline_text_extent(const text& aText, const font& aFont, bool aUseCache = false) const;
		size multiline_text_extent(const text& aText, const font& aFont, dimension aMaxWidth, bool aUseCache = false) const;
		glyph_text to_glyph_text(const text& aText, const font& aFont) const;
		glyph_text to_glyph_text(text::const_iterator aTextBegin, text::const_iterator aTextEnd, const font& aFont) const;
		bool is_text_left_to_right(const text& aText, const font& aFont, bool aUseCache = false) const;
		bool is_text_right_to_left(const text& aText, const font& aFont, bool aUseCache = false) const;
		void draw_text(const point& aPoint, const text& aText, const font& aFont, const colour& aColour, bool aUseCache = false) const;
		void draw_text(const point& aPoint, text::const_iterator aTextBegin, text::const_iterator aTextEnd, const font& aFont, const colour& aColour, bool aUseCache = false) const;
		void draw_multiline_text(const point& aPoint, const text& aText, const font& aFont, const colour& aColour, alignment aAlignment = alignment::Left, bool aUseCache = false) const;
		void draw_multiline_text(const point& aPoint, const text& aText, const font& aFont, dimension aMaxWidth, const colour& aColour, alignment aAlignment = alignment::Left, bool aUseCache = false) const;
		void draw_glyph_text(const point& aPoint, const glyph_text& aText, const font& aFont, const colour& aColour) const;
		void draw_glyph_text(const point& aPoint, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const font& aFont, const colour& aColour) const;
		void draw_glyph(const point& aPoint, const glyph& aGlyph, const font& aFont, const colour& aColour) const;
		void set_glyph_text_cache(glyph_text& aGlyphTextCache) const;
		void reset_glyph_text_cache() const;
		void draw_texture(const point& aPoint, const i_texture& aTexture) const;
		void draw_texture(const point& aPoint, const i_texture& aTexture, const rect& aTextureRect) const;
		// implementation
		// from i_device_metrics
	public:
		virtual size extents() const;
		virtual dimension horizontal_dpi() const;
		virtual dimension vertical_dpi() const;
		virtual dimension em_size() const;
		// from i_units_context
		virtual const i_device_metrics& device_metrics() const;
		virtual units_e units() const;
		virtual units_e set_units(units_e aUnits) const;
		// helpers
	protected:
		static i_native_font_face& to_native_font_face(const font& aFont);
		// attributes
	private:
		const i_surface& iSurface;
		std::unique_ptr<i_native_graphics_context> iNativeGraphicsContext;
		units_context iUnitsContext;
		mutable font iDefaultFont;
		mutable point iOrigin;
		mutable size iExtents;
		mutable glyph_text* iGlyphTextCache;
		mutable uint32_t iDrawingGlyphs;
	};
}