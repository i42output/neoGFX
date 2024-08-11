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
#include <neogfx/gfx/view.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/material.hpp>

namespace neogfx
{
    class i_surface;
    class i_texture;
    class i_widget;
    class i_render_target;

    class i_graphics_context;

    using layer_t = std::int32_t;
    layer_t constexpr LayerWidget       = 0x00000000u;
    layer_t constexpr LayerNestedWindow = 0x10000000u;

    struct ping_pong_buffers
    {
        scoped_render_target srt;
        std::unique_ptr<i_graphics_context> buffer1;
        std::unique_ptr<i_graphics_context> buffer2;
    };
        
    ping_pong_buffers create_ping_pong_buffers(const i_rendering_context& aContext, const size& aExtents, texture_sampling aSampling = texture_sampling::Multisample, const optional_color& aClearColor = color{ vec4{0.0, 0.0, 0.0, 0.0} });

    struct tab_stop
    {
        typedef tab_stop abstract_type;

        scalar pos;
        neogfx::alignment alignment = neogfx::alignment::Left;

        std::partial_ordering operator<=>(tab_stop const&) const noexcept = default;
    };

    class i_tab_stops
    {
    public:
        virtual tab_stop const& default_stop() const = 0;
        virtual neolib::i_vector<tab_stop> const& stops() const = 0;
    };

    class tab_stops : public i_tab_stops
    {
    public:
        tab_stops(tab_stop const& aDefaultStop) :
            iDefaultStop{ aDefaultStop }
        {
        }
        template <typename TabStopIter>
        tab_stops(tab_stop const& aDefaultStop, TabStopIter aStopsBegin, TabStopIter aStopsEnd) :
            iDefaultStop{ aDefaultStop },
            iStops{ aStopsBegin, aStopsEnd }
        {
        }
        tab_stops(i_tab_stops const& aOther) :
            tab_stops(aOther.default_stop(), aOther.stops().begin(), aOther.stops().end())
        {
        }
    public:
        bool operator==(tab_stops const& aOther) const noexcept
        {
            return std::forward_as_tuple(iDefaultStop, iStops) == std::forward_as_tuple(aOther.iDefaultStop, aOther.iStops);
        }
        std::partial_ordering operator<=>(tab_stops const& aOther) const noexcept
        {
            return std::forward_as_tuple(iDefaultStop, iStops) <=> std::forward_as_tuple(aOther.iDefaultStop, aOther.iStops);
        }
    public:
        tab_stop const& default_stop() const override 
        {
            return iDefaultStop;
        }
        neolib::i_vector<tab_stop> const& stops() const override
        {
            return iStops;
        }
    private:
        tab_stop iDefaultStop;
        neolib::vector<tab_stop> iStops;
    };
        
    class i_graphics_context : public i_rendering_context, public i_device_metrics, public i_units_context
    {
        // types
    public:
        enum class type
        {
            Attached,
            Unattached
        };
        // exceptions
    public:
        struct unattached : std::logic_error { unattached() : std::logic_error("neogfx::i_graphics_context::unattached") {} };
        struct no_tab_stops : std::logic_error { no_tab_stops() : std::logic_error("neogfx::i_graphics_context::no_tab_stops") {} };
        struct password_not_set : std::logic_error { password_not_set() : std::logic_error("neogfx::i_graphics_context::password_not_set") {} };
        // construction
    public:
        virtual ~i_graphics_context() = default;
        // units
    public:
        virtual delta to_device_units(delta const& aValue) const = 0;
        virtual size to_device_units(size const& aValue) const = 0;
        virtual point to_device_units(point const& aValue) const = 0;
        virtual vec2 to_device_units(vec2 const& aValue) const = 0;
        virtual rect to_device_units(rect const& aValue) const = 0;
        virtual path to_device_units(path const& aValue) const = 0;
        virtual delta from_device_units(delta const& aValue) const = 0;
        virtual size from_device_units(size const& aValue) const = 0;
        virtual point from_device_units(point const& aValue) const = 0;
        virtual rect from_device_units(rect const& aValue) const = 0;
        virtual path from_device_units(path const& aValue) const = 0;
        // state
    public:
        virtual void flush() const = 0;
        // layers
    public:
        virtual layer_t layer() const = 0;
        virtual void set_layer(layer_t aLayer) = 0;
        // coordinate system
    public:
        virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
        virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) const = 0;
        virtual neogfx::logical_coordinates logical_coordinates() const = 0;
        virtual void set_logical_coordinates(neogfx::logical_coordinates const& aCoordinates) const = 0;
        // viewport
    public:
        virtual void set_extents(size const& aExtents) const = 0;
        virtual void set_origin(point const& aOrigin) const = 0;
        virtual point origin() const = 0;
        virtual void set_viewport(optional_rect const& aViewport = {}) const = 0;
        virtual void set_view_transforamtion(optional_mat33 const& aViewTransforamtion = {}) const = 0;
        // clipping
    public:
        virtual void scissor_on(rect const& aRect) const = 0;
        virtual void scissor_off() const = 0;
        // anti-aliasing
    public:
        virtual neogfx::smoothing_mode smoothing_mode() const = 0;
        virtual void set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode) const = 0;
        virtual bool snap_to_pixel() const = 0;
        virtual void set_snap_to_pixel(bool aSnap) const = 0;
        // blending
    public:
        virtual double opacity() const = 0;
        virtual void set_opacity(double aOpacity) const = 0;
        virtual neogfx::blending_mode blending_mode() const = 0;
        virtual void set_blending_mode(neogfx::blending_mode aBlendingMode) const = 0;
        // drawing mode
    public:
        virtual void push_logical_operation(logical_operation aLogicalOperation) const = 0;
        virtual void pop_logical_operation() const = 0;
        virtual std::optional<stipple> const& line_stipple() const = 0;
        virtual void line_stipple_on(stipple const& aStipple) const = 0;
        virtual void line_stipple_off() const = 0;
        virtual bool is_subpixel_rendering_on() const = 0;
        virtual void subpixel_rendering_on() const = 0;
        virtual void subpixel_rendering_off() const = 0;
        // canvas
    public:
        virtual void clear(color const& aColor, std::optional<scalar> const& aZpos = std::optional<scalar>{}) const = 0;
        virtual void clear_depth_buffer() const = 0;
        virtual void clear_stencil_buffer() const = 0;
        virtual void blit(rect const& aDestinationRect, i_graphics_context const& aSource, rect const& aSourceRect) const = 0;
        virtual void blur(rect const& aDestinationRect, i_graphics_context const& aSource, rect const& aSourceRect, dimension aRadius, blurring_algorithm aAlgorithm = blurring_algorithm::Gaussian, scalar aParameter1 = 5, scalar aParameter2 = 1.0) const = 0;
        // gradient
    public:
        virtual void clear_gradient() = 0;
        virtual void set_gradient(gradient const& aGradient, rect const& aBoundingBox) = 0;
        // shape
    public:
        virtual void set_pixel(point const& aPoint, color const& aColor) const = 0;
        virtual void draw_pixel(point const& aPoint, color const& aColor) const = 0;
        virtual void draw_line(point const& aFrom, point const& aTo, pen const& aPen) const = 0;
        virtual void draw_triangle(point const& aP0, point const& aP1, point const& aP2, pen const& aPen, brush const& aFill = brush{}) const = 0;
        virtual void draw_rect(rect const& aRect, pen const& aPen, brush const& aFill = brush{}) const = 0;
        virtual void draw_rounded_rect(rect const& aRect, vec4 const& aRadius, pen const& aPen, brush const& aFill = brush{}) const = 0;
        virtual void draw_ellipse_rect(rect const& aRect, vec4 const& aRadiusX, vec4 const& aRadiusY, pen const& aPen, brush const& aFill = brush{}) const = 0;
        virtual void draw_checker_rect(rect const& aRect, size const& aSquareSize, pen const& aPen, brush const& aFill1, brush const& aFill2) const = 0;
        virtual void draw_circle(point const& aCenter, dimension aRadius, pen const& aPen, brush const& aFill = brush{}) const = 0;
        virtual void draw_ellipse(point const& aCenter, dimension aRadiusA, dimension aRadiusB, pen const& aPen, brush const& aFill = brush{}) const = 0;
        virtual void draw_pie(point const& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, pen const& aPen, brush const& aFill = brush{}) const = 0;
        virtual void draw_arc(point const& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, pen const& aPen, brush const& aFill = brush{}) const = 0;
        virtual void draw_cubic_bezier(point const& aP0, point const& aP1, point const& aP2, point const& aP3, pen const& aPen) const = 0;
        virtual void draw_path(path const& aPath, pen const& aPen, brush const& aFill = brush{}) const = 0;
        virtual void draw_shape(game::mesh const& aShape, vec3 const& aPosition, pen const& aPen, brush const& aFill = brush{}) const = 0;
        virtual void draw_entities(game::i_ecs& aEcs, game::scene_layer aLayer = 0) const = 0;
        virtual void draw_focus_rect(rect const& aRect) const = 0;
        // text
    public:
        virtual font const& default_font() const = 0;
        virtual void set_default_font(font const& aDefaultFont) const = 0;
        virtual bool has_tab_stops() const = 0;
        virtual i_tab_stops const& tab_stops() const = 0;
        virtual void set_tab_stops(i_tab_stops const& aTabStops) = 0;
        virtual void clear_tab_stops() = 0;
        virtual size text_extent(std::string const& aText) const = 0;
        virtual size text_extent(std::string const& aText, font const& aFont) const = 0;
        virtual size text_extent(std::string const& aText, std::function<font(std::string::size_type)> aFontSelector) const = 0;
        virtual size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd) const = 0;
        virtual size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, font const& aFont) const = 0;
        virtual size text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const = 0;
        virtual size multiline_text_extent(std::string const& aText) const = 0;
        virtual size multiline_text_extent(std::string const& aText, font const& aFont) const = 0;
        virtual size multiline_text_extent(std::string const& aText, std::function<font(std::string::size_type)> aFontSelector) const = 0;
        virtual size multiline_text_extent(std::string const& aText, dimension aMaxWidth) const = 0;
        virtual size multiline_text_extent(std::string const& aText, font const& aFont, dimension aMaxWidth) const = 0;
        virtual size multiline_text_extent(std::string const& aText, std::function<font(std::string::size_type)> aFontSelector, dimension aMaxWidth) const = 0;
        virtual size glyph_text_extent(glyph_text const& aText) const = 0;
        virtual size glyph_text_extent(glyph_text const& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd) const = 0;
        virtual size multiline_glyph_text_extent(glyph_text const& aText, dimension aMaxWidth) const = 0;
        virtual glyph_text to_glyph_text(std::string const& aText) const = 0;
        virtual glyph_text to_glyph_text(std::string const& aText, std::function<font(std::string::size_type)> aFontSelector) const = 0;
        virtual glyph_text to_glyph_text(std::string const& aText, font const& aFont) const = 0;
        virtual glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd) const = 0;
        virtual glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, font const& aFont) const = 0;
        virtual glyph_text to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const = 0;
        virtual glyph_text to_glyph_text(std::u32string const& aText) const = 0;
        virtual glyph_text to_glyph_text(std::u32string const& aText, font const& aFont) const = 0;
        virtual glyph_text to_glyph_text(std::u32string const& aText, std::function<font(std::u32string::size_type)> aFontSelector) const = 0;
        virtual glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd) const = 0;
        virtual glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, font const& aFont) const = 0;
        virtual glyph_text to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::string const& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::string const& aText, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::u32string const& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::u32string const& aText, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, font const& aFont, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual multiline_glyph_text to_multiline_glyph_text(glyph_text const& aText, dimension aMaxWidth, alignment aAlignment = alignment::Left) const = 0;
        virtual bool is_text_left_to_right(std::string const& aText) const = 0;
        virtual bool is_text_left_to_right(std::string const& aText, font const& aFont) const = 0;
        virtual bool is_text_right_to_left(std::string const& aText) const = 0;
        virtual bool is_text_right_to_left(std::string const& aText, font const& aFont) const = 0;
        virtual void draw_text(point const& aPoint, std::string const& aText, text_format const& aTextFormat) const = 0;
        virtual void draw_text(point const& aPoint, std::string const& aText, font const& aFont, text_format const& aTextFormat) const = 0;
        virtual void draw_text(point const& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, text_format const& aTextFormat) const = 0;
        virtual void draw_text(point const& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, font const& aFont, text_format const& aTextFormat) const = 0;
        virtual void draw_text(vec3 const& aPoint, std::string const& aText, text_format const& aTextFormat) const = 0;
        virtual void draw_text(vec3 const& aPoint, std::string const& aText, font const& aFont, text_format const& aTextFormat) const = 0;
        virtual void draw_text(vec3 const& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, text_format const& aTextFormat) const = 0;
        virtual void draw_text(vec3 const& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, font const& aFont, text_format const& aTextFormat) const = 0;
        virtual void draw_multiline_text(point const& aPoint, std::string const& aText, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_text(point const& aPoint, std::string const& aText, font const& aFont, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_text(point const& aPoint, std::string const& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_text(point const& aPoint, std::string const& aText, font const& aFont, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_text(vec3 const& aPoint, std::string const& aText, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_text(vec3 const& aPoint, std::string const& aText, font const& aFont, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_text(vec3 const& aPoint, std::string const& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_text(vec3 const& aPoint, std::string const& aText, font const& aFont, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_glyph_text(point const& aPoint, glyph_text const& aText, text_format const& aTextFormat) const = 0;
        virtual void draw_glyph_text(point const& aPoint, glyph_text const& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, text_format const& aTextFormat) const = 0;
        virtual void draw_glyph_text(vec3 const& aPoint, glyph_text const& aText, text_format const& aTextFormat) const = 0;
        virtual void draw_glyph_text(vec3 const& aPoint, glyph_text const& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, text_format const& aTextFormat) const = 0;
        virtual void draw_multiline_glyph_text(point const& aPoint, glyph_text const& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_glyph_text(vec3 const& aPoint, glyph_text const& aText, dimension aMaxWidth, text_format const& aTextFormat, alignment aAlignment = alignment::Left) const = 0;
        virtual void draw_multiline_glyph_text(point const& aPoint, multiline_glyph_text const& aText, text_format const& aTextFormat) const = 0;
        virtual void draw_multiline_glyph_text(vec3 const& aPoint, multiline_glyph_text const& aText, text_format const& aTextFormat) const = 0;
        virtual void draw_glyph(point const& aPoint, glyph_text const& aText, glyph_char const& aGlyphChar, text_format const& aTextFormat) const = 0;
        virtual void draw_glyph(vec3 const& aPoint, glyph_text const& aText, glyph_char const& aGlyphChar, text_format const& aTextFormat) const = 0;
        virtual void draw_glyphs(point const& aPoint, glyph_text const& aText, text_format_spans const& aSpans) const = 0;
        virtual void draw_glyphs(vec3 const& aPoint, glyph_text const& aText, text_format_spans const& aSpans) const = 0;
        virtual char mnemonic() const = 0;
        virtual bool mnemonic_set() const = 0;
        virtual void set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix = '&') const = 0;
        virtual void unset_mnemonic() const = 0;
        virtual bool mnemonics_shown() const = 0;
        virtual bool password() const = 0;
        virtual std::string const& password_mask() const = 0;
        virtual void set_password(bool aPassword, std::string const& aMask = "\xE2\x97\x8F") = 0;
        // texture
    public:
        virtual void draw_texture(point const& aPoint, i_texture const& aTexture, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_texture(rect const& aRect, i_texture const& aTexture, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_texture(point const& aPoint, i_texture const& aTexture, rect const& aTextureRect, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_texture(rect const& aRect, i_texture const& aTexture, rect const& aTextureRect, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_texture(game::mesh const& aMesh, i_texture const& aTexture, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const = 0;
        virtual void draw_texture(game::mesh const& aMesh, i_texture const& aTexture, rect const& aTextureRect, color_or_gradient const& aColor = {}, shader_effect aShaderEffect = shader_effect::None) const = 0;
        // 3D
    public:
        // todo: set_transformation et al
        virtual void draw_mesh(game::mesh const& aMesh, game::material const& aMaterial, optional_mat44 const& aTransformation = optional_mat44{}, std::optional<game::filter> const& aFilter = {}) const = 0;
        // helpers
    public:
        void draw_rounded_rect(rect const& aRect, dimension aRadius, pen const& aPen, brush const& aFill = brush{}) const
        {
            draw_rounded_rect(aRect, vec4{ aRadius, aRadius, aRadius, aRadius }, aPen, aFill);
        }
        void draw_ellipse_rect(rect const& aRect, dimension aRadius, pen const& aPen, brush const& aFill = brush{}) const
        {
            draw_ellipse_rect(aRect, vec4{ aRadius, aRadius, aRadius, aRadius }, vec4{ aRadius, aRadius, aRadius, aRadius }, aPen, aFill);
        }
        void fill_triangle(point const& aP0, point const& aP1, point const& aP2, brush const& aFill) const
        {
            draw_triangle(aP0, aP1, aP2, pen{}, aFill);
        }
        void fill_rect(rect const& aRect, brush const& aFill) const
        {
            draw_rect(aRect, pen{}, aFill);
        }
        void fill_rounded_rect(rect const& aRect, const vec4& aRadius, brush const& aFill) const
        {
            draw_rounded_rect(aRect, aRadius, pen{}, aFill);
        }
        void fill_ellipse_rect(rect const& aRect, const vec4& aRadiusX, const vec4& aRadiusY, brush const& aFill) const
        {
            draw_ellipse_rect(aRect, aRadiusX, aRadiusY, pen{}, aFill);
        }
        void fill_checker_rect(rect const& aRect, const size& aSquareSize, brush const& aFill1, brush const& aFill2) const
        {
            draw_checker_rect(aRect, aSquareSize, pen{}, aFill1, aFill2);
        }
        void fill_circle(point const& aCenter, dimension aRadius, brush const& aFill) const
        {
            draw_circle(aCenter, aRadius, pen{}, aFill);
        }
        void fill_ellipse(point const& aCenter, dimension aRadiusA, dimension aRadiusB, brush const& aFill) const
        {
            draw_ellipse(aCenter, aRadiusA, aRadiusB, pen{}, aFill);
        }
        void fill_pie(point const& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, brush const& aFill) const
        {
            draw_pie(aCenter, aRadius, aStartAngle, aEndAngle, pen{}, aFill);
        }
        void fill_arc(point const& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, brush const& aFill) const
        {
            draw_arc(aCenter, aRadius, aStartAngle, aEndAngle, pen{}, aFill);
        }
        void fill_path(const path& aPath, brush const& aFill) const
        {
            draw_path(aPath, pen{}, aFill);
        }
        void fill_shape(game::mesh const& aShape, vec3 const& aPosition, brush const& aFill) const
        {
            draw_shape(aShape, aPosition, pen{}, aFill);
        }
        void fill_rounded_rect(rect const& aRect, dimension aRadius, brush const& aFill) const
        {
            fill_rounded_rect(aRect, vec4{ aRadius, aRadius, aRadius, aRadius }, aFill);
        }
    };

    template <typename Iter>
    inline void draw_glyph_text(i_graphics_context const& aGc, vec3 const& aPoint, glyph_text const& aGlyphText, Iter aGlyphTextBegin, Iter aGlyphTextEnd, text_format const& aTextFormat)
    {
        vec3 pos = aPoint;
        for (auto iterGlyph = aGlyphTextBegin; iterGlyph != aGlyphTextEnd; ++iterGlyph)
        {
            aGc.draw_glyph(pos, aGlyphText, *iterGlyph, aTextFormat);
            pos.x += advance(*iterGlyph).cx;
        }
    }

    class scoped_mnemonics
    {
    public:
        scoped_mnemonics(i_graphics_context const& aGc, bool aShowMnemonics, char aMnemonicPrefix = '&') :
            iGc(aGc)
        {
            iGc.set_mnemonic(aShowMnemonics, aMnemonicPrefix);
        }
        ~scoped_mnemonics()
        {
            iGc.unset_mnemonic();
        }
    private:
        i_graphics_context const& iGc;
    };

    class scoped_coordinate_system
    {
    public:
        scoped_coordinate_system(i_graphics_context const& aGc, point const& aOrigin, const size& aExtents, logical_coordinate_system aCoordinateSystem) :
            iGc(aGc), iPreviousOrigin{ aGc.origin() }, iPreviousCoordinateSystem(aGc.logical_coordinate_system()), iPreviousCoordinates(aGc.logical_coordinates())
        {
            iGc.set_logical_coordinate_system(aCoordinateSystem);
            apply_origin(aOrigin, aExtents);
        }
        scoped_coordinate_system(i_graphics_context const& aGc, point const& aOrigin, const size& aExtents, logical_coordinate_system aCoordinateSystem, const neogfx::logical_coordinates& aCoordinates) :
            iGc(aGc), iPreviousOrigin{ aGc.origin() }, iPreviousCoordinateSystem(aGc.logical_coordinate_system()), iPreviousCoordinates(aGc.logical_coordinates())
        {
            iGc.set_logical_coordinate_system(aCoordinateSystem);
            iGc.set_logical_coordinates(aCoordinates);
            apply_origin(aOrigin, aExtents);
        }
        scoped_coordinate_system(i_graphics_context const& aGc, i_graphics_context const& aSource) :
            iGc(aGc), iPreviousOrigin{ aGc.origin() }, iPreviousCoordinateSystem(aGc.logical_coordinate_system()), iPreviousCoordinates(aGc.logical_coordinates())
        {
            iGc.set_logical_coordinate_system(aSource.logical_coordinate_system());
            iGc.set_logical_coordinates(aSource.logical_coordinates());
            iGc.set_origin(aSource.origin());
        }
        scoped_coordinate_system(i_graphics_context const& aGc, i_rendering_context const& aSource) :
            iGc(aGc), iPreviousOrigin{ aGc.origin() }, iPreviousCoordinateSystem(aGc.logical_coordinate_system()), iPreviousCoordinates(aGc.logical_coordinates())
        {
            iGc.set_logical_coordinate_system(aSource.logical_coordinate_system());
            iGc.set_logical_coordinates(aSource.logical_coordinates());
            iGc.set_origin({});
        }
        ~scoped_coordinate_system()
        {
            if (iGc.logical_coordinate_system() != iPreviousCoordinateSystem)
                iGc.set_logical_coordinate_system(iPreviousCoordinateSystem);
            if (iGc.logical_coordinates() != iPreviousCoordinates)
                iGc.set_logical_coordinates(iPreviousCoordinates);
            if (iGc.origin() != iPreviousOrigin)
                iGc.set_origin(iPreviousOrigin);
        }
    private:
        void apply_origin(point const& aOrigin, const size& aExtents)
        {
            if (iGc.logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui)
                iGc.set_origin(aOrigin);
            else if (iGc.logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGame)
                iGc.set_origin(point{ aOrigin.x, iGc.render_target().extents().cy - (aOrigin.y + aExtents.cy) });
        }
    private:
        i_graphics_context const& iGc;
        point iPreviousOrigin;
        logical_coordinate_system iPreviousCoordinateSystem;
        neogfx::logical_coordinates iPreviousCoordinates;
    };

    class scoped_snap_to_pixel
    {
    public:
        scoped_snap_to_pixel(i_graphics_context const& aGc, bool aSnapToPixel = true) :
            iGc{ aGc }, iPreviousSnapped{ aGc.snap_to_pixel() }
        {
            iGc.set_snap_to_pixel(aSnapToPixel);
        }
        ~scoped_snap_to_pixel()
        {
            iGc.set_snap_to_pixel(iPreviousSnapped);
        }
    private:
        i_graphics_context const& iGc;
        bool iPreviousSnapped;
    };

    class scoped_opacity
    {
    public:
        scoped_opacity(i_graphics_context const& aGc, double aOpacity) :
            iGc{ aGc }, iPreviousOpacity { aGc.opacity() }
        {
            iGc.set_opacity(iGc.opacity() * aOpacity);
        }
        ~scoped_opacity()
        {
            iGc.set_opacity(iPreviousOpacity);
        }
    private:
        i_graphics_context const& iGc;
        double iPreviousOpacity;
    };

    class scoped_blending_mode
    {
    public:
        scoped_blending_mode(i_graphics_context const& aGc, neogfx::blending_mode aBlendigMode) :
            iGc{ aGc }, iPreviousBlendingMode{ aGc.blending_mode() }
        {
            iGc.set_blending_mode(aBlendigMode);
        }
        ~scoped_blending_mode()
        {
            iGc.set_blending_mode(iPreviousBlendingMode);
        }
    private:
        i_graphics_context const& iGc;
        neogfx::blending_mode iPreviousBlendingMode;
    };

    class scoped_scissor
    {
    public:
        scoped_scissor(i_graphics_context const& aGc, rect const& aScissorRect) :
            iGc{ aGc }
        {
            iGc.scissor_on(aScissorRect);
        }
        ~scoped_scissor()
        {
            iGc.scissor_off();
        }
    private:
        i_graphics_context const& iGc;
    };

    class scoped_stipple
    {
    public:
        scoped_stipple(i_graphics_context const& aGc, std::optional<stipple> const& aStipple) :
            iGc{ aGc }, iPreviousStipple{ aGc.line_stipple() }
        {
            if (aStipple.has_value())
                iGc.line_stipple_on(aStipple.value());
            else
                iGc.line_stipple_off();
        }
        ~scoped_stipple()
        {
            if (iPreviousStipple.has_value())
                iGc.line_stipple_on(iPreviousStipple.value());
            else
                iGc.line_stipple_off();
        }
    private:
        i_graphics_context const& iGc;
        std::optional<stipple> iPreviousStipple;
    };

    inline void draw_alpha_background(i_graphics_context& aGc, rect const& aRect, dimension aAlphaPatternSize = 4.0_dip)
    {
        scoped_scissor scissor(aGc, aRect);
        aGc.fill_checker_rect(aRect, size{ aAlphaPatternSize }, color{ 160, 160, 160 }, color{ 255, 255, 255 });
    }

    struct blur_filter
    {
        rect region;
        dimension radius;
        blurring_algorithm algorithm = blurring_algorithm::Gaussian;
        scalar parameter1 = 5.0;
        scalar parameter2 = 1.0;
    };

    template <typename Filter>
    class scoped_filter
    {
    public:
        scoped_filter(i_graphics_context& aGc, Filter const& aFilter) :
            iGc{ aGc },
            iFilter{ aFilter },
            iBufferRect{ point{}, aFilter.region.extents() + size{ aFilter.radius * 2.0 } },
            iBuffers{ std::move(create_ping_pong_buffers(aGc, iBufferRect.extents())) },
            iRenderTarget{ front_buffer() }
        {
            front_buffer().set_origin(-aFilter.region.top_left() + point{ aFilter.radius, aFilter.radius });
        }
        ~scoped_filter()
        {
            front_buffer().set_origin({});
            {
                iRenderTarget.emplace(back_buffer());
                if constexpr (std::is_same_v<Filter, blur_filter>)
                    back_buffer().blur(iBufferRect, front_buffer(), iBufferRect, iFilter.radius, iFilter.algorithm, iFilter.parameter1, iFilter.parameter2);
            }
            iRenderTarget = {};
            scoped_blending_mode sbm{ iGc, neogfx::blending_mode::Blit };
            rect const drawRect{ iFilter.region.top_left() - point{ iFilter.radius, iFilter.radius }, iBufferRect.extents() };
            iGc.draw_texture(drawRect, back_buffer().render_target().target_texture(), iBufferRect);
        }
    public:
        i_graphics_context const& front_buffer() const
        {
            return *iBuffers.buffer1;
        }
        i_graphics_context const& back_buffer() const
        {
            return *iBuffers.buffer2;
        }
    private:
        i_graphics_context& iGc;
        Filter iFilter;
        rect iBufferRect;
        ping_pong_buffers iBuffers;
        std::optional<scoped_render_target> iRenderTarget;
    };

    template <typename ValueType = double, std::uint32_t W = 5>
    inline std::array<std::array<ValueType, W>, W> static_gaussian_filter(ValueType aSigma = 1.0)
    {
        const std::int32_t mean = static_cast<std::int32_t>(W / 2);
        std::array<std::array<ValueType, W>, W> kernel = {};
        if (aSigma != 0)
        {
            ValueType sum = 0.0;
            for (std::int32_t x = -mean; x <= mean; ++x)
            {
                for (std::int32_t y = -mean; y <= mean; ++y)
                {
                    kernel[x + mean][y + mean] =
                        static_cast<ValueType>((1.0 / (2.0 * boost::math::constants::pi<ValueType>() * aSigma * aSigma)) * std::exp(-((x * x + y * y) / (2.0 * aSigma * aSigma))));
                    sum += kernel[x + mean][y + mean];
                }
            }
            for (std::uint32_t x = 0; x < W; ++x)
                for (std::uint32_t y = 0; y < W; ++y)
                    kernel[x][y] /= sum;
        }
        else
            kernel[mean][mean] = static_cast<ValueType>(1.0);
        return kernel;
    }

    template <typename ValueType = double>
    inline boost::multi_array<ValueType, 2> dynamic_gaussian_filter(std::uint32_t aKernelSize = 5, ValueType aSigma = 1.0)
    {
        const std::int32_t mean = static_cast<std::int32_t>(aKernelSize / 2);
        boost::multi_array<ValueType, 2> kernel(boost::extents[aKernelSize][aKernelSize]);
        if (aSigma != 0)
        {
            ValueType sum = 0.0;
            for (std::int32_t x = -mean; x <= mean; ++x)
            {
                for (std::int32_t y = -mean; y <= mean; ++y)
                {
                    kernel[x + mean][y + mean] =
                        static_cast<ValueType>((1.0 / (2.0 * boost::math::constants::pi<ValueType>() * aSigma * aSigma)) * std::exp(-((x * x + y * y) / (2.0 * aSigma * aSigma))));
                    sum += kernel[x + mean][y + mean];
                }
            }
            for (std::uint32_t x = 0; x < aKernelSize; ++x)
                for (std::uint32_t y = 0; y < aKernelSize; ++y)
                    kernel[x][y] /= sum;
        }
        else
            kernel[mean][mean] = static_cast<ValueType>(1.0);
        return kernel;
    }

    class scoped_tab_stops
    {
    public:
        scoped_tab_stops(i_graphics_context& aGc, std::optional<tab_stops> const& aTabStops) :
            iGc{ aGc }
        {
            if (iGc.has_tab_stops())
                iPrevious.emplace(iGc.tab_stops());
            if (aTabStops.has_value())
                iGc.set_tab_stops(aTabStops.value());
            else
                iGc.clear_tab_stops();
        }
        ~scoped_tab_stops()
        {
            if (iPrevious.has_value())
                iGc.set_tab_stops(iPrevious.value());
            else
                iGc.clear_tab_stops();
        }
    private:
        i_graphics_context& iGc;
        std::optional<tab_stops> iPrevious;
    };
}