// i_native_graphics_context.hpp
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
#include <neogfx/core/path.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include "../../hid/native/i_native_surface.hpp"

namespace neogfx
{
	class i_native_graphics_context
	{
	public:
		struct texture_not_resident : std::runtime_error { texture_not_resident() : std::runtime_error("neogfx::i_native_graphics_context::texture_not_resident") {} };
		struct password_not_set : std::logic_error { password_not_set() : std::logic_error("neogfx::font_info::password_not_set") {} };
	public:
		virtual ~i_native_graphics_context() {}
		virtual std::unique_ptr<i_native_graphics_context> clone() const = 0;
	public:
		virtual const i_native_surface& surface() const = 0;
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
		virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) = 0;
		virtual const vector4& logical_coordinates() const = 0;
		virtual void set_logical_coordinates(const vector4& aCoordinates) const = 0;
		virtual void flush() = 0;
		virtual void scissor_on(const rect& aRect) = 0;
		virtual void scissor_off() = 0;
		virtual optional_rect scissor_rect() const = 0;
		virtual void clip_to(const rect& aRect) = 0;
		virtual void clip_to(const path& aPath, dimension aPathOutline = 0) = 0;
		virtual void reset_clip() = 0;
		virtual neogfx::smoothing_mode smoothing_mode() const = 0;
		virtual neogfx::smoothing_mode set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode) = 0;
		virtual void push_logical_operation(logical_operation aLogicalOperation) = 0;
		virtual void pop_logical_operation() = 0;
		virtual void line_stipple_on(uint32_t aFactor, uint16_t aPattern) = 0;
		virtual void line_stipple_off() = 0;
		virtual bool is_subpixel_rendering_on() const = 0;
		virtual void subpixel_rendering_on() = 0;
		virtual void subpixel_rendering_off() = 0;
		virtual void clear(const colour& aColour) = 0;
		virtual void set_pixel(const point& aPoint, const colour& aColour) = 0;
		virtual void draw_pixel(const point& aPoint, const colour& aColour) = 0;
		virtual void draw_line(const point& aFrom, const point& aTo, const pen& aPen) = 0;
		virtual void draw_rect(const rect& aRect, const pen& aPen) = 0;
		virtual void draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen) = 0;
		virtual void draw_circle(const point& aCentre, dimension aRadius, const pen& aPen) = 0;
		virtual void draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen) = 0;
		virtual void draw_path(const path& aPath, const pen& aPen) = 0;
		virtual void fill_rect(const rect& aRect, const fill& aFill) = 0;
		virtual void fill_rounded_rect(const rect& aRect, dimension aRadius, const fill& aFill) = 0;
		virtual void fill_circle(const point& aCentre, dimension aRadius, const fill& aFill) = 0;
		virtual void fill_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const fill& aFill) = 0;
		virtual void fill_shape(const point& aCentre, const vertex_list2& aVertices, const fill& aFill) = 0;
		virtual void fill_path(const path& aPath, const fill& aFill) = 0;
		virtual glyph_text to_glyph_text(string::const_iterator aTextBegin, string::const_iterator aTextEnd, const font& aFont) const = 0;
		virtual glyph_text to_glyph_text(string::const_iterator aTextBegin, string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const = 0;
		virtual glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont) const = 0;
		virtual glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const = 0;
		virtual void set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix = '&') = 0;
		virtual void unset_mnemonic() = 0;
		virtual bool mnemonics_shown() const = 0;
		virtual bool password() const = 0;
		virtual const std::string& password_mask() const = 0;
		virtual void set_password(bool aPassword, const std::string& aMask = "\xE2\x97\x8F") = 0;
		virtual void begin_drawing_glyphs() = 0;
		virtual size draw_glyph(const point& aPoint, const glyph& aGlyph, const font& aFont, const colour& aColour) = 0;
		virtual void end_drawing_glyphs() = 0;
		virtual void draw_texture(const texture_map& aTextureMap, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour, shader_effect aShaderEffect) = 0;
	};
}