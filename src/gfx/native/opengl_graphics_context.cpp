// opengl_graphics_context.cpp
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

#include <neogfx/neogfx.hpp>
#include <boost/math/constants/constants.hpp>
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/text/i_glyph_texture.hpp>
#include "../../hid/native/i_native_surface.hpp"
#include "i_native_texture.hpp"
#include "../text/native/i_native_font_face.hpp"
#include "opengl_graphics_context.hpp"
#include "opengl_renderer.hpp" // todo: remove this #include when base class interface abstraction complete

namespace neogfx
{
	namespace 
	{
		inline GLenum path_shape_to_gl_mode(path::shape_type_e aShape)
		{
			switch (aShape)
			{
			case path::Quads:
				return GL_QUADS;
			case path::Lines:
				return GL_LINES;
			case path::LineLoop:
				return GL_LINE_LOOP;
			case path::LineStrip:
				return GL_LINE_STRIP;
			case path::ConvexPolygon:
				return GL_TRIANGLE_FAN;
			default:
				return GL_POINTS;
			}
		}

		inline GLenum path_shape_to_gl_mode(const path& aPath)
		{
			return path_shape_to_gl_mode(aPath.shape());
		}

		inline std::vector<xyz> rect_vertices(const rect& aRect, dimension aPixelAdjust, bool aIncludeCentre)
		{
			std::vector<xyz> result;
			result.reserve(16);
			if (aIncludeCentre) // fill
			{
				result.push_back(xyz{ aRect.centre().x, aRect.centre().y });
				result.push_back(xyz{ aRect.top_left().x, aRect.top_left().y });
				result.push_back(xyz{ aRect.top_right().x, aRect.top_right().y });
				result.push_back(xyz{ aRect.bottom_right().x, aRect.bottom_right().y });
				result.push_back(xyz{ aRect.bottom_left().x, aRect.bottom_left().y });
				result.push_back(xyz{ aRect.top_left().x, aRect.top_left().y });
			}
			else // draw (outline)
			{
				result.push_back(xyz{ aRect.top_left().x, aRect.top_left().y + aPixelAdjust });
				result.push_back(xyz{ aRect.top_right().x, aRect.top_right().y + aPixelAdjust });
				result.push_back(xyz{ aRect.top_right().x - aPixelAdjust, aRect.top_right().y });
				result.push_back(xyz{ aRect.bottom_right().x - aPixelAdjust, aRect.bottom_right().y });
				result.push_back(xyz{ aRect.bottom_right().x, aRect.bottom_right().y - aPixelAdjust });
				result.push_back(xyz{ aRect.bottom_left().x, aRect.bottom_left().y - aPixelAdjust });
				result.push_back(xyz{ aRect.bottom_left().x + aPixelAdjust, aRect.bottom_left().y });
				result.push_back(xyz{ aRect.top_left().x + aPixelAdjust, aRect.top_left().y });
			}
			return result;
		};

		inline std::vector<xyz> arc_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, bool aIncludeCentre)
		{
			std::vector<xyz> result;
			angle arc = (aEndAngle != aStartAngle ? aEndAngle - aStartAngle : boost::math::constants::two_pi<angle>());
			uint32_t segments = static_cast<uint32_t>(std::ceil(std::sqrt(aRadius) * 10.0) * arc / boost::math::constants::two_pi<angle>());
			angle theta = arc / static_cast<angle>(segments);
			result.reserve((segments + (aIncludeCentre ? 2 : 1)) * 2);
			if (aIncludeCentre)
			{
				result.push_back(xyz{ aCentre.x, aCentre.y });
			}
			auto c = std::cos(theta);
			auto s = std::sin(theta);
			auto startCoordinate = mat22{ { std::cos(aStartAngle), std::sin(aStartAngle) },{ -std::sin(aStartAngle), std::cos(aStartAngle) } } *
				vec2{ aRadius, 0.0 };
			coordinate x = startCoordinate.x;
			coordinate y = startCoordinate.y;
			for (uint32_t i = 0; i < segments; ++i)
			{
				result.push_back(xyz{ x + aCentre.x, y + aCentre.y });
				coordinate t = x;
				x = c * x - s * y;
				y = s * t + c * y;
			}
			return result;
		}

		inline std::vector<xyz> circle_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, bool aIncludeCentre)
		{
			auto result = arc_vertices(aCentre, aRadius, aStartAngle, aStartAngle, aIncludeCentre);
			result.push_back(result[aIncludeCentre ? 1 : 0]);
			return result;
		}

		inline std::vector<xyz> rounded_rect_vertices(const rect& aRect, dimension aRadius, bool aIncludeCentre)
		{
			std::vector<xyz> result;
			auto topLeft = arc_vertices(
				aRect.top_left() + point{ aRadius, aRadius },
				aRadius,
				boost::math::constants::pi<coordinate>(),
				boost::math::constants::pi<coordinate>() * 1.5,
				false);
			auto topRight = arc_vertices(
				aRect.top_right() + point{ -aRadius, aRadius },
				aRadius,
				boost::math::constants::pi<coordinate>() * 1.5,
				boost::math::constants::pi<coordinate>() * 2.0,
				false);
			auto bottomRight = arc_vertices(
				aRect.bottom_right() + point{ -aRadius, -aRadius },
				aRadius,
				0.0,
				boost::math::constants::pi<coordinate>() * 0.5,
				false);
			auto bottomLeft = arc_vertices(
				aRect.bottom_left() + point{ aRadius, -aRadius },
				aRadius,
				boost::math::constants::pi<coordinate>() * 0.5,
				boost::math::constants::pi<coordinate>(),
				false);
			result.reserve(topLeft.size() + topRight.size() + bottomRight.size() + bottomLeft.size() + (aIncludeCentre ? 9 : 8));
			if (aIncludeCentre)
			{
				result.push_back(xyz{ aRect.centre().x, aRect.centre().y });
			}
			result.insert(result.end(), xyz{ (aRect.top_left() + point{ 0.0, aRadius }).x, (aRect.top_left() + point{ 0.0, aRadius }).y });
			result.insert(result.end(), topLeft.begin(), topLeft.end());
			result.insert(result.end(), xyz{ (aRect.top_left() + point{ aRadius, 0.0 }).x, (aRect.top_left() + point{ aRadius, 0.0 }).y });
			result.insert(result.end(), xyz{ (aRect.top_right() + point{ -aRadius, 0.0 }).x, (aRect.top_right() + point{ -aRadius, 0.0 }).y });
			result.insert(result.end(), topRight.begin(), topRight.end());
			result.insert(result.end(), xyz{ (aRect.top_right() + point{ 0.0, aRadius }).x, (aRect.top_right() + point{ 0.0, aRadius }).y });
			result.insert(result.end(), xyz{ (aRect.bottom_right() + point{ 0.0, -aRadius }).x, (aRect.bottom_right() + point{ 0.0, -aRadius }).y });
			result.insert(result.end(), bottomRight.begin(), bottomRight.end());
			result.insert(result.end(), xyz{ (aRect.bottom_right() + point{ -aRadius, 0.0 }).x, (aRect.bottom_right() + point{ -aRadius, 0.0 }).y });
			result.insert(result.end(), xyz{ (aRect.bottom_left() + point{ aRadius, 0.0 }).x, (aRect.bottom_left() + point{ aRadius, 0.0 }).y });
			result.insert(result.end(), bottomLeft.begin(), bottomLeft.end());
			result.insert(result.end(), xyz{ (aRect.bottom_left() + point{ 0.0, -aRadius }).x, (aRect.bottom_left() + point{ 0.0, -aRadius }).y });
			result.push_back(result[aIncludeCentre ? 1 : 0]);
			return result;
		}

		inline double pixel_adjust(const dimension aWidth)
		{
			return static_cast<uint32_t>(aWidth) % 2 == 1 ? 0.5 : 0.0;
		}

		inline double pixel_adjust(const pen& aPen)
		{
			return pixel_adjust(aPen.width());
		}

		inline std::vector<xyz> line_loop_to_lines(const std::vector<xyz>& aLineLoop)
		{
			std::vector<xyz> result;
			result.reserve(aLineLoop.size() * 2);
			for (auto v = aLineLoop.begin(); v != aLineLoop.end(); ++v)
			{
				result.push_back(*v);
				if (v != aLineLoop.begin() && v != aLineLoop.end() - 1)
					result.push_back(*v);
			}
			return result;
		}
	}

	opengl_graphics_context::opengl_graphics_context(i_rendering_engine& aRenderingEngine, const i_native_surface& aSurface) :
		iRenderingEngine(aRenderingEngine), 
		iSurface(aSurface), 
		iLogicalCoordinateSystem(aSurface.logical_coordinate_system()),
		iLogicalCoordinates(aSurface.logical_coordinates()), 
		iSmoothingMode(neogfx::smoothing_mode::None),
		iSubpixelRendering(aRenderingEngine.is_subpixel_rendering_on()),
		iClipCounter(0),
		iLineStippleActive(false)
	{
		iRenderingEngine.activate_context(iSurface);
		iRenderingEngine.activate_shader_program(*this, iRenderingEngine.default_shader_program());
		set_smoothing_mode(neogfx::smoothing_mode::AntiAlias);
	}

	opengl_graphics_context::opengl_graphics_context(i_rendering_engine& aRenderingEngine, const i_native_surface& aSurface, const i_widget& aWidget) :
		iRenderingEngine(aRenderingEngine), 
		iSurface(aSurface), 
		iLogicalCoordinateSystem(aWidget.logical_coordinate_system()),
		iLogicalCoordinates(aSurface.logical_coordinates()),
		iSmoothingMode(neogfx::smoothing_mode::None),
		iSubpixelRendering(aRenderingEngine.is_subpixel_rendering_on()),
		iClipCounter(0),
		iLineStippleActive(false)
	{
		iRenderingEngine.activate_context(iSurface);
		iRenderingEngine.activate_shader_program(*this, iRenderingEngine.default_shader_program());
		set_smoothing_mode(neogfx::smoothing_mode::AntiAlias);
	}

	opengl_graphics_context::opengl_graphics_context(const opengl_graphics_context& aOther) :
		iRenderingEngine(aOther.iRenderingEngine), 
		iSurface(aOther.iSurface), 
		iLogicalCoordinateSystem(aOther.iLogicalCoordinateSystem),
		iLogicalCoordinates(aOther.iLogicalCoordinates),
		iSmoothingMode(aOther.iSmoothingMode), 
		iSubpixelRendering(aOther.iSubpixelRendering),
		iClipCounter(0),
		iLineStippleActive(false)
	{
		iRenderingEngine.activate_context(iSurface);
		iRenderingEngine.activate_shader_program(*this, iRenderingEngine.default_shader_program());
		set_smoothing_mode(iSmoothingMode);
	}

	opengl_graphics_context::~opengl_graphics_context()
	{
		flush();
	}

	const i_native_surface& opengl_graphics_context::surface() const
	{
		return iSurface;
	}

	neogfx::logical_coordinate_system opengl_graphics_context::logical_coordinate_system() const
	{
		return iLogicalCoordinateSystem;
	}

	void opengl_graphics_context::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
	{
		iLogicalCoordinateSystem = aSystem;
	}

	const std::pair<vec2, vec2>& opengl_graphics_context::logical_coordinates() const
	{
		switch (iLogicalCoordinateSystem)
		{
		case neogfx::logical_coordinate_system::Specified:
			return iLogicalCoordinates;
		case neogfx::logical_coordinate_system::AutomaticGui:
			return iLogicalCoordinates = std::make_pair<vec2, vec2>({ 0.0, surface().surface_size().cy }, { surface().surface_size().cx, 0.0 });
		case neogfx::logical_coordinate_system::AutomaticGame:
			return iLogicalCoordinates = std::make_pair<vec2, vec2>({ 0.0, 0.0 }, { surface().surface_size().cx, surface().surface_size().cy });
		}
		return iLogicalCoordinates;
	}

	void opengl_graphics_context::set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates) const
	{
		iLogicalCoordinates = aCoordinates;
	}

	void opengl_graphics_context::enqueue(const graphics_operation::operation& aOperation)
	{
		if (!iQueue.empty() && graphics_operation::batchable(iQueue.back().back(), aOperation))
			iQueue.back().push_back(aOperation);
		else
			iQueue.push_back(graphics_operation::batch{ {aOperation} });
	}

	void opengl_graphics_context::flush()
	{
		while (!iQueue.empty())
		{
			const auto& opBatch = iQueue.front();
			switch (opBatch.front().which())
			{
			case graphics_operation::operation_type::SetLogicalCoordinateSystem:
				for (auto& op : opBatch)
					set_logical_coordinate_system(static_variant_cast<const graphics_operation::set_logical_coordinate_system&>(op).system);
				break;
			case graphics_operation::operation_type::SetLogicalCoordinates:
				for (auto& op : opBatch)
					set_logical_coordinates(static_variant_cast<const graphics_operation::set_logical_coordinates&>(op).coordinates);
				break;
			case graphics_operation::operation_type::ScissorOn:
				for (auto& op : opBatch)
					scissor_on(static_variant_cast<const graphics_operation::scissor_on&>(op).rect);
				break;
			case graphics_operation::operation_type::ScissorOff:
				for (auto& op : opBatch)
				{
					(void)op;
					scissor_off();
				}
				break;
			case graphics_operation::operation_type::ClipToRect:
				for (auto& op : opBatch)
					clip_to(static_variant_cast<const graphics_operation::clip_to_rect&>(op).rect);
				break;
			case graphics_operation::operation_type::ClipToPath:
				for (auto& op : opBatch)
					clip_to(static_variant_cast<const graphics_operation::clip_to_path&>(op).path, static_variant_cast<const graphics_operation::clip_to_path&>(op).pathOutline);
				break;
			case graphics_operation::operation_type::ResetClip:
				for (auto& op : opBatch)
				{
					(void)op;
					reset_clip();
				}
				break;
			case graphics_operation::operation_type::SetSmoothingMode:
				for (auto& op : opBatch)
					set_smoothing_mode(static_variant_cast<const graphics_operation::set_smoothing_mode&>(op).smoothingMode);
				break;
			case graphics_operation::operation_type::PushLogicalOperation:
				for (auto& op : opBatch)
					push_logical_operation(static_variant_cast<const graphics_operation::push_logical_operation&>(op).logicalOperation);
				break;
			case graphics_operation::operation_type::PopLogicalOperation:
				for (auto& op : opBatch)
				{
					(void)op;
					pop_logical_operation();
				}
				break;
			case graphics_operation::operation_type::LineStippleOn:
				for (auto& op : opBatch)
					line_stipple_on(static_variant_cast<const graphics_operation::line_stipple_on&>(op).factor, static_variant_cast<const graphics_operation::line_stipple_on&>(op).pattern);
				break;
			case graphics_operation::operation_type::LineStippleOff:
				for (auto& op : opBatch)
				{
					(void)op;
					line_stipple_off();
				}
				break;
			case graphics_operation::operation_type::SubpixelRenderingOn:
				subpixel_rendering_on();
				break;
			case graphics_operation::operation_type::SubpixelRenderingOff:
				subpixel_rendering_off();
				break;
			case graphics_operation::operation_type::Clear:
				for (auto& op : opBatch)
					clear(static_variant_cast<const graphics_operation::clear&>(op).colour);
				break;
			case graphics_operation::operation_type::SetPixel:
				for (auto& op : opBatch)
					set_pixel(static_variant_cast<const graphics_operation::set_pixel&>(op).point, static_variant_cast<const graphics_operation::set_pixel&>(op).colour);
				break;
			case graphics_operation::operation_type::DrawPixel:
				for (auto& op : opBatch)
					draw_pixel(static_variant_cast<const graphics_operation::draw_pixel&>(op).point, static_variant_cast<const graphics_operation::draw_pixel&>(op).colour);
				break;
			case graphics_operation::operation_type::DrawLine:
				for (auto& op : opBatch)
				{
					const auto& args = static_cast<const graphics_operation::draw_line&>(op);
					draw_line(args.from, args.to, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawRect:
				for (auto& op : opBatch)
				{
					const auto& args = static_cast<const graphics_operation::draw_rect&>(op);
					draw_rect(args.rect, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawRoundedRect:
				for (auto& op : opBatch)
				{
					const auto& args = static_cast<const graphics_operation::draw_rounded_rect&>(op);
					draw_rounded_rect(args.rect, args.radius, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawCircle:
				for (auto& op : opBatch)
				{
					const auto& args = static_cast<const graphics_operation::draw_circle&>(op);
					draw_circle(args.centre, args.radius, args.pen, args.startAngle);
				}
				break;
			case graphics_operation::operation_type::DrawArc:
				for (auto& op : opBatch)
				{
					const auto& args = static_cast<const graphics_operation::draw_arc&>(op);
					draw_arc(args.centre, args.radius, args.startAngle, args.endAngle, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawPath:
				for (auto& op : opBatch)
				{
					const auto& args = static_cast<const graphics_operation::draw_path&>(op);
					draw_path(args.path, args.pen);
				}
				break;
			case graphics_operation::operation_type::DrawShape:
				for (auto& op : opBatch)
				{
					const auto& args = static_cast<const graphics_operation::draw_shape&>(op);
					draw_shape(args.vertices, args.pen);
				}
				break;
			case graphics_operation::operation_type::FillRect:
				for (auto& op : opBatch)
					fill_rect(static_cast<const graphics_operation::fill_rect&>(op).rect, static_cast<const graphics_operation::fill_rect&>(op).fill);
				break;
			case graphics_operation::operation_type::FillRoundedRect:
				for (auto& op : opBatch)
				{
					const auto& args = static_cast<const graphics_operation::fill_rounded_rect&>(op);
					fill_rounded_rect(args.rect, args.radius, args.fill);
				}
				break;
			case graphics_operation::operation_type::FillCircle:
				for (auto& op : opBatch)
				{
					const auto& args = static_cast<const graphics_operation::fill_circle&>(op);
					fill_circle(args.centre, args.radius, args.fill);
				}
				break;
			case graphics_operation::operation_type::FillArc:
				for (auto& op : opBatch)
				{
					const auto& args = static_cast<const graphics_operation::fill_arc&>(op);
					fill_arc(args.centre, args.radius, args.startAngle, args.endAngle, args.fill);
				}
				break;
			case graphics_operation::operation_type::FillPath:
				for (auto& op : opBatch)
					fill_path(static_variant_cast<const graphics_operation::fill_path&>(op).path, static_variant_cast<const graphics_operation::fill_path&>(op).fill);
				break;
			case graphics_operation::operation_type::FillShape:
				for (auto& op : opBatch)
					fill_shape(static_variant_cast<const graphics_operation::fill_shape&>(op).vertices, static_variant_cast<const graphics_operation::fill_shape&>(op).fill);
				break;
			case graphics_operation::operation_type::DrawGlyph:
				begin_drawing_glyphs();
				for (auto& op : opBatch)
				{
					const auto& args = static_variant_cast<const graphics_operation::draw_glyph&>(op);
					draw_glyph(args.point, args.glyph, args.font, args.colour);
				}
				end_drawing_glyphs();
				break;
			case graphics_operation::operation_type::DrawTexture:
				for (auto& op : opBatch)
				{
					const auto& args = static_variant_cast<const graphics_operation::draw_texture&>(op);
					draw_texture(args.textureMap, args.texture, args.textureRect, args.colour, args.shaderEffect);
				}
				break;
			}
			iQueue.pop_front();
		}
	}

	void opengl_graphics_context::scissor_on(const rect& aRect)
	{
		if (iScissorRects.empty())
		{
			glCheck(glEnable(GL_SCISSOR_TEST));
		}
		iScissorRects.push_back(aRect);
		apply_scissor();
	}

	void opengl_graphics_context::scissor_off()
	{
		iScissorRects.pop_back();
		if (iScissorRects.empty())
		{
			glCheck(glDisable(GL_SCISSOR_TEST));
		}
		else
			apply_scissor();
	}

	optional_rect opengl_graphics_context::scissor_rect() const
	{
		if (iScissorRects.empty())
			return optional_rect();
		rect result = *iScissorRects.begin();
		for (auto& r : iScissorRects)
			result = result.intersection(r);
		return result;
	}

	void opengl_graphics_context::apply_scissor()
	{
		auto sr = *scissor_rect();
		GLint x = static_cast<GLint>(std::ceil(sr.x));
		GLint y = static_cast<GLint>(std::ceil(rendering_area(false).cy - sr.cy - sr.y));
		GLsizei cx = static_cast<GLsizei>(std::ceil(sr.cx));
		GLsizei cy = static_cast<GLsizei>(std::ceil(sr.cy));
		glCheck(glScissor(x, y, cx, cy));
	}

	void opengl_graphics_context::clip_to(const rect& aRect)
	{
		if (iClipCounter++ == 0)
		{
			glCheck(glClear(GL_STENCIL_BUFFER_BIT));
			glCheck(glEnable(GL_STENCIL_TEST));
		}
		glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
		glCheck(glDepthMask(GL_FALSE));
		glCheck(glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP));  // draw 1s on test fail (always)
		glCheck(glStencilMask(static_cast<GLuint>(-1)));
		glCheck(glStencilFunc(GL_NEVER, 0, static_cast<GLuint>(-1)));
		fill_rect(rendering_area(), colour::White);
		glCheck(glStencilFunc(GL_NEVER, 1, static_cast<GLuint>(-1)));
		fill_rect(aRect, colour::White);
		glCheck(glStencilFunc(GL_NEVER, 1, static_cast<GLuint>(-1)));
		glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		glCheck(glDepthMask(GL_TRUE));
		glCheck(glStencilMask(0x00));
		// draw only where stencil's value is 1
		glCheck(glStencilFunc(GL_EQUAL, 1, static_cast<GLuint>(-1)));
	}

	void opengl_graphics_context::clip_to(const path& aPath, dimension aPathOutline)
	{
		if (iClipCounter++ == 0)
		{
			glCheck(glClear(GL_STENCIL_BUFFER_BIT));
			glCheck(glEnable(GL_STENCIL_TEST));
		}
		glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
		glCheck(glDepthMask(GL_FALSE));
		glCheck(glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP));  // draw 1s on test fail (always)
		glCheck(glStencilMask(static_cast<GLuint>(-1)));
		glCheck(glStencilFunc(GL_NEVER, 0, static_cast<GLuint>(-1)));
		fill_rect(rendering_area(), colour::White);
		glCheck(glStencilFunc(GL_EQUAL, 1, static_cast<GLuint>(-1)));
		for (std::size_t i = 0; i < aPath.paths().size(); ++i)
		{
			if (aPath.paths()[i].size() > 2)
			{
				iVertexArrays.vertices() = aPath.to_vertices(aPath.paths()[i]);
				iVertexArrays.colours().assign(iVertexArrays.vertices().size(), std::array <uint8_t, 4>{ {0xFF, 0xFF, 0xFF, 0xFF}});
				iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
				iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

				glCheck(glDrawArrays(path_shape_to_gl_mode(aPath), 0, iVertexArrays.vertices().size()));
			}
		}
		if (aPathOutline != 0)
		{
			glCheck(glStencilFunc(GL_NEVER, 0, static_cast<GLuint>(-1)));
			path innerPath = aPath;
			innerPath.deflate(aPathOutline);
			for (std::size_t i = 0; i < innerPath.paths().size(); ++i)
			{
				if (innerPath.paths()[i].size() > 2)
				{
					iVertexArrays.vertices() = aPath.to_vertices(innerPath.paths()[i]);
					iVertexArrays.colours().assign(iVertexArrays.vertices().size(), std::array <uint8_t, 4>{ {0xFF, 0xFF, 0xFF, 0xFF}});
					iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
					iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

					glCheck(glDrawArrays(path_shape_to_gl_mode(innerPath), 0, iVertexArrays.vertices().size()));
				}
			}
		}
		glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		glCheck(glDepthMask(GL_TRUE));
		glCheck(glStencilMask(0x00));
		// draw only where stencil's value is 1
		glCheck(glStencilFunc(GL_EQUAL, 1, static_cast<GLuint>(-1)));
	}

	void opengl_graphics_context::reset_clip()
	{
		if (--iClipCounter == 0)
		{
			glCheck(glDisable(GL_STENCIL_TEST));
		}
	}

	smoothing_mode opengl_graphics_context::smoothing_mode() const
	{
		return iSmoothingMode;
	}

	void opengl_graphics_context::set_smoothing_mode(neogfx::smoothing_mode aSmoothingMode)
	{
		iSmoothingMode = aSmoothingMode;
		if (iSmoothingMode == neogfx::smoothing_mode::AntiAlias)
		{
			glCheck(glEnable(GL_LINE_SMOOTH));
			glCheck(glEnable(GL_POLYGON_SMOOTH));
		}
		else
		{
			glCheck(glDisable(GL_LINE_SMOOTH));
			glCheck(glDisable(GL_POLYGON_SMOOTH));
		}
	}

	void opengl_graphics_context::push_logical_operation(logical_operation aLogicalOperation)
	{
		iLogicalOperationStack.push_back(aLogicalOperation);
		apply_logical_operation();
	}

	void opengl_graphics_context::pop_logical_operation()
	{
		if (!iLogicalOperationStack.empty())
			iLogicalOperationStack.pop_back();
		apply_logical_operation();
	}

	void opengl_graphics_context::apply_logical_operation()
	{
		if (iLogicalOperationStack.empty() || iLogicalOperationStack.back() == logical_operation::None)
		{
			glCheck(glDisable(GL_COLOR_LOGIC_OP));
		}
		else
		{
			glCheck(glEnable(GL_COLOR_LOGIC_OP));
			switch (iLogicalOperationStack.back())
			{
			case logical_operation::Xor:
				glCheck(glLogicOp(GL_XOR));
				break;
			}
		}	
	}

	void opengl_graphics_context::gradient_on(const gradient& aGradient, const rect& aBoundingBox)
	{
		basic_rect<float> boundingBox{ aBoundingBox };
		iShaderProgramStack.emplace_back(*this, iRenderingEngine, iRenderingEngine.gradient_shader_program());
		iRenderingEngine.gradient_shader_program().set_uniform_variable("posViewportTop", static_cast<float>(logical_coordinates().first.y));
		iRenderingEngine.gradient_shader_program().set_uniform_variable("posTopLeft", boundingBox.top_left().x, boundingBox.top_left().y);
		iRenderingEngine.gradient_shader_program().set_uniform_variable("posBottomRight", boundingBox.bottom_right().x, boundingBox.bottom_right().y);
		iRenderingEngine.gradient_shader_program().set_uniform_variable("nGradientDirection", static_cast<int>(aGradient.direction()));
		iRenderingEngine.gradient_shader_program().set_uniform_variable("radGradientAngle", aGradient.orientation().is<double>() ? static_cast<float>(static_variant_cast<double>(aGradient.orientation())) : 0.0f);
		iRenderingEngine.gradient_shader_program().set_uniform_variable("nGradientStartFrom", aGradient.orientation().is<gradient::corner_e>() ? static_cast<int>(static_variant_cast<gradient::corner_e>(aGradient.orientation())) : -1);
		iRenderingEngine.gradient_shader_program().set_uniform_variable("nGradientSize", static_cast<int>(aGradient.size()));
		iRenderingEngine.gradient_shader_program().set_uniform_variable("nGradientShape", static_cast<int>(aGradient.shape()));
		basic_point<float> gradientCentre = (aGradient.centre() != boost::none ? *aGradient.centre() : point{});
		iRenderingEngine.gradient_shader_program().set_uniform_variable("posGradientCentre", gradientCentre.x, gradientCentre.y);
		auto combinedStops = aGradient.combined_stops();
		iGradientStopPositions.reserve(combinedStops.size());
		iGradientStopColours.reserve(combinedStops.size());
		iGradientStopPositions.clear();
		iGradientStopColours.clear();
		for (const auto& stop : combinedStops)
		{
			iGradientStopPositions.push_back(static_cast<float>(stop.first));
			iGradientStopColours.push_back(std::array<float, 4>{ {stop.second.red<float>(), stop.second.green<float>(), stop.second.blue<float>(), stop.second.alpha<float>()}});
		}
		iRenderingEngine.gradient_shader_program().set_uniform_variable("nStopCount", static_cast<int>(iGradientStopPositions.size()));
		iRenderingEngine.gradient_shader_program().set_uniform_variable("nFilterSize", static_cast<int>(opengl_renderer::GRADIENT_FILTER_SIZE));
		auto filter = gaussian_filter<float, opengl_renderer::GRADIENT_FILTER_SIZE>(static_cast<float>(aGradient.smoothness() * 10.0));
		// todo: remove the following cast when gradient textures abstracted in rendering engine base class interface
		auto& gradientTextures = static_cast<opengl_renderer&>(iRenderingEngine).gradient_textures(); 
		glCheck(glActiveTexture(GL_TEXTURE2));
		glCheck(glClientActiveTexture(GL_TEXTURE2));
		glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, gradientTextures[0]));
		glCheck(glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, iGradientStopPositions.size(), 1, GL_RED, GL_FLOAT, &iGradientStopPositions[0]));
		glCheck(glActiveTexture(GL_TEXTURE3));
		glCheck(glClientActiveTexture(GL_TEXTURE3));
		glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, gradientTextures[1]));
		glCheck(glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, iGradientStopColours.size(), 1, GL_RGBA, GL_FLOAT, &iGradientStopColours[0]));
		glCheck(glActiveTexture(GL_TEXTURE4));
		glCheck(glClientActiveTexture(GL_TEXTURE4));
		glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, gradientTextures[2]));
		glCheck(glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, opengl_renderer::GRADIENT_FILTER_SIZE, opengl_renderer::GRADIENT_FILTER_SIZE, GL_RED, GL_FLOAT, &filter[0][0]));
		iRenderingEngine.gradient_shader_program().set_uniform_variable("texStopPositions", 2);
		iRenderingEngine.gradient_shader_program().set_uniform_variable("texStopColours", 3);
		iRenderingEngine.gradient_shader_program().set_uniform_variable("texFilter", 4);
		glCheck(glActiveTexture(GL_TEXTURE1));
		glCheck(glClientActiveTexture(GL_TEXTURE1));
	}

	void opengl_graphics_context::gradient_off()
	{
		iShaderProgramStack.pop_back();
		glCheck(glDisable(GL_TEXTURE_RECTANGLE));
	}

	void opengl_graphics_context::line_stipple_on(uint32_t aFactor, uint16_t aPattern)
	{
		glCheck(glEnable(GL_LINE_STIPPLE));
		glCheck(glLineStipple(static_cast<GLint>(aFactor), static_cast<GLushort>(aPattern)));
		iLineStippleActive = true;
	}

	void opengl_graphics_context::line_stipple_off()
	{
		glCheck(glDisable(GL_LINE_STIPPLE));
		iLineStippleActive = false;
	}

	bool opengl_graphics_context::is_subpixel_rendering_on() const
	{
		return iSubpixelRendering;
	}

	void opengl_graphics_context::subpixel_rendering_on()
	{
		iSubpixelRendering = true;
	}

	void opengl_graphics_context::subpixel_rendering_off()
	{
		iSubpixelRendering = false;
	}

	void opengl_graphics_context::clear(const colour& aColour)
	{
		disable_anti_alias daa(*this);
		fill_rect(rendering_area(), aColour);
	}

	void opengl_graphics_context::set_pixel(const point& aPoint, const colour& aColour)
	{
		/* todo: faster alternative to this... */
		disable_anti_alias daa(*this);
		draw_pixel(aPoint, aColour.with_alpha(0xFF));
	}

	void opengl_graphics_context::draw_pixel(const point& aPoint, const colour& aColour)
	{
		/* todo: faster alternative to this... */
		fill_rect(rect{ aPoint, size{1.0, 1.0} }, aColour);
	}

	void opengl_graphics_context::draw_line(const point& aFrom, const point& aTo, const pen& aPen)
	{
		double pixelAdjust = pixel_adjust(aPen);
		iVertexArrays.vertices().assign({ xyz{aFrom.x + pixelAdjust, aFrom.y + pixelAdjust}, xyz{aTo.x + pixelAdjust, aTo.y + pixelAdjust} });
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), std::array <uint8_t, 4>{{aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glDrawArrays(GL_LINES, 0, iVertexArrays.vertices().size()));
		glCheck(glLineWidth(1.0f));
	}

	void opengl_graphics_context::draw_rect(const rect& aRect, const pen& aPen)
	{
		iVertexArrays.vertices() = rect_vertices(aRect, pixel_adjust(aPen), false);
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), std::array <uint8_t, 4>{ {aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glDrawArrays(GL_LINES, 0, iVertexArrays.vertices().size()));
		glCheck(glLineWidth(1.0f));
	}

	void opengl_graphics_context::draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen)
	{
		double pixelAdjust = pixel_adjust(aPen);
		iVertexArrays.vertices() = rounded_rect_vertices(aRect + point{ pixelAdjust, pixelAdjust }, aRadius, false);
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), std::array <uint8_t, 4>{ {aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glDrawArrays(GL_LINE_LOOP, 0, iVertexArrays.vertices().size()));
		glCheck(glLineWidth(1.0f));
	}

	void opengl_graphics_context::draw_circle(const point& aCentre, dimension aRadius, const pen& aPen, angle aStartAngle)
	{
		iVertexArrays.vertices() = circle_vertices(aCentre, aRadius, aStartAngle, false);
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), std::array <uint8_t, 4>{{aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});

		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glDrawArrays(GL_LINE_LOOP, 0, iVertexArrays.vertices().size()));
		glCheck(glLineWidth(1.0f));
	}

	void opengl_graphics_context::draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen)
	{
		iVertexArrays.vertices() = line_loop_to_lines(arc_vertices(aCentre, aRadius, aStartAngle, aEndAngle, false));
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), std::array <uint8_t, 4>{ {aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glDrawArrays(GL_LINES, 0, iVertexArrays.vertices().size()));
		glCheck(glLineWidth(1.0f));
	}

	void opengl_graphics_context::draw_path(const path& aPath, const pen& aPen)
	{
		for (std::size_t i = 0; i < aPath.paths().size(); ++i)
		{
			if (aPath.paths()[i].size() > 2)
			{
				if (aPath.shape() == path::ConvexPolygon)
					clip_to(aPath, aPen.width());

				iVertexArrays.vertices() = aPath.to_vertices(aPath.paths()[i]);
				iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
				iVertexArrays.colours().assign(iVertexArrays.vertices().size(), std::array <uint8_t, 4>{{aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
				iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

				glCheck(glDrawArrays(path_shape_to_gl_mode(aPath.shape()), 0, iVertexArrays.vertices().size()));
				if (aPath.shape() == path::ConvexPolygon)
					reset_clip();
			}
		}
	}

	void opengl_graphics_context::draw_shape(const vec2_list& aVertices, const pen& aPen)
	{
		iVertexArrays.vertices().clear();
		for (auto const& v : aVertices)
			iVertexArrays.vertices().push_back(xyz{ v[0], v[1] });
		iVertexArrays.vertices().push_back(iVertexArrays.vertices()[0]);
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(),
			std::array <uint8_t, 4>{ { aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_LINE_LOOP, 0, iVertexArrays.vertices().size()));
	}

	void opengl_graphics_context::fill_rect(const rect& aRect, const fill& aFill)
	{
		if (aRect.empty())
			return;
		if (aFill.is<gradient>())
			gradient_on(static_variant_cast<const gradient&>(aFill), aRect);

		iVertexArrays.vertices() = rect_vertices(aRect, 0.0, true);
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), aFill.is<colour>() ?
			std::array <uint8_t, 4>{{
					static_variant_cast<const colour&>(aFill).red(), 
					static_variant_cast<const colour&>(aFill).green(), 
					static_variant_cast<const colour&>(aFill).blue(),
					static_variant_cast<const colour&>(aFill).alpha()}} : 
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, iVertexArrays.vertices().size()));
		if (aFill.is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::fill_rounded_rect(const rect& aRect, dimension aRadius, const fill& aFill)
	{
		if (aRect.empty())
			return;
		if (aFill.is<gradient>())
			gradient_on(static_variant_cast<const gradient&>(aFill), aRect);

		iVertexArrays.vertices() = rounded_rect_vertices(aRect, aRadius, true);
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), aFill.is<colour>() ?
			std::array <uint8_t, 4>{ {
					static_variant_cast<const colour&>(aFill).red(),
						static_variant_cast<const colour&>(aFill).green(),
						static_variant_cast<const colour&>(aFill).blue(),
						static_variant_cast<const colour&>(aFill).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, iVertexArrays.vertices().size()));
		if (aFill.is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::fill_circle(const point& aCentre, dimension aRadius, const fill& aFill)
	{
		if (aFill.is<gradient>())
			gradient_on(static_variant_cast<const gradient&>(aFill), rect{ aCentre - point{ aRadius, aRadius }, size{ aRadius * 2.0 } });

		iVertexArrays.vertices() = circle_vertices(aCentre, aRadius, 0.0, true);
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), aFill.is<colour>() ?
			std::array <uint8_t, 4>{ {
					static_variant_cast<const colour&>(aFill).red(),
						static_variant_cast<const colour&>(aFill).green(),
						static_variant_cast<const colour&>(aFill).blue(),
						static_variant_cast<const colour&>(aFill).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, iVertexArrays.vertices().size()));
		if (aFill.is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::fill_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const fill& aFill)
	{
		if (aFill.is<gradient>())
			gradient_on(static_variant_cast<const gradient&>(aFill), rect{ aCentre - point{ aRadius, aRadius }, size{ aRadius * 2.0 } });
		
		iVertexArrays.vertices() = arc_vertices(aCentre, aRadius, aStartAngle, aEndAngle, true);
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), aFill.is<colour>() ?
			std::array <uint8_t, 4>{ {
					static_variant_cast<const colour&>(aFill).red(),
						static_variant_cast<const colour&>(aFill).green(),
						static_variant_cast<const colour&>(aFill).blue(),
						static_variant_cast<const colour&>(aFill).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, iVertexArrays.vertices().size()));
		if (aFill.is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::fill_path(const path& aPath, const fill& aFill)
	{
		for (std::size_t i = 0; i < aPath.paths().size(); ++i)
		{
			if (aPath.paths()[i].size() > 2)
			{
				clip_to(aPath, 0.0);
				point min = aPath.paths()[i][0];
				point max = min;
				for (auto const& pt : aPath.paths()[i])
				{
					min.x = std::min(min.x, pt.x);
					max.x = std::max(max.x, pt.x);
					min.y = std::min(min.y, pt.y);
					max.y = std::max(max.y, pt.y);
				}
				if (aFill.is<gradient>())
					gradient_on(static_variant_cast<const gradient&>(aFill), rect{ point{ min.x, min.y }, size{ max.x - min.y, max.y - min.y } });

				iVertexArrays.vertices() = aPath.to_vertices(aPath.paths()[i]);
				iVertexArrays.colours().assign(iVertexArrays.vertices().size(), aFill.is<colour>() ?
					std::array <uint8_t, 4>{ {
							static_variant_cast<const colour&>(aFill).red(),
								static_variant_cast<const colour&>(aFill).green(),
								static_variant_cast<const colour&>(aFill).blue(),
								static_variant_cast<const colour&>(aFill).alpha()}} :
					std::array <uint8_t, 4>{});
				iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
				iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

				glCheck(glDrawArrays(path_shape_to_gl_mode(aPath.shape()), 0, iVertexArrays.vertices().size()));
				reset_clip();
				if (aFill.is<gradient>())
					gradient_off();
			}
		}
	}

	void opengl_graphics_context::fill_shape(const vec2_list& aVertices, const fill& aFill)
	{
		vec2 min = aVertices[0];
		vec2 max = min;
		for (auto const& v : aVertices)
		{
			min.x = std::min(min.x, v.x);
			max.x = std::max(max.x, v.x);
			min.y = std::min(min.y, v.y);
			max.y = std::max(max.y, v.y);
		}
		if (aFill.is<gradient>())
			gradient_on(static_variant_cast<const gradient&>(aFill), 
				rect{ 
					point{ min.x, min.y }, 
					size{ max.x - min.y, max.y - min.y } });

		iVertexArrays.vertices().clear();
		for (auto const& v : aVertices)
			iVertexArrays.vertices().push_back(xyz{ v[0], v[1] });
		iVertexArrays.vertices().push_back(iVertexArrays.vertices()[1]);
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), aFill.is<colour>() ?
			std::array <uint8_t, 4>{ {
					static_variant_cast<const colour&>(aFill).red(),
						static_variant_cast<const colour&>(aFill).green(),
						static_variant_cast<const colour&>(aFill).blue(),
						static_variant_cast<const colour&>(aFill).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, iVertexArrays.vertices().size()));
		if (aFill.is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::begin_drawing_glyphs()
	{
		glCheck(glTextureBarrierNV());
		glCheck(glActiveTexture(GL_TEXTURE1));
		glCheck(glClientActiveTexture(GL_TEXTURE1));
		glCheck(glEnable(GL_TEXTURE_2D));
		glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousTexture));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		iActiveGlyphTexture = static_cast<GLuint>(iPreviousTexture);
		iShaderProgramStack.emplace_back(*this, iRenderingEngine, iRenderingEngine.glyph_shader_program(is_subpixel_rendering_on()));
	}

	namespace
	{
		std::vector<std::array<double, 2>> texture_vertices(const size& aTextureStorageSize, const rect& aTextureRect, const std::pair<vec2, vec2>& aLogicalCoordinates)
		{
			typedef std::array<double, 2> xy;
			std::vector<xy> result;
			rect normalizedRect = aTextureRect / aTextureStorageSize;
			result.push_back(xy{ normalizedRect.top_left().x, normalizedRect.top_left().y });
			result.push_back(xy{ normalizedRect.top_right().x, normalizedRect.top_right().y });
			result.push_back(xy{ normalizedRect.bottom_right().x, normalizedRect.bottom_right().y });
			result.push_back(xy{ normalizedRect.bottom_left().x, normalizedRect.bottom_left().y });
			if (aLogicalCoordinates.first.y < aLogicalCoordinates.second.y)
			{
				std::swap(result[0][1], result[2][1]);
				std::swap(result[1][1], result[3][1]);
			}
			return result;
		}
	}

	void opengl_graphics_context::draw_glyph(const point& aPoint, const glyph& aGlyph, const font& aFont, const colour& aColour)
	{
		if (aGlyph.is_whitespace())
			return;

		if (aGlyph.is_emoji())
		{
			use_shader_program usp{ *this, iRenderingEngine, iRenderingEngine.default_shader_program() };
			auto const& emojiAtlas = iRenderingEngine.font_manager().emoji_atlas();
			auto const& emojiTexture = emojiAtlas.emoji_texture(aGlyph.value());
			draw_texture(rect{ aPoint, size{aFont.height(), aFont.height()} }.to_vector(), emojiTexture, rect{ point{}, emojiTexture.extents() }, optional_colour{}, shader_effect::None);
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			return;
		}

		use_shader_program usp{ *this, iRenderingEngine, iRenderingEngine.glyph_shader_program(aGlyph.subpixel()) };

		const font* glyphFont = &aFont;
		if (aGlyph.use_fallback())
		{
			if (iLastDrawGlyphFallbackFont != aFont || iLastDrawGlyphFallbackFontIndex == boost::none || *iLastDrawGlyphFallbackFontIndex != aGlyph.fallback_font_index())
			{
				iLastDrawGlyphFallbackFont = aGlyph.fallback_font(aFont);
				iLastDrawGlyphFallbackFontIndex = aGlyph.fallback_font_index();
			}
			glyphFont = &iLastDrawGlyphFallbackFont;
		}

		const i_glyph_texture& glyphTexture = glyphFont->native_font_face().glyph_texture(aGlyph);

		point glyphOrigin(aPoint.x + glyphTexture.placement().x, 
			logical_coordinates().first.y < logical_coordinates().second.y ? 
				aPoint.y + (glyphTexture.placement().y + -aFont.descender()) :
				aPoint.y + aFont.height() - (glyphTexture.placement().y + -aFont.descender()) - glyphTexture.texture().extents().cy);

		iVertexArrays.vertices().clear();
		iVertexArrays.vertices().insert(iVertexArrays.vertices().begin(),
		{
			to_shader_vertex(glyphOrigin),
			to_shader_vertex(glyphOrigin + point{ glyphTexture.texture().extents().cx, 0.0 }),
			to_shader_vertex(glyphOrigin + point{ glyphTexture.texture().extents().cx, glyphTexture.texture().extents().cy }),
			to_shader_vertex(glyphOrigin + point{ 0.0, glyphTexture.texture().extents().cy })
		});
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), std::array<uint8_t, 4>{{aColour.red(), aColour.green(), aColour.blue(), aColour.alpha()}});
		iVertexArrays.texture_coords() = texture_vertices(glyphTexture.texture().atlas_texture().storage_extents(), rect{ glyphTexture.texture().atlas_location().top_left(), glyphTexture.texture().extents() } + point{ 1.0, 1.0 }, logical_coordinates());
		iVertexArrays.instantiate(*this, iRenderingEngine.glyph_shader_program(aGlyph.subpixel()));

		if (iActiveGlyphTexture != reinterpret_cast<GLuint>(glyphTexture.texture().native_texture()->handle()))
		{
			iActiveGlyphTexture = reinterpret_cast<GLuint>(glyphTexture.texture().native_texture()->handle());
			glCheck(glBindTexture(GL_TEXTURE_2D, iActiveGlyphTexture));
		}
		
		iRenderingEngine.glyph_shader_program(aGlyph.subpixel()).set_uniform_variable("glyphTexture", 1);
		if (aGlyph.subpixel())
		{
			glCheck(glActiveTexture(GL_TEXTURE2));
			glCheck(glClientActiveTexture(GL_TEXTURE2));
			glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, reinterpret_cast<GLuint>(iSurface.rendering_target_texture_handle())));
			glCheck(glActiveTexture(GL_TEXTURE1));
			glCheck(glClientActiveTexture(GL_TEXTURE1));
			iRenderingEngine.glyph_shader_program(aGlyph.subpixel()).set_uniform_variable("guiCoordinates", logical_coordinates().first.y > logical_coordinates().second.y);
			iRenderingEngine.glyph_shader_program(aGlyph.subpixel()).set_uniform_variable("outputExtents", static_cast<float>(iSurface.surface_size().cx), static_cast<float>(iSurface.surface_size().cy));
			iRenderingEngine.glyph_shader_program(aGlyph.subpixel()).set_uniform_variable("outputTexture", 2);
		}

		glCheck(glEnable(GL_BLEND));
		glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		disable_anti_alias daa(*this);
		glCheck(glDrawArrays(GL_QUADS, 0, iVertexArrays.vertices().size()));
	}

	void opengl_graphics_context::end_drawing_glyphs()
	{
		glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(iPreviousTexture)));
		iShaderProgramStack.pop_back();
	}

	void opengl_graphics_context::draw_texture(const texture_map& aTextureMap, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour, shader_effect aShaderEffect)
	{
		if (aTexture.is_empty())
			return;
		rect textureRect = aTextureRect;
		if (aTexture.type() == i_texture::SubTexture)
			textureRect.position() += static_cast<const i_sub_texture&>(aTexture).atlas_location().top_left();
		glCheck(glActiveTexture(GL_TEXTURE1));
		glCheck(glClientActiveTexture(GL_TEXTURE1));
		glCheck(glEnable(GL_TEXTURE_2D));
		glCheck(glEnable(GL_BLEND));
		glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLint previousTexture;
		glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, aTexture.sampling() == texture_sampling::NormalMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
		glCheck(glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLuint>(aTexture.native_texture()->handle())));
		if (!aTexture.native_texture()->is_resident())
			throw texture_not_resident();

		iVertexArrays.vertices().clear();
		for (auto& v : aTextureMap)
			iVertexArrays.vertices().push_back(vertex{ v.x, v.y });
		iVertexArrays.texture_coords() = texture_vertices(aTexture.storage_extents(), textureRect + point{1.0, 1.0}, logical_coordinates());
		colour c{0xFF, 0xFF, 0xFF, 0xFF};
		if (aColour != boost::none)
			c = *aColour;
		iVertexArrays.colours().assign(aTextureMap.size(), std::array<uint8_t, 4>{ {c.red(), c.green(), c.blue(), c.alpha()}});

		use_shader_program usp{ *this, iRenderingEngine, aShaderEffect == shader_effect::Monochrome ?
			iRenderingEngine.monochrome_shader_program() :
			iRenderingEngine.texture_shader_program() };

		iRenderingEngine.active_shader_program().set_uniform_variable("tex", 1);

		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_QUADS, 0, 4));
		glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture)));
	}

	opengl_graphics_context::vertex opengl_graphics_context::to_shader_vertex(const point& aPoint) const
	{
		return vertex{{aPoint.x, aPoint.y, 0.0}};
	}

}