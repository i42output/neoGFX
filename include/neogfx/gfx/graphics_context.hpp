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
        class glyph_shapes;
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
        std::unique_ptr<i_rendering_context> clone() const override;
    public:
        i_rendering_engine& rendering_engine() override;
        const i_render_target& render_target() const override;
        rect rendering_area(bool aConsiderScissor = true) const override;
        const graphics_operation::queue& queue() const override;
        graphics_operation::queue& queue() override;
        void enqueue(const graphics_operation::operation& aOperation) override;
        void flush() override;
    public:
        neogfx::logical_coordinates logical_coordinates() const override;
        vec2 offset() const override;
        void set_offset(const optional_vec2& aOffset) override;
        bool gradient_set() const override;
        void apply_gradient(i_gradient_shader& aShader) override;
        // i_graphics_context
    public:
        ping_pong_buffers_t ping_pong_buffers(const size& aExtents, texture_sampling aSampling = texture_sampling::Multisample, const optional_color& aClearColor = color{ vec4{0.0, 0.0, 0.0, 0.0} }) const override;
    public:
        delta to_device_units(const delta& aValue) const override;
        size to_device_units(const size& aValue) const override;
        point to_device_units(const point& aValue) const override;
        vec2 to_device_units(const vec2& aValue) const override;
        rect to_device_units(const rect& aValue) const override;
        path to_device_units(const path& aValue) const override;
        delta from_device_units(const delta& aValue) const override;
        size from_device_units(const size& aValue) const override;
        point from_device_units(const point& aValue) const override;
        rect from_device_units(const rect& aValue) const override;
        path from_device_units(const path& aValue) const override;
        int32_t layer() const override;
        void set_layer(int32_t aLayer) override;
        neogfx::logical_coordinate_system logical_coordinate_system() const override;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) const override;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) const override;
        void set_default_font(const font& aDefaultFont) const override;
        void set_extents(const size& aExtents) const override;
        void set_origin(const point& aOrigin) const override;
        point origin() const override;
        void flush() const override;
        void scissor_on(const rect& aRect) const override;
        void scissor_off() const override;
        bool snap_to_pixel() const override;
        void set_snap_to_pixel(bool aSnap) const override;
        double opacity() const override;
        void set_opacity(double aOpacity) const override;
        neogfx::blending_mode blending_mode() const override;
        void set_blending_mode(neogfx::blending_mode aBlendingMode) const override;
        neogfx::smoothing_mode smoothing_mode() const override;
        void set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode) const override;
        void push_logical_operation(logical_operation aLogicalOperation) const override;
        void pop_logical_operation() const override;
        void line_stipple_on(scalar aFactor, uint16_t aPattern, scalar aPosition = 0.0) const override;
        void line_stipple_off() const override;
        bool is_subpixel_rendering_on() const override;
        void subpixel_rendering_on() const override;
        void subpixel_rendering_off() const override;
        void clear(const color& aColor, const std::optional<scalar>& aZpos = std::optional<scalar>{}) const override;
        void clear_depth_buffer() const override;
        void clear_stencil_buffer() const override;
        void blit(const rect& aDestinationRect, const i_graphics_context& aSource, const rect& aSourceRect) const override;
        void blur(const rect& aDestinationRect, const i_graphics_context& aSource, const rect& aSourceRect, dimension aRadius, blurring_algorithm aAlgorithm = blurring_algorithm::Gaussian, scalar aParameter1 = 5, scalar aParameter2 = 1.0) const override;
        void clear_gradient() override;
        void set_gradient(const gradient& aGradient, const rect& aBoundingBox) override;
        void set_pixel(const point& aPoint, const color& aColor) const override;
        void draw_pixel(const point& aPoint, const color& aColor) const override;
        void draw_line(const point& aFrom, const point& aTo, const pen& aPen) const override;
        void draw_rect(const rect& aRect, const pen& aPen, const brush& aFill = brush{}) const override;
        void draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen, const brush& aFill = brush{}) const override;
        void draw_circle(const point& aCenter, dimension aRadius, const pen& aPen, const brush& aFill = brush{}, angle aStartAngle = 0.0) const override;
        void draw_arc(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen, const brush& aFill = brush{}) const override;
        void draw_path(const path& aPath, const pen& aPen, const brush& aFill = brush{}) const override;
        void draw_shape(const game::mesh& aShape, const vec3& aPosition, const pen& aPen, const brush& aFill = brush{}) const override;
        void draw_entities(game::i_ecs& aEcs, int32_t aLayer = 0) const override;
        void draw_focus_rect(const rect& aRect) const override;
        void fill_rect(const rect& aRect, const brush& aFill, scalar aZpos = 0.0) const override;
        void fill_rounded_rect(const rect& aRect, dimension aRadius, const brush& aFill) const override;
        void fill_circle(const point& aCenter, dimension aRadius, const brush& aFill) const override;
        void fill_arc(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill) const override;
        void fill_path(const path& aPath, const brush& aFill) const override;
        void fill_shape(const game::mesh& aShape, const vec3& aPosition, const brush& aFill) const override;
        size text_extent(const std::string& aText, const font& aFont) const override;
        size text_extent(const std::string& aText, std::function<font(std::string::size_type)> aFontSelector) const override;
        size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont) const override;
        size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const override;
        size multiline_text_extent(const std::string& aText, const font& aFont) const override;
        size multiline_text_extent(const std::string& aText, std::function<font(std::string::size_type)> aFontSelector) const override;
        size multiline_text_extent(const std::string& aText, const font& aFont, dimension aMaxWidth) const override;
        size multiline_text_extent(const std::string& aText, std::function<font(std::string::size_type)> aFontSelector, dimension aMaxWidth) const override;
        size glyph_text_extent(const glyph_text& aText) const override;
        size glyph_text_extent(const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd) const override;
        size multiline_glyph_text_extent(const glyph_text& aText, dimension aMaxWidth) const override;
        glyph_text to_glyph_text(const std::string& aText, const font& aFont) const override;
        glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont) const override;
        glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const override;
        glyph_text to_glyph_text(const std::u32string& aText, const font& aFont) const override;
        glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont) const override;
        glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const override;
        multiline_glyph_text to_multiline_glyph_text(const std::string& aText, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(const std::u32string& aText, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        multiline_glyph_text to_multiline_glyph_text(const glyph_text& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const override;
        bool is_text_left_to_right(const std::string& aText, const font& aFont) const override;
        bool is_text_right_to_left(const std::string& aText, const font& aFont) const override;
        void draw_text(const point& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance) const override;
        void draw_text(const point& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, const text_appearance& aAppearance) const override;
        void draw_text(const vec3& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance) const override;
        void draw_text(const vec3& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, const text_appearance& aAppearance) const override;
        void draw_multiline_text(const point& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const override;
        void draw_multiline_text(const point& aPoint, const std::string& aText, const font& aFont, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const override;
        void draw_multiline_text(const vec3& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const override;
        void draw_multiline_text(const vec3& aPoint, const std::string& aText, const font& aFont, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const override;
        void draw_glyph_text(const point& aPoint, const glyph_text& aText, const text_appearance& aAppearance) const override;
        void draw_glyph_text(const point& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const text_appearance& aAppearance) const override;
        void draw_glyph_text(const vec3& aPoint, const glyph_text& aText, const text_appearance& aAppearance) const override;
        void draw_glyph_text(const vec3& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const text_appearance& aAppearance) const override;
        void draw_multiline_glyph_text(const point& aPoint, const glyph_text& aText, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const override;
        void draw_multiline_glyph_text(const vec3& aPoint, const glyph_text& aText, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const override;
        void draw_glyph(const point& aPoint, const glyph& aGlyph, const text_appearance& aAppearance) const override;
        void draw_glyph(const vec3& aPoint, const glyph& aGlyph, const text_appearance& aAppearance) const override;
        void draw_glyph_underline(const point& aPoint, const glyph& aGlyph, const text_appearance& aAppearance) const override;
        void draw_glyph_underline(const vec3& aPoint, const glyph& aGlyph, const text_appearance& aAppearance) const override;
        void set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix = '&') const override;
        void unset_mnemonic() const override;
        bool mnemonics_shown() const override;
        bool password() const override;
        const std::string& password_mask() const override;
        void set_password(bool aPassword, const std::string& aMask = "\xE2\x97\x8F") override;
        void draw_texture(const point& aPoint, const i_texture& aTexture, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const override;
        void draw_texture(const rect& aRect, const i_texture& aTexture, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const override;
        void draw_texture(const point& aPoint, const i_texture& aTexture, const rect& aTextureRect, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const override;
        void draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const override;
        void draw_texture(const game::mesh& aMesh, const i_texture& aTexture, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const override;
        void draw_texture(const game::mesh& aMesh, const i_texture& aTexture, const rect& aTextureRect, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const override;
        void draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const optional_mat44& aTransformation = optional_mat44{}, const std::optional<game::filter>& aFilter = {}) const override;

        // implementation
        // from i_rendering_context
    public:
        neogfx::subpixel_format subpixel_format() const override;
        // from i_device_metrics
    public:
        bool metrics_available() const override;
        size extents() const override;
        dimension horizontal_dpi() const override;
        dimension vertical_dpi() const override;
        dimension ppi() const override;
        dimension em_size() const override;
        // from i_units_context
    public:
        bool high_dpi() const override;
        dimension dpi_scale_factor() const override;
    public:
        bool device_metrics_available() const override;
        const i_device_metrics& device_metrics() const override;
    protected:
        bool attached() const;
        i_rendering_context& native_context() const;
        // helpers
        // own
    private:
        glyph_text to_glyph_text_impl(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const;
        glyph_text to_glyph_text_impl(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const;
        // attributes
    private:
        type iType;
        const i_render_target& iRenderTarget;
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
        struct glyph_text_data;
        std::unique_ptr<glyph_text_data> iGlyphTextData;
        mutable std::optional<size> iPreviousPingPongBufferSize;
    };
}