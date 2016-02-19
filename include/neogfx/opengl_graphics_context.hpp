// opengl_graphics_context.hpp
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
#include <GL/glew.h>
#include <GL/GL.h>
#ifdef u8
#undef u8
#include <hb.h>
#include <hb-ft.h>
#include <hb-ucdn\ucdn.h>
#define u8
#else
#include <hb.h>
#include <hb-ft.h>
#include <hb-unicode.h>
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include "opengl_error.hpp"
#include "i_native_graphics_context.hpp"

namespace neogfx
{
	class i_rendering_engine;

	class opengl_graphics_context : public i_native_graphics_context
	{
	private:
		class scoped_anti_alias
		{
		public:
			scoped_anti_alias(opengl_graphics_context& aParent, smoothing_mode_e aNewSmoothingMode) : iParent(aParent), iOldSmoothingMode(aParent.smoothing_mode())
			{
				iParent.set_smoothing_mode(aNewSmoothingMode);
			}
			~scoped_anti_alias()
			{
				iParent.set_smoothing_mode(iOldSmoothingMode);
			}
		private:
			opengl_graphics_context& iParent;
			smoothing_mode_e iOldSmoothingMode;
		};
		class disable_anti_alias : public scoped_anti_alias
		{
		public:
			disable_anti_alias(opengl_graphics_context& aParent) : scoped_anti_alias(aParent, SmoothingModeNone)
			{
			}
		};
		typedef std::array<double, 3> vertex;
	public:
		opengl_graphics_context(i_rendering_engine& aRenderingEngine);
		opengl_graphics_context(const opengl_graphics_context& aOther);
		~opengl_graphics_context();
	public:
		virtual rect rendering_area(bool aConsiderScissor = true) const = 0;
	public:
		virtual void flush();
		virtual void scissor_on(const rect& aRect);
		virtual void scissor_off();
		virtual optional_rect scissor_rect() const;
		virtual void clip_to(const rect& aRect);
		virtual void clip_to(const path& aPath, dimension aPathOutline);
		virtual void reset_clip();
		virtual smoothing_mode_e smoothing_mode() const;
		virtual smoothing_mode_e set_smoothing_mode(smoothing_mode_e aSmoothingMode);
		virtual void push_logical_operation(logical_operation_e aLogicalOperation);
		virtual void pop_logical_operation();
		virtual void line_stipple_on(uint32_t aFactor, uint16_t aPattern);
		virtual void line_stipple_off();
		virtual void clear(const colour& aColour);
		virtual void set_pixel(const point& aPoint, const colour& aColour);
		virtual void draw_pixel(const point& aPoint, const colour& aColour);
		virtual void draw_line(const point& aFrom, const point& aTo, const pen& aPen);
		virtual void draw_rect(const rect& aRect, const pen& aPen);
		virtual void draw_circle(const point& aCentre, dimension aRadius, const pen& aPen);
		virtual void draw_path(const path& aPath, const pen& aPen);
		virtual void fill_solid_rect(const rect& aRect, const colour& aColour);
		virtual void fill_gradient_rect(const rect& aRect, const gradient& aGradient);	
		virtual void fill_solid_circle(const point& aCentre, dimension aRadius, const colour& aColour);
		virtual void fill_and_draw_path(const path& aPath, const colour& aFillColour, const pen& aPen);
		virtual glyph_text to_glyph_text(text::const_iterator aTextBegin, text::const_iterator aTextEnd, const font& aFont) const;
		virtual void begin_drawing_glyphs();
		virtual void draw_glyph(const point& aPoint, const glyph& aGlyph, const font& aFont, const colour& aColour);
		virtual void end_drawing_glyphs();
		virtual void draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect);
	private:
		void apply_scissor();
		void apply_logical_operation();
		vertex to_shader_vertex(const point& aPoint) const;
		glyph_text::container to_glyph_text_impl(text::const_iterator aTextBegin, text::const_iterator aTextEnd, const font& aFont, bool& aFallbackFontNeeded) const;
	private:
		i_rendering_engine& iRenderingEngine;
		smoothing_mode_e iSmoothingMode; 
		std::vector<logical_operation_e> iLogicalOperationStack;
		uint32_t iClipCounter;
		std::vector<rect> iScissorRects;
		mutable std::vector<vertex> iVertices;
		mutable std::vector<GLdouble> iTextureCoords;
		mutable std::vector<GLshort> iIndices;
		mutable std::vector<std::array<GLdouble, 4>> iColours;
		mutable std::vector<std::array<GLdouble, 2>> iShifts;
		typedef std::vector<std::pair<std::string::size_type, std::u32string::size_type>> cluster_map_t;
		mutable cluster_map_t iClusterMap;
		mutable std::vector<text_direction> iTextDirections;
		mutable std::vector<std::tuple<uint32_t*, uint32_t*, text_direction, hb_script_t>> iRuns;
		GLint iPreviousTexture;
		GLuint iActiveGlyphTexture;
		bool iLineStippleActive;
	};
}