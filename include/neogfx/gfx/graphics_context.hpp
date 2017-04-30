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

#include <neogfx/neogfx.hpp>
#include <memory>
#include <neogfx/core/primitives.hpp>
#include <neogfx/core/path.hpp>
#include <neogfx/game/i_shape.hpp>
#include <neogfx/gfx/pen.hpp>
#include <neogfx/gfx/text/font.hpp>

namespace neogfx
{
	typedef neolib::variant<colour, gradient> fill;

	enum class logical_coordinate_system
	{
		Specified,
		AutomaticGui,
		AutomaticGame
	};

	enum class colour_format
	{
		RGBA8
	};

	enum class smoothing_mode
	{
		None,
		AntiAlias
	};

	enum class logical_operation
	{
		None,
		Xor
	};

	enum class shader_effect
	{
		None,
		Monochrome
	};

	typedef basic_vector<vector2, 4> texture_map2;
	typedef basic_vector<vector3, 4> texture_map3;

	typedef texture_map2 texture_map;

	class i_surface;
	class i_texture;
	class i_widget;
	class i_native_graphics_context;

	class graphics_context : public i_device_metrics, public i_units_context
	{
		// types
	public:
		struct glyph_drawing
		{
			const graphics_context& iParent;
			glyph_drawing(const graphics_context& aParent);
			~glyph_drawing();
		};
	private:
		friend class generic_surface;
		// construction
	public:
		graphics_context(const i_surface& aSurface);
		graphics_context(const i_surface& aSurface, const font& aDefaultFont);
		graphics_context(const i_widget& aWidget);
		graphics_context(const graphics_context& aOther);
		virtual ~graphics_context();
	public:
		const i_surface& surface() const;
		// operations
	public:
		delta to_device_units(const delta& aValue) const;
		size to_device_units(const size& aValue) const;
		point to_device_units(const point& aValue) const;
		vec2 to_device_units(const vec2& aValue) const;
		rect to_device_units(const rect& aValue) const;
		texture_map to_device_units(const texture_map& aValue) const;
		path to_device_units(const path& aValue) const;
		delta from_device_units(const delta& aValue) const;
		size from_device_units(const size& aValue) const;
		point from_device_units(const point& aValue) const;
		rect from_device_units(const rect& aValue) const;
		texture_map from_device_units(const texture_map& aValue) const;
		path from_device_units(const path& aValue) const;
		neogfx::logical_coordinate_system logical_coordinate_system() const;
		void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) const;
		const vector4& logical_coordinates() const;
		void set_logical_coordinates(const vector4& aCoordinates) const;
		void set_default_font(const font& aDefaultFont) const;
		void set_extents(const size& aExtents) const;
		void set_origin(const point& aOrigin) const;
		point origin() const;
		void flush() const;
		void scissor_on(const rect& aRect) const;
		void scissor_off() const;
		void clip_to(const rect& aRect) const;
		void clip_to(const path& aPath, dimension aPathOutline = 0) const;
		void reset_clip() const;
		neogfx::smoothing_mode smoothing_mode() const;
		neogfx::smoothing_mode set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode) const;
		void push_logical_operation(logical_operation aLogicalOperation) const;
		void pop_logical_operation() const;
		void line_stipple_on(uint32_t aFactor, uint16_t aPattern) const;
		void line_stipple_off() const;
		bool is_subpixel_rendering_on() const;
		void subpixel_rendering_on() const;
		void subpixel_rendering_off() const;
		void clear(const colour& aColour) const;
		void set_pixel(const point& aPoint, const colour& aColour) const;
		void draw_pixel(const point& aPoint, const colour& aColour) const;
		void draw_line(const point& aFrom, const point& aTo, const pen& aPen) const;
		void draw_rect(const rect& aRect, const pen& aPen, const fill& = fill{}) const;
		void draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen, const fill& = fill{}) const;
		void draw_circle(const point& aCentre, dimension aRadius, const pen& aPen, const fill& = fill{}, angle aStartAngle = 0.0) const;
		void draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen, const fill& = fill{}) const;
		void draw_path(const path& aPath, const pen& aPen, const fill& = fill{}) const;
		void draw_shape(const i_shape& aShape, const pen& aPen, const fill& = fill{}) const;
		void draw_shape(const vec2_list& aVertices, const pen& aPen) const;
		void draw_shape(const vec3_list& aVertices, const pen& aPen) const;
		void draw_focus_rect(const rect& aRect) const;
		void fill_rect(const rect& aRect, const fill& aFill) const;
		void fill_rounded_rect(const rect& aRect, dimension aRadius, const fill& aFill) const;
		void fill_circle(const point& aCentre, dimension aRadius, const fill& aFill) const;
		void fill_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const fill& aFill) const;
		void fill_path(const path& aPath, const fill& aFill) const;
		void fill_shape(const vec2_list& aVertices, const fill& aFill) const;
		void fill_shape(const vec3_list& aVertices, const fill& aFill) const;
		size text_extent(const string& aText, const font& aFont, bool aUseCache = false) const;
		size text_extent(string::const_iterator aTextBegin, string::const_iterator aTextEnd, const font& aFont, bool aUseCache = false) const;
		size multiline_text_extent(const string& aText, const font& aFont, bool aUseCache = false) const;
		size multiline_text_extent(const string& aText, const font& aFont, dimension aMaxWidth, bool aUseCache = false) const;
		glyph_text to_glyph_text(const string& aText, const font& aFont) const;
		glyph_text to_glyph_text(string::const_iterator aTextBegin, string::const_iterator aTextEnd, const font& aFont) const;
		glyph_text to_glyph_text(string::const_iterator aTextBegin, string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const;
		glyph_text to_glyph_text(const std::u32string& aText, const font& aFont) const;
		glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont) const;
		glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const;
		bool is_text_left_to_right(const string& aText, const font& aFont, bool aUseCache = false) const;
		bool is_text_right_to_left(const string& aText, const font& aFont, bool aUseCache = false) const;
		void draw_text(const point& aPoint, const string& aText, const font& aFont, const colour& aColour, bool aUseCache = false) const;
		void draw_text(const point& aPoint, string::const_iterator aTextBegin, string::const_iterator aTextEnd, const font& aFont, const colour& aColour, bool aUseCache = false) const;
		void draw_multiline_text(const point& aPoint, const string& aText, const font& aFont, const colour& aColour, alignment aAlignment = alignment::Left, bool aUseCache = false) const;
		void draw_multiline_text(const point& aPoint, const string& aText, const font& aFont, dimension aMaxWidth, const colour& aColour, alignment aAlignment = alignment::Left, bool aUseCache = false) const;
		void draw_glyph_text(const point& aPoint, const glyph_text& aText, const font& aFont, const colour& aColour) const;
		void draw_glyph_text(const point& aPoint, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const font& aFont, const colour& aColour) const;
		size draw_glyph(const point& aPoint, const glyph& aGlyph, const font& aFont, const colour& aColour) const;
		void draw_glyph_underline(const point& aPoint, const glyph& aGlyph, const font& aFont, const colour& aColour) const;
		void set_glyph_text_cache(glyph_text& aGlyphTextCache) const;
		void reset_glyph_text_cache() const;
		void set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix = '&') const;
		void unset_mnemonic() const;
		bool mnemonics_shown() const;
		bool password() const;
		const std::string& password_mask() const;
		void set_password(bool aPassword, const std::string& aMask = "\xE2\x97\x8F");
		void draw_texture(const point& aPoint, const i_texture& aTexture, const optional_colour& aColour = optional_colour(), shader_effect aShaderEffect = shader_effect::None) const;
		void draw_texture(const rect& aRect, const i_texture& aTexture, const optional_colour& aColour = optional_colour(), shader_effect aShaderEffect = shader_effect::None) const;
		void draw_texture(const texture_map& aMap, const i_texture& aTexture, const optional_colour& aColour = optional_colour(), shader_effect aShaderEffect = shader_effect::None) const;
		void draw_texture(const point& aPoint, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour = optional_colour(), shader_effect aShaderEffect = shader_effect::None) const;
		void draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour = optional_colour(), shader_effect aShaderEffect = shader_effect::None) const;
		void draw_texture(const texture_map& aMap, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour = optional_colour(), shader_effect aShaderEffect = shader_effect::None) const;
		// implementation
		// from i_device_metrics
	public:
		virtual bool metrics_available() const;
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

	template <typename Iter>
	inline void draw_glyph_text(const graphics_context& aGraphicsContext, const point& aPoint, Iter aTextBegin, Iter aTextEnd, const font& aFont, const colour& aColour)
	{
		{
			graphics_context::glyph_drawing gd(aGraphicsContext);
			point pos = aPoint;
			for (Iter i = aTextBegin; i != aTextEnd; ++i)
			{
				aGraphicsContext.draw_glyph(pos + i->offset(), *i, aFont, aColour);
				pos.x += i->advance().cx;
			}
		}
		point pos = aPoint;
		for (Iter i = aTextBegin; i != aTextEnd; ++i)
		{
			if (i->underline() || (aGraphicsContext.mnemonics_shown() && i->mnemonic()))
				aGraphicsContext.draw_glyph_underline(pos, *i, aFont, aColour);
			pos.x += i->advance().cx;
		}
	}

	class scoped_mnemonics
	{
	public:
		scoped_mnemonics(graphics_context& aGc, bool aShowMnemonics, char aMnemonicPrefix = '&') :
			iGc(aGc)
		{
			iGc.set_mnemonic(aShowMnemonics, aMnemonicPrefix);
		}
		~scoped_mnemonics()
		{
			iGc.unset_mnemonic();
		}
	private:
		graphics_context& iGc;
	};

	class scoped_coordinate_system
	{
	public:
		scoped_coordinate_system(graphics_context& aGc, const point& aOrigin, const size& aExtents, logical_coordinate_system aCoordinateSystem);
		scoped_coordinate_system(graphics_context& aGc, const point& aOrigin, const size& aExtents, logical_coordinate_system aCoordinateSystem, const vector4& aCoordinates);
		~scoped_coordinate_system();
	private:
		void apply_origin(const point& aOrigin, const size& aExtents);
	private:
		graphics_context& iGc;
		logical_coordinate_system iPreviousCoordinateSystem;
		vector4 iPreviousCoordinates;
	};

	template <typename ValueType = double, uint32_t W = 5>
	inline std::array<std::array<ValueType, W>, W> gaussian_filter(ValueType aSigma = 1.0)
	{
		const int32_t mean = static_cast<int32_t>(W / 2);
		std::array<std::array<ValueType, W>, W> kernel = {};
		if (aSigma != 0)
		{
			ValueType sum = 0.0;
			for (int32_t x = -mean; x <= mean; ++x)
			{
				for (int32_t y = -mean; y <= mean; ++y)
				{
					kernel[x + mean][y + mean] =
						static_cast<ValueType>((1.0 / (2.0 * boost::math::constants::pi<ValueType>() * aSigma * aSigma)) * std::exp(-((x * x + y * y) / (2.0 * aSigma * aSigma))));
					sum += kernel[x + mean][y + mean];
				}
			}
			for (uint32_t x = 0; x < W; ++x)
				for (uint32_t y = 0; y < W; ++y)
					kernel[x][y] /= sum;
		}
		else
			kernel[mean][mean] = static_cast<ValueType>(1.0);
		return kernel;
	}
}