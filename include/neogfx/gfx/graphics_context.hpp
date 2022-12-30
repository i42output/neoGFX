// graphics_context.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/i_graphics_context.hpp>

namespace neogfx
{
    class graphics_context : public i_graphics_context
    {
    public:
        struct not_implemented : std::logic_error { not_implemented() : std::logic_error("neogfx::graphics_context::not_implemented") {} };
    private:
        friend class generic_surface;
        // exceptions
        // construction
    public:
        graphics_context(const i_surface& aSurface, type aType = type::Attached);
        graphics_context(const i_surface& aSurface, const font& aDefaultFont, type aType = type::Attached);
        graphics_context(const i_widget& aWidget, type aType = type::Attached);
        graphics_context(const i_texture& aTexture, type aType = type::Attached);
        graphics_context(const graphics_context& aOther);
        virtual ~graphics_context();
        // i_rendering_context
    public:
        std::unique_ptr<i_rendering_context> clone() const final;
    public:
        i_rendering_engine& rendering_engine() const final;
        const i_render_target& render_target() const final;
        rect rendering_area(bool aConsiderScissor = true) const final;
        const graphics_operation::queue& queue() const final;
        graphics_operation::queue& queue() final;
        void enqueue(const graphics_operation::operation& aOperation) final;
        void flush() final;
    public:
        neogfx::logical_coordinates logical_coordinates() const final;
        vec2 offset() const final;
        void set_offset(const optional_vec2& aOffset) final;
        bool gradient_set() const final;
        void apply_gradient(i_gradient_shader& aShader) final;
        // i_graphics_context
    public:
        delta to_device_units(const delta& aValue) const final;
        size to_device_units(const size& aValue) const final;
        point to_device_units(const point& aValue) const final;
        vec2 to_device_units(const vec2& aValue) const final;
        rect to_device_units(const rect& aValue) const final;
        path to_device_units(const path& aValue) const final;
        delta from_device_units(const delta& aValue) const final;
        size from_device_units(const size& aValue) const final;
        point from_device_units(const point& aValue) const final;
        rect from_device_units(const rect& aValue) const final;
        path from_device_units(const path& aValue) const final;
        layer_t layer() const final;
        void set_layer(layer_t aLayer) final;
        neogfx::logical_coordinate_system logical_coordinate_system() const final;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) const final;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) const final;
        font const& default_font() const final;
        void set_default_font(const font& aDefaultFont) const final;
        void set_extents(const size& aExtents) const final;
        void set_origin(const point& aOrigin) const final;
        point origin() const final;
        void flush() const final;
        void set_default_viewport() const final;
        void set_viewport(const rect& aViewportRect) const final;
        void scissor_on(const rect& aRect) const final;
        void scissor_off() const final;
        bool snap_to_pixel() const final;
        void set_snap_to_pixel(bool aSnap) const final;
        double opacity() const final;
        void set_opacity(double aOpacity) const final;
        neogfx::blending_mode blending_mode() const final;
        void set_blending_mode(neogfx::blending_mode aBlendingMode) const final;
        neogfx::smoothing_mode smoothing_mode() const final;
        void set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode) const final;
        void push_logical_operation(logical_operation aLogicalOperation) const final;
        void pop_logical_operation() const final;
        void line_stipple_on(scalar aFactor, uint16_t aPattern, scalar aPosition = 0.0) const final;
        void line_stipple_off() const final;
        bool is_subpixel_rendering_on() const final;
        void subpixel_rendering_on() const final;
        void subpixel_rendering_off() const final;
        void clear(const color& aColor, const std::optional<scalar>& aZpos = std::optional<scalar>{}) const final;
        void clear_depth_buffer() const final;
        void clear_stencil_buffer() const final;
        void blit(const rect& aDestinationRect, const i_graphics_context& aSource, const rect& aSourceRect) const final;
        void blur(const rect& aDestinationRect, const i_graphics_context& aSource, const rect& aSourceRect, dimension aRadius, blurring_algorithm aAlgorithm = blurring_algorithm::Gaussian, scalar aParameter1 = 5, scalar aParameter2 = 1.0) const final;
        void clear_gradient() final;
        void set_gradient(const gradient& aGradient, const rect& aBoundingBox) final;
        void set_pixel(const point& aPoint, const color& aColor) const final;
        void draw_pixel(const point& aPoint, const color& aColor) const final;
        void draw_line(const point& aFrom, const point& aTo, const pen& aPen) const final;
        void draw_triangle(const point& aP0, const point& aP1, const point& aP2, const pen& aPen, const brush& aFill = brush{}) const final;
        void draw_rect(const rect& aRect, const pen& aPen, const brush& aFill = brush{}) const final;
        void draw_rounded_rect(const rect& aRect, const vec4& aRadius, const pen& aPen, const brush& aFill = brush{}) const final;
        void draw_circle(const point& aCenter, dimension aRadius, const pen& aPen, const brush& aFill = brush{}) const final;
        void draw_ellipse(const point& aCenter, dimension aRadiusA, dimension aRadiusB, const pen& aPen, const brush& aFill = brush{}) const final;
        void draw_pie(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen, const brush& aFill = brush{}) const final;
        void draw_arc(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen, const brush& aFill = brush{}) const final;
        void draw_cubic_bezier(const point& aP0, const point& aP1, const point& aP2, const point& aP3, const pen& aPen) const final;
        void draw_path(const path& aPath, const pen& aPen, const brush& aFill = brush{}) const final;
        void draw_shape(const game::mesh& aShape, const vec3& aPosition, const pen& aPen, const brush& aFill = brush{}) const final;
        void draw_entities(game::i_ecs& aEcs, int32_t aLayer = 0) const final;
        void draw_focus_rect(const rect& aRect) const final;
        void fill_triangle(const point& aP0, const point& aP1, const point& aP2, const brush& aFill = brush{}) const final;
        void fill_rect(const rect& aRect, const brush& aFill) const final;
        void fill_rounded_rect(const rect& aRect, const vec4& aRadius, const brush& aFill) const final;
        void fill_checker_rect(const rect& aRect, const size& aSquareSize, const brush& aFill1, const brush& aFill2) const final;
        void fill_circle(const point& aCenter, dimension aRadius, const brush& aFill) const final;
        void fill_ellipse(const point& aCenter, dimension aRadiusA, dimension aRadiusB, const brush& aFill = brush{}) const final;
        void fill_pie(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill) const final;
        void fill_arc(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill) const final;
        void fill_path(const path& aPath, const brush& aFill) const final;
        void fill_shape(const game::mesh& aShape, const vec3& aPosition, const brush& aFill) const final;
        size text_extent(std::string const& aText) const final;
        size text_extent(std::string const& aText, const font& aFont) const final;
        size text_extent(std::string const& aText, std::function<font(std::size_t)> aFontSelector) const final;
        size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd) const final;
        size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont) const final;
        size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector) const final;
        size multiline_text_extent(std::string const& aText) const final;
        size multiline_text_extent(std::string const& aText, const font& aFont) const final;
        size multiline_text_extent(std::string const& aText, std::function<font(std::size_t)> aFontSelector) const final;
        size multiline_text_extent(std::string const& aText, dimension aMaxWidth) const final;
        size multiline_text_extent(std::string const& aText, const font& aFont, dimension aMaxWidth) const final;
        size multiline_text_extent(std::string const& aText, std::function<font(std::size_t)> aFontSelector, dimension aMaxWidth) const final;
        size glyph_text_extent(const glyph_text& aText) const final;
        size glyph_text_extent(const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd) const final;
        size multiline_glyph_text_extent(const glyph_text& aText, dimension aMaxWidth) const final;
        glyph_text to_glyph_text(std::string const& aText) const final;
        glyph_text to_glyph_text(std::string const& aText, const font& aFont) const final;
        glyph_text to_glyph_text(std::string const& aText, std::function<font(std::size_t)> aFontSelector) const final;
        glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd) const final;
        glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont) const final;
        glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector) const final;
        glyph_text to_glyph_text(const std::u32string& aText) const final;
        glyph_text to_glyph_text(const std::u32string& aText, const font& aFont) const final;
        glyph_text to_glyph_text(const std::u32string& aText, std::function<font(std::size_t)> aFontSelector) const final;
        glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd) const final;
        glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont) const final;
        glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector) const final;
        multiline_glyph_text to_multiline_glyph_text(std::string const& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::string const& aText, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(const std::u32string& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(const std::u32string& aText, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(const glyph_text& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        bool is_text_left_to_right(std::string const& aText) const final;
        bool is_text_left_to_right(std::string const& aText, const font& aFont) const final;
        bool is_text_right_to_left(std::string const& aText) const final;
        bool is_text_right_to_left(std::string const& aText, const font& aFont) const final;
        void draw_text(const point& aPoint, std::string const& aText, const text_format& aTextFormat) const final;
        void draw_text(const point& aPoint, std::string const& aText, const font& aFont, const text_format& aTextFormat) const final;
        void draw_text(const point& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const text_format& aTextFormat) const final;
        void draw_text(const point& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, const text_format& aTextFormat) const final;
        void draw_text(const vec3& aPoint, std::string const& aText, const text_format& aTextFormat) const final;
        void draw_text(const vec3& aPoint, std::string const& aText, const font& aFont, const text_format& aTextFormat) const final;
        void draw_text(const vec3& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const text_format& aTextFormat) const final;
        void draw_text(const vec3& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, const text_format& aTextFormat) const final;
        void draw_multiline_text(const point& aPoint, std::string const& aText, const text_format& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(const point& aPoint, std::string const& aText, const font& aFont, const text_format& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(const point& aPoint, std::string const& aText, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(const point& aPoint, std::string const& aText, const font& aFont, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(const vec3& aPoint, std::string const& aText, const text_format& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(const vec3& aPoint, std::string const& aText, const font& aFont, const text_format& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(const vec3& aPoint, std::string const& aText, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(const vec3& aPoint, std::string const& aText, const font& aFont, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_glyph_text(const point& aPoint, const glyph_text& aText, const text_format& aTextFormat) const final;
        void draw_glyph_text(const point& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const text_format& aTextFormat) const final;
        void draw_glyph_text(const vec3& aPoint, const glyph_text& aText, const text_format& aTextFormat) const final;
        void draw_glyph_text(const vec3& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const text_format& aTextFormat) const final;
        void draw_multiline_glyph_text(const point& aPoint, const glyph_text& aText, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_glyph_text(const vec3& aPoint, const glyph_text& aText, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_glyph(const point& aPoint, const glyph_text& aText, const glyph_char& aGlyphChar, const text_format& aTextFormat) const final;
        void draw_glyph(const vec3& aPoint, const glyph_text& aText, const glyph_char& aGlyphChar, const text_format& aTextFormat) const final;
        void draw_glyphs(const point& aPoint, const glyph_text& aText, const text_format_spans& aSpans) const final;
        void draw_glyphs(const vec3& aPoint, const glyph_text& aText, const text_format_spans& aSpans) const final;
        char mnemonic() const final;
        bool mnemonic_set() const final;
        void set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix = '&') const final;
        void unset_mnemonic() const final;
        bool mnemonics_shown() const final;
        bool password() const final;
        std::string const& password_mask() const final;
        void set_password(bool aPassword, std::string const& aMask = "\xE2\x97\x8F") final;
        void draw_texture(const point& aPoint, const i_texture& aTexture, const color_or_gradient& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_texture(const rect& aRect, const i_texture& aTexture, const color_or_gradient& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_texture(const point& aPoint, const i_texture& aTexture, const rect& aTextureRect, const color_or_gradient& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect, const color_or_gradient& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_texture(const game::mesh& aMesh, const i_texture& aTexture, const color_or_gradient& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_texture(const game::mesh& aMesh, const i_texture& aTexture, const rect& aTextureRect, const color_or_gradient& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const optional_mat44& aTransformation = optional_mat44{}, const std::optional<game::filter>& aFilter = {}) const final;

        // implementation
        // from i_rendering_context
    public:
        neogfx::subpixel_format subpixel_format() const final;
        // from i_device_metrics
    public:
        bool metrics_available() const final;
        size extents() const final;
        dimension horizontal_dpi() const final;
        dimension vertical_dpi() const final;
        dimension ppi() const final;
        dimension em_size() const final;
        // from i_units_context
    public:
        bool device_metrics_available() const final;
        const i_device_metrics& device_metrics() const final;
    protected:
        bool attached() const;
        bool active() const;
        i_rendering_context& native_context() const;
        // helpers
        // attributes
    private:
        type iType;
        const i_render_target& iRenderTarget;
        mutable std::optional<scoped_render_target> iSrt;
        mutable std::unique_ptr<i_rendering_context> iNativeGraphicsContext;
        mutable font iDefaultFont;
        mutable point iOrigin;
        mutable size iExtents;
        mutable int32_t iLayer;
        mutable std::optional<neogfx::logical_coordinate_system> iLogicalCoordinateSystem;
        mutable std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        mutable bool iSnapToPixel;
        mutable double iOpacity;
        mutable neogfx::blending_mode iBlendingMode;
        mutable neogfx::smoothing_mode iSmoothingMode;
        mutable bool iSubpixelRendering;
        mutable std::optional<std::pair<bool, char>> iMnemonic;
        mutable std::optional<std::string> iPassword;
        mutable std::optional<size> iPreviousPingPongBufferSize;
    };
}