// graphics_context.cpp
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

#include <neogfx/neogfx.hpp>
#include <neolib/core/string_utils.hpp>
#include "native/i_native_texture.hpp"
#include "text/native/native_font_face.hpp"
#include <neogfx/hid/i_native_surface.hpp>
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gfx/text/text_category_map.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/rectangle.hpp>
#include <neogfx/game/ecs_helpers.hpp>
#include <neogfx/gfx/graphics_context.hpp>

namespace neogfx
{
    ping_pong_buffers create_ping_pong_buffers(const i_rendering_context& aContext, const size& aExtents, texture_sampling aSampling, const optional_color& aClearColor)
    {
        size previousExtents;
        auto& buffer1 = service<i_rendering_engine>().ping_pong_buffer1(aExtents, previousExtents, aSampling);
        buffer1.as_render_target().set_logical_coordinate_system(aContext.logical_coordinate_system());
        auto gcBuffer1 = std::make_unique<graphics_context>(buffer1);
        {
            if (aClearColor != std::nullopt)
            {
                scoped_render_target srt{ *gcBuffer1 };
                scoped_scissor ss{ *gcBuffer1, rect{ point{}, previousExtents }.inflate(1.0) };
                gcBuffer1->clear(*aClearColor);
                gcBuffer1->clear_depth_buffer();
                gcBuffer1->clear_stencil_buffer();
            }
        }
        auto& buffer2 = service<i_rendering_engine>().ping_pong_buffer2(aExtents, previousExtents, aSampling);
        buffer2.as_render_target().set_logical_coordinate_system(aContext.logical_coordinate_system());
        auto gcBuffer2 = std::make_unique<graphics_context>(buffer2);
        {
            if (aClearColor != std::nullopt)
            {
                scoped_render_target srt{ *gcBuffer2 };
                scoped_scissor ss{ *gcBuffer2, rect{ point{}, previousExtents }.inflate(1.0) };
                gcBuffer2->clear(*aClearColor);
                gcBuffer2->clear_depth_buffer();
                gcBuffer2->clear_stencil_buffer();
            }
        }
        return ping_pong_buffers{ {}, std::move(gcBuffer1), std::move(gcBuffer2) };
    }

    graphics_context::graphics_context(const i_surface& aSurface, type aType) :
        iType{ aType },
        iRenderTarget{ aSurface.native_surface() },
        iNativeGraphicsContext{ nullptr },
        iDefaultFont{},
        iExtents{ aSurface.extents() },
        iLayer{ LayerWidget },
        iSnapToPixel{ false },
        iOpacity{ 1.0 },
        iBlendingMode{ neogfx::blending_mode::Default },
        iSmoothingMode{ neogfx::smoothing_mode::None },
        iSubpixelRendering{ service<i_rendering_engine>().is_subpixel_rendering_on() }
    {
    }

    graphics_context::graphics_context(const i_surface& aSurface, const font& aDefaultFont, type aType) :
        iType{ aType },
        iRenderTarget{ aSurface.native_surface() },
        iNativeGraphicsContext{ nullptr },
        iDefaultFont{ aDefaultFont },
        iExtents{ aSurface.extents() },
        iLayer{ LayerWidget },
        iSnapToPixel{ false },
        iOpacity{ 1.0 },
        iBlendingMode{ neogfx::blending_mode::Default },
        iSmoothingMode{ neogfx::smoothing_mode::None },
        iSubpixelRendering{ service<i_rendering_engine>().is_subpixel_rendering_on() }
    {
    }

    graphics_context::graphics_context(const i_widget& aWidget, type aType) :
        iType{ aType },
        iRenderTarget{ aWidget.surface().native_surface() },
        iNativeGraphicsContext{ nullptr },
        iDefaultFont{ aWidget.font() },
        iOrigin{ aWidget.origin() },
        iExtents{ aWidget.extents() },
        iLayer{ LayerWidget },
        iSnapToPixel{ false },
        iOpacity{ 1.0 },
        iBlendingMode{ neogfx::blending_mode::Default },
        iSmoothingMode{ neogfx::smoothing_mode::None },
        iSubpixelRendering{ service<i_rendering_engine>().is_subpixel_rendering_on() }
    {
        set_logical_coordinate_system(aWidget.logical_coordinate_system());
    }

    graphics_context::graphics_context(const i_texture& aTexture, type aType) :
        iType{ aType },
        iRenderTarget{ static_cast<i_native_texture&>(aTexture.native_texture()) },
        iNativeGraphicsContext{ nullptr },
        iDefaultFont{ font() },
        iExtents{ aTexture.extents() },
        iLayer{ LayerWidget },
        iSnapToPixel{ false },
        iOpacity{ 1.0 },
        iBlendingMode{ neogfx::blending_mode::Default },
        iSmoothingMode{ neogfx::smoothing_mode::None },
        iSubpixelRendering{ service<i_rendering_engine>().is_subpixel_rendering_on() }
    {
    }

    graphics_context::graphics_context(const graphics_context& aOther) :
        iType{ aOther.iType },
        iRenderTarget{ aOther.iRenderTarget },
        iSrt{ aOther.iRenderTarget },
        iNativeGraphicsContext{ aOther.active() ? aOther.native_context().clone() : nullptr },
        iDefaultFont{ aOther.iDefaultFont },
        iOrigin{ aOther.origin() },
        iExtents{ aOther.extents() },
        iLayer{ LayerWidget },
        iLogicalCoordinateSystem{ aOther.iLogicalCoordinateSystem },
        iLogicalCoordinates{ aOther.iLogicalCoordinates },
        iSnapToPixel{ aOther.iSnapToPixel },
        iOpacity{ 1.0 },
        iBlendingMode{ neogfx::blending_mode::Default },
        iSmoothingMode{ neogfx::smoothing_mode::None },
        iSubpixelRendering{ service<i_rendering_engine>().is_subpixel_rendering_on() }
    {
    }

    graphics_context::~graphics_context()
    {
        flush();
    }

    std::unique_ptr<i_rendering_context> graphics_context::clone() const
    {
        return std::make_unique<graphics_context>(*this);
    }

    i_rendering_engine& graphics_context::rendering_engine() const
    {
        return native_context().rendering_engine();
    }

    const i_render_target& graphics_context::render_target() const
    {
        return iRenderTarget;
    }

    rect graphics_context::rendering_area(bool aConsiderScissor) const
    {
        return native_context().rendering_area(aConsiderScissor);
    }

    const graphics_operation::queue& graphics_context::queue() const
    {
        return native_context().queue();
    }

    graphics_operation::queue& graphics_context::queue()
    {
        return native_context().queue();
    }

    void graphics_context::enqueue(const graphics_operation::operation& aOperation)
    {
        native_context().enqueue(aOperation);
    }

    void graphics_context::flush()
    {
        if (attached() && active())
            native_context().flush();
    }

    delta graphics_context::to_device_units(const delta& aValue) const
    {
        return units_converter{ *this }.to_device_units(aValue);
    }

    size graphics_context::to_device_units(const size& aValue) const
    {
        return units_converter{ *this }.to_device_units(aValue);
    }

    point graphics_context::to_device_units(const point& aValue) const
    {
        return units_converter{ *this }.to_device_units(aValue);
    }

    vec2 graphics_context::to_device_units(const vec2& aValue) const
    {
        return units_converter{ *this }.to_device_units(aValue);
    }

    rect graphics_context::to_device_units(const rect& aValue) const
    {
        return units_converter{ *this }.to_device_units(aValue);
    }

    path graphics_context::to_device_units(const path& aValue) const
    {
        path result = aValue;
        result.set_position(to_device_units(result.position()));
        for (std::size_t i = 0; i < result.sub_paths().size(); ++i)
            for (std::size_t j = 0; j < result.sub_paths()[i].size(); ++j)
                result.sub_paths()[i][j] = to_device_units(result.sub_paths()[i][j]);
        return result;
    }

    delta graphics_context::from_device_units(const delta& aValue) const
    {
        return units_converter{ *this }.from_device_units(aValue);
    }

    size graphics_context::from_device_units(const size& aValue) const
    {
        return units_converter{ *this }.from_device_units(aValue);
    }

    point graphics_context::from_device_units(const point& aValue) const
    {
        return units_converter{ *this }.from_device_units(aValue);
    }

    rect graphics_context::from_device_units(const rect& aValue) const
    {
        return units_converter{ *this }.from_device_units(aValue);
    }

    path graphics_context::from_device_units(const path& aValue) const
    {
        path result = aValue;
        result.set_position(from_device_units(result.position()));
        for (std::size_t i = 0; i < result.sub_paths().size(); ++i)
            for (std::size_t j = 0; j < result.sub_paths()[i].size(); ++j)
                result.sub_paths()[i][j] = from_device_units(result.sub_paths()[i][j]);
        return result;
    }

    layer_t graphics_context::layer() const
    {
        return iLayer;
    }

    void graphics_context::set_layer(layer_t aLayer)
    {
        if (iLayer != aLayer)
        {
            iLayer = aLayer;
            // todo:-
//            if (attached())
//                native_context().enqueue(graphics_operation::set_layer{ aLayer });
        }
    }

    neogfx::logical_coordinate_system graphics_context::logical_coordinate_system() const
    {
        if (iLogicalCoordinateSystem != std::nullopt)
            return *iLogicalCoordinateSystem;
        return render_target().logical_coordinate_system();
    }

    void graphics_context::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) const
    {
        if (iLogicalCoordinateSystem != aSystem)
        {
            iLogicalCoordinateSystem = aSystem;
            if (attached())
                native_context().enqueue(graphics_operation::set_logical_coordinate_system{ aSystem });
        }
    }

    neogfx::logical_coordinates graphics_context::logical_coordinates() const
    {
        if (iLogicalCoordinates != std::nullopt)
            return *iLogicalCoordinates;
        return render_target().logical_coordinates();
    }

    void graphics_context::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) const
    {
        if (iLogicalCoordinates != aCoordinates)
        {
            iLogicalCoordinates = aCoordinates;
            if (attached())
                native_context().enqueue(graphics_operation::set_logical_coordinates{ aCoordinates });
        }
    }

    vec2 graphics_context::offset() const
    {
        throw not_implemented();
    }

    void graphics_context::set_offset(const optional_vec2& aOffset)
    {
        throw not_implemented();
    }

    bool graphics_context::gradient_set() const
    {
        throw not_implemented();
    }

    void graphics_context::apply_gradient(i_gradient_shader& aShader)
    {
        throw not_implemented();
    }

    font const& graphics_context::default_font() const
    {
        return iDefaultFont;
    }

    void graphics_context::set_default_font(const font& aDefaultFont) const
    {
        iDefaultFont = aDefaultFont;
    }

    void graphics_context::set_extents(const size& aExtents) const
    {
        iExtents = aExtents;
    }

    void graphics_context::set_origin(const point& aOrigin) const
    {
        if (iOrigin != to_device_units(aOrigin))
        {
            iOrigin = to_device_units(aOrigin);
            native_context().enqueue(graphics_operation::set_origin{ iOrigin });
        }
    }

    point graphics_context::origin() const
    {
        return from_device_units(iOrigin);
    }

    void graphics_context::clear_gradient()
    {
        native_context().enqueue(graphics_operation::clear_gradient{});
    }

    void graphics_context::set_gradient(const gradient& aGradient, const rect& aBoundingBox)
    {
        native_context().enqueue(graphics_operation::set_gradient{ aGradient.with_bounding_box(to_device_units(aBoundingBox) + iOrigin) });
    }

    void graphics_context::set_pixel(const point& aPoint, const color& aColor) const
    {
        native_context().enqueue(graphics_operation::set_pixel{ to_device_units(aPoint) + iOrigin, aColor });
    }

    void graphics_context::draw_pixel(const point& aPoint, const color& aColor) const
    {
        native_context().enqueue(graphics_operation::draw_pixel{ to_device_units(aPoint) + iOrigin, aColor });
    }

    void graphics_context::draw_line(const point& aFrom, const point& aTo, const pen& aPen) const
    {
        native_context().enqueue(graphics_operation::draw_line{ to_device_units(aFrom) + iOrigin, to_device_units(aTo) + iOrigin, aPen });
    }

    void graphics_context::draw_triangle(const point& aP0, const point& aP1, const point& aP2, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_triangle(aP0, aP1, aP2, aFill);
        native_context().enqueue(graphics_operation::draw_triangle{ to_device_units(aP0) + iOrigin, to_device_units(aP1) + iOrigin, to_device_units(aP2) + iOrigin, aPen });
    }

    void graphics_context::draw_rect(const rect& aRect, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_rect(aRect, aFill);
        native_context().enqueue(graphics_operation::draw_rect{ to_device_units(aRect) + iOrigin, aPen });
    }

    void graphics_context::draw_rounded_rect(const rect& aRect, const vec4& aRadius, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_rounded_rect(aRect, aRadius, aFill);
        native_context().enqueue(graphics_operation::draw_rounded_rect{ to_device_units(aRect) + iOrigin, aRadius, aPen });
    }

    void graphics_context::draw_circle(const point& aCenter, dimension aRadius, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_circle(aCenter, aRadius, aFill);
        native_context().enqueue(graphics_operation::draw_circle{ to_device_units(aCenter) + iOrigin, aRadius, aPen });
    }

    void graphics_context::draw_ellipse(const point& aCenter, dimension aRadiusA, dimension aRadiusB, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_ellipse(aCenter, aRadiusA, aRadiusB, aFill);
        native_context().enqueue(graphics_operation::draw_ellipse{ to_device_units(aCenter) + iOrigin, aRadiusA, aRadiusB, aPen });
    }

    void graphics_context::draw_pie(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_pie(aCenter, aRadius, aStartAngle, aEndAngle, aFill);
        native_context().enqueue(graphics_operation::draw_pie{ to_device_units(aCenter) + iOrigin, aRadius, aStartAngle, aEndAngle, aPen });
    }

    void graphics_context::draw_arc(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_arc(aCenter, aRadius, aStartAngle, aEndAngle, aFill);
        native_context().enqueue(graphics_operation::draw_arc{ to_device_units(aCenter) + iOrigin, aRadius, aStartAngle, aEndAngle, aPen });
    }

    void graphics_context::draw_cubic_bezier(const point& aP0, const point& aP1, const point& aP2, const point& aP3, const pen& aPen) const
    {
        native_context().enqueue(graphics_operation::draw_cubic_bezier{ to_device_units(aP0) + iOrigin, to_device_units(aP1) + iOrigin, to_device_units(aP2) + iOrigin, to_device_units(aP3) + iOrigin, aPen });
    }

    void graphics_context::draw_path(const path& aPath, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_path(aPath, aFill);
        path path = to_device_units(aPath);
        path.set_position(path.position() + iOrigin);
        native_context().enqueue(graphics_operation::draw_path{ path, aPen });
    }

    void graphics_context::draw_shape(const game::mesh& aShape, const vec3& aPosition, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_shape(aShape, aPosition, aFill);
        vec2 const toDeviceUnits = to_device_units(vec2{ 1.0, 1.0 });
        native_context().enqueue(
            graphics_operation::draw_shape{
                mat44{ 
                    { toDeviceUnits.x, 0.0, 0.0, 0.0 },
                    { 0.0, toDeviceUnits.y, 0.0, 0.0 },
                    { 0.0, 0.0, 1.0, 0.0 }, 
                    { iOrigin.x, iOrigin.y, 0.0, 1.0 } } * aShape,
                aPosition,
                aPen });
    }

    void graphics_context::draw_entities(game::i_ecs& aEcs, int32_t aLayer) const
    {
        vec2 const toDeviceUnits = to_device_units(vec2{ 1.0, 1.0 });
        native_context().enqueue(
            graphics_operation::draw_entities{ 
                aEcs, 
                aLayer,
                mat44{
                    { toDeviceUnits.x, 0.0, 0.0, 0.0 },
                    { 0.0, toDeviceUnits.y, 0.0, 0.0 },
                    { 0.0, 0.0, 1.0, 0.0 },
                    { iOrigin.x, iOrigin.y, 0.0, 1.0 } }
                });
    }

    void graphics_context::draw_focus_rect(const rect& aRect) const
    {
        push_logical_operation(neogfx::logical_operation::Xor);
        line_stipple_on(1.0, 0x5555);
        draw_rect(aRect, pen{ color::White });
        line_stipple_off();
        pop_logical_operation();
    }

    void graphics_context::fill_triangle(const point& aP0, const point& aP1, const point& aP2, const brush& aFill) const
    {
        native_context().enqueue(graphics_operation::fill_triangle{ to_device_units(aP0) + iOrigin, to_device_units(aP1) + iOrigin, to_device_units(aP2) + iOrigin, aFill });
    }
        
    void graphics_context::fill_rect(const rect& aRect, const brush& aFill) const
    {
        native_context().enqueue(graphics_operation::fill_rect{ to_device_units(aRect) + iOrigin, aFill });
    }

    void graphics_context::fill_rounded_rect(const rect& aRect, const vec4& aRadius, const brush& aFill) const
    {
        native_context().enqueue(graphics_operation::fill_rounded_rect{ to_device_units(aRect) + iOrigin, aRadius, aFill });
    }

    void graphics_context::fill_checker_rect(const rect& aRect, const size& aSquareSize, const brush& aFill1, const brush& aFill2) const
    {
        native_context().enqueue(graphics_operation::fill_checker_rect{ to_device_units(aRect) + iOrigin, to_device_units(aSquareSize), aFill1, aFill2 });
    }

    void graphics_context::fill_circle(const point& aCenter, dimension aRadius, const brush& aFill) const
    {
        native_context().enqueue(graphics_operation::fill_circle{ to_device_units(aCenter) + iOrigin, aRadius, aFill });
    }

    void graphics_context::fill_ellipse(const point& aCenter, dimension aRadiusA, dimension aRadiusB, const brush& aFill) const
    {
        native_context().enqueue(graphics_operation::fill_ellipse{ to_device_units(aCenter) + iOrigin, aRadiusA, aRadiusB, aFill });
    }

    void graphics_context::fill_pie(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill) const
    {
        native_context().enqueue(graphics_operation::fill_pie{ to_device_units(aCenter) + iOrigin, aRadius, aStartAngle, aEndAngle, aFill });
    }

    void graphics_context::fill_arc(const point& aCenter, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill) const
    {
        native_context().enqueue(graphics_operation::fill_arc{ to_device_units(aCenter) + iOrigin, aRadius, aStartAngle, aEndAngle, aFill });
    }

    void graphics_context::fill_path(const path& aPath, const brush& aFill) const
    {
        path path = to_device_units(aPath);
        path.set_position(path.position() + iOrigin);
        native_context().enqueue(graphics_operation::fill_path{ path, aFill });
    }

    void graphics_context::fill_shape(const game::mesh& aShape, const vec3& aPosition, const brush& aFill) const
    {
        vec2 const toDeviceUnits = to_device_units(vec2{ 1.0, 1.0 });
        native_context().enqueue(
            graphics_operation::fill_shape{
                mat44{ 
                    { toDeviceUnits.x, 0.0, 0.0, 0.0 },
                    { 0.0, toDeviceUnits.y, 0.0, 0.0 },
                    { 0.0, 0.0, 1.0, 0.0 }, 
                    { iOrigin.x, iOrigin.y, 0.0, 1.0 } } * aShape,
                aPosition,
                aFill });
    }

    size graphics_context::text_extent(std::string const& aText) const
    {
        return text_extent(aText, default_font());
    }

    size graphics_context::text_extent(std::string const& aText, const font& aFont) const
    {
        return text_extent(aText.begin(), aText.end(), [&aFont](std::size_t) { return aFont; });
    }

    size graphics_context::text_extent(std::string const& aText, std::function<font(std::size_t)> aFontSelector) const
    {
        return text_extent(aText.begin(), aText.end(), aFontSelector);
    }

    size graphics_context::text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd) const
    {
        return text_extent(aTextBegin, aTextEnd, default_font());
    }

    size graphics_context::text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont) const
    {
        return text_extent(aTextBegin, aTextEnd, [&aFont](std::size_t) { return aFont; });
    }

    size graphics_context::text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector) const
    {
        return glyph_text_extent(to_glyph_text(aTextBegin, aTextEnd, aFontSelector));
    }

    size graphics_context::multiline_text_extent(std::string const& aText) const
    {
        return multiline_text_extent(aText, default_font());
    }

    size graphics_context::multiline_text_extent(std::string const& aText, const font& aFont) const
    {
        return multiline_text_extent(aText, [&aFont](std::size_t) { return aFont; }, 0);
    }

    size graphics_context::multiline_text_extent(std::string const& aText, std::function<font(std::size_t)> aFontSelector) const
    {
        return multiline_text_extent(aText, aFontSelector, 0);
    }

    size graphics_context::multiline_text_extent(std::string const& aText, dimension aMaxWidth) const
    {
        return multiline_text_extent(aText, default_font(), aMaxWidth);
    }

    size graphics_context::multiline_text_extent(std::string const& aText, const font& aFont, dimension aMaxWidth) const
    {
        return multiline_text_extent(aText, [&aFont](std::size_t) { return aFont; }, aMaxWidth);
    }
        
    size graphics_context::multiline_text_extent(std::string const& aText, std::function<font(std::size_t)> aFontSelector, dimension aMaxWidth) const
    {
        return multiline_glyph_text_extent(to_glyph_text(aText.begin(), aText.end(), aFontSelector), aMaxWidth);
    }

    size graphics_context::glyph_text_extent(const glyph_text& aText) const
    {
        return from_device_units(aText.extents());
    }

    size graphics_context::glyph_text_extent(const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd) const
    {
        return from_device_units(aText.extents(aTextBegin, aTextEnd));
    }

    size graphics_context::multiline_glyph_text_extent(const glyph_text& aText, dimension aMaxWidth) const
    {
        auto multilineGlyphText = to_multiline_glyph_text(aText, aMaxWidth);
        size result;
        for (auto& line : multilineGlyphText.lines)
            result.cx = std::max(result.cx, line.extents.cx);
        result.cy = (!multilineGlyphText.lines.empty() ? multilineGlyphText.lines.back().pos.y + multilineGlyphText.lines.back().extents.cy : 0.0);
        return result;
    }

    bool graphics_context::is_text_left_to_right(std::string const& aText) const
    {
        return is_text_left_to_right(aText, default_font());
    }

    bool graphics_context::is_text_left_to_right(std::string const& aText, const font& aFont) const
    {
        auto const& glyphText = to_glyph_text(aText.begin(), aText.end(), aFont);
        return glyph_text_direction(glyphText.cbegin(), glyphText.cend()) == text_direction::LTR;
    }

    bool graphics_context::is_text_right_to_left(std::string const& aText) const
    {
        return is_text_right_to_left(aText, default_font());
    }

    bool graphics_context::is_text_right_to_left(std::string const& aText, const font& aFont) const
    {
        return !is_text_left_to_right(aText, aFont);
    }

    void graphics_context::draw_text(const point& aPoint, std::string const& aText, const text_format& aTextFormat) const
    {
        draw_text(aPoint, aText, default_font(), aTextFormat);
    }

    void graphics_context::draw_text(const point& aPoint, std::string const& aText, const font& aFont, const text_format& aTextFormat) const
    {
        draw_text(aPoint.to_vec3(), aText, aFont, aTextFormat);
    }

    void graphics_context::draw_text(const point& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const text_format& aTextFormat) const
    {
        draw_text(aPoint, aTextBegin, aTextEnd, default_font(), aTextFormat);
    }

    void graphics_context::draw_text(const point& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, const text_format& aTextFormat) const
    {
        draw_text(aPoint.to_vec3(), aTextBegin, aTextEnd, aFont, aTextFormat);
    }

    void graphics_context::draw_text(const vec3& aPoint, std::string const& aText, const text_format& aTextFormat) const
    {
        draw_text(aPoint, aText, default_font(), aTextFormat);
    }

    void graphics_context::draw_text(const vec3& aPoint, std::string const& aText, const font& aFont, const text_format& aTextFormat) const
    {
        draw_text(aPoint, aText.begin(), aText.end(), aFont, aTextFormat);
    }

    void graphics_context::draw_text(const vec3& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const text_format& aTextFormat) const
    {
        draw_text(aPoint, aTextBegin, aTextEnd, default_font() , aTextFormat);
    }

    void graphics_context::draw_text(const vec3& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, const text_format& aTextFormat) const
    {
        draw_glyph_text(aPoint, to_glyph_text(aTextBegin, aTextEnd, aFont), aTextFormat);
    }

    void graphics_context::draw_multiline_text(const point& aPoint, std::string const& aText, const text_format& aTextFormat, alignment aAlignment) const
    {
        draw_multiline_text(aPoint, aText, default_font(), aTextFormat, aAlignment);
    }

    void graphics_context::draw_multiline_text(const point& aPoint, std::string const& aText, const font& aFont, const text_format& aTextFormat, alignment aAlignment) const
    {
        draw_multiline_text(aPoint.to_vec3(), aText, aFont, aTextFormat, aAlignment);
    }

    void graphics_context::draw_multiline_text(const point& aPoint, std::string const& aText, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment) const
    {
        draw_multiline_text(aPoint, aText, default_font(), aMaxWidth, aTextFormat, aAlignment);
    }

    void graphics_context::draw_multiline_text(const point& aPoint, std::string const& aText, const font& aFont, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment) const
    {
        draw_multiline_text(aPoint.to_vec3(), aText, aFont, aMaxWidth, aTextFormat, aAlignment);
    }
        
    void graphics_context::draw_multiline_text(const vec3& aPoint, std::string const& aText, const text_format& aTextFormat, alignment aAlignment) const
    {
        draw_multiline_text(aPoint, aText, default_font(), aTextFormat, aAlignment);
    }

    void graphics_context::draw_multiline_text(const vec3& aPoint, std::string const& aText, const font& aFont, const text_format& aTextFormat, alignment aAlignment) const
    {
        draw_multiline_text(aPoint, aText, aFont, 0, aTextFormat, aAlignment);
    }

    void graphics_context::draw_multiline_text(const vec3& aPoint, std::string const& aText, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment) const
    {
        draw_multiline_text(aPoint, aText, default_font(), aMaxWidth, aTextFormat, aAlignment);
    }
    
    void graphics_context::draw_multiline_text(const vec3& aPoint, std::string const& aText, const font& aFont, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment) const
    {
        auto multilineGlyphText = to_multiline_glyph_text(aText, aFont, aMaxWidth, aAlignment);
        for (auto& line : multilineGlyphText.lines)
        {
            if (line.begin == line.end)
                continue;
            draw_glyph_text(aPoint + line.pos.to_vec3(), multilineGlyphText.glyphText, multilineGlyphText.glyphText.begin() + line.begin, multilineGlyphText.glyphText.begin() + line.end, aTextFormat);
        }
    }

    void graphics_context::draw_glyph_text(const point& aPoint, const glyph_text& aText, const text_format& aTextFormat) const
    {
        draw_glyph_text(aPoint.to_vec3(), aText, aTextFormat);
    }

    void graphics_context::draw_glyph_text(const point& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const text_format& aTextFormat) const
    {
        draw_glyph_text(aPoint.to_vec3(), aText, aTextBegin, aTextEnd, aTextFormat);
    }

    void graphics_context::draw_glyph_text(const vec3& aPoint, const glyph_text& aText, const text_format& aTextFormat) const
    {
        draw_glyph_text(aPoint, aText, aText.cbegin(), aText.cend(), aTextFormat);
    }

    void graphics_context::draw_glyph_text(const vec3& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const text_format& aTextFormat) const
    {
        if (aTextBegin == aTextEnd)
            return;
        auto adjustedPos = (to_device_units(point{ aPoint }) + iOrigin).to_vec3() + vec3{ 0.0, 0.0, aPoint.z };
        native_context().enqueue(graphics_operation::draw_glyphs{ adjustedPos, aText, aTextBegin, aTextEnd, text_format_span{ 0, aTextEnd - aTextBegin, aTextFormat }, mnemonics_shown() });
    }

    void graphics_context::draw_multiline_glyph_text(const point& aPoint, const glyph_text& aText, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment) const
    {
        draw_multiline_glyph_text(aPoint.to_vec3(), aText, aMaxWidth, aTextFormat, aAlignment);
    }

    void graphics_context::draw_multiline_glyph_text(const vec3& aPoint, const glyph_text& aText, dimension aMaxWidth, const text_format& aTextFormat, alignment aAlignment) const
    {
        auto multilineGlyphText = to_multiline_glyph_text(aText, aMaxWidth, aAlignment);
        for (auto& line : multilineGlyphText.lines)
        {
            if (line.begin == line.end)
                continue;
            draw_glyph_text(aPoint + line.pos.to_vec3(), multilineGlyphText.glyphText, multilineGlyphText.glyphText.begin() + line.begin, multilineGlyphText.glyphText.begin() + line.end, aTextFormat);
        }
    }

    subpixel_format graphics_context::subpixel_format() const
    {
        return native_context().subpixel_format();
    }

    bool graphics_context::metrics_available() const
    {
        return true;
    }

    size graphics_context::extents() const
    {
        return iExtents;
    }

    dimension graphics_context::horizontal_dpi() const
    {
        return iRenderTarget.horizontal_dpi();
    }

    dimension graphics_context::vertical_dpi() const
    {
        return iRenderTarget.vertical_dpi();
    }

    dimension graphics_context::ppi() const
    {
        return iRenderTarget.ppi();
    }

    dimension graphics_context::em_size() const
    {
        return static_cast<dimension>(iDefaultFont.size() / 72.0 * horizontal_dpi());
    }

    bool graphics_context::device_metrics_available() const
    {
        return device_metrics().metrics_available();
    }

    const i_device_metrics& graphics_context::device_metrics() const
    {
        return *this;
    }

    bool graphics_context::attached() const
    {
        return iType == type::Attached;
    }

    bool graphics_context::active() const
    {
        return iNativeGraphicsContext != nullptr;
    }

    i_rendering_context& graphics_context::native_context() const
    {
        if (attached())
        {
            if (!active())
            {
                if (iSrt == std::nullopt)
                    iSrt.emplace(render_target());
                iNativeGraphicsContext = iRenderTarget.create_graphics_context(iBlendingMode);
            }
            return *iNativeGraphicsContext;
        }
        throw unattached();
    }

    void graphics_context::flush() const
    {
        native_context().flush();
    }

    void graphics_context::set_default_viewport() const
    {
        native_context().enqueue(graphics_operation::set_viewport{});
    }

    void graphics_context::set_viewport(const rect& aViewportRect) const
    {
        native_context().enqueue(graphics_operation::set_viewport{ to_device_units(aViewportRect) });
        }

    void graphics_context::scissor_on(const rect& aRect) const
    {
        native_context().enqueue(graphics_operation::scissor_on{ to_device_units(aRect) + iOrigin });
    }

    void graphics_context::scissor_off() const
    {
        native_context().enqueue(graphics_operation::scissor_off{});
    }

    bool graphics_context::snap_to_pixel() const
    {
        return iSnapToPixel;
    }

    void graphics_context::set_snap_to_pixel(bool aSnap) const
    {
        if (iSnapToPixel != aSnap)
        {
            iSnapToPixel = aSnap;
            if (snap_to_pixel())
                native_context().enqueue(graphics_operation::snap_to_pixel_on{});
            else
                native_context().enqueue(graphics_operation::snap_to_pixel_off{});
        }
    }

    double graphics_context::opacity() const
    {
        return iOpacity;
    }

    void graphics_context::set_opacity(double aOpacity) const
    {
        if (iOpacity != aOpacity)
        {
            iOpacity = aOpacity;
            native_context().enqueue(graphics_operation::set_opacity{ aOpacity });
        }
    }

    blending_mode graphics_context::blending_mode() const
    {
        return iBlendingMode;
    }

    void graphics_context::set_blending_mode(neogfx::blending_mode aBlendingMode) const
    {
        if (iBlendingMode != aBlendingMode)
        {
            iBlendingMode = aBlendingMode;
            native_context().enqueue(graphics_operation::set_blending_mode{ aBlendingMode });
        }
    }

    smoothing_mode graphics_context::smoothing_mode() const
    {
        return iSmoothingMode;
    }

    void graphics_context::set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode) const
    {
        if (iSmoothingMode != aSmoothingMode)
        {
            iSmoothingMode = aSmoothingMode;
            native_context().enqueue(graphics_operation::set_smoothing_mode{ aSmoothingMode });
        }
    }

    void graphics_context::push_logical_operation(logical_operation aLogicalOperation) const
    {
        native_context().enqueue(graphics_operation::push_logical_operation{ aLogicalOperation });
    }

    void graphics_context::pop_logical_operation() const
    {
        native_context().enqueue(graphics_operation::pop_logical_operation{});
    }

    void graphics_context::line_stipple_on(scalar aFactor, uint16_t aPattern, scalar aPosition) const
    {
        native_context().enqueue(graphics_operation::line_stipple_on{ aFactor, aPattern, aPosition });
    }

    void graphics_context::line_stipple_off() const
    {
        native_context().enqueue(graphics_operation::line_stipple_off{});
    }

    bool graphics_context::is_subpixel_rendering_on() const
    {
        return iSubpixelRendering;
    }

    void graphics_context::subpixel_rendering_on() const
    {
        if (iSubpixelRendering != true)
        {
            iSubpixelRendering = true;
            native_context().enqueue(graphics_operation::subpixel_rendering_on{});
        }
    }

    void graphics_context::subpixel_rendering_off() const
    {
        if (iSubpixelRendering != false)
        {
            iSubpixelRendering = false;
            native_context().enqueue(graphics_operation::subpixel_rendering_off{});
        }
    }

    void graphics_context::clear(const color& aColor, const std::optional<scalar>& aZpos) const
    {
        if (aZpos == std::nullopt)
            native_context().enqueue(graphics_operation::clear{ aColor });
        else
            fill_rect(rect{ render_target().target_type() == render_target_type::Surface ? 
                point{ 0.0, 0.0, aZpos ? *aZpos : 0.0 } : point{ -1.0, -1.0 }, iRenderTarget.target_texture().storage_extents() }, aColor);
    }

    void graphics_context::clear_depth_buffer() const
    {
        native_context().enqueue(graphics_operation::clear_depth_buffer{});
    }

    void graphics_context::clear_stencil_buffer() const
    {
        native_context().enqueue(graphics_operation::clear_stencil_buffer{});
    }

    void graphics_context::blit(const rect& aDestinationRect, const i_graphics_context& aSource, const rect& aSourceRect) const
    {
        scoped_blending_mode sbm{ *this, neogfx::blending_mode::Blit };
        draw_texture(aDestinationRect, aSource.render_target().target_texture(), aSourceRect);
    }

    void blur(const i_graphics_context& aDestination, const rect& aDestinationRect, const i_graphics_context& aSource, const rect& aSourceRect, blurring_algorithm aAlgorithm, scalar aParameter1, scalar aParameter2)
    {
        scoped_render_target srt{ aDestination };
        auto mesh = aDestination.logical_coordinate_system() == logical_coordinate_system::AutomaticGui ?
            to_ecs_component(aDestinationRect) : to_ecs_component(game_rect{ aDestinationRect });
        auto const& source = aSource.render_target();
        for (auto& uv : mesh.uv)
            uv = (aSourceRect.top_left() / source.extents()).to_vec2() + uv.scale((aSourceRect.extents() / source.extents()).to_vec2());
        aDestination.draw_mesh(
            mesh,
            game::material
            {
                {},
                {},
                {},
                to_ecs_component(aSource.render_target().target_texture()),
                shader_effect::Filter
            },
            optional_mat44{},
            to_ecs_component(aAlgorithm, aParameter1, aParameter2));
    }

    void graphics_context::blur(const rect& aDestinationRect, const i_graphics_context& aSource, const rect& aSourceRect, dimension aRadius, blurring_algorithm aAlgorithm, scalar aParameter1, scalar aParameter2) const
    {
        scoped_render_target srt1{ *this };
        scoped_blending_mode sbm1{ *this, neogfx::blending_mode::Blit };
        scoped_scissor ss1{ *this, aDestinationRect };
        scoped_render_target srt2{ aSource };
        scoped_blending_mode sbm2{ aSource, neogfx::blending_mode::Blit };
        scoped_scissor ss2{ aSource, aSourceRect };
        int32_t passes = static_cast<int32_t>(aRadius);
        if (passes % 2 == 0)
            ++passes;
        for (int32_t pass = 0; pass < passes; ++pass)
        {
            if (pass % 2 == 0)
                neogfx::blur(*this, aDestinationRect, aSource, aSourceRect, aAlgorithm, aParameter1, aParameter2);
            else
                neogfx::blur(aSource, aSourceRect, *this, aDestinationRect, aAlgorithm, aParameter1, aParameter2);
        }
    }

    glyph_text graphics_context::to_glyph_text(std::string const& aText) const
    {
        return to_glyph_text(aText, default_font());
    }

    glyph_text graphics_context::to_glyph_text(std::string const& aText, const font& aFont) const
    {
        return to_glyph_text(aText.begin(), aText.end(), aFont);
    }

    glyph_text graphics_context::to_glyph_text(std::string const& aText, std::function<font(std::size_t)> aFontSelector) const
    {
        return to_glyph_text(aText.begin(), aText.end(), aFontSelector);
    }

    glyph_text graphics_context::to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd) const
    {
        return to_glyph_text(aTextBegin, aTextEnd, default_font());
    }

    glyph_text graphics_context::to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont) const
    {
        return to_glyph_text(aTextBegin, aTextEnd, [&aFont](std::size_t) { return aFont; });
    }

    glyph_text graphics_context::to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector) const
    {
        return service<i_font_manager>().glyph_text_factory().to_glyph_text(*this, std::string_view{ aTextBegin, aTextEnd }, aFontSelector);
    }

    glyph_text graphics_context::to_glyph_text(const std::u32string& aText) const
    {
        return to_glyph_text(aText, default_font());
    }

    glyph_text graphics_context::to_glyph_text(const std::u32string& aText, const font& aFont) const
    {
        return to_glyph_text(aText.begin(), aText.end(), aFont);
    }

    glyph_text graphics_context::to_glyph_text(const std::u32string& aText, std::function<font(std::size_t)> aFontSelector) const
    {
        return to_glyph_text(aText.begin(), aText.end(), aFontSelector);
    }

    glyph_text graphics_context::to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd) const
    {
        return to_glyph_text(aTextBegin, aTextEnd, default_font());
    }

    glyph_text graphics_context::to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont) const
    {
        return to_glyph_text(aTextBegin, aTextEnd, [&aFont](std::size_t) { return aFont; });
    }

    glyph_text graphics_context::to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector) const
    {
        return service<i_font_manager>().glyph_text_factory().to_glyph_text(*this, std::u32string_view{ aTextBegin, aTextEnd }, aFontSelector);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::string const& aText, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aText, default_font(), aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::string const& aText, const font& aFont, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aText.begin(), aText.end(), aFont, aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aTextBegin, aTextEnd, default_font(), aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aTextBegin, aTextEnd, [aFont](std::size_t) { return aFont; }, aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(to_glyph_text(aTextBegin, aTextEnd, aFontSelector), aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(const std::u32string& aText, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aText, default_font(), aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(const std::u32string& aText, const font& aFont, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aText.begin(), aText.end(), aFont, aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aTextBegin, aTextEnd, default_font(), aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aTextBegin, aTextEnd, [&aFont](std::size_t) { return aFont; }, aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::size_t)> aFontSelector, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(to_glyph_text(aTextBegin, aTextEnd, aFontSelector), aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(const glyph_text& aText, dimension aMaxWidth, alignment aAlignment) const
    {
        multiline_glyph_text result{ aText };
        typedef std::pair<glyph_text::const_iterator, glyph_text::const_iterator> line_t;
        typedef std::vector<line_t> lines_t;
        lines_t lines;
        std::array<glyph, 2> delimeters = { glyph{ text_category::Whitespace, '\r' }, glyph{ text_category::Whitespace, '\n' } };
        neolib::tokens(result.glyphText.cbegin(), result.glyphText.cend(), delimeters.begin(), delimeters.end(), lines, 0, false);
        vec3 pos;
        dimension maxLineWidth = 0.0;
        for (lines_t::const_iterator i = lines.begin(); i != lines.end(); ++i)
        {
            auto const& line = (logical_coordinates().is_gui_orientation() ? *i : *(lines.rbegin() + (i - lines.begin())));
            if (aMaxWidth == 0)
            {
                vec3 linePos = pos;
                size lineExtent = from_device_units(result.glyphText.extents(line.first, line.second));
                result.lines.push_back(multiline_glyph_text::line{ linePos, lineExtent.to_vec2(), std::distance(result.glyphText.cbegin(), line.first), std::distance(result.glyphText.cbegin(), line.second) });
                maxLineWidth = std::max(maxLineWidth, lineExtent.cx);
                pos.y += lineExtent.cy;
            }
            else
            {
                glyph_text::const_iterator next = line.first;
                glyph_text::const_iterator lineStart = next;
                glyph_text::const_iterator lineEnd = line.second;
                dimension maxWidth = to_device_units(size(aMaxWidth, 0.0)).cx;
                dimension lineWidth = 0.0;
                while (next != line.second)
                {
                    bool gotLine = false;
                    if (lineWidth + result.glyphText.extents(*next).cx > maxWidth)
                    {
                        if (next != lineStart)
                        {
                            std::pair<glyph_text::const_iterator, glyph_text::const_iterator> wordBreak = result.glyphText.word_break(lineStart, next);
                            lineWidth -= result.glyphText.extents(wordBreak.first, next, false).cx;
                            lineEnd = wordBreak.first;
                            next = wordBreak.second;
                            if (lineEnd == next)
                            {
                                while (lineEnd != line.second && (lineEnd + 1)->source == wordBreak.first->source)
                                    ++lineEnd;
                                next = lineEnd;
                            }
                        }
                        else
                        {
                            lineWidth += advance(*next).cx;
                            ++next;
                        }
                        gotLine = true;
                    }
                    else
                    {
                        lineWidth += advance(*next).cx;
                        ++next;
                    }
                    if (gotLine || next == line.second)
                    {
                        lineWidth += (result.glyphText.extents(*std::prev(next)).cx - advance(*std::prev(next)).cx);
                        vec3 linePos = pos;
                        result.lines.push_back(multiline_glyph_text::line{ linePos, result.glyphText.extents(lineStart, lineEnd).to_vec2(), std::distance(result.glyphText.cbegin(), lineStart), std::distance(result.glyphText.cbegin(), lineEnd) });
                        maxLineWidth = std::max(maxLineWidth, lineWidth);
                        pos.y += result.lines.back().extents.cy;
                        lineStart = next;
                        lineEnd = line.second;
                        lineWidth = 0;
                    }
                }
            }
            if (line.first == line.second)
                pos.y += aText.glyph_font().height();
        }

        for (auto& line : result.lines)
        {
            auto const textDirection = glyph_text_direction(result.glyphText.cbegin() + line.begin, result.glyphText.cbegin() + line.end);
            if ((aAlignment == alignment::Left && textDirection == text_direction::RTL) || (aAlignment == alignment::Right && textDirection == text_direction::LTR))
                line.pos.x += maxLineWidth - from_device_units(size{ line.extents.cx, 0.0 }).cx;
            else if (aAlignment == alignment::Center)
                line.pos.x += std::ceil((maxLineWidth - from_device_units(size{ line.extents.cx, 0.0 }).cx) / 2);
        }

        return result;
    }

    void graphics_context::draw_glyph(const point& aPoint, const glyph_text& aText, const glyph& aGlyph, const text_format& aTextFormat) const
    {
        draw_glyph(aPoint.to_vec3(), aText, aGlyph, aTextFormat);
    }

    void graphics_context::draw_glyph(const vec3& aPoint, const glyph_text& aText, const glyph& aGlyph, const text_format& aTextFormat) const
    {
        auto adjustedPos = (to_device_units(point{ aPoint }) + iOrigin).to_vec3() + vec3{ 0.0, 0.0, aPoint.z };
        native_context().enqueue(graphics_operation::draw_glyphs{ adjustedPos, aText, &aGlyph, std::next(&aGlyph), text_format_span{ 0, 1, aTextFormat }, mnemonics_shown() });
    }

    void graphics_context::draw_glyphs(const point& aPoint, const glyph_text& aText, const text_format_spans& aSpans) const
    {
        draw_glyphs(aPoint.to_vec3(), aText, aSpans);
    }

    void graphics_context::draw_glyphs(const vec3& aPoint, const glyph_text& aText, const text_format_spans& aSpans) const
    {
        auto adjustedPos = (to_device_units(point{ aPoint }) + iOrigin).to_vec3() + vec3{ 0.0, 0.0, aPoint.z };
        native_context().enqueue(graphics_operation::draw_glyphs{ adjustedPos, aText, aText.begin(), aText.end(), aSpans, mnemonics_shown()});
    }

    char graphics_context::mnemonic() const
    {
        if (mnemonic_set())
            return iMnemonic->second;
        return '&';
    }

    bool graphics_context::mnemonic_set() const
    {
        return iMnemonic != std::nullopt;
    }

    void graphics_context::set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix) const
    {
        iMnemonic = std::make_pair(aShowMnemonics, aMnemonicPrefix);
    }

    void graphics_context::unset_mnemonic() const
    {
        iMnemonic = std::nullopt;
    }

    bool graphics_context::mnemonics_shown() const
    {
        return iMnemonic != std::nullopt && iMnemonic->first;
    }

    bool graphics_context::password() const
    {
        return iPassword != std::nullopt;
    }

    std::string const& graphics_context::password_mask() const
    {
        if (password())
        {
            if (iPassword->empty())
                iPassword = "\xE2\x97\x8F";
            return *iPassword;
        }
        throw password_not_set();
    }

    void graphics_context::set_password(bool aPassword, std::string const& aMask)
    {
        if (aPassword)
            iPassword = aMask;
        else
            iPassword = std::nullopt;
    }

    void graphics_context::draw_texture(const point& aPoint, const i_texture& aTexture, const color_or_gradient& aColor, shader_effect aShaderEffect) const
    {
        draw_texture(rect{ aPoint, aTexture.extents() }, aTexture, with_bounding_box(aColor, rect{ aPoint, aTexture.extents() }), aShaderEffect);
    }

    void graphics_context::draw_texture(const rect& aRect, const i_texture& aTexture, const color_or_gradient& aColor, shader_effect aShaderEffect) const
    {
        if (logical_coordinates().is_gui_orientation())
            draw_texture(to_ecs_component(aRect), aTexture, with_bounding_box(aColor, aRect), aShaderEffect);
        else
            draw_texture(to_ecs_component(game_rect{ aRect }), aTexture, with_bounding_box(aColor, game_rect{ aRect }), aShaderEffect);
    }

    void graphics_context::draw_texture(const point& aPoint, const i_texture& aTexture, const rect& aTextureRect, const color_or_gradient& aColor, shader_effect aShaderEffect) const
    {
        draw_texture(rect{ aPoint, aTextureRect.extents() }, aTexture, aTextureRect, with_bounding_box(aColor, rect{ aPoint, aTextureRect.extents() }), aShaderEffect);
    }

    void graphics_context::draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect, const color_or_gradient& aColor, shader_effect aShaderEffect) const
    {
        if (logical_coordinates().is_gui_orientation())
            draw_texture(to_ecs_component(aRect), aTexture, aTextureRect, with_bounding_box(aColor, aRect), aShaderEffect);
        else
            draw_texture(to_ecs_component(game_rect{ aRect }), aTexture, aTextureRect, with_bounding_box(aColor, game_rect{ aRect }), aShaderEffect);
    }

    void graphics_context::draw_texture(const game::mesh& aMesh, const i_texture& aTexture, const color_or_gradient& aColor, shader_effect aShaderEffect) const
    {
        draw_mesh(
            aMesh, 
            game::material
            { 
                std::holds_alternative<color>(aColor) ? game::color{ to_ecs_component(std::get<color>(aColor)) } : std::optional<game::color>{},
                std::holds_alternative<gradient>(aColor) ? game::gradient{ to_ecs_component(std::get<gradient>(aColor)) } : std::optional<game::gradient>{},
                {},
                to_ecs_component(aTexture),
                aShaderEffect
            },
            optional_mat44{});
    }

    void graphics_context::draw_texture(const game::mesh& aMesh, const i_texture& aTexture, const rect& aTextureRect, const color_or_gradient& aColor, shader_effect aShaderEffect) const
    {
        auto adjustedMesh = aMesh;
        for (auto& uv : adjustedMesh.uv)
            uv = (aTextureRect.top_left() / aTexture.extents()).to_vec2() + uv.scale((aTextureRect.extents() / aTexture.extents()).to_vec2());
        draw_texture(adjustedMesh, aTexture, aColor, aShaderEffect);
    }

    void graphics_context::draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const optional_mat44& aTransformation, const std::optional<game::filter>& aFilter) const
    {
        vec2 const toDeviceUnits = to_device_units(vec2{ 1.0, 1.0 });
        native_context().enqueue(
            graphics_operation::draw_mesh{
                aMesh,
                aMaterial,
                mat44{
                    { toDeviceUnits.x, 0.0, 0.0, 0.0 },
                    { 0.0, toDeviceUnits.y, 0.0, 0.0 },
                    { 0.0, 0.0, 1.0, 0.0 },
                    { iOrigin.x, iOrigin.y, 0.0, 1.0 } } * (aTransformation != std::nullopt ? *aTransformation : mat44::identity()),
                aFilter
            });
    }
}