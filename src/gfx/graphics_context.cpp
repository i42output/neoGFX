// graphics_context.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present Leigh Johnston
  
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
#include <neogfx/gfx/graphics_context.hpp>
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
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/gfx/i_texture.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gfx/text/text_category_map.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/game/mesh.hpp>
#include <neogfx/game/rectangle.hpp>
#include "native/i_native_graphics_context.hpp"
#include "text/native/native_font_face.hpp"
#include "../hid/native/i_native_surface.hpp"

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
		mutable glyph_text::container iGlyphTextResult;
		mutable glyph_text::container iGlyphTextResult2;
	};

	graphics_context::graphics_context(const i_surface& aSurface, type aType) :
		iSurface{ aSurface },
		iNativeGraphicsContext{ aType == type::Attached ? aSurface.native_surface().create_graphics_context() : nullptr },
		iUnitsContext{ *this },
		iDefaultFont{},
		iOrigin{ 0.0, 0.0 },
		iExtents{ aSurface.extents() },
		iLogicalCoordinateSystem{ iSurface.logical_coordinate_system() },
		iLogicalCoordinates{ iSurface.logical_coordinates() },
		iOpacity{ 1.0 },
		iSmoothingMode{ neogfx::smoothing_mode::None },
		iSubpixelRendering{ iSurface.rendering_engine().is_subpixel_rendering_on() },
		iGlyphTextData{ std::make_unique<glyph_text_data>() }
	{
	}

	graphics_context::graphics_context(const i_surface& aSurface, const font& aDefaultFont, type aType) :
		iSurface{ aSurface },
		iNativeGraphicsContext{ aType == type::Attached ? aSurface.native_surface().create_graphics_context() : nullptr },
		iUnitsContext{ *this },
		iDefaultFont{ aDefaultFont },
		iOrigin{ 0.0, 0.0 },
		iExtents{ aSurface.extents() },
		iLogicalCoordinateSystem{ iSurface.logical_coordinate_system() },
		iLogicalCoordinates{ iSurface.logical_coordinates() },
		iOpacity{ 1.0 },
		iSmoothingMode{ neogfx::smoothing_mode::None },
		iSubpixelRendering{ iSurface.rendering_engine().is_subpixel_rendering_on() },
		iGlyphTextData{ std::make_unique<glyph_text_data>() }
	{
	}

	graphics_context::graphics_context(const i_widget& aWidget, type aType) :
		iSurface{ aWidget.surface() },
		iNativeGraphicsContext{ aType == type::Attached ? aWidget.surface().native_surface().create_graphics_context(aWidget) : nullptr },
		iUnitsContext{ *this },
		iDefaultFont{ aWidget.font() },
		iOrigin{ aWidget.origin() },
		iExtents{ aWidget.extents() },
		iLogicalCoordinateSystem{ aWidget.logical_coordinate_system() },
		iLogicalCoordinates{ iSurface.logical_coordinates() },
		iOpacity{ 1.0 },
		iSmoothingMode{ neogfx::smoothing_mode::None },
		iSubpixelRendering{ iSurface.rendering_engine().is_subpixel_rendering_on() },
		iGlyphTextData{ std::make_unique<glyph_text_data>() }
	{
	}

	graphics_context::graphics_context(const graphics_context& aOther) :
		iSurface{ aOther.iSurface },
		iNativeGraphicsContext{ aOther.iNativeGraphicsContext != nullptr ? aOther.native_context().clone() : nullptr },
		iUnitsContext{ *this },
		iDefaultFont{ aOther.iDefaultFont },
		iOrigin{ aOther.origin() },
		iExtents{ aOther.extents() },
		iLogicalCoordinateSystem{ aOther.logical_coordinate_system() },
		iLogicalCoordinates{ aOther.logical_coordinates() },
		iOpacity{ 1.0 },
		iSmoothingMode{ neogfx::smoothing_mode::None },
		iSubpixelRendering{ iSurface.rendering_engine().is_subpixel_rendering_on() },
		iGlyphTextData{ std::make_unique<glyph_text_data>() }
	{
	}

	graphics_context::~graphics_context()
	{
	}

	const i_surface& graphics_context::surface() const
	{
		return iSurface;
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

	neogfx::logical_coordinate_system graphics_context::logical_coordinate_system() const
	{
		return iLogicalCoordinateSystem;
	}

	void graphics_context::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) const
	{
		if (iLogicalCoordinateSystem != aSystem)
		{
			iLogicalCoordinateSystem = aSystem;
			native_context().enqueue(graphics_operation::set_logical_coordinate_system{ aSystem });
		}
	}

	const std::pair<vec2, vec2>& graphics_context::logical_coordinates() const
	{
		return get_logical_coordinates(surface().extents(), iLogicalCoordinateSystem, iLogicalCoordinates);
	}

	void graphics_context::set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates) const
	{
		if (iLogicalCoordinates != aCoordinates)
		{
			iLogicalCoordinates = aCoordinates;
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
		if (!aFill.empty())
			fill_rect(aRect, aFill);
		native_context().enqueue(graphics_operation::draw_rect{ to_device_units(aRect) + iOrigin, aPen });
	}

	void graphics_context::draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen, const brush& aFill) const
	{
		if (!aFill.empty())
			fill_rounded_rect(aRect, aRadius, aFill);
		native_context().enqueue(graphics_operation::draw_rounded_rect{ to_device_units(aRect) + iOrigin, aRadius, aPen });
	}

	void graphics_context::draw_circle(const point& aCentre, dimension aRadius, const pen& aPen, const brush& aFill, angle aStartAngle) const
	{
		if (!aFill.empty())
			fill_circle(aCentre, aRadius, aFill);
		native_context().enqueue(graphics_operation::draw_circle{ to_device_units(aCentre) + iOrigin, aRadius, aPen, aStartAngle });
	}

	void graphics_context::draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen, const brush& aFill) const
	{
		if (!aFill.empty())
			fill_arc(aCentre, aRadius, aStartAngle, aEndAngle, aFill);
		native_context().enqueue(graphics_operation::draw_arc{ to_device_units(aCentre) + iOrigin, aRadius, aStartAngle, aEndAngle, aPen });
	}

	void graphics_context::draw_path(const path& aPath, const pen& aPen, const brush& aFill) const
	{
		if (!aFill.empty())
			fill_path(aPath, aFill);
		path path = to_device_units(aPath);
		path.set_position(path.position() + iOrigin);
		native_context().enqueue(graphics_operation::draw_path{ path, aPen });
	}

	void graphics_context::draw_shape(const i_shape& aShape, const pen& aPen, const brush& aFill) const
	{
		if (!aFill.empty())
			fill_shape(aShape, aFill);
		vec2 toDeviceUnits = to_device_units(vec2{ 1.0, 1.0 });
		native_context().enqueue(
			graphics_operation::draw_shape{
				mesh{ 
					aShape, 
					mat44{ 
						{ toDeviceUnits.x, 0.0, 0.0, 0.0 },
						{ 0.0, toDeviceUnits.y, 0.0, 0.0 },
						{ 0.0, 0.0, 1.0, 0.0 }, 
						{ iOrigin.x, iOrigin.y, 0.0, 1.0 } } },
				aPen });
	}

	void graphics_context::draw_focus_rect(const rect& aRect) const
	{
		push_logical_operation(neogfx::logical_operation::Xor);
		line_stipple_on(1, 0xAAAA);
		draw_rect(aRect, pen(colour::White, 1.0));
		line_stipple_off();
		pop_logical_operation();
	}

	void graphics_context::fill_rect(const rect& aRect, const brush& aFill) const
	{
		native_context().enqueue(graphics_operation::fill_rect{ to_device_units(aRect) + iOrigin, aFill });
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

	void graphics_context::fill_shape(const i_shape& aShape, const brush& aFill) const
	{
		vec2 toDeviceUnits = to_device_units(vec2{ 1.0, 1.0 });
		native_context().enqueue(
			graphics_operation::fill_shape{
				mesh{ 
					aShape, 
					mat44{ 
						{ toDeviceUnits.x, 0.0, 0.0, 0.0 },
						{ 0.0, toDeviceUnits.y, 0.0, 0.0 },
						{ 0.0, 0.0, 1.0, 0.0 }, 
						{ iOrigin.x, iOrigin.y, 0.0, 1.0 } } },
				aFill });
	}

	size graphics_context::text_extent(const string& aText, const font& aFont, const glyph_text_cache_usage& aCacheUsage) const
	{
		return text_extent(aText.begin(), aText.end(), aFont, aCacheUsage);
	}

	size graphics_context::text_extent(string::const_iterator aTextBegin, string::const_iterator aTextEnd, const font& aFont, const glyph_text_cache_usage& aCacheUsage) const
	{
		const auto& glyphText = aCacheUsage.use && !iGlyphTextCache->empty() ? *iGlyphTextCache : to_glyph_text(aTextBegin, aTextEnd, aFont);
		if (aCacheUsage.use && iGlyphTextCache->empty())
			*iGlyphTextCache = glyphText;
		return from_device_units(size(glyphText.extents().cx, glyphText.extents().cy));
	}

	size graphics_context::multiline_text_extent(const string& aText, const font& aFont, const glyph_text_cache_usage& aCacheUsage) const
	{
		return multiline_text_extent(aText, aFont, 0, aCacheUsage);
	}

	size graphics_context::multiline_text_extent(const string& aText, const font& aFont, dimension aMaxWidth, const glyph_text_cache_usage& aCacheUsage) const
	{
		const auto& glyphText = aCacheUsage.use && !iGlyphTextCache->empty() ? *iGlyphTextCache : to_glyph_text(aText.begin(), aText.end(), aFont);
		if (aCacheUsage.use && iGlyphTextCache->empty())
			*iGlyphTextCache = glyphText;
		typedef std::pair<glyph_text::const_iterator, glyph_text::const_iterator> line_t;
		typedef std::vector<line_t> lines_t;
		lines_t lines;
		std::array<glyph, 2> delimeters = { glyph{ text_category::Whitespace, '\r' }, glyph{ text_category::Whitespace, '\n' } };
		neolib::tokens(glyphText.cbegin(), glyphText.cend(), delimeters.begin(), delimeters.end(), lines, 0, false);
		size result;
		for (lines_t::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			if (aMaxWidth == 0)
			{
				size lineExtent = from_device_units(glyph_text::extents(aFont, i->first, i->second));
				result.cx = std::max(result.cx, lineExtent.cx);
				result.cy += lineExtent.cy;
			}
			else if (i->first != i->second)
			{
				glyph_text::const_iterator next = i->first;
				glyph_text::const_iterator lineStart = next;
				glyph_text::const_iterator lineEnd = i->second;
				dimension maxWidth = to_device_units(size(aMaxWidth, 0)).cx;
				dimension lineWidth = 0;
				bool gotLine = false;
				while (next != i->second)
				{
					if (lineWidth + next->advance().cx > maxWidth)
					{
						std::pair<glyph_text::const_iterator, glyph_text::const_iterator> wordBreak = glyphText.word_break(lineStart, next);
						lineWidth -= glyph_text::extents(aFont, wordBreak.first, next).cx;
						lineEnd = wordBreak.first;
						next = wordBreak.second;
						if (lineEnd == next)
						{
							while (lineEnd != i->second && (lineEnd + 1)->source() == wordBreak.first->source())
								++lineEnd;
							next = lineEnd;
						}
						gotLine = true;
					}
					else
					{
						lineWidth += next->advance().cx;
						++next;
					}
					if (gotLine || next == i->second)
					{
						result.cx = std::max(result.cx, from_device_units(size(lineWidth, 0)).cx);
						result.cy += from_device_units(glyph_text::extents(aFont, i->first, i->second)).cy;
						lineStart = next;
						lineEnd = i->second;
						lineWidth = 0;
						gotLine = false;
					}
				}
			}
			else
				result.cy += aFont.height();
		}
		if (result.cy == 0)
			result.cy = from_device_units(size(0, aFont.height())).cy;
		return result;
	}

	bool graphics_context::is_text_left_to_right(const string& aText, const font& aFont, const glyph_text_cache_usage& aCacheUsage) const
	{
		const auto& glyphText = aCacheUsage.use && !iGlyphTextCache->empty() ? *iGlyphTextCache : to_glyph_text(aText.begin(), aText.end(), aFont);
		if (aCacheUsage.use && iGlyphTextCache->empty())
			*iGlyphTextCache = glyphText;
		return glyph_text_direction(glyphText.cbegin(), glyphText.cend()) == text_direction::LTR;
	}

	bool graphics_context::is_text_right_to_left(const string& aText, const font& aFont, const glyph_text_cache_usage& aCacheUsage) const
	{
		return !is_text_left_to_right(aText, aFont, aCacheUsage);
	}

	void graphics_context::draw_text(const point& aPoint, const string& aText, const font& aFont, const text_appearance& aAppearance, const glyph_text_cache_usage& aCacheUsage) const
	{
		draw_text(aPoint.to_vec3(), aText, aFont, aAppearance, aCacheUsage);
	}

	void graphics_context::draw_text(const point& aPoint, string::const_iterator aTextBegin, string::const_iterator aTextEnd, const font& aFont, const text_appearance& aAppearance, const glyph_text_cache_usage& aCacheUsage) const
	{
		draw_text(aPoint.to_vec3(), aTextBegin, aTextEnd, aFont, aAppearance, aCacheUsage);
	}

	void graphics_context::draw_text(const vec3& aPoint, const string& aText, const font& aFont, const text_appearance& aAppearance, const glyph_text_cache_usage& aCacheUsage) const
	{
		draw_text(aPoint, aText.begin(), aText.end(), aFont, aAppearance, aCacheUsage);
	}

	void graphics_context::draw_text(const vec3& aPoint, string::const_iterator aTextBegin, string::const_iterator aTextEnd, const font& aFont, const text_appearance& aAppearance, const glyph_text_cache_usage& aCacheUsage) const
	{
		const auto& glyphText = aCacheUsage.use && !iGlyphTextCache->empty() ? *iGlyphTextCache : to_glyph_text(aTextBegin, aTextEnd, aFont);
		if (aCacheUsage.use && iGlyphTextCache->empty())
			*iGlyphTextCache = glyphText;
		draw_glyph_text(aPoint, glyphText, aFont, aAppearance);
	}

	void graphics_context::draw_multiline_text(const point& aPoint, const string& aText, const font& aFont, const text_appearance& aAppearance, alignment aAlignment, const glyph_text_cache_usage& aCacheUsage) const
	{
		draw_multiline_text(aPoint.to_vec3(), aText, aFont, aAppearance, aAlignment, aCacheUsage);
	}

	void graphics_context::draw_multiline_text(const point& aPoint, const string& aText, const font& aFont, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment, const glyph_text_cache_usage& aCacheUsage) const
	{
		draw_multiline_text(aPoint.to_vec3(), aText, aFont, aMaxWidth, aAppearance, aAlignment, aCacheUsage);
	}
		
	void graphics_context::draw_multiline_text(const vec3& aPoint, const string& aText, const font& aFont, const text_appearance& aAppearance, alignment aAlignment, const glyph_text_cache_usage& aCacheUsage) const
	{
		draw_multiline_text(aPoint, aText, aFont, 0, aAppearance, aAlignment, aCacheUsage);
	}

	void graphics_context::draw_multiline_text(const vec3& aPoint, const string& aText, const font& aFont, dimension aMaxWidth, const text_appearance& aAppearance, alignment aAlignment, const glyph_text_cache_usage& aCacheUsage) const
	{
		const auto& glyphText = aCacheUsage.use && !iGlyphTextCache->empty() ? *iGlyphTextCache : to_glyph_text(aText.begin(), aText.end(), aFont);
		if (aCacheUsage.use && iGlyphTextCache->empty())
			*iGlyphTextCache = glyphText;
		typedef std::pair<glyph_text::const_iterator, glyph_text::const_iterator> line_t;
		typedef std::vector<line_t> lines_t;
		lines_t lines;
		std::array<glyph, 2> delimeters = { glyph{ text_category::Whitespace, '\r' }, glyph{ text_category::Whitespace, '\n' } };
		neolib::tokens(glyphText.cbegin(), glyphText.cend(), delimeters.begin(), delimeters.end(), lines, 0, false);
		size textExtent = multiline_text_extent(aText, aFont, aMaxWidth, aCacheUsage);
		vec3 pos = aPoint;
		for (lines_t::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			const auto& line = (logical_coordinates().first.y > logical_coordinates().second.y ? *i : *(lines.rbegin() + (i - lines.begin())));
			if (aMaxWidth == 0)
			{
				vec3 linePos = pos;
				size lineExtent = from_device_units(glyph_text::extents(aFont, line.first, line.second));
				if (glyph_text_direction(line.first, line.second) == text_direction::RTL)
					linePos.x += textExtent.cx - lineExtent.cx;
				draw_glyph_text(linePos, line.first, line.second, aFont, aAppearance);
				pos.y += lineExtent.cy;
			}
			else
			{
				glyph_text::const_iterator next = line.first;
				glyph_text::const_iterator lineStart = next;
				glyph_text::const_iterator lineEnd = line.second;
				dimension maxWidth = to_device_units(size(aMaxWidth, 0)).cx;
				dimension lineWidth = 0;
				while (next != line.second)
				{
					bool gotLine = false;
					if (lineWidth + next->advance().cx > maxWidth)
					{
						std::pair<glyph_text::const_iterator, glyph_text::const_iterator> wordBreak = glyphText.word_break(lineStart, next);
						lineWidth -= glyph_text::extents(aFont, wordBreak.first, next).cx;
						lineEnd = wordBreak.first;
						next = wordBreak.second;
						if (lineEnd == next)
						{
							while (lineEnd != line.second && (lineEnd + 1)->source() == wordBreak.first->source())
								++lineEnd;
							next = lineEnd;
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
						vec3 linePos = pos;
						if (aAlignment == alignment::Left && glyph_text_direction(lineStart, next) == text_direction::RTL ||
							aAlignment == alignment::Right && glyph_text_direction(lineStart, next) == text_direction::LTR)
							linePos.x += textExtent.cx - from_device_units(size(lineWidth, 0)).cx;
						else if (aAlignment == alignment::Centre)
							linePos.x += std::ceil((textExtent.cx - from_device_units(size(lineWidth, 0)).cx) / 2);
						draw_glyph_text(linePos, lineStart, lineEnd, aFont, aAppearance);
						pos.y += glyph_text::extents(aFont, lineStart, lineEnd).cy;
						lineStart = next;
						lineEnd = line.second;
						lineWidth = 0;
					}
				}
				if (line.first == line.second)
					pos.y += font().height();
			}
		}
	}

	void graphics_context::draw_glyph_text(const point& aPoint, const glyph_text& aText, const font& aFont, const text_appearance& aAppearance) const
	{
		draw_glyph_text(aPoint.to_vec3(), aText, aFont, aAppearance);
	}

	void graphics_context::draw_glyph_text(const point& aPoint, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const font& aFont, const text_appearance& aAppearance) const
	{
		draw_glyph_text(aPoint.to_vec3(), aTextBegin, aTextEnd, aFont, aAppearance);
	}

	void graphics_context::draw_glyph_text(const vec3& aPoint, const glyph_text& aText, const font& aFont, const text_appearance& aAppearance) const
	{
		draw_glyph_text(aPoint, aText.cbegin(), aText.cend(), aFont, aAppearance);
	}

	void graphics_context::draw_glyph_text(const vec3& aPoint, glyph_text::const_iterator aTextBegin, glyph_text::const_iterator aTextEnd, const font& aFont, const text_appearance& aAppearance) const
	{
		neogfx::draw_glyph_text(*this, aPoint, aTextBegin, aTextEnd, aFont, aAppearance);
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
		return iSurface.horizontal_dpi();
	}

	dimension graphics_context::vertical_dpi() const
	{
		return iSurface.vertical_dpi();
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

	units graphics_context::units() const
	{
		return iUnitsContext.units();
	}

	units graphics_context::set_units(neogfx::units aUnits) const
	{
		return iUnitsContext.set_units(aUnits);
	}

	i_native_graphics_context& graphics_context::native_context() const
	{
		if (iNativeGraphicsContext != nullptr)
			return *iNativeGraphicsContext;
		throw unattached();
	}

	i_native_font_face& graphics_context::to_native_font_face(const font& aFont)
	{
		return aFont.native_font_face();
	}

	// Forward on to native..

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

	void graphics_context::set_opacity(double aOpacity)
	{
		if (iOpacity != aOpacity)
		{
			iOpacity = aOpacity;
			native_context().enqueue(graphics_operation::set_opacity{ aOpacity });
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

	void graphics_context::clear(const colour& aColour) const
	{
		if (origin() == point{} && extents() == iSurface.extents())
			native_context().enqueue(graphics_operation::clear{ aColour });
		else
			fill_rect(rect{ point{}, extents() }, aColour);
	}

	void graphics_context::clear_depth_buffer() const
	{
		native_context().enqueue(graphics_operation::clear_depth_buffer{});
	}

	glyph_text graphics_context::to_glyph_text(const string& aText, const font& aFont) const
	{
		return to_glyph_text(aText.begin(), aText.end(), aFont);
	}

	glyph_text graphics_context::to_glyph_text(string::const_iterator aTextBegin, string::const_iterator aTextEnd, const font& aFont) const
	{
		return to_glyph_text(aTextBegin, aTextEnd, [&aFont](std::string::size_type) { return aFont; });
	}

	glyph_text graphics_context::to_glyph_text(string::const_iterator aTextBegin, string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const
	{
		return glyph_text(aFontSelector(0), to_glyph_text_impl(aTextBegin, aTextEnd, aFontSelector));
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
		return glyph_text(aFontSelector(0), to_glyph_text_impl(aTextBegin, aTextEnd, aFontSelector));
	}

	void graphics_context::draw_glyph(const point& aPoint, const glyph& aGlyph, const font& aFont, const text_appearance& aAppearance) const
	{
		draw_glyph(aPoint.to_vec3(), aGlyph, aFont, aAppearance);
	}

	void graphics_context::draw_glyph(const vec3& aPoint, const glyph& aGlyph, const font& aFont, const text_appearance& aAppearance) const
	{
		try
		{
			if (!aGlyph.is_whitespace())
				native_context().enqueue(graphics_operation::draw_glyph{ (to_device_units(point{aPoint}) + iOrigin).to_vec3() + vec3{0.0, 0.0, aPoint.z}, aGlyph, aFont, aAppearance });
			if (aGlyph.underline() || (mnemonics_shown() && aGlyph.mnemonic()))
				draw_glyph_underline(aPoint, aGlyph, aFont, aAppearance);
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

	void graphics_context::draw_glyph_underline(const point& aPoint, const glyph& aGlyph, const font& aFont, const text_appearance& aAppearance) const
	{
		draw_glyph_underline(aPoint.to_vec3(), aGlyph, aFont, aAppearance);
	}

	void graphics_context::draw_glyph_underline(const vec3& aPoint, const glyph& aGlyph, const font& aFont, const text_appearance& aAppearance) const
	{
		auto yLine = logical_coordinates().first.y > logical_coordinates().second.y ?
			(aFont.height() + aFont.descender()) - std::ceil(aFont.native_font_face().underline_position()) :
			-aFont.descender() + std::ceil(aFont.native_font_face().underline_position());
		const i_glyph_texture& glyphTexture = !aGlyph.use_fallback() ? aFont.native_font_face().glyph_texture(aGlyph) : aGlyph.fallback_font(aFont).native_font_face().glyph_texture(aGlyph);
		draw_line(
			aPoint + vec3{ glyphTexture.placement().x, yLine },
			aPoint + vec3{ glyphTexture.placement().x + glyphTexture.texture().extents().cx, yLine },
			pen{ aAppearance.ink(), std::ceil(aFont.native_font_face().underline_thickness()) });
	}

	void graphics_context::set_glyph_text_cache(glyph_text& aGlyphTextCache) const
	{
		iGlyphTextCache = &aGlyphTextCache;
	}

	void graphics_context::reset_glyph_text_cache() const
	{
		iGlyphTextCache = 0;
	}

	void graphics_context::set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix) const
	{
		iMnemonic = std::make_pair(aShowMnemonics, aMnemonicPrefix);
	}

	void graphics_context::unset_mnemonic() const
	{
		iMnemonic = boost::none;
	}

	bool graphics_context::mnemonics_shown() const
	{
		return iMnemonic != boost::none && iMnemonic->first;
	}

	bool graphics_context::password() const
	{
		return iPassword != boost::none;
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
			iPassword = boost::none;
	}

	void graphics_context::draw_texture(const point& aPoint, const i_texture& aTexture, const optional_colour& aColour, shader_effect aShaderEffect) const
	{
		draw_texture(rect{ aPoint, aTexture.extents() }, aTexture, aColour, aShaderEffect);
	}

	void graphics_context::draw_texture(const rect& aRect, const i_texture& aTexture, const optional_colour& aColour, shader_effect aShaderEffect) const
	{
		vec2 extents = aRect.extents().to_vec2();
		vec2 offset = extents / 2.0;
		vec3 centre = vec3{ aRect.x, aRect.y, 0.0 } + vec3{ offset.x, offset.y, 0.0 };
		draw_texture(rectangle{ centre, extents }, aTexture, aColour, aShaderEffect);
	}

	void graphics_context::draw_texture(const i_shape& aShape, const i_texture& aTexture, const optional_colour& aColour, shader_effect aShaderEffect) const
	{
		draw_texture(aShape, aTexture, rect{ point{ 0.0, 0.0 }, aTexture.extents() }, aColour, aShaderEffect);
	}

	void graphics_context::draw_texture(const point& aPoint, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour, shader_effect aShaderEffect) const
	{
		draw_texture(rect{ aPoint, aTexture.extents() }, aTexture, aTextureRect, aColour, aShaderEffect);
	}

	void graphics_context::draw_texture(const rect& aRect, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour, shader_effect aShaderEffect) const
	{
		vec2 extents = aRect.extents().to_vec2();
		vec2 offset = extents / 2.0;
		vec3 centre = vec3{ aRect.x, aRect.y, 0.0 } + vec3{ offset.x, offset.y, 0.0 };
		draw_texture(rectangle{ centre, extents }, aTexture, aTextureRect, aColour, aShaderEffect);
	}

	void graphics_context::draw_texture(const i_shape& aShape, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour, shader_effect aShaderEffect) const
	{
		draw_textures(aShape, to_texture_list_pointer(aTexture, aTextureRect), aColour, aShaderEffect);
	}

	void graphics_context::draw_textures(const i_shape& aShape, texture_list_pointer aTextures, const optional_colour& aColour, shader_effect aShaderEffect) const
	{
		vec2 toDeviceUnits = to_device_units(vec2{ 1.0, 1.0 });
		neogfx::mesh mesh{
			aShape,
			mat44{
				{ toDeviceUnits.x, 0.0, 0.0, 0.0 },
				{ 0.0, toDeviceUnits.y, 0.0, 0.0 },
				{ 0.0, 0.0, 1.0, 0.0 },
				{ iOrigin.x, iOrigin.y, 0.0, 1.0 } } };
		mesh.set_textures(aTextures);
		native_context().enqueue(graphics_operation::draw_textures{mesh,	aColour, aShaderEffect});	
	}

	class graphics_context::glyph_shapes
	{
	public:
		struct not_using_fallback : std::logic_error { not_using_fallback() : std::logic_error("neogfx::opengl_graphics_context::glyph_shapes::not_using_fallback") {} };
	public:
		class glyphs
		{
		public:
			glyphs(const graphics_context& aParent, const font& aFont, const glyph_text_data::glyph_run& aGlyphRun) :
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
					hb_buffer_add_utf32(iBuf, reinterpret_cast<const uint32_t*>(std::get<0>(aGlyphRun)), std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun), 0, std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun));
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
					hb_buffer_add_utf32(iBuf, &*reversed.begin(), reversed.size(), 0, reversed.size());
				}
				hb_shape(iFont, iBuf, NULL, 0);
				unsigned int glyphCount = 0;
				iGlyphInfo = hb_buffer_get_glyph_infos(iBuf, &glyphCount);
				iGlyphPos = hb_buffer_get_glyph_positions(iBuf, &glyphCount);
				iGlyphCount = glyphCount;
				if (std::get<2>(aGlyphRun) == text_direction::None_RTL)
					for (uint32_t i = 0; i < iGlyphCount; ++i)
						iGlyphInfo[i].cluster = std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun) - 1 - iGlyphInfo[i].cluster;
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
					auto tc = get_text_category(iParent.surface().rendering_engine().font_manager().emoji_atlas(), std::get<0>(iGlyphRun), std::get<1>(iGlyphRun));
					if (glyph_info(i).codepoint == 0 && tc != text_category::Whitespace && tc != text_category::Emoji)
						return true;
				}
				return false;
			}
		private:
			const graphics_context& iParent;
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
		glyph_shapes(const graphics_context& aParent, const font& aFont, const glyph_text_data::glyph_run& aGlyphRun)
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
			auto g = iGlyphsList.begin();
			iResults.reserve(g->glyph_count());
			for (uint32_t i = 0; i < g->glyph_count();)
			{
				const auto& gi = g->glyph_info(i);
				auto tc = get_text_category(aParent.surface().rendering_engine().font_manager().emoji_atlas(), std::get<0>(aGlyphRun) + gi.cluster, std::get<1>(aGlyphRun));
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
								auto c = std::find(clusters.begin(), clusters.end(), fallbackGlyphs.glyph_info(j).cluster);
								if (c != clusters.end())
								{
									iResults.push_back(std::make_pair(currentFallback, j));
									clusters.erase(c);
								}
							}
							else
							{
								tc = get_text_category(aParent.surface().rendering_engine().font_manager().emoji_atlas(), std::get<0>(aGlyphRun) + fallbackGlyphs.glyph_info(j).cluster, std::get<1>(aGlyphRun));
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
			return iResults.size();
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
			return std::distance(iGlyphsList.begin(), iResults[aIndex].first) - 1;
		}
	private:
		glyphs_list iGlyphsList;
		result_type iResults;
	};

	glyph_text::container graphics_context::to_glyph_text_impl(string::const_iterator aTextBegin, string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const
	{
		auto& clusterMap = iGlyphTextData->iClusterMap;
		clusterMap.clear();
		iGlyphTextData->iCodePointsBuffer.clear();
		std::u32string& codePoints = iGlyphTextData->iCodePointsBuffer;

		codePoints = neolib::utf8_to_utf32(aTextBegin, aTextEnd, [&clusterMap](std::string::size_type aFrom, std::u32string::size_type)
		{
			clusterMap.push_back(glyph_text_data::cluster{ aFrom });
		});

		auto result = to_glyph_text_impl(codePoints.begin(), codePoints.end(), [&aFontSelector, &clusterMap](std::u32string::size_type aIndex)->font
		{
			return aFontSelector(clusterMap[aIndex].from);
		});

		return result;
	}

	glyph_text::container graphics_context::to_glyph_text_impl(std::u32string::const_iterator aTextBegin, std::u32string::const_iterator aTextEnd, std::function<font(std::u32string::size_type)> aFontSelector) const
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
		auto const& emojiAtlas = surface().rendering_engine().font_manager().emoji_atlas();
		text_category previousCategory = get_text_category(emojiAtlas, codePoints, codePoints + codePointCount);
		if (iMnemonic != boost::none && codePoints[0] == static_cast<char32_t>(iMnemonic->second))
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
			font currentFont = aFontSelector(codePointIndex);
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
			if (iMnemonic != boost::none && codePoints[codePointIndex] == static_cast<char32_t>(iMnemonic->second))
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
			if (currentScript == HB_SCRIPT_COMMON)
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
			int j = i - 1;
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
				auto const& font = aFontSelector(startCluster);
				if (j > 0 && !result.empty())
					result.back().kerning_adjust(static_cast<float>(font.kerning(shapes.glyph_info(j - 1).codepoint, shapes.glyph_info(j).codepoint)));
				size advance = textDirections[startCluster].category != text_category::Emoji ?
					size{ shapes.glyph_position(j).x_advance / 64.0, shapes.glyph_position(j).y_advance / 64.0 } :
					size{ font.height(), 0.0 };
				result.push_back(glyph(textDirections[startCluster],
					shapes.glyph_info(j).codepoint,
					glyph::source_type(startCluster, endCluster), advance, size(shapes.glyph_position(j).x_offset / 64.0, shapes.glyph_position(j).y_offset / 64.0)));
				if (result.back().category() == text_category::Whitespace)
					result.back().set_value(aTextBegin[startCluster]);
				else if (result.back().category() == text_category::Emoji)
					result.back().set_value(emojiAtlas.emoji(aTextBegin[startCluster], font.height()));
				if ((aFontSelector(startCluster).style() & font::Underline) == font::Underline)
					result.back().set_underline(true);
				if (is_subpixel_rendering_on())
					result.back().set_subpixel(true);
				if (drawMnemonic && ((j == 0 && std::get<2>(runs[i]) == text_direction::LTR) || (j == shapes.glyph_count() - 1 && std::get<2>(runs[i]) == text_direction::RTL)))
					result.back().set_mnemonic(true);
				if (shapes.using_fallback(j))
					result.back().set_use_fallback(true, shapes.fallback_index(j));
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
						if (s.second < codePointCount && get_text_category(surface().rendering_engine().font_manager().emoji_atlas(), aTextBegin[s.second]) == text_category::Control)
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
					for (; j != result.end(); ++j)
					{
						auto prev = aTextBegin[cluster + (j - i) - 1];
						auto ch = aTextBegin[cluster + (j - i)];
						if (ch == 0x200D)
							continue;
						else if (surface().rendering_engine().font_manager().emoji_atlas().is_emoji(sequence + ch) || prev == 0x200D)
							sequence += ch;
						else
							break;
					}
					if (sequence.size() > 1 && surface().rendering_engine().font_manager().emoji_atlas().is_emoji(sequence))
					{
						auto g = *i;
						g.set_value(surface().rendering_engine().font_manager().emoji_atlas().emoji(sequence, aFontSelector(cluster).height()));
						g.set_source(std::make_pair(g.source().first, g.source().first + sequence.size()));
						emojiResult.push_back(g);
						i = j - 1;
					}
				}
				emojiResult.push_back(*i);
			}
			return emojiResult;
		}
		return result;
	}

	scoped_coordinate_system::scoped_coordinate_system(graphics_context& aGc, const point& aOrigin, const size& aExtents, logical_coordinate_system aCoordinateSystem) :
		iGc(aGc), iPreviousCoordinateSystem(aGc.logical_coordinate_system()), iPreviousCoordinates(aGc.logical_coordinates())
	{
		iGc.set_logical_coordinate_system(aCoordinateSystem);
		apply_origin(aOrigin, aExtents);
	}

	scoped_coordinate_system::scoped_coordinate_system(graphics_context& aGc, const point& aOrigin, const size& aExtents, logical_coordinate_system aCoordinateSystem, const std::pair<vec2, vec2>& aCoordinates) :
		iGc(aGc), iPreviousCoordinateSystem(aGc.logical_coordinate_system()), iPreviousCoordinates(aGc.logical_coordinates())
	{
		iGc.set_logical_coordinate_system(aCoordinateSystem);
		iGc.set_logical_coordinates(aCoordinates);
		apply_origin(aOrigin, aExtents);
	}

	scoped_coordinate_system::~scoped_coordinate_system()
	{
		if (iGc.logical_coordinate_system() != iPreviousCoordinateSystem)
			iGc.set_logical_coordinate_system(iPreviousCoordinateSystem);
		if (iGc.logical_coordinates() != iPreviousCoordinates)
			iGc.set_logical_coordinates(iPreviousCoordinates);
	}

	void scoped_coordinate_system::apply_origin(const point& aOrigin, const size& aExtents)
	{
		if (iGc.logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui)
			iGc.set_origin(aOrigin);
		else if (iGc.logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGame)
			iGc.set_origin(point{ aOrigin.x, iGc.surface().extents().cy - (aOrigin.y + aExtents.cy) });
	}

	const std::pair<vec2, vec2>& get_logical_coordinates(const size& aSurfaceSize, logical_coordinate_system aSystem, std::pair<vec2, vec2>& aCoordinates)
	{
		switch (aSystem)
		{
		case neogfx::logical_coordinate_system::Specified:
			break;
		case neogfx::logical_coordinate_system::AutomaticGui:
			aCoordinates.first = vec2{ 0.0, aSurfaceSize.cy };
			aCoordinates.second = vec2{ aSurfaceSize.cx, 0.0 };
			break;
		case neogfx::logical_coordinate_system::AutomaticGame:
			aCoordinates.first = vec2{ 0.0, 0.0 };
			aCoordinates.second = vec2{ aSurfaceSize.cx, aSurfaceSize.cy };
			break;
		}
		return aCoordinates;
	}
}