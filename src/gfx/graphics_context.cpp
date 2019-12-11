// graphics_context.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/string_utils.hpp>
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
#include "native/i_native_texture.hpp"
#include "text/native/native_font_face.hpp"
#include "../hid/native/i_native_surface.hpp"
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gfx/text/text_category_map.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/rectangle.hpp>
#include <neogfx/game/ecs_helpers.hpp>
#include <neogfx/gfx/graphics_context.hpp>

namespace neogfx
{
    struct graphics_context::glyph_text_data
    {
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
        mutable glyph_text iGlyphTextResult;
        mutable glyph_text iGlyphTextResult2;
    };

    graphics_context::graphics_context(const i_surface& aSurface, type aType) :
        iType{ aType },
        iRenderTarget{ aSurface.native_surface() },
        iNativeGraphicsContext{ nullptr },
        iDefaultFont{},
        iOrigin{ 0.0, 0.0 },
        iExtents{ aSurface.extents() },
        iLayer{ 0 },
        iOpacity{ 1.0 },
        iBlendingMode{ neogfx::blending_mode::Default },
        iSmoothingMode{ neogfx::smoothing_mode::None },
        iSubpixelRendering{ service<i_rendering_engine>().is_subpixel_rendering_on() },
        iGlyphTextData{ std::make_unique<glyph_text_data>() }
    {
    }

    graphics_context::graphics_context(const i_surface& aSurface, const font& aDefaultFont, type aType) :
        iType{ aType },
        iRenderTarget{ aSurface.native_surface() },
        iNativeGraphicsContext{ nullptr },
        iDefaultFont{ aDefaultFont },
        iOrigin{ 0.0, 0.0 },
        iExtents{ aSurface.extents() },
        iLayer{ 0 },
        iOpacity{ 1.0 },
        iBlendingMode{ neogfx::blending_mode::Default },
        iSmoothingMode{ neogfx::smoothing_mode::None },
        iSubpixelRendering{ service<i_rendering_engine>().is_subpixel_rendering_on() },
        iGlyphTextData{ std::make_unique<glyph_text_data>() }
    {
    }

    graphics_context::graphics_context(const i_widget& aWidget, type aType) :
        iType{ aType },
        iRenderTarget{ aWidget.surface().native_surface() },
        iNativeGraphicsContext{ nullptr },
        iDefaultFont{ aWidget.font() },
        iOrigin{ aWidget.origin() },
        iExtents{ aWidget.extents() },
        iLayer{ 0 },
        iOpacity{ 1.0 },
        iBlendingMode{ neogfx::blending_mode::Default },
        iSmoothingMode{ neogfx::smoothing_mode::None },
        iSubpixelRendering{ service<i_rendering_engine>().is_subpixel_rendering_on() },
        iGlyphTextData{ std::make_unique<glyph_text_data>() }
    {
        set_logical_coordinate_system(aWidget.logical_coordinate_system());
    }

    graphics_context::graphics_context(const i_texture& aTexture, type aType) :
        iType{ aType },
        iRenderTarget{ *aTexture.native_texture() },
        iNativeGraphicsContext{ nullptr },
        iDefaultFont{ font() },
        iOrigin{},
        iExtents{ aTexture.extents() },
        iLayer{ 0 },
        iOpacity{ 1.0 },
        iBlendingMode{ neogfx::blending_mode::Default },
        iSmoothingMode{ neogfx::smoothing_mode::None },
        iSubpixelRendering{ service<i_rendering_engine>().is_subpixel_rendering_on() },
        iGlyphTextData{ std::make_unique<glyph_text_data>() }
    {
    }

    graphics_context::graphics_context(const graphics_context& aOther) :
        iType{ aOther.iType },
        iRenderTarget{ aOther.iRenderTarget },
        iNativeGraphicsContext{ aOther.iNativeGraphicsContext != nullptr ? aOther.native_context().clone() : nullptr },
        iDefaultFont{ aOther.iDefaultFont },
        iOrigin{ aOther.origin() },
        iExtents{ aOther.extents() },
        iLayer{ 0 },
        iLogicalCoordinateSystem{ aOther.iLogicalCoordinateSystem },
        iLogicalCoordinates{ aOther.iLogicalCoordinates },
        iOpacity{ 1.0 },
        iBlendingMode{ neogfx::blending_mode::Default },
        iSmoothingMode{ neogfx::smoothing_mode::None },
        iSubpixelRendering{ service<i_rendering_engine>().is_subpixel_rendering_on() },
        iGlyphTextData{ std::make_unique<glyph_text_data>() }
    {
    }

    graphics_context::~graphics_context()
    {
    }

    std::unique_ptr<i_rendering_context> graphics_context::clone() const
    {
        return std::make_unique<graphics_context>(*this);
    }

    i_rendering_engine& graphics_context::rendering_engine()
    {
        return native_context().rendering_engine();
    }

    const i_render_target& graphics_context::render_target() const
    {
        return iRenderTarget;
    }

    const i_render_target& graphics_context::render_target()
    {
        return iRenderTarget;
    }

    rect graphics_context::rendering_area(bool aConsiderScissor) const
    {
        return native_context().rendering_area(aConsiderScissor);
    }

    void graphics_context::enqueue(const graphics_operation::operation& aOperation)
    {
        native_context().enqueue(aOperation);
    }

    void graphics_context::flush()
    {
        native_context().flush();
    }

    graphics_context::ping_pong_buffers_t graphics_context::ping_pong_buffers(const size& aExtents, texture_sampling aSampling, const optional_colour& aClearColour) const
    {
        auto buffer1 = std::make_unique<graphics_context>(service<i_rendering_engine>().ping_pong_buffer1(aExtents, aSampling));
        {
            scoped_scissor ss{ *buffer1, rect{ point{}, aExtents} };
            if (aClearColour != std::nullopt)
            {
                buffer1->clear(*aClearColour);
                buffer1->clear_depth_buffer();
                buffer1->clear_stencil_buffer();
            }
        }
        buffer1->render_target().deactivate_target();
        auto buffer2 = std::make_unique<graphics_context>(service<i_rendering_engine>().ping_pong_buffer2(aExtents, aSampling));
        {
            scoped_scissor ss{ *buffer2, rect{ point{}, aExtents} };
            if (aClearColour != std::nullopt)
            {
                buffer2->clear(*aClearColour);
                buffer2->clear_depth_buffer();
                buffer2->clear_stencil_buffer();
            }
        }
        buffer2->render_target().deactivate_target();
        render_target().activate_target();
        return ping_pong_buffers_t{ std::move(buffer1), std::move(buffer2) };
    }
    
    delta graphics_context::to_device_units(const delta& aValue) const
    {
        return units_converter(*this).to_device_units(aValue);
    }

    size graphics_context::to_device_units(const size& aValue) const
    {
        return units_converter(*this).to_device_units(aValue);
    }

    point graphics_context::to_device_units(const point& aValue) const
    {
        return units_converter(*this).to_device_units(aValue);
    }

    vec2 graphics_context::to_device_units(const vec2& aValue) const
    {
        return units_converter(*this).to_device_units(aValue);
    }

    rect graphics_context::to_device_units(const rect& aValue) const
    {
        return units_converter(*this).to_device_units(aValue);
    }

    path graphics_context::to_device_units(const path& aValue) const
    {
        path result = aValue;
        result.set_position(to_device_units(result.position()));
        for (std::size_t i = 0; i < result.paths().size(); ++i)
            for (std::size_t j = 0; j < result.paths()[i].size(); ++j)
                result.paths()[i][j] = to_device_units(result.paths()[i][j]);
        return result;
    }

    delta graphics_context::from_device_units(const delta& aValue) const
    {
        return units_converter(*this).from_device_units(aValue);
    }

    size graphics_context::from_device_units(const size& aValue) const
    {
        return units_converter(*this).from_device_units(aValue);
    }

    point graphics_context::from_device_units(const point& aValue) const
    {
        return units_converter(*this).from_device_units(aValue);
    }

    rect graphics_context::from_device_units(const rect& aValue) const
    {
        return units_converter(*this).from_device_units(aValue);
    }

    path graphics_context::from_device_units(const path& aValue) const
    {
        path result = aValue;
        result.set_position(from_device_units(result.position()));
        for (std::size_t i = 0; i < result.paths().size(); ++i)
            for (std::size_t j = 0; j < result.paths()[i].size(); ++j)
                result.paths()[i][j] = from_device_units(result.paths()[i][j]);
        return result;
    }

    int32_t graphics_context::layer() const
    {
        return iLayer;
    }

    void graphics_context::set_layer(int32_t aLayer)
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
        iOrigin = to_device_units(aOrigin);
    }

    point graphics_context::origin() const
    {
        return from_device_units(iOrigin);
    }

    void graphics_context::set_pixel(const point& aPoint, const colour& aColour) const
    {
        native_context().enqueue(graphics_operation::set_pixel{ to_device_units(aPoint) + iOrigin, aColour });
    }

    void graphics_context::draw_pixel(const point& aPoint, const colour& aColour) const
    {
        native_context().enqueue(graphics_operation::draw_pixel{ to_device_units(aPoint) + iOrigin, aColour });
    }

    void graphics_context::draw_line(const point& aFrom, const point& aTo, const pen& aPen) const
    {
        native_context().enqueue(graphics_operation::draw_line{ to_device_units(aFrom) + iOrigin, to_device_units(aTo) + iOrigin, aPen });
    }

    void graphics_context::draw_rect(const rect& aRect, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_rect(aRect, aFill);
        native_context().enqueue(graphics_operation::draw_rect{ to_device_units(aRect) + iOrigin, aPen });
    }

    void graphics_context::draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_rounded_rect(aRect, aRadius, aFill);
        native_context().enqueue(graphics_operation::draw_rounded_rect{ to_device_units(aRect) + iOrigin, aRadius, aPen });
    }

    void graphics_context::draw_circle(const point& aCentre, dimension aRadius, const pen& aPen, const brush& aFill, angle aStartAngle) const
    {
        if (aFill != neolib::none)
            fill_circle(aCentre, aRadius, aFill);
        native_context().enqueue(graphics_operation::draw_circle{ to_device_units(aCentre) + iOrigin, aRadius, aPen, aStartAngle });
    }

    void graphics_context::draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_arc(aCentre, aRadius, aStartAngle, aEndAngle, aFill);
        native_context().enqueue(graphics_operation::draw_arc{ to_device_units(aCentre) + iOrigin, aRadius, aStartAngle, aEndAngle, aPen });
    }

    void graphics_context::draw_path(const path& aPath, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_path(aPath, aFill);
        path path = to_device_units(aPath);
        path.set_position(path.position() + iOrigin);
        native_context().enqueue(graphics_operation::draw_path{ path, aPen });
    }

    void graphics_context::draw_shape(const game::mesh& aShape, const pen& aPen, const brush& aFill) const
    {
        if (aFill != neolib::none)
            fill_shape(aShape, aFill);
        vec2 const toDeviceUnits = to_device_units(vec2{ 1.0, 1.0 });
        native_context().enqueue(
            graphics_operation::draw_shape{
                mat44{ 
                    { toDeviceUnits.x, 0.0, 0.0, 0.0 },
                    { 0.0, toDeviceUnits.y, 0.0, 0.0 },
                    { 0.0, 0.0, 1.0, 0.0 }, 
                    { iOrigin.x, iOrigin.y, 0.0, 1.0 } } * aShape,
                aPen });
    }

    void graphics_context::draw_entities(game::i_ecs& aEcs) const
    {
        vec2 const toDeviceUnits = to_device_units(vec2{ 1.0, 1.0 });
        native_context().enqueue(
            graphics_operation::draw_entities{ 
                aEcs, 
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
        line_stipple_on(1, 0xAAAA);
        draw_rect(aRect, pen(colour::White, 1.0));
        line_stipple_off();
        pop_logical_operation();
    }

    void graphics_context::fill_rect(const rect& aRect, const brush& aFill, scalar aZpos) const
    {
        native_context().enqueue(graphics_operation::fill_rect{ to_device_units(aRect) + iOrigin, aFill, aZpos });
    }

    void graphics_context::fill_rounded_rect(const rect& aRect, dimension aRadius, const brush& aFill) const
    {
        native_context().enqueue(graphics_operation::fill_rounded_rect{ to_device_units(aRect) + iOrigin, aRadius, aFill });
    }

    void graphics_context::fill_circle(const point& aCentre, dimension aRadius, const brush& aFill) const
    {
        native_context().enqueue(graphics_operation::fill_circle{ to_device_units(aCentre) + iOrigin, aRadius, aFill });
    }

    void graphics_context::fill_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill) const
    {
        native_context().enqueue(graphics_operation::fill_arc{ to_device_units(aCentre) + iOrigin, aRadius, aStartAngle, aEndAngle, aFill });
    }

    void graphics_context::fill_path(const path& aPath, const brush& aFill) const
    {
        path path = to_device_units(aPath);
        path.set_position(path.position() + iOrigin);
        native_context().enqueue(graphics_operation::fill_path{ path, aFill });
    }

    void graphics_context::fill_shape(const game::mesh& aShape, const brush& aFill) const
    {
        vec2 const toDeviceUnits = to_device_units(vec2{ 1.0, 1.0 });
        native_context().enqueue(
            graphics_operation::fill_shape{
                mat44{ 
                    { toDeviceUnits.x, 0.0, 0.0, 0.0 },
                    { 0.0, toDeviceUnits.y, 0.0, 0.0 },
                    { 0.0, 0.0, 1.0, 0.0 }, 
                    { iOrigin.x, iOrigin.y, 0.0, 1.0 } } * aShape,
                aFill });
    }

    size graphics_context::text_extent(const std::string& aText, const font& aFont) const
    {
        return text_extent(aText.begin(), aText.end(), [&aFont](std::u32string::size_type) { return aFont; });
    }

    size graphics_context::text_extent(const std::string& aText, std::function<font(std::string::size_type)> aFontSelector) const
    {
        return text_extent(aText.begin(), aText.end(), aFontSelector);
    }

    size graphics_context::text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont) const
    {
        return text_extent(aTextBegin, aTextEnd, [&aFont](std::u32string::size_type) { return aFont; });
    }

    size graphics_context::text_extent(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const
    {
        return glyph_text_extent(to_glyph_text(aTextBegin, aTextEnd, aFontSelector));
    }

    size graphics_context::multiline_text_extent(const std::string& aText, const font& aFont) const
    {
        return multiline_text_extent(aText, [&aFont](std::u32string::size_type) { return aFont; }, 0);
    }

    size graphics_context::multiline_text_extent(const std::string& aText, std::function<font(std::string::size_type)> aFontSelector) const
    {
        return multiline_text_extent(aText, aFontSelector, 0);
    }

    size graphics_context::multiline_text_extent(const std::string& aText, const font& aFont, dimension aMaxWidth) const
    {
        return multiline_text_extent(aText, [&aFont](std::u32string::size_type) { return aFont; }, aMaxWidth);
    }
        
    size graphics_context::multiline_text_extent(const std::string& aText, std::function<font(std::string::size_type)> aFontSelector, dimension aMaxWidth) const
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

    bool graphics_context::is_text_left_to_right(const std::string& aText, const font& aFont) const
    {
        const auto& glyphText = to_glyph_text(aText.begin(), aText.end(), aFont);
        return glyph_text_direction(glyphText.cbegin(), glyphText.cend()) == text_direction::LTR;
    }

    bool graphics_context::is_text_right_to_left(const std::string& aText, const font& aFont) const
    {
        return !is_text_left_to_right(aText, aFont);
    }

    void graphics_context::draw_text(const point& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance) const
    {
        draw_text(aPoint.to_vec3(), aText, aFont, aAppearance);
    }

    void graphics_context::draw_text(const point& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, const text_appearance& aAppearance) const
    {
        draw_text(aPoint.to_vec3(), aTextBegin, aTextEnd, aFont, aAppearance);
    }

    void graphics_context::draw_text(const vec3& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance) const
    {
        draw_text(aPoint, aText.begin(), aText.end(), aFont, aAppearance);
    }

    void graphics_context::draw_text(const vec3& aPoint, std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, const text_appearance& aAppearance) const
    {
        draw_glyph_text(aPoint, to_glyph_text(aTextBegin, aTextEnd, aFont), aAppearance);
    }

    void graphics_context::draw_multiline_text(const point& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance, alignment aAlignment) const
    {
        draw_multiline_text(aPoint.to_vec3(), aText, aFont, aAppearance, aAlignment);
    }

    void graphics_context::draw_multiline_text(const point& aPoint, const std::string& aText, const font& aFont, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment) const
    {
        draw_multiline_text(aPoint.to_vec3(), aText, aFont, aMaxWidth, aAppearance, aAlignment);
    }
        
    void graphics_context::draw_multiline_text(const vec3& aPoint, const std::string& aText, const font& aFont, const text_appearance& aAppearance, alignment aAlignment) const
    {
        draw_multiline_text(aPoint, aText, aFont, 0, aAppearance, aAlignment);
    }

    void graphics_context::draw_multiline_text(const vec3& aPoint, const std::string& aText, const font& aFont, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment) const
    {
        auto multilineGlyphText = to_multiline_glyph_text(aText, aFont, aMaxWidth, aAlignment);
        for (auto& line : multilineGlyphText.lines)
        {
            if (line.begin == line.end)
                continue;
            draw_glyph_text(aPoint + line.pos.to_vec3(), multilineGlyphText.glyphText, multilineGlyphText.glyphText.begin() + line.begin, multilineGlyphText.glyphText.begin() + line.end, aAppearance);
        }
    }

    void graphics_context::draw_glyph_text(const point& aPoint, const glyph_text& aText, const text_appearance& aAppearance) const
    {
        draw_glyph_text(aPoint.to_vec3(), aText, aAppearance);
    }

    void graphics_context::draw_glyph_text(const point& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const text_appearance& aAppearance) const
    {
        draw_glyph_text(aPoint.to_vec3(), aText, aTextBegin, aTextEnd, aAppearance);
    }

    void graphics_context::draw_glyph_text(const vec3& aPoint, const glyph_text& aText, const text_appearance& aAppearance) const
    {
        draw_glyph_text(aPoint, aText, aText.cbegin(), aText.cend(), aAppearance);
    }

    void graphics_context::draw_glyph_text(const vec3& aPoint, const glyph_text& aText, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const text_appearance& aAppearance) const
    {
        neogfx::draw_glyph_text(*this, aPoint, aText, aTextBegin, aTextEnd, aAppearance);
    }

    void graphics_context::draw_multiline_glyph_text(const point& aPoint, const glyph_text& aText, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment) const
    {
        draw_multiline_glyph_text(aPoint.to_vec3(), aText, aMaxWidth, aAppearance, aAlignment);
    }

    void graphics_context::draw_multiline_glyph_text(const vec3& aPoint, const glyph_text& aText, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment) const
    {
        auto multilineGlyphText = to_multiline_glyph_text(aText, aMaxWidth, aAlignment);
        for (auto& line : multilineGlyphText.lines)
        {
            if (line.begin == line.end)
                continue;
            draw_glyph_text(aPoint + line.pos.to_vec3(), multilineGlyphText.glyphText, multilineGlyphText.glyphText.begin() + line.begin, multilineGlyphText.glyphText.begin() + line.end, aAppearance);
        }
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

    bool graphics_context::high_dpi() const
    {
        return device_metrics().ppi() >= 150.0;
    }

    dimension graphics_context::dpi_scale_factor() const
    {
        return default_dpi_scale_factor(device_metrics().ppi());
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

    i_rendering_context& graphics_context::native_context() const
    {
        if (attached())
        {
            if (iNativeGraphicsContext == nullptr)
                iNativeGraphicsContext = iRenderTarget.create_graphics_context(iBlendingMode);
            return *iNativeGraphicsContext;
        }
        throw unattached();
    }

    void graphics_context::flush() const
    {
        native_context().flush();
    }

    void graphics_context::scissor_on(const rect& aRect) const
    {
        native_context().enqueue(graphics_operation::scissor_on{ to_device_units(aRect) + iOrigin });
    }

    void graphics_context::scissor_off() const
    {
        native_context().enqueue(graphics_operation::scissor_off{});
    }

    void graphics_context::clip_to(const rect& aRect) const
    {
        native_context().enqueue(graphics_operation::clip_to_rect{ to_device_units(aRect) + iOrigin });
    }

    void graphics_context::clip_to(const path& aPath, dimension aPathOutline) const
    {
        path path = to_device_units(aPath);
        path.set_shape(path::ConvexPolygon);
        path.set_position(path.position() + iOrigin);
        native_context().enqueue(graphics_operation::clip_to_path{ path, aPathOutline });
    }

    void graphics_context::reset_clip() const
    {
        native_context().enqueue(graphics_operation::reset_clip{});
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

    void graphics_context::line_stipple_on(uint32_t aFactor, uint16_t aPattern) const
    {
        native_context().enqueue(graphics_operation::line_stipple_on{ aFactor, aPattern });
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

    void graphics_context::clear(const colour& aColour, const std::optional<scalar>& aZpos) const
    {
        if (aZpos == std::nullopt)
            native_context().enqueue(graphics_operation::clear{ aColour });
        else
            fill_rect(rect{ render_target().target_type() == render_target_type::Surface ? point{} : point{-1.0, -1.0}, iRenderTarget.target_texture().storage_extents() }, aColour, *aZpos);
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

    void graphics_context::blur(const rect& aDestinationRect, const i_graphics_context& aSource, const rect& aSourceRect, blurring_algorithm aAlgorithm, uint32_t aParameter1, double aParameter2) const
    {
        // todo
        scoped_scissor ss{ *this, aDestinationRect };
        dimension const w = aParameter1;
        for (coordinate y = -w; y <= w; y += 1.0)
            for (coordinate x = -w; x <= w; x += 1.0)
                blit(aDestinationRect + point{ x, y }, aSource, aSourceRect);
    }

    glyph_text graphics_context::to_glyph_text(const std::string& aText, const font& aFont) const
    {
        return to_glyph_text(aText.begin(), aText.end(), aFont);
    }

    glyph_text graphics_context::to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont) const
    {
        return to_glyph_text(aTextBegin, aTextEnd, [&aFont](std::string::size_type) { return aFont; });
    }

    glyph_text graphics_context::to_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const
    {
        return to_glyph_text_impl(aTextBegin, aTextEnd, aFontSelector);
    }

    glyph_text graphics_context::to_glyph_text(const std::u32string& aText, const font& aFont) const
    {
        return to_glyph_text(aText.begin(), aText.end(), aFont);
    }

    glyph_text graphics_context::to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont) const
    {
        return to_glyph_text(aTextBegin, aTextEnd, [&aFont](std::u32string::size_type) { return aFont; });
    }

    glyph_text graphics_context::to_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const
    {
        return to_glyph_text_impl(aTextBegin, aTextEnd, aFontSelector);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(const std::string& aText, const font& aFont, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aText.begin(), aText.end(), aFont, aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, const font& aFont, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aTextBegin, aTextEnd, [aFont](std::u32string::size_type) { return aFont; }, aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector, dimension aMaxWidth, alignment aAlignment) const
    {
        auto& clusterMap = iGlyphTextData->iClusterMap;
        clusterMap.clear();
        iGlyphTextData->iCodePointsBuffer.clear();
        std::u32string& codePoints = iGlyphTextData->iCodePointsBuffer;

        codePoints = neolib::utf8_to_utf32(aTextBegin, aTextEnd, [&clusterMap](std::string::size_type aFrom, std::u32string::size_type)
        {
            clusterMap.push_back(glyph_text_data::cluster{ aFrom });
        });

        return to_multiline_glyph_text(codePoints.begin(), codePoints.end(), [&aFontSelector, &clusterMap](std::u32string::size_type aIndex)->font
        {
            return aFontSelector(clusterMap[aIndex].from);
        }, aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(const std::u32string& aText, const font& aFont, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aText.begin(), aText.end(), aFont, aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, const font& aFont, dimension aMaxWidth, alignment aAlignment) const
    {
        return to_multiline_glyph_text(aTextBegin, aTextEnd, [&aFont](std::u32string::size_type) { return aFont; }, aMaxWidth, aAlignment);
    }

    graphics_context::multiline_glyph_text graphics_context::to_multiline_glyph_text(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector, dimension aMaxWidth, alignment aAlignment) const
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
            const auto& line = (logical_coordinates().is_gui_orientation() ? *i : *(lines.rbegin() + (i - lines.begin())));
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
                    if (lineWidth + next->extents(result.glyphText).cx > maxWidth)
                    {
                        if (next != lineStart)
                        {
                            std::pair<glyph_text::const_iterator, glyph_text::const_iterator> wordBreak = result.glyphText.word_break(lineStart, next);
                            lineWidth -= result.glyphText.extents(wordBreak.first, next, false).cx;
                            lineEnd = wordBreak.first;
                            next = wordBreak.second;
                            if (lineEnd == next)
                            {
                                while (lineEnd != line.second && (lineEnd + 1)->source() == wordBreak.first->source())
                                    ++lineEnd;
                                next = lineEnd;
                            }
                        }
                        else
                        {
                            lineWidth += next->advance().cx;
                            ++next;
                        }
                        gotLine = true;
                    }
                    else
                    {
                        lineWidth += next->advance().cx;
                        ++next;
                    }
                    if (gotLine || next == line.second)
                    {
                        lineWidth += (std::prev(next)->extents(result.glyphText).cx - std::prev(next)->advance().cx);
                        vec3 linePos = pos;
                        result.lines.push_back(multiline_glyph_text::line{ linePos, result.glyphText.extents(lineStart, lineEnd).to_vec2(), std::distance(result.glyphText.cbegin(), lineStart), std::distance(result.glyphText.cbegin(), lineEnd) });
                        maxLineWidth = std::max(maxLineWidth, lineWidth);
                        pos.y += result.lines.back().extents.cy;
                        lineStart = next;
                        lineEnd = line.second;
                        lineWidth = 0;
                    }
                }
                if (line.first == line.second)
                    pos.y += font().height();
            }
        }

        for (auto& line : result.lines)
        {
            auto const textDirection = glyph_text_direction(result.glyphText.cbegin() + line.begin, result.glyphText.cbegin() + line.end);
            if ((aAlignment == alignment::Left && textDirection == text_direction::RTL) || (aAlignment == alignment::Right && textDirection == text_direction::LTR))
                line.pos.x += maxLineWidth - from_device_units(size{ line.extents.cx, 0.0 }).cx;
            else if (aAlignment == alignment::Centre)
                line.pos.x += std::ceil((maxLineWidth - from_device_units(size{ line.extents.cx, 0.0 }).cx) / 2);
        }

        return result;
    }

    void graphics_context::draw_glyph(const point& aPoint, const font& aFont, const glyph& aGlyph, const text_appearance& aAppearance) const
    {
        draw_glyph(aPoint.to_vec3(), aFont, aGlyph, aAppearance);
    }

    void graphics_context::draw_glyph(const vec3& aPoint, const font& aFont, const glyph& aGlyph, const text_appearance& aAppearance) const
    {
        try
        {
            auto adjustedPos = (to_device_units(point{ aPoint }) + iOrigin).to_vec3() + vec3{ 0.0, 0.0, aPoint.z };
            native_context().enqueue(graphics_operation::draw_glyph{ adjustedPos, aGlyph, aFont.id(), aAppearance });
            if (aGlyph.underline() || (mnemonics_shown() && aGlyph.mnemonic()))
                draw_glyph_underline(aPoint, aFont, aGlyph, aAppearance);
        }
        catch (const freetype_error& fe)
        {
            // do nothing except report error
            thread_local bool sReported;
            if (!sReported)
            {
                sReported = true;
                std::cerr << "FreeType error in draw_glyph(): " << fe.what() << ", ignoring further errors." << std::endl;
            }
        }
    }

    void graphics_context::draw_glyph_underline(const point& aPoint, const font& aFont, const glyph& aGlyph, const text_appearance& aAppearance) const
    {
        draw_glyph_underline(aPoint.to_vec3(), aFont, aGlyph, aAppearance);
    }

    void graphics_context::draw_glyph_underline(const vec3& aPoint, const font& aFont, const glyph& aGlyph, const text_appearance& aAppearance) const
    {
        auto yLine = logical_coordinates().is_gui_orientation() ?
            (aFont.height() - 1.0 + aFont.descender()) - aFont.native_font_face().underline_position() :
            -aFont.descender() + aFont.native_font_face().underline_position();
        draw_line(
            aPoint + vec3{ 0.0, yLine },
            aPoint + vec3{ mnemonics_shown() && aGlyph.mnemonic() ? aGlyph.extents(aFont).cx : aGlyph.advance().cx, yLine },
            pen{ aAppearance.ink(), aFont.native_font_face().underline_thickness() });
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

    const std::string& graphics_context::password_mask() const
    {
        if (password())
        {
            if (iPassword->empty())
                iPassword = "\xE2\x97\x8F";
            return *iPassword;
        }
        throw password_not_set();
    }

    void graphics_context::set_password(bool aPassword, const std::string& aMask)
    {
        if (aPassword)
            iPassword = aMask;
        else
            iPassword = std::nullopt;
    }

    void graphics_context::draw_texture(const point& aPoint, const i_texture& aTexture, const optional_colour& aColour, shader_effect aShaderEffect) const
    {
        draw_texture(rect{ aPoint, aTexture.extents() }, aTexture, aColour, aShaderEffect);
    }

    void graphics_context::draw_texture(const rect& aRect, const i_texture& aTexture, const optional_colour& aColour, shader_effect aShaderEffect) const
    {
        if (logical_coordinates().is_gui_orientation())
            draw_texture(to_ecs_component(aRect), aTexture, aColour, aShaderEffect);
        else
            draw_texture(to_ecs_component(game_rect{ aRect }), aTexture, aColour, aShaderEffect);
    }

    void graphics_context::draw_texture(const point& aPoint, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour, shader_effect aShaderEffect) const
    {
        draw_texture(rect{ aPoint, aTextureRect.extents() }, aTexture, aTextureRect, aColour, aShaderEffect);
    }

    void graphics_context::draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour, shader_effect aShaderEffect) const
    {
        if (logical_coordinates().is_gui_orientation())
            draw_texture(to_ecs_component(aRect), aTexture, aTextureRect, aColour, aShaderEffect);
        else
            draw_texture(to_ecs_component(game_rect{ aRect }), aTexture, aTextureRect, aColour, aShaderEffect);
    }

    void graphics_context::draw_texture(const game::mesh& aMesh, const i_texture& aTexture, const optional_colour& aColour, shader_effect aShaderEffect) const
    {
        draw_mesh(
            aMesh, 
            game::material
            { 
                aColour != std::nullopt ? game::colour{ aColour->to_vec4() } : std::optional<game::colour>{},
                {}, 
                {},
                to_ecs_component(aTexture),
                aShaderEffect
            },
            optional_mat44{});
    }

    void graphics_context::draw_texture(const game::mesh& aMesh, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour, shader_effect aShaderEffect) const
    {
        auto adjustedMesh = aMesh;
        for (auto& uv : adjustedMesh.uv)
            uv = (aTextureRect.top_left() / aTexture.extents()).to_vec2() + uv * (aTextureRect.extents() / aTexture.extents()).to_vec2();
         draw_texture(adjustedMesh, aTexture, aColour, aShaderEffect);
    }

    void graphics_context::draw_mesh(const game::mesh& aMesh, const game::material& aMaterial, const optional_mat44& aTransformation) const
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
                    { iOrigin.x, iOrigin.y, 0.0, 1.0 } } * (aTransformation != std::nullopt ? *aTransformation : mat44::identity())
            });
    }

    class graphics_context::glyph_shapes
    {
    public:
        struct not_using_fallback : std::logic_error { not_using_fallback() : std::logic_error("neogfx::graphics_context::glyph_shapes::not_using_fallback") {} };
    public:
        class glyphs
        {
        public:
            glyphs(const i_graphics_context& aParent, const font& aFont, const glyph_text_data::glyph_run& aGlyphRun) :
                iParent{ aParent },
                iFont{ static_cast<native_font_face::hb_handle*>(aFont.native_font_face().aux_handle())->font },
                iGlyphRun{ aGlyphRun },
                iBuf{ static_cast<native_font_face::hb_handle*>(aFont.native_font_face().aux_handle())->buf },
                iGlyphCount{ 0u },
                iGlyphInfo{ nullptr },
                iGlyphPos{ nullptr }
            {
                hb_ft_font_set_load_flags(iFont, aParent.is_subpixel_rendering_on() ? FT_LOAD_TARGET_LCD : FT_LOAD_TARGET_NORMAL);
                hb_buffer_set_direction(iBuf, std::get<2>(aGlyphRun) == text_direction::RTL ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
                hb_buffer_set_script(iBuf, std::get<4>(aGlyphRun));
                std::vector<uint32_t> reversed;
                if (std::get<2>(aGlyphRun) != text_direction::None_RTL)
                    hb_buffer_add_utf32(iBuf, reinterpret_cast<const uint32_t*>(std::get<0>(aGlyphRun)), static_cast<int>(std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun)), 0, static_cast<int>(std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun)));
                else
                {
                    reversed.reserve(std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun));
                    for (auto ch = std::get<1>(aGlyphRun); ch != std::get<0>(aGlyphRun); --ch)
                    {
                        switch (*(ch - 1))
                        {
                        case U'(':
                            reversed.push_back(U')');
                            break;
                        case U')':
                            reversed.push_back(U'(');
                            break;
                        case U'[':
                            reversed.push_back(U']');
                            break;
                        case U']':
                            reversed.push_back(U'[');
                            break;
                        case U'{':
                            reversed.push_back(U'}');
                            break;
                        case U'}':
                            reversed.push_back(U'{');
                            break;
                        default:
                            reversed.push_back(*(ch - 1));
                        }
                    }
                    hb_buffer_add_utf32(iBuf, &*reversed.begin(), static_cast<int>(reversed.size()), 0u, static_cast<int>(reversed.size()));
                }
                hb_shape(iFont, iBuf, NULL, 0);
                unsigned int glyphCount = 0;
                iGlyphInfo = hb_buffer_get_glyph_infos(iBuf, &glyphCount);
                iGlyphPos = hb_buffer_get_glyph_positions(iBuf, &glyphCount);
                iGlyphCount = glyphCount;
                if (std::get<2>(aGlyphRun) == text_direction::None_RTL)
                    for (uint32_t i = 0; i < iGlyphCount; ++i)
                        iGlyphInfo[i].cluster = static_cast<uint32_t>(std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun) - 1 - iGlyphInfo[i].cluster);
            }
            ~glyphs()
            {
                hb_buffer_clear_contents(iBuf);
            }
        public:
            uint32_t glyph_count() const
            {
                return iGlyphCount;
            }
            const hb_glyph_info_t& glyph_info(uint32_t aIndex) const
            {
                return iGlyphInfo[aIndex];
            }
            const hb_glyph_position_t& glyph_position(uint32_t aIndex) const
            {
                return iGlyphPos[aIndex];
            }
            bool needs_fallback_font() const
            {
                for (uint32_t i = 0; i < glyph_count(); ++i)
                {
                    auto const tc = get_text_category(service<i_font_manager>().emoji_atlas(), std::get<0>(iGlyphRun), std::get<1>(iGlyphRun));
                    if (glyph_info(i).codepoint == 0 && tc != text_category::Whitespace && tc != text_category::Emoji)
                        return true;
                }
                return false;
            }
        private:
            const i_graphics_context& iParent;
            hb_font_t* iFont;
            const glyph_text_data::glyph_run& iGlyphRun;
            hb_buffer_t* iBuf;
            uint32_t iGlyphCount;
            hb_glyph_info_t* iGlyphInfo;
            hb_glyph_position_t* iGlyphPos;
        };
        typedef std::list<glyphs> glyphs_list;
        typedef std::vector<std::pair<glyphs_list::const_iterator, uint32_t>> result_type;
    public:
        glyph_shapes(const i_graphics_context& aParent, const font& aFont, const glyph_text_data::glyph_run& aGlyphRun)
        {
            font tryFont = aFont;
            iGlyphsList.emplace_back(glyphs{ aParent, tryFont, aGlyphRun });
            while (iGlyphsList.back().needs_fallback_font())
            {
                if (tryFont.has_fallback())
                {
                    tryFont = tryFont.fallback();
                    iGlyphsList.emplace_back(glyphs{ aParent, tryFont, aGlyphRun });
                }
                else
                {
                    std::u32string lastResort{ std::get<0>(aGlyphRun), std::get<1>(aGlyphRun) };
                    for (uint32_t i = 0; i < iGlyphsList.back().glyph_count(); ++i)
                        if (iGlyphsList.back().glyph_info(i).codepoint == 0)
                            lastResort[iGlyphsList.back().glyph_info(i).cluster] = neolib::INVALID_CHAR32; // replacement character
                    iGlyphsList.emplace_back(glyphs{ aParent, aFont, glyph_text_data::glyph_run{&lastResort[0], &lastResort[0] + lastResort.size(), std::get<2>(aGlyphRun), std::get<3>(aGlyphRun), std::get<4>(aGlyphRun) } });
                    break;
                }
            }
            auto const g = iGlyphsList.begin();
            iResults.reserve(g->glyph_count());
            for (uint32_t i = 0; i < g->glyph_count();)
            {
                const auto& gi = g->glyph_info(i);
                auto tc = get_text_category(service<i_font_manager>().emoji_atlas(), std::get<0>(aGlyphRun) + gi.cluster, std::get<1>(aGlyphRun));
                if (gi.codepoint != 0 || tc == text_category::Whitespace || tc == text_category::Emoji)
                    iResults.push_back(std::make_pair(g, i++));
                 else
                {
                    std::vector<uint32_t> clusters;
                    while (i < g->glyph_count() && g->glyph_info(i).codepoint == 0 && tc != text_category::Whitespace && tc != text_category::Emoji)
                    {
                        clusters.push_back(g->glyph_info(i).cluster);
                        ++i;
                    }
                    std::sort(clusters.begin(), clusters.end());
                    auto nextFallback = std::next(g);
                    while (nextFallback != iGlyphsList.end() && !clusters.empty())
                    {
                        auto currentFallback = nextFallback++;
                        const auto& fallbackGlyphs = *currentFallback;
                        for (uint32_t j = 0; j < fallbackGlyphs.glyph_count(); ++j)
                        {
                            if (fallbackGlyphs.glyph_info(j).codepoint != 0)
                            {
                                auto const c = std::find(clusters.begin(), clusters.end(), fallbackGlyphs.glyph_info(j).cluster);
                                if (c != clusters.end())
                                {
                                    iResults.push_back(std::make_pair(currentFallback, j));
                                    clusters.erase(c);
                                }
                            }
                            else
                            {
                                tc = get_text_category(service<i_font_manager>().emoji_atlas(), std::get<0>(aGlyphRun) + fallbackGlyphs.glyph_info(j).cluster, std::get<1>(aGlyphRun));
                                if (tc != text_category::Whitespace && tc != text_category::Emoji)
                                    break;
                                else
                                    goto whitespace_break;
                            }
                        }
                    }
                }
            whitespace_break:
                ;
            }
        }
    public:
        uint32_t glyph_count() const
        {
            return static_cast<uint32_t>(iResults.size());
        }
        const hb_glyph_info_t& glyph_info(uint32_t aIndex) const
        {
            return iResults[aIndex].first->glyph_info(iResults[aIndex].second);
        }
        const hb_glyph_position_t& glyph_position(uint32_t aIndex) const
        {
            return iResults[aIndex].first->glyph_position(iResults[aIndex].second);
        }
        bool using_fallback(uint32_t aIndex) const
        {
            return iResults[aIndex].first != iGlyphsList.begin();
        }
        uint32_t fallback_index(uint32_t aIndex) const
        {
            if (!using_fallback(aIndex))
                throw not_using_fallback();
            return static_cast<uint32_t>(std::distance(iGlyphsList.begin(), iResults[aIndex].first) - 1);
        }
    private:
        glyphs_list iGlyphsList;
        result_type iResults;
    };

    glyph_text graphics_context::to_glyph_text_impl(std::string::const_iterator aTextBegin, std::string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const
    {
        auto& clusterMap = iGlyphTextData->iClusterMap;
        clusterMap.clear();
        iGlyphTextData->iCodePointsBuffer.clear();
        std::u32string& codePoints = iGlyphTextData->iCodePointsBuffer;

        codePoints = neolib::utf8_to_utf32(aTextBegin, aTextEnd, [&clusterMap](std::string::size_type aFrom, std::u32string::size_type)
        {
            clusterMap.push_back(glyph_text_data::cluster{ aFrom });
        });

        return to_glyph_text_impl(codePoints.begin(), codePoints.end(), [&aFontSelector, &clusterMap](std::u32string::size_type aIndex)->font
        {
            return aFontSelector(clusterMap[aIndex].from);
        });
    }

    glyph_text graphics_context::to_glyph_text_impl(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const
    {
        auto& result = iGlyphTextData->iGlyphTextResult;
        result.clear();

        if (aTextEnd == aTextBegin)
            return result;

        bool hasEmojis = false;

        auto& textDirections = iGlyphTextData->iTextDirections;
        textDirections.clear();

        std::u32string::size_type codePointCount = aTextEnd - aTextBegin;

        std::u32string adjustedCodepoints;
        if (password())
            adjustedCodepoints.assign(codePointCount, neolib::utf8_to_utf32(password_mask())[0]);
        auto codePoints = adjustedCodepoints.empty() ? &*aTextBegin : &adjustedCodepoints[0];

        auto& runs = iGlyphTextData->iRuns;
        runs.clear();
        auto const& emojiAtlas = service<i_font_manager>().emoji_atlas();
        text_category previousCategory = get_text_category(emojiAtlas, codePoints, codePoints + codePointCount);
        if (iMnemonic != std::nullopt && codePoints[0] == static_cast<char32_t>(iMnemonic->second))
            previousCategory = text_category::Mnemonic;
        text_direction previousDirection = (previousCategory != text_category::RTL ? text_direction::LTR : text_direction::RTL);
        const char32_t* runStart = &codePoints[0];
        std::u32string::size_type lastCodePointIndex = codePointCount - 1;
        font previousFont = aFontSelector(0);
        hb_script_t previousScript = hb_unicode_script(static_cast<native_font_face::hb_handle*>(previousFont.native_font_face().aux_handle())->unicodeFuncs, codePoints[0]);

        std::deque<std::pair<text_direction, bool>> directionStack;
        const char32_t LRE = U'\u202A';
        const char32_t RLE = U'\u202B';
        const char32_t LRO = U'\u202D';
        const char32_t RLO = U'\u202E';
        const char32_t PDF = U'\u202C';

        bool currentLineHasLTR = false;

        for (std::size_t codePointIndex = 0; codePointIndex <= lastCodePointIndex; ++codePointIndex)
        {
            font const currentFont = aFontSelector(codePointIndex);
            switch (codePoints[codePointIndex])
            {
            case PDF:
                if (!directionStack.empty())
                    directionStack.pop_back();
                break;
            case LRE:
                directionStack.push_back(std::make_pair(text_direction::LTR, false));
                break;
            case RLE:
                directionStack.push_back(std::make_pair(text_direction::RTL, false));
                break;
            case LRO:
                directionStack.push_back(std::make_pair(text_direction::LTR, true));
                break;
            case RLO:
                directionStack.push_back(std::make_pair(text_direction::RTL, true));
                break;
            default:
                break;
            }

            hb_unicode_funcs_t* unicodeFuncs = static_cast<native_font_face::hb_handle*>(currentFont.native_font_face().aux_handle())->unicodeFuncs;
            text_category currentCategory = get_text_category(emojiAtlas, codePoints + codePointIndex, codePoints + codePointCount);
            if (iMnemonic != std::nullopt && codePoints[codePointIndex] == static_cast<char32_t>(iMnemonic->second))
                currentCategory = text_category::Mnemonic;
            text_direction currentDirection = previousDirection;
            if (currentCategory == text_category::LTR)
                currentDirection = text_direction::LTR;
            else if (currentCategory == text_category::RTL)
                currentDirection = text_direction::RTL;

            bool newLine = (codePoints[codePointIndex] == '\r' || codePoints[codePointIndex] == '\n');
            if (newLine)
            {
                currentLineHasLTR = false;
                currentDirection = text_direction::LTR;
            }
            auto bidi_check = [&directionStack](text_category aCategory, text_direction aDirection)
            {
                if (!directionStack.empty())
                {
                    switch (aCategory)
                    {
                    case text_category::LTR:
                    case text_category::RTL:
                    case text_category::Digit:
                    case text_category::Emoji:
                        if (directionStack.back().second == true)
                            return directionStack.back().first;
                        break;
                    case text_category::Mark:
                    case text_category::None:
                    case text_category::Whitespace:
                    case text_category::Mnemonic:
                        return directionStack.back().first;
                        break;
                    default:
                        break;
                    }
                }
                return aDirection;
            };
            currentDirection = bidi_check(currentCategory, currentDirection);
            switch (currentCategory)
            {
            case text_category::LTR:
                currentLineHasLTR = true;
                if (currentDirection == text_direction::None_LTR || currentDirection == text_direction::None_RTL ||
                    currentDirection == text_direction::Digit_LTR || currentDirection == text_direction::Digit_RTL ||
                    currentDirection == text_direction::Emoji_LTR || currentDirection == text_direction::Emoji_RTL)
                    currentDirection = text_direction::LTR;
                break;
            case text_category::RTL:
                if (currentDirection == text_direction::None_LTR || currentDirection == text_direction::None_RTL ||
                    currentDirection == text_direction::Digit_LTR || currentDirection == text_direction::Digit_RTL ||
                    currentDirection == text_direction::Emoji_LTR || currentDirection == text_direction::Emoji_RTL)
                    currentDirection = text_direction::RTL;
                break;
            case text_category::None:
                if (currentDirection == text_direction::LTR)
                    currentDirection = text_direction::None_LTR;
                else if (currentDirection == text_direction::RTL)
                    currentDirection = text_direction::None_RTL;
                break;
            case text_category::Digit:
                if (currentDirection == text_direction::LTR)
                    currentDirection = text_direction::Digit_LTR;
                else if (currentDirection == text_direction::RTL)
                    currentDirection = text_direction::Digit_RTL;
                break;
            case text_category::Emoji:
                if (currentDirection == text_direction::LTR)
                    currentDirection = text_direction::Emoji_LTR;
                else if (currentDirection == text_direction::RTL)
                    currentDirection = text_direction::Emoji_RTL;
                break;
            }
            if (currentDirection == text_direction::None_LTR || currentDirection == text_direction::Digit_LTR) // optimization (less runs for LTR text)
                currentDirection = text_direction::LTR;
            hb_script_t currentScript = hb_unicode_script(unicodeFuncs, codePoints[codePointIndex]);
            if (currentScript == HB_SCRIPT_COMMON || currentScript == HB_SCRIPT_INHERITED)
                currentScript = previousScript;
            bool newRun =
                previousFont != currentFont ||
                (newLine && (previousDirection == text_direction::RTL || previousDirection == text_direction::None_RTL || previousDirection == text_direction::Digit_RTL || previousDirection == text_direction::Emoji_RTL)) ||
                currentCategory == text_category::Mnemonic ||
                previousCategory == text_category::Mnemonic ||
                previousDirection != currentDirection ||
                previousScript != currentScript;
            if (!newRun)
            {
                if ((currentCategory == text_category::Whitespace || currentCategory == text_category::None || currentCategory == text_category::Mnemonic) &&
                    (currentDirection == text_direction::RTL || currentDirection == text_direction::None_RTL || currentDirection == text_direction::Digit_RTL || currentDirection == text_direction::Emoji_RTL))
                {
                    for (std::size_t j = codePointIndex + 1; j <= lastCodePointIndex; ++j)
                    {
                        text_direction nextDirection = bidi_check(get_text_category(emojiAtlas, codePoints + j, codePoints + codePointCount), get_text_direction(emojiAtlas, codePoints + j, codePoints + codePointCount, currentDirection));
                        if (nextDirection == text_direction::RTL)
                            break;
                        else if (nextDirection == text_direction::LTR || (j == lastCodePointIndex && currentLineHasLTR))
                        {
                            newRun = true;
                            currentDirection = text_direction::LTR;
                            break;
                        }
                    }
                }
            }
            textDirections.push_back(character_type{ currentCategory, currentDirection });
            if (currentCategory == text_category::Emoji)
                hasEmojis = true;
            if (newRun && codePointIndex > 0)
            {
                runs.push_back(std::make_tuple(runStart, &codePoints[codePointIndex], previousDirection, previousCategory == text_category::Mnemonic, previousScript));
                runStart = &codePoints[codePointIndex];
            }
            previousDirection = currentDirection;
            previousCategory = currentCategory;
            previousScript = currentScript;
            if (codePointIndex == lastCodePointIndex)
                runs.push_back(std::make_tuple(runStart, &codePoints[codePointIndex + 1], previousDirection, previousCategory == text_category::Mnemonic, previousScript));
            if (newLine && (newRun || codePointIndex == lastCodePointIndex))
            {
                for (auto i = runs.rbegin(); i != runs.rend(); ++i)
                {
                    if (std::get<2>(*i) == text_direction::RTL)
                        break;
                    else
                    {
                        switch (std::get<2>(*i))
                        {
                        case text_direction::None_RTL:
                            std::get<2>(*i) = text_direction::None_LTR;
                            break;
                        case text_direction::Digit_RTL:
                            std::get<2>(*i) = text_direction::LTR;
                            break;
                        case text_direction::Emoji_RTL:
                            std::get<2>(*i) = text_direction::Emoji_LTR;
                            break;
                        }
                    }
                }
            }
            previousFont = currentFont;
        }

        for (std::size_t i = 1; i < runs.size(); ++i)
        {
            std::size_t j = i - 1;
            auto startDirection = std::get<2>(runs[j]);
            do
            {
                auto direction = std::get<2>(runs[i]);
                if ((startDirection == text_direction::RTL || startDirection == text_direction::None_RTL || startDirection == text_direction::Digit_RTL || startDirection == text_direction::Emoji_RTL) &&
                    (direction == text_direction::RTL || direction == text_direction::None_RTL || direction == text_direction::Digit_RTL || direction == text_direction::Emoji_RTL))
                {
                    auto m = runs[i];
                    runs.erase(runs.begin() + i);
                    runs.insert(runs.begin() + j, m);
                    ++i;
                }
                else
                {
                    break;
                }
            } while (i < runs.size());
        }

        for (std::size_t i = 0; i < runs.size(); ++i)
        {
            if (std::get<3>(runs[i]))
                continue;
            bool drawMnemonic = (i > 0 && std::get<3>(runs[i - 1]));
            std::string::size_type sourceClusterRunStart = std::get<0>(runs[i]) - &codePoints[0];
            glyph_shapes shapes{ *this, aFontSelector(sourceClusterRunStart), runs[i] };
            for (uint32_t j = 0; j < shapes.glyph_count(); ++j)
            {
                std::u32string::size_type startCluster = shapes.glyph_info(j).cluster;
                std::u32string::size_type endCluster;
                if (std::get<2>(runs[i]) != text_direction::RTL)
                {
                    uint32_t k = j + 1;
                    while (k < shapes.glyph_count() && shapes.glyph_info(k).cluster == startCluster)
                        ++k;
                    endCluster = (k < shapes.glyph_count() ? shapes.glyph_info(k).cluster : startCluster + 1);
                }
                else
                {
                    uint32_t k = j;
                    while (k > 0 && shapes.glyph_info(k).cluster == startCluster)
                        --k;
                    endCluster = (shapes.glyph_info(k).cluster != startCluster ? shapes.glyph_info(k).cluster : startCluster + 1);
                }
                startCluster += (std::get<0>(runs[i]) - &codePoints[0]);
                endCluster += (std::get<0>(runs[i]) - &codePoints[0]);
                neogfx::font selectedFont = aFontSelector(startCluster);
                neogfx::font font = selectedFont;
                if (shapes.using_fallback(j))
                {
                    font = font.has_fallback() ? font.fallback() : selectedFont;
                    for (auto fi = shapes.fallback_index(j); font != selectedFont && fi > 0; --fi)
                        font = font.has_fallback() ? font.fallback() : selectedFont;
                }
                if (j > 0 && !result.empty())
                    result.back().kerning_adjust(static_cast<float>(font.kerning(shapes.glyph_info(j - 1).codepoint, shapes.glyph_info(j).codepoint)));
                size advance = textDirections[startCluster].category != text_category::Emoji ?
                    size{ shapes.glyph_position(j).x_advance / 64.0, shapes.glyph_position(j).y_advance / 64.0 } :
                    size{ font.height(), 0.0 };
                result.emplace_back(textDirections[startCluster],
                    shapes.glyph_info(j).codepoint,
                    glyph::source_type{ static_cast<uint32_t>(startCluster), static_cast<uint32_t>(endCluster) },
                    result,
                    font,
                    advance, size(shapes.glyph_position(j).x_offset / 64.0, shapes.glyph_position(j).y_offset / 64.0));
                if (result.back().category() == text_category::Whitespace)
                    result.back().set_value(aTextBegin[startCluster]);
                else if (result.back().category() == text_category::Emoji)
                    result.back().set_value(emojiAtlas.emoji(aTextBegin[startCluster], font.height()));
                if ((aFontSelector(startCluster).style() & font_style::Underline) == font_style::Underline)
                    result.back().set_underline(true);
                if (is_subpixel_rendering_on() && !font.is_bitmap_font())
                    result.back().set_subpixel(true);
                if (drawMnemonic && ((j == 0 && std::get<2>(runs[i]) == text_direction::LTR) || (j == shapes.glyph_count() - 1 && std::get<2>(runs[i]) == text_direction::RTL)))
                    result.back().set_mnemonic(true);
                if (result.back().category() != text_category::Whitespace && result.back().category() != text_category::Emoji)
                {
                    auto& glyph = result.back();
                    if (glyph.advance() != advance.ceil())
                    {
                        const i_glyph_texture& glyphTexture = aFontSelector(startCluster).native_font_face().glyph_texture(glyph);
                        auto visibleAdvance = std::ceil(glyph.offset().cx + glyphTexture.placement().x + glyphTexture.texture().extents().cx);
                        if (visibleAdvance > advance.cx)
                        {
                            advance.cx = visibleAdvance;
                            glyph.set_advance(advance);
                        }
                    }
                }
            }
        }
        if (hasEmojis)
        {
            auto& emojiResult = iGlyphTextData->iGlyphTextResult2;
            emojiResult.clear();
            for (auto i = result.begin(); i != result.end(); ++i)
            {
                auto cluster = i->source().first;
                auto chStart = aTextBegin[cluster];
                if (i->category() == text_category::Emoji)
                {
                    if (!emojiResult.empty() && emojiResult.back().is_emoji() && emojiResult.back().source() == i->source())
                    {
                        // probable variant selector fubar'd by harfbuzz
                        auto s = emojiResult.back().source();
                        if (s.second < codePointCount && get_text_category(service<i_font_manager>().emoji_atlas(), aTextBegin[s.second]) == text_category::Control)
                        {
                            ++s.first;
                            ++s.second;
                            i->set_source(s);
                            i->set_category(text_category::Control);
                            i->set_advance(size{});
                        }
                    }
                    std::u32string sequence;
                    sequence += chStart;
                    auto j = i + 1;
                    bool absorbNext = false;
                    for (; j != result.end(); ++j)
                    {
                        auto ch = aTextBegin[cluster + (j - i)];
                        if (ch == 0x200D)
                            continue;
                        else if (ch == 0xFE0F)
                        {
                            absorbNext = true;
                            break;
                        }
                        else if (service<i_font_manager>().emoji_atlas().is_emoji(sequence + ch))
                            sequence += ch;
                        else
                            break;
                    }
                    if (sequence.size() > 1 && service<i_font_manager>().emoji_atlas().is_emoji(sequence))
                    {
                        auto g = *i;
                        g.set_value(service<i_font_manager>().emoji_atlas().emoji(sequence, aFontSelector(cluster).height()));
                        g.set_source(glyph::source_type{ g.source().first, g.source().first + static_cast<uint32_t>(sequence.size()) });
                        emojiResult.push_back(g);
                        i = j - 1;
                    }
                    else
                        emojiResult.push_back(*i);
                    if (absorbNext)
                    {
                        emojiResult.back().set_source(glyph::source_type{ emojiResult.back().source().first, emojiResult.back().source().first + static_cast<uint32_t>(sequence.size()) + 1u });
                        ++i;
                    }
                }
                else
                    emojiResult.push_back(*i);
            }
            return std::move(emojiResult);
        }
        return std::move(result);
    }
}