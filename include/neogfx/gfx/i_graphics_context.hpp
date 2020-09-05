// i_graphics_context.hpp
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
#include <memory>
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4459 ) // declaration of 'name' hides global declaration
#endif
#include <boost/multi_array.hpp>
#ifdef _WIN32
#pragma warning( pop )
#endif
#include <optional>
#include <neogfx/core/primitives.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/gfx/sub_texture.hpp>
#include <neogfx/gfx/path.hpp>
#include <neogfx/gfx/pen.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/material.hpp>

namespace neogfx
{
    class i_surface;
    class i_texture;
    class i_widget;
    class i_render_target;

    class i_graphics_context;

    struct ping_pong_buffers
    {
        scoped_render_target srt;
        std::unique_ptr<i_graphics_context> buffer1;
        std::unique_ptr<i_graphics_context> buffer2;
    };
        
    // todo: move to i_string

    class i_graphics_context : public i_rendering_context, public i_device_metrics, public i_units_context
    {
        // types
    public:
        enum class type
        {
            Attached,
            Unattached
        };
        struct multiline_glyph_text
        {
            glyph_text glyphText;
            struct line
            {
                point pos;
                size extents;
                glyph_text::difference_type begin;
                glyph_text::difference_type end;
            };
            typedef neolib::vecarray<line, 8> lines_t;
            lines_t lines;
        };
        // exceptions
    public:
        struct unattached : std::logic_error { unattached() : std::logic_error("neogfx::i_graphics_context::unattached") {} };
        struct password_not_set : std::logic_error { password_not_set() : std::logic_error("neogfx::i_graphics_context::password_not_set") {} };
        // construction
    public:
        virtual ~i_graphics_context() = default;
        // operations
    public:
        virtual neogfx::ping_pong_buffers ping_pong_buffers(const size& aExtents, texture_sampling aSampling = texture_sampling::Multisample, const optional_color& aClearColor = color{ vec4{0.0, 0.0, 0.0, 0.0} }) const = 0;
    public:
        virtual delta to_device_units(const delta& aValue) const = 0;
        virtual size to_device_units(const size& aValue) const = 0;
        virtual point to_device_units(const point& aValue) const = 0;
        virtual vec2 to_device_units(const vec2& aValue) const = 0;
        virtual rect to_device_units(const rect& aValue) const = 0;
        virtual path to_device_units(const path& aValue) const = 0;
        virtual delta from_device_units(const delta& aValue) const = 0;
        virtual size from_device_units(const size& aValue) const = 0;
        virtual point from_device_units(const point& aValue) const = 0;
        virtual rect from_device_units(const rect& aValue) const = 0;
        virtual path from_device_units(const path& aValue) const = 0;
        virtual int32_t layer() const = 0;
        virtual void set_layer(int32_t aLayer) = 0;
        virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
        virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) const = 0;
        virtual neogfx::logical_coordinates logical_coordinates() const = 0;
        virtual void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) const = 0;
        virtual void set_default_font(const font& aDefaultFont) const = 0;
        virtual void set_extents(const size& aExtents) const = 0;
        virtual void set_origin(const point& aOrigin) const = 0;
        virtual point origin() const = 0;
        virtual void flush() const = 0;
        virtual void scissor_on(const rect& aRect) const = 0;
        virtual void scissor_off() const = 0;
        virtual bool snap_to_pixel() const = 0;
        virtual void set_snap_to_pixel(bool aSnap) const = 0;
        virtual double opacity() const = 0;
        virtual void set_opacity(double aOpacity) const = 0;
        virtual neogfx::blending_mode blending_mode() const = 0;
        virtual void set_blending_mode(neogfx::blending_mode aBlendingMode) const = 0;
        virtual neogfx::smoothing_mode smoothing_mode() const = 0;
        virtual void set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode) const = 0;
        virtual void push_logical_operation(logical_operation aLogicalOperation) const = 0;
        virtual void pop_logical_operation() const = 0;
        virtual void line_stipple_on(scalar aFactor, uint16_t aPattern, scalar aPosition = 0.0) const = 0;
        virtual void line_stipple_off() const = 0;
        virtual bool is_subpixel_rendering_on() const = 0;
        virtual void subpixel_rendering_on() const = 0;
        virtual void subpixel_rendering_off() const = 0;
        virtual void clear(const color& aColor, const std::optional<scalar>& aZpos = std::optional<scalar>{}) const = 0;
        virtual void clear_depth_buffer() const = 0;
        virtual void clear_stencil_buffer() const = 0;
        virtual void blit(const rect& aDestinationRect, const i_graphics_context& aSource, const rect& aSourceRect) const = 0;
        virtual void blur(const rect& aDestinationRect, const i_graphics_context& aSource, const rect& aSourceRect, dimension aRadius, blurring_algorithm aAlgorithm = blurring_algorithm::Gaussian, scalar aParameter1 = 5, scalar aParameter2 = 1.0) const = 0;
        virtual void clear_gradient() = 0;
        virtual void set_gradient(const gradient& aGradient, const rect& aBoundingBox) = 0;
        virtual void set_pixel(const point& aPoint, const color& aColor) const = 0;
        virtual void draw_pixel(const point& aPoint, const color& aColor) const = 0;
        virtual void draw_line(const point& aFrom, const point& aTo, const pen& aPen) const = 0;
        virtual void draw_rect(const rect& aRect, const pen& aPen, const brush& aFill = brush{}) const = 0;
        virtual void draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen, const brush& aFill = brush{}) const = 0;
        virtual void draw_circle(const point& aCenter, dimension aRadius, const pen& aPen, const brush& aFill = brush{}, angle aStartAngle = 0.0) const = 0;
        virtual void draw_arc(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen, const brush& aFill = brush{}) const = 0;
        virtual void draw_path(const path& aPath, const pen& aPen, const brush& aFill = brush{}) const = 0;
        virtual void draw_shape(const game::mesh& aShape, const vec3& aPosition, const pen& aPen, const brush& aFill = brush{}) const = 0;
        virtual void draw_entities(game::i_ecs& aEcs, int32_t aLayer = 0) const = 0;
        virtual void draw_focus_rect(const rect& aRect) const = 0;
        virtual void fill_rect(const rect& aRect, const brush& aFill, scalar aZpos = 0.0) const = 0;
        virtual void fill_rounded_rect(const rect& aRect, dimension aRadius, const brush& aFill) const = 0;
        virtual void fill_circle(const point& aCenter, dimension aRadius, const brush& aFill) const = 0;
        virtual void fill_arc(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill) const = 0;
        virtual void fill_path(const path& aPath, const brush& aFill) const = 0;
        virtual void fill_shape(const game::mesh& aShape, const vec3& aPosition, const brush& aFill) const = 0;
        virtual size text_extent(const std::string& aText, const font& aFont) const = 0;
        virtual size text_extent(const std::string& aText, std::function<font(std::string::size_type)> aFontSelector) const = 0;
        virtual size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont) const = 0;
        virtual size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const = 0;
        virtual size multiline_text_extent(const std::string& aText, const font& aFont) const = 0;
        virtual size multiline_text_extent(const std::string& aText, std::function<font(std::string::size_type)> aFontSelector) const = 0;
        virtual size multiline_text_extent(const std::string& aText, const font& aFont, dimension aMaxWidth) const = 0;
        virtual size multiline_text_extent(const std::string& aText, std::function<font(std::string::size_type)> aFontSelector, dimension aMaxWidth) const = 0;
        virtual size glyph_text_extent(const glyph_text& aText) const = 0;
        virtual size glyph_text_extent(const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd) const = 0;
        virtual size multiline_glyph_text_extent(const glyph_text& aText, dimension aMaxWidth) const = 0;
        virtual glyph_text to_glyph_text(const std::string& aText, const font& aFont) const = 0;
        virtual glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont) const = 0;
        virtual glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const = 0;
        virtual glyph_text to_glyph_text(const std::u32string& aText, const font& aFont) const = 0;
        virtual glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont) const = 0;
        virtual glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(const std::string& aText, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(const std::u32string& aText, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(const glyph_text& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual bool is_text_left_to_right(const std::string& aText, const font& aFont) const = 0;
        virtual bool is_text_right_to_left(const std::string& aText, const font& aFont) const = 0;
        virtual void draw_text(const point& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance) const = 0;
        virtual void draw_text(const point& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, const text_appearance& aAppearance) const = 0;
        virtual void draw_text(const vec3& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance) const = 0;
        virtual void draw_text(const vec3& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, const text_appearance& aAppearance) const = 0;
        virtual void draw_multiline_text(const point& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_text(const point& aPoint, const std::string& aText, const font& aFont, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_text(const vec3& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_text(const vec3& aPoint, const std::string& aText, const font& aFont, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_glyph_text(const point& aPoint, const glyph_text& aText, const text_appearance& aAppearance) const = 0;
        virtual void draw_glyph_text(const point& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const text_appearance& aAppearance) const = 0;
        virtual void draw_glyph_text(const vec3& aPoint, const glyph_text& aText, const text_appearance& aAppearance) const = 0;
        virtual void draw_glyph_text(const vec3& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const text_appearance& aAppearance) const = 0;
        virtual void draw_multiline_glyph_text(const point& aPoint, const glyph_text& aText, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_glyph_text(const vec3& aPoint, const glyph_text& aText, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_glyph(const point& aPoint, const glyph& aGlyph, const text_appearance& aAppearance) const = 0;
        virtual void draw_glyph(const vec3& aPoint, const glyph& aGlyph, const text_appearance& aAppearance) const = 0;
        virtual void draw_glyph_underline(const point& aPoint, const glyph& aGlyph, const text_appearance& aAppearance) const = 0;
        virtual void draw_glyph_underline(const vec3& aPoint, const glyph& aGlyph, const text_appearance& aAppearance) const = 0;
        virtual void set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix = '&') const = 0;
        virtual void unset_mnemonic() const = 0;
        virtual bool mnemonics_shown() const = 0;
        virtual bool password() const = 0;
        virtual const std::string& password_mask() const = 0;
        virtual void set_password(bool aPassword, const std::string& aMask = "\xE2\x97\x8F") = 0;
        virtual void draw_texture(const point& aPoint, const i_texture& aTexture, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_texture(const rect& aRect, const i_texture& aTexture, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_texture(const point& aPoint, const i_texture& aTexture, const rect& aTextureRect, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_texture(const game::mesh& aMesh, const i_texture& aTexture, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_texture(const game::mesh& aMesh, const i_texture& aTexture, const rect& aTextureRect, const optional_color& aColor = optional_color(), shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const optional_mat44& aTransformation = optional_mat44{}, const std::optional<game::filter>& aFilter = {}) const = 0;
    };

    template <typename Iter>
    inline void draw_glyph_text_normal(const i_graphics_context& aGc, const vec3& aPoint, const glyph_text& aGlyphText, Iter aGlyphTextBegin, Iter aGlyphTextEnd, const text_appearance& aAppearance)
    {
        vec3 pos = aPoint;
        for (auto iterGlyph = aGlyphTextBegin; iterGlyph != aGlyphTextEnd; ++iterGlyph)
        {
            aGc.draw_glyph(pos + iterGlyph->offset().to_vec3(), *iterGlyph, aAppearance);
            pos.x += iterGlyph->advance().cx;
        }
    }

    template <typename Iter>
    inline void draw_glyph_text_glow_pass_1(const neogfx::ping_pong_buffers& aPingPongBuffers, const glyph_text& aGlyphText, Iter aGlyphTextBegin, Iter aGlyphTextEnd, const text_color& aGlowColor, dimension aGlowSize)
    {
        scoped_render_target srt{ *aPingPongBuffers.buffer1 };
        point const effectOffset{ aGlowSize, aGlowSize };
        vec3 pos{ effectOffset.x, effectOffset.y, 0.0 };
        for (auto iterGlyph = aGlyphTextBegin; iterGlyph != aGlyphTextEnd; ++iterGlyph)
        {
            aPingPongBuffers.buffer1->draw_glyph(pos + iterGlyph->offset().to_vec3(), *iterGlyph, aGlowColor);
            pos.x += iterGlyph->advance().cx;
        }
    }

    template <typename Iter>
    inline void draw_glyph_text_glow_pass_2(const neogfx::ping_pong_buffers& aPingPongBuffers, const glyph_text& aGlyphText, Iter aGlyphTextBegin, Iter aGlyphTextEnd, dimension aGlowSize)
    {
        scoped_render_target srt{ *aPingPongBuffers.buffer2 };
        point const effectOffset{ aGlowSize, aGlowSize };
        auto const effectExtents = aGlyphText.extents(aGlyphTextBegin, aGlyphTextEnd) + effectOffset * 2.0;
        rect const effectRect{ point{}, effectExtents };
        aPingPongBuffers.buffer2->blur(effectRect, *aPingPongBuffers.buffer1, effectRect, aGlowSize, blurring_algorithm::Gaussian, 5.0, 1.0);
    }

    template <typename Iter>
    inline void draw_glyph_text_glow_pass_3(const i_graphics_context& aGc, const neogfx::ping_pong_buffers& aPingPongBuffers, const vec3& aPoint, const glyph_text& aGlyphText, Iter aGlyphTextBegin, Iter aGlyphTextEnd, dimension aGlowSize)
    {
        point const effectOffset{ aGlowSize, aGlowSize };
        auto const effectExtents = aGlyphText.extents(aGlyphTextBegin, aGlyphTextEnd) + effectOffset * 2.0;
        rect const effectRect{ point{}, effectExtents };
        rect const outputRect{ point{ aPoint } -effectOffset, effectExtents };
        aGc.blit(outputRect, *aPingPongBuffers.buffer2, effectRect);
    }

    template <typename Iter>
    inline void draw_glyph_text_glow(const i_graphics_context& aGc, const vec3& aPoint, const glyph_text& aGlyphText, Iter aGlyphTextBegin, Iter aGlyphTextEnd, const text_color& aGlowColor, dimension aGlowSize)
    {
        point const effectOffset{ aGlowSize, aGlowSize };
        auto const effectExtents = aGlyphText.extents(aGlyphTextBegin, aGlyphTextEnd) + effectOffset * 2.0;
        auto pingPongBuffers = aGc.ping_pong_buffers(effectExtents);
        draw_glyph_text_glow_pass_1(pingPongBuffers, aGlyphText, aGlyphTextBegin, aGlyphTextEnd, aGlowColor, aGlowSize);
        draw_glyph_text_glow_pass_2(pingPongBuffers, aGlyphText, aGlyphTextBegin, aGlyphTextEnd, aGlowSize);
        draw_glyph_text_glow_pass_3(aGc, pingPongBuffers, aPoint, aGlyphText, aGlyphTextBegin, aGlyphTextEnd, aGlowSize);
    }

    template <typename Iter>
    inline void draw_glyph_text(const i_graphics_context& aGc, const vec3& aPoint, const glyph_text& aGlyphText, Iter aGlyphTextBegin, Iter aGlyphTextEnd, const text_appearance& aAppearance)
    {
        if (aAppearance.effect() && aAppearance.effect()->type() == text_effect_type::Glow)
            draw_glyph_text_glow(aGc, aPoint, aGlyphText, aGlyphTextBegin, aGlyphTextEnd, aAppearance.effect()->color(), aAppearance.effect()->width());
        else if (aAppearance.effect() && aAppearance.effect()->type() == text_effect_type::Shadow)
            draw_glyph_text_glow(aGc, aPoint, aGlyphText, aGlyphTextBegin, aGlyphTextEnd, aAppearance.effect()->color(), aAppearance.effect()->width());
        draw_glyph_text_normal(aGc, aPoint, aGlyphText, aGlyphTextBegin, aGlyphTextEnd, aAppearance);
    }

    class scoped_mnemonics
    {
    public:
        scoped_mnemonics(const i_graphics_context& aGc, bool aShowMnemonics, char aMnemonicPrefix = '&') :
            iGc(aGc)
        {
            iGc.set_mnemonic(aShowMnemonics, aMnemonicPrefix);
        }
        ~scoped_mnemonics()
        {
            iGc.unset_mnemonic();
        }
    private:
        const i_graphics_context& iGc;
    };

    class scoped_coordinate_system
    {
    public:
        scoped_coordinate_system(const i_graphics_context& aGc, const point& aOrigin, const size& aExtents, logical_coordinate_system aCoordinateSystem) :
            iGc(aGc), iPreviousCoordinateSystem(aGc.logical_coordinate_system()), iPreviousCoordinates(aGc.logical_coordinates())
        {
            iGc.set_logical_coordinate_system(aCoordinateSystem);
            apply_origin(aOrigin, aExtents);
        }
        scoped_coordinate_system(const i_graphics_context& aGc, const point& aOrigin, const size& aExtents, logical_coordinate_system aCoordinateSystem, const neogfx::logical_coordinates& aCoordinates) :
            iGc(aGc), iPreviousCoordinateSystem(aGc.logical_coordinate_system()), iPreviousCoordinates(aGc.logical_coordinates())
        {
            iGc.set_logical_coordinate_system(aCoordinateSystem);
            iGc.set_logical_coordinates(aCoordinates);
            apply_origin(aOrigin, aExtents);
        }
        ~scoped_coordinate_system()
        {
            if (iGc.logical_coordinate_system() != iPreviousCoordinateSystem)
                iGc.set_logical_coordinate_system(iPreviousCoordinateSystem);
            if (iGc.logical_coordinates() != iPreviousCoordinates)
                iGc.set_logical_coordinates(iPreviousCoordinates);
        }
    private:
        void apply_origin(const point& aOrigin, const size& aExtents)
        {
            if (iGc.logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui)
                iGc.set_origin(aOrigin);
            else if (iGc.logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGame)
                iGc.set_origin(point{ aOrigin.x, iGc.render_target().extents().cy - (aOrigin.y + aExtents.cy) });
        }
    private:
        const i_graphics_context& iGc;
        logical_coordinate_system iPreviousCoordinateSystem;
        neogfx::logical_coordinates iPreviousCoordinates;
    };

    class scoped_snap_to_pixel
    {
    public:
        scoped_snap_to_pixel(const i_graphics_context& aGc, bool aSnapToPixel = true) :
            iGc{ aGc }, iPreviousSnapped{ aGc.snap_to_pixel() }
        {
            iGc.set_snap_to_pixel(aSnapToPixel);
        }
        ~scoped_snap_to_pixel()
        {
            iGc.set_snap_to_pixel(iPreviousSnapped);
        }
    private:
        const i_graphics_context& iGc;
        bool iPreviousSnapped;
    };

    class scoped_opacity
    {
    public:
        scoped_opacity(const i_graphics_context& aGc, double aOpacity) :
            iGc{ aGc }, iPreviousOpacity { aGc.opacity() }
        {
            iGc.set_opacity(iGc.opacity() * aOpacity);
        }
        ~scoped_opacity()
        {
            iGc.set_opacity(iPreviousOpacity);
        }
    private:
        const i_graphics_context& iGc;
        double iPreviousOpacity;
    };

    class scoped_blending_mode
    {
    public:
        scoped_blending_mode(const i_graphics_context& aGc, neogfx::blending_mode aBlendigMode) :
            iGc{ aGc }, iPreviousBlendingMode{ aGc.blending_mode() }
        {
            iGc.set_blending_mode(aBlendigMode);
        }
        ~scoped_blending_mode()
        {
            iGc.set_blending_mode(iPreviousBlendingMode);
        }
    private:
        const i_graphics_context& iGc;
        neogfx::blending_mode iPreviousBlendingMode;
    };

    class scoped_scissor
    {
    public:
        scoped_scissor(const i_graphics_context& aGc, const rect& aScissorRect) :
            iGc{ aGc }
        {
            iGc.scissor_on(aScissorRect);
        }
        ~scoped_scissor()
        {
            iGc.scissor_off();
        }
    private:
        const i_graphics_context& iGc;
    };

    template <typename ValueType = double, uint32_t W = 5>
    inline std::array<std::array<ValueType, W>, W> static_gaussian_filter(ValueType aSigma = 1.0)
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

    template <typename ValueType = double>
    inline boost::multi_array<ValueType, 2> dynamic_gaussian_filter(uint32_t aKernelSize = 5, ValueType aSigma = 1.0)
    {
        const int32_t mean = static_cast<int32_t>(aKernelSize / 2);
        boost::multi_array<ValueType, 2> kernel(boost::extents[aKernelSize][aKernelSize]);
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
            for (uint32_t x = 0; x < aKernelSize; ++x)
                for (uint32_t y = 0; y < aKernelSize; ++y)
                    kernel[x][y] /= sum;
        }
        else
            kernel[mean][mean] = static_cast<ValueType>(1.0);
        return kernel;
    }
}