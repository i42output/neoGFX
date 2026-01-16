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
        graphics_operation::queue& queue() const final;
        void enqueue(graphics_operation::operation const& aOperation) final;
        // i_rendering_context
    public:
        neogfx::logical_coordinates logical_coordinates() const final;
        vec2 offset() const final;
        void set_offset(const optional_vec2& aOffset) final;
        void blit(const rect& aDestinationRect, const i_texture& aTexture, const rect& aSourceRect) final;
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
        void flush() final;
        // layers
    public:
        layer_t layer() const final;
        void set_layer(layer_t aLayer) final;
        // coordinate system
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const final;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) final;
        void set_logical_coordinates(neogfx::logical_coordinates const& aCoordinates) final;
        // viewport
    public:
        void set_extents(size const& aExtents) final;
        void set_origin(point const& aOrigin) final;
        point origin() const final;
        void set_viewport(optional_rect const& aViewport = {}) final;
        void set_view_transforamtion(optional_mat33 const& aViewTransforamtion = {}) final;
        // clipping
    public:
        void scissor_on(rect const& aRect) final;
        void scissor_off() final;
        // rendering
    public:
        neogfx::front_face front_face() const final;
        void set_front_face(neogfx::front_face aFrontFace) final;
        neogfx::face_culling face_culling() const final;
        void set_face_culling(neogfx::face_culling aCulling) final;
        // anti-aliasing
    public:
        neogfx::smoothing_mode smoothing_mode() const final;
        void set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode) final;
        bool snap_to_pixel() const final;
        void set_snap_to_pixel(bool aSnap) final;
        // blending
    public:
        double opacity() const final;
        void set_opacity(double aOpacity) final;
        neogfx::blending_mode blending_mode() const final;
        void set_blending_mode(neogfx::blending_mode aBlendingMode) final;
        // drawing mode
    public:
        void push_logical_operation(logical_operation aLogicalOperation) final;
        void pop_logical_operation() final;
        std::optional<stipple> const& line_stipple() const final;
        void line_stipple_on(stipple const& aStipple) final;
        void line_stipple_off() final;
        bool is_subpixel_rendering_on() const final;
        void subpixel_rendering_on() final;
        void subpixel_rendering_off() final;
        void clear(color const& aColor, std::optional<scalar> const& aZpos = std::optional<scalar>{}) final;
        void clear_depth_buffer() final;
        void clear_stencil_buffer() final;
        void blit(rect const& aDestinationRect, i_graphics_context& aSource, rect const& aSourceRect) final;
        void blur(rect const& aDestinationRect, i_graphics_context& aSource, rect const& aSourceRect, dimension aRadius, blurring_algorithm aAlgorithm = blurring_algorithm::Gaussian, scalar aParameter1 = 5, scalar aParameter2 = 1.0) final;
        void clear_gradient() final;
        void set_gradient(gradient const& aGradient, rect const& aBoundingBox) final;
        void set_pixel(point const& aPoint, color const& aColor) final;
        void draw_pixel(point const& aPoint, color const& aColor) final;
        void draw_line(point const& aFrom, point const& aTo, pen const& aPen) final;
        void draw_triangle(point const& aP0, point const& aP1, point const& aP2, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_rect(rect const& aRect, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_rounded_rect(rect const& aRect, vec4 const& aRadius, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_ellipse_rect(rect const& aRect, vec4 const& aRadiusX, vec4 const& aRadiusY, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_checkerboard(rect const& aRect, size const& aSquareSize, pen const& aPen, brush const& aFill1, brush const& aFill2) final;
        void draw_circle(point const& aCenter, dimension aRadius, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_ellipse(point const& aCenter, dimension aRadiusA, dimension aRadiusB, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_pie(point const& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_arc(point const& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_cubic_bezier(point const& aP0, point const& aP1, point const& aP2, point const& aP3, pen const& aPen) final;
        void draw_path(path const& aPath, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_path(ssbo_range const& aPathVertices, path_shape aPathShape, rect const& aBoundingRect, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_shape(game::mesh const& aShape, vec3 const& aPosition, pen const& aPen, brush const& aFill = brush{}) final;
        void draw_entities(game::i_ecs& aEcs, game::scene_layer aLayer = 0) final;
        void draw_focus_rect(rect const& aRect) final;
        // text
    public:
        font const& default_font() const final;
        void set_default_font(font const& aDefaultFont) final;
        bool has_tab_stops() const override;
        i_tab_stops const& tab_stops() const override;
        void set_tab_stops(i_tab_stops const& aTabStops) override;
        void clear_tab_stops() override;
        size text_extent(string const& aText) const override;
        size text_extent(string const& aText, font const& aFont) const override;
        size text_extent(string const& aText, std::function<font(string::size_type)> aFontSelector) const override;
        size multiline_text_extent(string const& aText) const override;
        size multiline_text_extent(string const& aText, font const& aFont) const override;
        size multiline_text_extent(string const& aText, std::function<font(string::size_type)> aFontSelector) const override;
        size multiline_text_extent(string const& aText, dimension aMaxWidth) const override;
        size multiline_text_extent(string const& aText, font const& aFont, dimension aMaxWidth) const override;
        size multiline_text_extent(string const& aText, std::function<font(string::size_type)> aFontSelector, dimension aMaxWidth) const override;
        size glyph_text_extent(glyph_text const& aText) const override;
        size glyph_text_extent(glyph_text const& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd) const override;
        size multiline_glyph_text_extent(glyph_text const& aText, dimension aMaxWidth) const override;
        glyph_text to_glyph_text(string const& aText) const override;
        glyph_text to_glyph_text(string const& aText, std::function<font(string::size_type)> aFontSelector) const override;
        glyph_text to_glyph_text(string const& aText, font const& aFont) const override;
        glyph_text to_glyph_text(std::u32string const& aText) const override;
        glyph_text to_glyph_text(std::u32string const& aText, font const& aFont) const override;
        glyph_text to_glyph_text(std::u32string const& aText, std::function<font(std::u32string::size_type)> aFontSelector) const override;
        multiline_glyph_text to_multiline_glyph_text(string const& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(string const& aText, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(std::u32string const& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(std::u32string const& aText, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(glyph_text const& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        size text_extent(char const* aTextBegin, char const* aTextEnd) const override;
        size text_extent(char const* aTextBegin, char const* aTextEnd, font const& aFont) const override;
        size text_extent(char const* aTextBegin, char const* aTextEnd, std::function<font(string::size_type)> aFontSelector) const override;
        size text_extent(char32_t const* aTextBegin, char32_t const* aTextEnd) const override;
        size text_extent(char32_t const* aTextBegin, char32_t const* aTextEnd, font const& aFont) const override;
        size text_extent(char32_t const* aTextBegin, char32_t const* aTextEnd, std::function<font(string::size_type)> aFontSelector) const override;
        glyph_text to_glyph_text(char const* aTextBegin, char const* aTextEnd) const override;
        glyph_text to_glyph_text(char const* aTextBegin, char const* aTextEnd, font const& aFont) const override;
        glyph_text to_glyph_text(char const* aTextBegin, char const* aTextEnd, std::function<font(string::size_type)> aFontSelector) const override;
        glyph_text to_glyph_text(char32_t const* aTextBegin, char32_t const* aTextEnd) const override;
        glyph_text to_glyph_text(char32_t const* aTextBegin, char32_t const* aTextEnd, font const& aFont) const override;
        glyph_text to_glyph_text(char32_t const* aTextBegin, char32_t const* aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const override;
        multiline_glyph_text to_multiline_glyph_text(char const* aTextBegin, char const* aTextEnd, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(char const* aTextBegin, char const* aTextEnd, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(char const* aTextBegin, char const* aTextEnd, std::function<font(string::size_type)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(char32_t const* aTextBegin, char32_t const* aTextEnd, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(char32_t const* aTextBegin, char32_t const* aTextEnd, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(char32_t const* aTextBegin, char32_t const* aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        bool is_text_left_to_right(string const& aText) const final;
        bool is_text_left_to_right(string const& aText, font const& aFont) const final;
        bool is_text_right_to_left(string const& aText) const final;
        bool is_text_right_to_left(string const& aText, font const& aFont) const final;
        void draw_text(point const& aPoint, string const& aText, text_format const& aTextFormat) final;
        void draw_text(point const& aPoint, string const& aText, font const& aFont, text_format const& aTextFormat) final;
        void draw_text(point const& aPoint, char const* aTextBegin, char const* aTextEnd, text_format const& aTextFormat) final;
        void draw_text(point const& aPoint, char const* aTextBegin, char const* aTextEnd, font const& aFont, text_format const& aTextFormat) final;
        void draw_text(vec3 const& aPoint, string const& aText, text_format const& aTextFormat) final;
        void draw_text(vec3 const& aPoint, string const& aText, font const& aFont, text_format const& aTextFormat) final;
        void draw_text(vec3 const& aPoint, char const* aTextBegin, char const* aTextEnd, text_format const& aTextFormat) final;
        void draw_text(vec3 const& aPoint, char const* aTextBegin, char const* aTextEnd, font const& aFont, text_format const& aTextFormat) final;
        void draw_multiline_text(point const& aPoint, string const& aText, text_format const& aTextFormat, alignment aAlignment = alignment::Left) final;
        void draw_multiline_text(point const& aPoint, string const& aText, font const& aFont, text_format const& aTextFormat, alignment aAlignment = alignment::Left) final;
        void draw_multiline_text(point const& aPoint, string const& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) final;
        void draw_multiline_text(point const& aPoint, string const& aText, font const& aFont, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) final;
        void draw_multiline_text(vec3 const& aPoint, string const& aText, text_format const& aTextFormat, alignment aAlignment = alignment::Left) final;
        void draw_multiline_text(vec3 const& aPoint, string const& aText, font const& aFont, text_format const& aTextFormat, alignment aAlignment = alignment::Left) final;
        void draw_multiline_text(vec3 const& aPoint, string const& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) final;
        void draw_multiline_text(vec3 const& aPoint, string const& aText, font const& aFont, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) final;
        void draw_glyph_text(point const& aPoint, const glyph_text& aText, text_format const& aTextFormat) final;
        void draw_glyph_text(point const& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, text_format const& aTextFormat) final;
        void draw_glyph_text(vec3 const& aPoint, const glyph_text& aText, text_format const& aTextFormat) final;
        void draw_glyph_text(vec3 const& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, text_format const& aTextFormat) final;
        void draw_multiline_glyph_text(point const& aPoint, const glyph_text& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) final;
        void draw_multiline_glyph_text(vec3 const& aPoint, const glyph_text& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) final;
        void draw_multiline_glyph_text(point const& aPoint, const multiline_glyph_text& aText, text_format const& aTextFormat) final;
        void draw_multiline_glyph_text(vec3 const& aPoint, const multiline_glyph_text& aText, text_format const& aTextFormat) final;
        void draw_glyph(point const& aPoint, const glyph_text& aText, const glyph_char& aGlyphChar, text_format const& aTextFormat) final;
        void draw_glyph(vec3 const& aPoint, const glyph_text& aText, const glyph_char& aGlyphChar, text_format const& aTextFormat) final;
        void draw_glyphs(point const& aPoint, const glyph_text& aText, const text_format_spans& aSpans) final;
        void draw_glyphs(vec3 const& aPoint, const glyph_text& aText, const text_format_spans& aSpans) final;
        char mnemonic() const final;
        bool mnemonic_set() const final;
        void set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix = '&') final;
        void unset_mnemonic() final;
        bool mnemonics_shown() const final;
        bool password() const final;
        string const& password_mask() const final;
        void set_password(bool aPassword, string const& aMask = "\xE2\x97\x8F") final;
        // texture
    public:
        void draw_texture(point const& aPoint, i_texture const& aTexture, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) final;
        void draw_texture(rect const& aRect, i_texture const& aTexture, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) final;
        void draw_texture(point const& aPoint, i_texture const& aTexture, rect const& aTextureRect, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) final;
        void draw_texture(rect const& aRect, i_texture const& aTexture, rect const& aTextureRect, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) final;
        void draw_texture(game::mesh const& aMesh, i_texture const& aTexture, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) final;
        void draw_texture(game::mesh const& aMesh, i_texture const& aTexture, rect const& aTextureRect, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) final;
        // 3D
    public:
        // todo: set_transformation et al
        void draw_mesh(game::mesh const& aMesh, game::material const& aMaterial, optional_mat44 const& aTransformation = optional_mat44{}, const std::optional<game::filter>& aFilter = {}) final;

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
        i_rendering_context& rendering_context() const;
        // helpers
        // attributes
    private:
        type iType;
        const i_render_target& iRenderTarget;
        mutable std::optional<scoped_render_target> iSrt;
        mutable std::unique_ptr<i_rendering_context> iRenderingContext;
        mutable font iDefaultFont;
        mutable point iOrigin;
        mutable size iExtents;
        mutable std::int32_t iLayer;
        mutable std::optional<neogfx::logical_coordinate_system> iLogicalCoordinateSystem;
        mutable std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        mutable bool iSnapToPixel;
        mutable neogfx::front_face iFrontFace;
        mutable neogfx::face_culling iCulling;
        mutable double iOpacity;
        mutable neogfx::blending_mode iBlendingMode;
        mutable neogfx::smoothing_mode iSmoothingMode;
        mutable bool iSubpixelRendering;
        mutable std::optional<stipple> iLineStipple;
        mutable std::optional<neogfx::tab_stops> iTabStops;
        mutable std::optional<std::pair<bool, char>> iMnemonic;
        mutable std::optional<string> iPassword;
        mutable std::optional<size> iPreviousPingPongBufferSize;
        mutable std::vector<ssbo_range> iSsboRanges;
    };
}