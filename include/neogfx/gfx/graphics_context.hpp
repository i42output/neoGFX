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
        graphics_context(i_surface const& aSurface, type aType = type::Attached);
        graphics_context(i_surface const& aSurface, font const& aDefaultFont, type aType = type::Attached);
        graphics_context(i_widget const& aWidget, type aType = type::Attached);
        graphics_context(i_texture const& aTexture, type aType = type::Attached);
        graphics_context(graphics_context const& aOther);
        virtual ~graphics_context();
        // i_rendering_context
    public:
        std::unique_ptr<i_rendering_context> clone() const final;
        // i_rendering_context
    public:
        i_rendering_engine& rendering_engine() const final;
        i_render_target const& render_target() const final;
        rect rendering_area(bool aConsiderScissor = true) const final;
        graphics_operation::queue const& queue() const final;
        graphics_operation::queue& queue() final;
        void enqueue(graphics_operation::operation const& aOperation) final;
        void flush() final;
        // i_rendering_context
    public:
        neogfx::logical_coordinates logical_coordinates() const final;
        vec2 offset() const final;
        void set_offset(const optional_vec2& aOffset) final;
        bool gradient_set() const final;
        void apply_gradient(i_gradient_shader& aShader) final;
        // units
    public:
        delta to_device_units(const delta& aValue) const final;
        size to_device_units(size const& aValue) const final;
        point to_device_units(point const& aValue) const final;
        vec2 to_device_units(const vec2& aValue) const final;
        rect to_device_units(rect const& aValue) const final;
        path to_device_units(const path& aValue) const final;
        delta from_device_units(const delta& aValue) const final;
        size from_device_units(size const& aValue) const final;
        point from_device_units(point const& aValue) const final;
        rect from_device_units(rect const& aValue) const final;
        path from_device_units(const path& aValue) const final;
        // state
    public:
        void flush() const final;
        // layers
    public:
        layer_t layer() const final;
        void set_layer(layer_t aLayer) final;
        // coordinate system
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const final;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) const final;
        void set_logical_coordinates(neogfx::logical_coordinates const& aCoordinates) const final;
        // viewport
    public:
        void set_extents(size const& aExtents) const final;
        void set_origin(point const& aOrigin) const final;
        point origin() const final;
        void set_viewport(optional_rect const& aViewport = {}) const final;
        void set_view_transforamtion(optional_mat33 const& aViewTransforamtion = {}) const final;
        // clipping
    public:
        void scissor_on(rect const& aRect) const final;
        void scissor_off() const final;
        // anti-aliasing
    public:
        neogfx::smoothing_mode smoothing_mode() const final;
        void set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode) const final;
        bool snap_to_pixel() const final;
        void set_snap_to_pixel(bool aSnap) const final;
        // blending
    public:
        double opacity() const final;
        void set_opacity(double aOpacity) const final;
        neogfx::blending_mode blending_mode() const final;
        void set_blending_mode(neogfx::blending_mode aBlendingMode) const final;
        // drawing mode
    public:
        void push_logical_operation(logical_operation aLogicalOperation) const final;
        void pop_logical_operation() const final;
        void line_stipple_on(scalar aFactor, uint16_t aPattern, scalar aPosition = 0.0) const final;
        void line_stipple_off() const final;
        bool is_subpixel_rendering_on() const final;
        void subpixel_rendering_on() const final;
        void subpixel_rendering_off() const final;
        void clear(color const& aColor, std::optional<scalar> const& aZpos = std::optional<scalar>{}) const final;
        void clear_depth_buffer() const final;
        void clear_stencil_buffer() const final;
        void blit(rect const& aDestinationRect, i_graphics_context const& aSource, rect const& aSourceRect) const final;
        void blur(rect const& aDestinationRect, i_graphics_context const& aSource, rect const& aSourceRect, dimension aRadius, blurring_algorithm aAlgorithm = blurring_algorithm::Gaussian, scalar aParameter1 = 5, scalar aParameter2 = 1.0) const final;
        void clear_gradient() final;
        void set_gradient(gradient const& aGradient, rect const& aBoundingBox) final;
        void set_pixel(point const& aPoint, color const& aColor) const final;
        void draw_pixel(point const& aPoint, color const& aColor) const final;
        void draw_line(point const& aFrom, point const& aTo, pen const& aPen) const final;
        void draw_triangle(point const& aP0, point const& aP1, point const& aP2, pen const& aPen, brush const& aFill = brush{}) const final;
        void draw_rect(rect const& aRect, pen const& aPen, brush const& aFill = brush{}) const final;
        void draw_rounded_rect(rect const& aRect, vec4 const& aRadius, pen const& aPen, brush const& aFill = brush{}) const final;
        void draw_checker_rect(rect const& aRect, size const& aSquareSize, pen const& aPen, brush const& aFill1, brush const& aFill2) const final;
        void draw_circle(point const& aCenter, dimension aRadius, pen const& aPen, brush const& aFill = brush{}) const final;
        void draw_ellipse(point const& aCenter, dimension aRadiusA, dimension aRadiusB, pen const& aPen, brush const& aFill = brush{}) const final;
        void draw_pie(point const& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, pen const& aPen, brush const& aFill = brush{}) const final;
        void draw_arc(point const& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, pen const& aPen, brush const& aFill = brush{}) const final;
        void draw_cubic_bezier(point const& aP0, point const& aP1, point const& aP2, point const& aP3, pen const& aPen) const final;
        void draw_path(path const& aPath, pen const& aPen, brush const& aFill = brush{}) const final;
        void draw_shape(game::mesh const& aShape, vec3 const& aPosition, pen const& aPen, brush const& aFill = brush{}) const final;
        void draw_entities(game::i_ecs& aEcs, game::scene_layer aLayer = 0) const final;
        void draw_focus_rect(rect const& aRect) const final;
        // text
    public:
        font const& default_font() const final;
        void set_default_font(font const& aDefaultFont) const final;
        bool has_tab_stops() const override;
        i_tab_stops const& tab_stops() const override;
        void set_tab_stops(i_tab_stops const& aTabStops) override;
        void clear_tab_stops() override;
        size text_extent(std::string const& aText) const final;
        size text_extent(std::string const& aText, font const& aFont) const final;
        size text_extent(std::string const& aText, std::function<font(std::size_t)> aFontSelector) const final;
        size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd) const final;
        size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, font const& aFont) const final;
        size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector) const final;
        size multiline_text_extent(std::string const& aText) const final;
        size multiline_text_extent(std::string const& aText, font const& aFont) const final;
        size multiline_text_extent(std::string const& aText, std::function<font(std::size_t)> aFontSelector) const final;
        size multiline_text_extent(std::string const& aText, dimension aMaxWidth) const final;
        size multiline_text_extent(std::string const& aText, font const& aFont, dimension aMaxWidth) const final;
        size multiline_text_extent(std::string const& aText, std::function<font(std::size_t)> aFontSelector, dimension aMaxWidth) const final;
        size glyph_text_extent(glyph_text const& aText) const final;
        size glyph_text_extent(glyph_text const& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd) const final;
        size multiline_glyph_text_extent(const glyph_text& aText, dimension aMaxWidth) const final;
        glyph_text to_glyph_text(std::string const& aText) const final;
        glyph_text to_glyph_text(std::string const& aText, font const& aFont) const final;
        glyph_text to_glyph_text(std::string const& aText, std::function<font(std::size_t)> aFontSelector) const final;
        glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd) const final;
        glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, font const& aFont) const final;
        glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector) const final;
        glyph_text to_glyph_text(std::u32string const& aText) const final;
        glyph_text to_glyph_text(std::u32string const& aText, font const& aFont) const final;
        glyph_text to_glyph_text(std::u32string const& aText, std::function<font(std::size_t)> aFontSelector) const final;
        glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd) const final;
        glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, font const& aFont) const final;
        glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector) const final;
        multiline_glyph_text to_multiline_glyph_text(std::string const& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::string const& aText, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::u32string const& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::u32string const& aText, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        multiline_glyph_text to_multiline_glyph_text(glyph_text const& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const final;
        bool is_text_left_to_right(std::string const& aText) const final;
        bool is_text_left_to_right(std::string const& aText, font const& aFont) const final;
        bool is_text_right_to_left(std::string const& aText) const final;
        bool is_text_right_to_left(std::string const& aText, font const& aFont) const final;
        void draw_text(point const& aPoint, std::string const& aText, text_format const& aTextFormat) const final;
        void draw_text(point const& aPoint, std::string const& aText, font const& aFont, text_format const& aTextFormat) const final;
        void draw_text(point const& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, text_format const& aTextFormat) const final;
        void draw_text(point const& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, font const& aFont, text_format const& aTextFormat) const final;
        void draw_text(vec3 const& aPoint, std::string const& aText, text_format const& aTextFormat) const final;
        void draw_text(vec3 const& aPoint, std::string const& aText, font const& aFont, text_format const& aTextFormat) const final;
        void draw_text(vec3 const& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, text_format const& aTextFormat) const final;
        void draw_text(vec3 const& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, font const& aFont, text_format const& aTextFormat) const final;
        void draw_multiline_text(point const& aPoint, std::string const& aText, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(point const& aPoint, std::string const& aText, font const& aFont, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(point const& aPoint, std::string const& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(point const& aPoint, std::string const& aText, font const& aFont, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(vec3 const& aPoint, std::string const& aText, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(vec3 const& aPoint, std::string const& aText, font const& aFont, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(vec3 const& aPoint, std::string const& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_text(vec3 const& aPoint, std::string const& aText, font const& aFont, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_glyph_text(point const& aPoint, const glyph_text& aText, text_format const& aTextFormat) const final;
        void draw_glyph_text(point const& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, text_format const& aTextFormat) const final;
        void draw_glyph_text(vec3 const& aPoint, const glyph_text& aText, text_format const& aTextFormat) const final;
        void draw_glyph_text(vec3 const& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, text_format const& aTextFormat) const final;
        void draw_multiline_glyph_text(point const& aPoint, const glyph_text& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_glyph_text(vec3 const& aPoint, const glyph_text& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const final;
        void draw_multiline_glyph_text(point const& aPoint, const multiline_glyph_text& aText, text_format const& aTextFormat) const final;
        void draw_multiline_glyph_text(vec3 const& aPoint, const multiline_glyph_text& aText, text_format const& aTextFormat) const final;
        void draw_glyph(point const& aPoint, const glyph_text& aText, const glyph_char& aGlyphChar, text_format const& aTextFormat) const final;
        void draw_glyph(vec3 const& aPoint, const glyph_text& aText, const glyph_char& aGlyphChar, text_format const& aTextFormat) const final;
        void draw_glyphs(point const& aPoint, const glyph_text& aText, const text_format_spans& aSpans) const final;
        void draw_glyphs(vec3 const& aPoint, const glyph_text& aText, const text_format_spans& aSpans) const final;
        char mnemonic() const final;
        bool mnemonic_set() const final;
        void set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix = '&') const final;
        void unset_mnemonic() const final;
        bool mnemonics_shown() const final;
        bool password() const final;
        std::string const& password_mask() const final;
        void set_password(bool aPassword, std::string const& aMask = "\xE2\x97\x8F") final;
        // texture
    public:
        void draw_texture(point const& aPoint, i_texture const& aTexture, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_texture(rect const& aRect, i_texture const& aTexture, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_texture(point const& aPoint, i_texture const& aTexture, rect const& aTextureRect, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_texture(rect const& aRect, i_texture const& aTexture, rect const& aTextureRect, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_texture(game::mesh const& aMesh, i_texture const& aTexture, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        void draw_texture(game::mesh const& aMesh, i_texture const& aTexture, rect const& aTextureRect, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const final;
        // 3D
    public:
        // todo: set_transformation et al
        void draw_mesh(game::mesh const& aMesh, game::material const& aMaterial, optional_mat44 const& aTransformation = optional_mat44{}, const std::optional<game::filter>& aFilter = {}) const final;

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
        mutable std::optional<neogfx::tab_stops> iTabStops;
        mutable std::optional<std::pair<bool, char>> iMnemonic;
        mutable std::optional<std::string> iPassword;
        mutable std::optional<size> iPreviousPingPongBufferSize;
    };
}