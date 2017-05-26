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

#include <neogfx/neogfx.hpp>
#include "opengl.hpp"
#ifdef u8
#undef u8
#include <hb.h>
#include <hb-ft.h>
#include <hb-ucdn\ucdn.h>
#define u8
#else
#include <hb.h>
#include <hb-ft.h>
#include <hb-ucdn\ucdn.h>
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include "opengl_error.hpp"
#include "i_native_graphics_context.hpp"
#include "opengl_helpers.hpp"

namespace neogfx
{
	class i_rendering_engine;

	class opengl_graphics_context : public i_native_graphics_context
	{
	private:
		class scoped_anti_alias
		{
		public:
			scoped_anti_alias(opengl_graphics_context& aParent, neogfx::smoothing_mode aNewSmoothingMode) : iParent(aParent), iOldSmoothingMode(aParent.smoothing_mode())
			{
				iParent.set_smoothing_mode(aNewSmoothingMode);
			}
			~scoped_anti_alias()
			{
				iParent.set_smoothing_mode(iOldSmoothingMode);
			}
		private:
			opengl_graphics_context& iParent;
			neogfx::smoothing_mode iOldSmoothingMode;
		};
		class disable_anti_alias : public scoped_anti_alias
		{
		public:
			disable_anti_alias(opengl_graphics_context& aParent) : scoped_anti_alias(aParent, neogfx::smoothing_mode::None)
			{
			}
		};
		typedef xyz vertex;
		class glyph_shapes;
	public:
		opengl_graphics_context(i_rendering_engine& aRenderingEngine, const i_native_surface& aSurface);
		opengl_graphics_context(i_rendering_engine& aRenderingEngine, const i_native_surface& aSurface, const i_widget& aWidget);
		opengl_graphics_context(const opengl_graphics_context& aOther);
		~opengl_graphics_context();
	public:
		virtual const i_native_surface& surface() const;
		virtual rect rendering_area(bool aConsiderScissor = true) const = 0;
	public:
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const;
		virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem);
		virtual const std::pair<vec2, vec2>& logical_coordinates() const;
		virtual void set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates) const;
		virtual void flush();
		virtual void scissor_on(const rect& aRect);
		virtual void scissor_off();
		virtual optional_rect scissor_rect() const;
		virtual void clip_to(const rect& aRect);
		virtual void clip_to(const path& aPath, dimension aPathOutline);
		virtual void reset_clip();
		virtual neogfx::smoothing_mode smoothing_mode() const;
		virtual neogfx::smoothing_mode set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode);
		virtual void push_logical_operation(logical_operation aLogicalOperation);
		virtual void pop_logical_operation();
		virtual void line_stipple_on(uint32_t aFactor, uint16_t aPattern);
		virtual void line_stipple_off();
		virtual bool is_subpixel_rendering_on() const;
		virtual void subpixel_rendering_on();
		virtual void subpixel_rendering_off();
		virtual void clear(const colour& aColour);
		virtual void set_pixel(const point& aPoint, const colour& aColour);
		virtual void draw_pixel(const point& aPoint, const colour& aColour);
		virtual void draw_line(const point& aFrom, const point& aTo, const pen& aPen);
		virtual void draw_rect(const rect& aRect, const pen& aPen);
		virtual void draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen);
		virtual void draw_circle(const point& aCentre, dimension aRadius, const pen& aPen, angle aStartAngle);
		virtual void draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen);
		virtual void draw_path(const path& aPath, const pen& aPen);
		virtual void draw_shape(const vec2_list& aVertices, const pen& aPen);
		virtual void fill_rect(const rect& aRect, const fill& aFill);
		virtual void fill_rounded_rect(const rect& aRect, dimension aRadius, const fill& aFill);
		virtual void fill_circle(const point& aCentre, dimension aRadius, const fill& aFill);
		virtual void fill_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const fill& aFill);
		virtual void fill_path(const path& aPath, const fill& aFill);
		virtual void fill_shape(const vec2_list& aVertices, const fill& aFill);
		virtual glyph_text to_glyph_text(string::const_iterator aTextBegin, string::const_iterator aTextEnd, const font& aFont) const;
		virtual glyph_text to_glyph_text(string::const_iterator aTextBegin, string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const;
		virtual glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont) const;
		virtual glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const;
		virtual void set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix = '&');
		virtual void unset_mnemonic();
		virtual bool mnemonics_shown() const;
		virtual bool password() const;
		virtual const std::string& password_mask() const;
		virtual void set_password(bool aPassword, const std::string& aMask = "\xE2\x97\x8F");
		virtual void begin_drawing_glyphs();
		virtual size draw_glyph(const point& aPoint, const glyph& aGlyph, const font& aFont, const colour& aColour);
		virtual void end_drawing_glyphs();
		virtual void draw_texture(const texture_map& aTextureMap, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour, shader_effect aShaderEffect);
	private:
		void apply_scissor();
		void apply_logical_operation();
		void gradient_on(const gradient& aGradient, const rect& aBoundingBox);
		void gradient_off();
		vertex to_shader_vertex(const point& aPoint) const;
		glyph_text::container to_glyph_text_impl(string::const_iterator aTextBegin, string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const;
		glyph_text::container to_glyph_text_impl(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const;
	private:
		i_rendering_engine& iRenderingEngine;
		const i_native_surface& iSurface;
		neogfx::logical_coordinate_system iSavedCoordinateSystem;
		neogfx::logical_coordinate_system iLogicalCoordinateSystem;
		mutable std::pair<vec2, vec2> iLogicalCoordinates;
		neogfx::smoothing_mode iSmoothingMode; 
		std::vector<logical_operation> iLogicalOperationStack;
		opengl_standard_vertex_arrays iVertexArrays;
		std::list<use_shader_program> iShaderProgramStack;
		uint32_t iClipCounter;
		std::vector<rect> iScissorRects;
		struct cluster
		{
			std::string::size_type from;
			glyph::flags_e flags;
		};
		typedef std::vector<cluster> cluster_map_t;
		mutable cluster_map_t iClusterMap;
		mutable std::vector<character_type> iTextDirections;
		mutable std::u32string iCodePointsBuffer;
		typedef std::tuple<const char32_t*, const char32_t*, text_direction, bool, hb_script_t> glyph_run;
		typedef std::vector<glyph_run> run_list;
		mutable run_list iRuns;
		mutable glyph_text::container iGlyphTextResult;
		mutable glyph_text::container iGlyphTextResult2;
		GLint iPreviousTexture;
		GLuint iActiveGlyphTexture;
		bool iLineStippleActive;
		bool iSubpixelRendering;
		boost::optional<std::pair<bool, char>> iMnemonic;
		mutable boost::optional<std::string> iPassword;
		std::vector<float> iGradientStopPositions;
		std::vector<std::array<float, 4>> iGradientStopColours;
		font iLastDrawGlyphFallbackFont;
		boost::optional<uint8_t> iLastDrawGlyphFallbackFontIndex;
	};
}