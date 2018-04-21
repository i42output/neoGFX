// opengl_graphics_context.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/game/rectangle.hpp>
#include <neogfx/game/shapes.hpp>
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
		return get_logical_coordinates(surface().surface_size(), iLogicalCoordinateSystem, iLogicalCoordinates);
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
		/*
		std::map<graphics_operation::operation_type, std::pair<int, int>> debug;
		for (auto& i : iQueue)
		{
			auto op = static_cast<graphics_operation::operation_type>(i.front().which());
			debug[op].first++;
			debug[op].second += i.size();
		}
		std::cout << "---" << std::endl;
		for (auto& d : debug)
		{
			std::cout << to_string(d.first) << ": " << d.second.first << ", " << d.second.second << std::endl;
		}
		std::cout << "---" << std::endl;
		*/

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
			case graphics_operation::operation_type::ClearDepthBuffer:
				clear_depth_buffer();
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
					draw_shape(args.mesh, args.pen);
				}
				break;
			case graphics_operation::operation_type::FillRect:
				fill_rect(opBatch);
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
				fill_shape(opBatch);
				break;
			case graphics_operation::operation_type::DrawGlyph:
				draw_glyph(opBatch);
				break;
			case graphics_operation::operation_type::DrawTextures:
				{
					use_shader_program usp{ *this, iRenderingEngine, iRenderingEngine.texture_shader_program() };
					for (auto& op : opBatch)
					{
						const auto& args = static_variant_cast<const graphics_operation::draw_textures&>(op);
						draw_textures(args.mesh, args.colour, args.shaderEffect);
					}
				}
				break;
			}
			iQueue.pop_front();
		}
	}

	void opengl_graphics_context::scissor_on(const rect& aRect)
	{
		if (iScissorRect == boost::none)
		{
			glCheck(glEnable(GL_SCISSOR_TEST));
			iScissorRect = aRect;
		}
		iScissorRects.push_back(*iScissorRect);
		iScissorRect = iScissorRect->intersection(aRect);
		apply_scissor();
	}

	void opengl_graphics_context::scissor_off()
	{
		auto previousScissorRect = iScissorRects.back();
		iScissorRects.pop_back();
		if (iScissorRects.empty())
		{
			glCheck(glDisable(GL_SCISSOR_TEST));
			iScissorRect = boost::none;
		}
		else
		{
			iScissorRect = previousScissorRect;
			apply_scissor();
		}
	}

	const optional_rect& opengl_graphics_context::scissor_rect() const
	{
		return iScissorRect;
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
				auto vertices = aPath.to_vertices(aPath.paths()[i]);
				iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
				iVertexArrays.colours().assign(vertices.size(), std::array <uint8_t, 4>{ {0xFF, 0xFF, 0xFF, 0xFF}});
				iVertexArrays.texture_coords().resize(vertices.size());
				iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

				glCheck(glDrawArrays(path_shape_to_gl_mode(aPath), 0, vertices.size()));
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
					auto vertices = aPath.to_vertices(innerPath.paths()[i]);
					iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
					iVertexArrays.colours().assign(vertices.size(), std::array <uint8_t, 4>{ {0xFF, 0xFF, 0xFF, 0xFF}});
					iVertexArrays.texture_coords().resize(vertices.size());
					iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

					glCheck(glDrawArrays(path_shape_to_gl_mode(innerPath), 0, vertices.size()));
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
		auto filter = static_gaussian_filter<float, opengl_renderer::GRADIENT_FILTER_SIZE>(static_cast<float>(aGradient.smoothness() * 10.0));
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

	void opengl_graphics_context::clear_depth_buffer()
	{
		glCheck(glClear(GL_DEPTH_BUFFER_BIT));
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
		if (aPen.colour().is<gradient>())
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, gradient.rect() != boost::none ? *gradient.rect() : rect{ aFrom, aTo });
		}

		double pixelAdjust = pixel_adjust(aPen);
		iVertexArrays.vertices().assign({ xyz{aFrom.x + pixelAdjust, aFrom.y + pixelAdjust}, xyz{aTo.x + pixelAdjust, aTo.y + pixelAdjust} });
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), aPen.colour().is<colour>() ? 
			std::array <uint8_t, 4>{{
				static_cast<colour>(aPen.colour()).red(), 
				static_cast<colour>(aPen.colour()).green(),
				static_cast<colour>(aPen.colour()).blue(),
				static_cast<colour>(aPen.colour()).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glDrawArrays(GL_LINES, 0, iVertexArrays.vertices().size()));
		glCheck(glLineWidth(1.0f));

		if (aPen.colour().is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::draw_rect(const rect& aRect, const pen& aPen)
	{
		if (aPen.colour().is<gradient>())
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, gradient.rect() != boost::none ? *gradient.rect() : aRect);
		}

		auto vertices = rect_vertices(aRect, pixel_adjust(aPen), rect_type::Outline);
		iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
		iVertexArrays.texture_coords().resize(vertices.size());
		iVertexArrays.colours().assign(vertices.size(), aPen.colour().is<colour>() ?
			std::array <uint8_t, 4>{{
				static_cast<colour>(aPen.colour()).red(),
				static_cast<colour>(aPen.colour()).green(),
				static_cast<colour>(aPen.colour()).blue(),
				static_cast<colour>(aPen.colour()).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glDrawArrays(GL_LINES, 0, vertices.size()));
		glCheck(glLineWidth(1.0f));

		if (aPen.colour().is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen)
	{
		if (aPen.colour().is<gradient>())
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, gradient.rect() != boost::none ? *gradient.rect() : aRect);
		}

		double pixelAdjust = pixel_adjust(aPen);
		auto vertices = rounded_rect_vertices(aRect + point{ pixelAdjust, pixelAdjust }, aRadius, false);
		iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
		iVertexArrays.texture_coords().resize(vertices.size());
		iVertexArrays.colours().assign(vertices.size(), aPen.colour().is<colour>() ?
			std::array <uint8_t, 4>{{
				static_cast<colour>(aPen.colour()).red(),
				static_cast<colour>(aPen.colour()).green(),
				static_cast<colour>(aPen.colour()).blue(),
				static_cast<colour>(aPen.colour()).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glDrawArrays(GL_LINE_LOOP, 0, vertices.size()));
		glCheck(glLineWidth(1.0f));

		if (aPen.colour().is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::draw_circle(const point& aCentre, dimension aRadius, const pen& aPen, angle aStartAngle)
	{
		if (aPen.colour().is<gradient>())
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, gradient.rect() != boost::none ? *gradient.rect() : rect{ aCentre - size{aRadius, aRadius}, size{aRadius * 2.0, aRadius * 2.0 } });
		}

		auto vertices = circle_vertices(aCentre, aRadius, aStartAngle, false);
		iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
		iVertexArrays.texture_coords().resize(vertices.size());
		iVertexArrays.colours().assign(vertices.size(), aPen.colour().is<colour>() ?
			std::array <uint8_t, 4>{{
				static_cast<colour>(aPen.colour()).red(),
				static_cast<colour>(aPen.colour()).green(),
				static_cast<colour>(aPen.colour()).blue(),
				static_cast<colour>(aPen.colour()).alpha()}} :
			std::array <uint8_t, 4>{});

		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glDrawArrays(GL_LINE_LOOP, 0, vertices.size()));
		glCheck(glLineWidth(1.0f));

		if (aPen.colour().is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen)
	{
		if (aPen.colour().is<gradient>())
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, gradient.rect() != boost::none ? *gradient.rect() : rect{ aCentre - size{ aRadius, aRadius }, size{ aRadius * 2.0, aRadius * 2.0 } });
		}

		auto vertices = line_loop_to_lines(arc_vertices(aCentre, aRadius, aStartAngle, aEndAngle, false));;
		iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
		iVertexArrays.texture_coords().resize(vertices.size());
		iVertexArrays.colours().assign(vertices.size(), aPen.colour().is<colour>() ?
			std::array <uint8_t, 4>{{
				static_cast<colour>(aPen.colour()).red(),
				static_cast<colour>(aPen.colour()).green(),
				static_cast<colour>(aPen.colour()).blue(),
				static_cast<colour>(aPen.colour()).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glDrawArrays(GL_LINES, 0, vertices.size()));
		glCheck(glLineWidth(1.0f));

		if (aPen.colour().is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::draw_path(const path& aPath, const pen& aPen)
	{
		if (aPen.colour().is<gradient>())
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, gradient.rect() != boost::none ? *gradient.rect() : aPath.bounding_rect());
		}

		for (std::size_t i = 0; i < aPath.paths().size(); ++i)
		{
			if (aPath.paths()[i].size() > 2)
			{
				if (aPath.shape() == path::ConvexPolygon)
					clip_to(aPath, aPen.width());

				auto vertices = aPath.to_vertices(aPath.paths()[i]);
				iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
				iVertexArrays.texture_coords().resize(vertices.size());
				iVertexArrays.colours().assign(vertices.size(), aPen.colour().is<colour>() ?
					std::array <uint8_t, 4>{{
						static_cast<colour>(aPen.colour()).red(),
						static_cast<colour>(aPen.colour()).green(),
						static_cast<colour>(aPen.colour()).blue(),
						static_cast<colour>(aPen.colour()).alpha()}} :
					std::array <uint8_t, 4>{});
				iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

				glCheck(glDrawArrays(path_shape_to_gl_mode(aPath.shape()), 0, vertices.size()));
				if (aPath.shape() == path::ConvexPolygon)
					reset_clip();
			}
		}

		if (aPen.colour().is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::draw_shape(const i_mesh& aMesh, const pen& aPen)
	{
		auto tvs = aMesh.transformed_vertices();

		if (aPen.colour().is<gradient>())
		{
			auto const& gradient = static_variant_cast<const neogfx::gradient&>(aPen.colour());
			gradient_on(gradient, gradient.rect() != boost::none ? *gradient.rect() : bounding_rect(tvs));
		}

		iVertexArrays.vertices().clear();
		for (auto const& v : tvs) // todo: have vertex shader do this transformation
			iVertexArrays.vertices().push_back(xyz{ v.coordinates.x, v.coordinates.y, v.coordinates.z });
		iVertexArrays.vertices().push_back(iVertexArrays.vertices()[0]);
		iVertexArrays.texture_coords().resize(iVertexArrays.vertices().size());
		iVertexArrays.colours().assign(iVertexArrays.vertices().size(), aPen.colour().is<colour>() ?
			std::array <uint8_t, 4>{{
				static_cast<colour>(aPen.colour()).red(),
				static_cast<colour>(aPen.colour()).green(),
				static_cast<colour>(aPen.colour()).blue(),
				static_cast<colour>(aPen.colour()).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_LINE_LOOP, 0, iVertexArrays.vertices().size()));

		if (aPen.colour().is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::fill_rect(const rect& aRect, const brush& aFill)
	{
		thread_local graphics_operation::batch batch;
		batch.clear();
		batch.push_back(graphics_operation::fill_rect{aRect, aFill});
		fill_rect(batch);
	}

	void opengl_graphics_context::fill_rect(const graphics_operation::batch& aFillRectOps)
	{
		auto& firstOp = static_variant_cast<const graphics_operation::fill_rect&>(aFillRectOps.front());

		if (firstOp.fill.is<gradient>())
			gradient_on(static_variant_cast<const gradient&>(firstOp.fill), firstOp.rect);

		iVertexArrays.vertices().clear();
		iVertexArrays.colours().clear();
		iVertexArrays.texture_coords().clear();

		iVertexArrays.vertices().reserve(aFillRectOps.size() * 6);
		iVertexArrays.colours().reserve(aFillRectOps.size() * 6);
		iVertexArrays.texture_coords().reserve(aFillRectOps.size() * 6);

		for (const auto& op : aFillRectOps)
		{
			auto& drawOp = static_variant_cast<const graphics_operation::fill_rect&>(op);
			auto rv = rect_vertices(drawOp.rect, 0.0, rect_type::FilledTriangles);
			iVertexArrays.vertices().insert(iVertexArrays.vertices().end(), rv.begin(), rv.end());
			iVertexArrays.texture_coords().insert(iVertexArrays.texture_coords().end(), 6, vec2{});
			auto c = drawOp.fill.is<colour>() ?
				std::array<uint8_t, 4>{{
						static_variant_cast<const colour&>(drawOp.fill).red(),
						static_variant_cast<const colour&>(drawOp.fill).green(),
						static_variant_cast<const colour&>(drawOp.fill).blue(),
						static_variant_cast<const colour&>(drawOp.fill).alpha()}} :
				std::array<uint8_t, 4>{};
			iVertexArrays.colours().insert(iVertexArrays.colours().end(), 6, c);
		}

		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_TRIANGLES, 0, iVertexArrays.vertices().size()));

		if (firstOp.fill.is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::fill_rounded_rect(const rect& aRect, dimension aRadius, const brush& aFill)
	{
		if (aRect.empty())
			return;

		if (aFill.is<gradient>())
			gradient_on(static_variant_cast<const gradient&>(aFill), aRect);

		auto vertices = rounded_rect_vertices(aRect, aRadius, true);
		iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
		iVertexArrays.texture_coords().resize(vertices.size());
		iVertexArrays.colours().assign(vertices.size(), aFill.is<colour>() ?
			std::array <uint8_t, 4>{ {
					static_variant_cast<const colour&>(aFill).red(),
						static_variant_cast<const colour&>(aFill).green(),
						static_variant_cast<const colour&>(aFill).blue(),
						static_variant_cast<const colour&>(aFill).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size()));

		if (aFill.is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::fill_circle(const point& aCentre, dimension aRadius, const brush& aFill)
	{
		if (aFill.is<gradient>())
			gradient_on(static_variant_cast<const gradient&>(aFill), rect{ aCentre - point{ aRadius, aRadius }, size{ aRadius * 2.0 } });

		auto vertices = circle_vertices(aCentre, aRadius, 0.0, true);
		iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
		iVertexArrays.texture_coords().resize(vertices.size());
		iVertexArrays.colours().assign(vertices.size(), aFill.is<colour>() ?
			std::array <uint8_t, 4>{ {
					static_variant_cast<const colour&>(aFill).red(),
						static_variant_cast<const colour&>(aFill).green(),
						static_variant_cast<const colour&>(aFill).blue(),
						static_variant_cast<const colour&>(aFill).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size()));

		if (aFill.is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::fill_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const brush& aFill)
	{
		if (aFill.is<gradient>())
			gradient_on(static_variant_cast<const gradient&>(aFill), rect{ aCentre - point{ aRadius, aRadius }, size{ aRadius * 2.0 } });

		auto vertices = arc_vertices(aCentre, aRadius, aStartAngle, aEndAngle, true);
		iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
		iVertexArrays.texture_coords().resize(vertices.size());
		iVertexArrays.colours().assign(vertices.size(), aFill.is<colour>() ?
			std::array <uint8_t, 4>{ {
					static_variant_cast<const colour&>(aFill).red(),
						static_variant_cast<const colour&>(aFill).green(),
						static_variant_cast<const colour&>(aFill).blue(),
						static_variant_cast<const colour&>(aFill).alpha()}} :
			std::array <uint8_t, 4>{});
		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size()));
		if (aFill.is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::fill_path(const path& aPath, const brush& aFill)
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
					min = min.min(pt);
					max = max.max(pt);
				}

				if (aFill.is<gradient>())
					gradient_on(static_variant_cast<const gradient&>(aFill), rect{ point{ min.x, min.y }, size{ max.x - min.y, max.y - min.y } });

				auto vertices = aPath.to_vertices(aPath.paths()[i]);
				iVertexArrays.vertices().assign(vertices.begin(), vertices.end());
				iVertexArrays.colours().assign(vertices.size(), aFill.is<colour>() ?
					std::array <uint8_t, 4>{ {
							static_variant_cast<const colour&>(aFill).red(),
								static_variant_cast<const colour&>(aFill).green(),
								static_variant_cast<const colour&>(aFill).blue(),
								static_variant_cast<const colour&>(aFill).alpha()}} :
					std::array <uint8_t, 4>{});
				iVertexArrays.texture_coords().resize(vertices.size());
				iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

				glCheck(glDrawArrays(path_shape_to_gl_mode(aPath.shape()), 0, vertices.size()));

				reset_clip();

				if (aFill.is<gradient>())
					gradient_off();
			}
		}
	}

	void opengl_graphics_context::fill_shape(const graphics_operation::batch& aFillShapeOps)
	{
		auto& firstOp = static_variant_cast<const graphics_operation::fill_shape&>(aFillShapeOps.front());

		if (firstOp.fill.is<gradient>())
		{
			auto tvs = firstOp.mesh.transformed_vertices();
			vec3 min = tvs[0].coordinates.xyz;
			vec3 max = min;
			for (auto const& v : tvs)
			{
				min.x = std::min(min.x, v.coordinates.x);
				max.x = std::max(max.x, v.coordinates.x);
				min.y = std::min(min.y, v.coordinates.y);
				max.y = std::max(max.y, v.coordinates.y);
			}
			gradient_on(static_variant_cast<const gradient&>(firstOp.fill),
				rect{
					point{ min.x, min.y },
					size{ max.x - min.y, max.y - min.y } });
		}

		iVertexArrays.vertices().clear();
		iVertexArrays.texture_coords().clear();
		iVertexArrays.colours().clear();

		for (auto const& op : aFillShapeOps)
		{
			auto& drawOp = static_variant_cast<const graphics_operation::fill_shape&>(op);
			auto tvs = drawOp.mesh.transformed_vertices(); // todo: have vertex shader do this transformation
			for (auto const& f : drawOp.mesh.faces())
			{
				for (auto vi : f.vertices)
				{
					auto const& v = tvs[vi];
					iVertexArrays.vertices().push_back(xyz{ v.coordinates.x, v.coordinates.y, v.coordinates.z });
					iVertexArrays.texture_coords().push_back((v.textureCoordinates.xy).v);
					iVertexArrays.colours().insert(iVertexArrays.colours().end(), 1, drawOp.fill.is<colour>() ?
						std::array <uint8_t, 4>{ {
								static_variant_cast<const colour&>(drawOp.fill).red(),
									static_variant_cast<const colour&>(drawOp.fill).green(),
									static_variant_cast<const colour&>(drawOp.fill).blue(),
									static_variant_cast<const colour&>(drawOp.fill).alpha()}} :
						std::array <uint8_t, 4>{});
				}
			}
		}

		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());

		glCheck(glDrawArrays(GL_TRIANGLES, 0, iVertexArrays.vertices().size()));

		if (firstOp.fill.is<gradient>())
			gradient_off();
	}

	namespace
	{
		void texture_vertices(const size& aTextureStorageSize, const rect& aTextureRect, const std::pair<vec2, vec2>& aLogicalCoordinates, std::vector<vec2>& aResult)
		{
			rect normalizedRect = aTextureRect / aTextureStorageSize;
			aResult.emplace_back(normalizedRect.top_left().x, normalizedRect.top_left().y);
			aResult.emplace_back(normalizedRect.top_right().x, normalizedRect.top_right().y);
			aResult.emplace_back(normalizedRect.bottom_right().x, normalizedRect.bottom_right().y);
			aResult.emplace_back(normalizedRect.bottom_left().x, normalizedRect.bottom_left().y);
			if (aLogicalCoordinates.first.y < aLogicalCoordinates.second.y)
			{
				std::swap(aResult[0][1], aResult[2][1]);
				std::swap(aResult[1][1], aResult[3][1]);
			}
		}
	}

	void opengl_graphics_context::draw_glyph(const graphics_operation::batch& aDrawGlyphOps)
	{
		auto& firstOp = static_variant_cast<const graphics_operation::draw_glyph&>(aDrawGlyphOps.front());

		if (firstOp.glyph.is_emoji())
		{
			use_shader_program usp{ *this, iRenderingEngine, iRenderingEngine.default_shader_program() };
			auto const& emojiAtlas = iRenderingEngine.font_manager().emoji_atlas();
			auto const& emojiTexture = emojiAtlas.emoji_texture(firstOp.glyph.value()).as_sub_texture();
			rectangle r{ firstOp.point, size{ firstOp.glyph.extents().cx, firstOp.glyph.extents().cy }.to_vec2() };
			r.set_position(r.position() + vec3{ r.extents().x, r.extents().y, 0.0 } / 2.0);
			r.set_textures(to_texture_list_pointer(emojiTexture));
			draw_textures(r, optional_colour{}, shader_effect::None);
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			return;
		}

		iVertexArrays.vertices().clear();
		iVertexArrays.colours().clear();
		iVertexArrays.texture_coords().clear();

		for (uint32_t pass = 1; pass <= 2; ++pass)
		{
			for (const auto& op : aDrawGlyphOps)
			{
				auto& drawOp = static_variant_cast<const graphics_operation::draw_glyph&>(op);

				const font& glyphFont = drawOp.glyph.font();
				const i_glyph_texture& glyphTexture = drawOp.glyph.glyph_texture();

				vec3 glyphOrigin(
					drawOp.point.x + glyphTexture.placement().x,
					logical_coordinates().first.y < logical_coordinates().second.y ? 
						drawOp.point.y + (glyphTexture.placement().y + -glyphFont.descender()) :
						drawOp.point.y + glyphFont.height() - (glyphTexture.placement().y + -glyphFont.descender()) - glyphTexture.texture().extents().cy,
					drawOp.point.z);

				iTempTextureCoords.clear();
				texture_vertices(glyphTexture.texture().atlas_texture().storage_extents(), rect{ glyphTexture.texture().atlas_location().top_left(), glyphTexture.texture().extents() } +point{ 1.0, 1.0 }, logical_coordinates(), iTempTextureCoords);

				rect outputRect{ point{glyphOrigin}, glyphTexture.texture().extents() };

				if (drawOp.appearance.has_effect() && pass == 1)
				{
					if (drawOp.appearance.effect().type() == text_effect::Outline)
					{
						for (double y = -drawOp.appearance.effect().width(); y <= drawOp.appearance.effect().width(); y += 1.0)
						{
							for (double x = -drawOp.appearance.effect().width(); x <= drawOp.appearance.effect().width(); x += 1.0)
							{
								rect effectRect = outputRect + point{ x, y };
								iVertexArrays.vertices().push_back(effectRect.top_left().to_vec3(glyphOrigin.z));
								iVertexArrays.vertices().push_back(effectRect.top_right().to_vec3(glyphOrigin.z));
								iVertexArrays.vertices().push_back(effectRect.bottom_right().to_vec3(glyphOrigin.z));
								iVertexArrays.vertices().push_back(effectRect.bottom_left().to_vec3(glyphOrigin.z));
								iVertexArrays.colours().insert(iVertexArrays.colours().end(), 4, drawOp.appearance.effect().colour().is<colour>() ?
									std::array <uint8_t, 4>{ {
										static_variant_cast<const colour&>(drawOp.appearance.effect().colour()).red(),
										static_variant_cast<const colour&>(drawOp.appearance.effect().colour()).green(),
										static_variant_cast<const colour&>(drawOp.appearance.effect().colour()).blue(),
										static_variant_cast<const colour&>(drawOp.appearance.effect().colour()).alpha()}} :
									std::array <uint8_t, 4>{});
								iVertexArrays.texture_coords().insert(iVertexArrays.texture_coords().end(), iTempTextureCoords.begin(), iTempTextureCoords.end());
							}
						}
					}
				}
				else if (pass == 2)
				{
					iVertexArrays.vertices().push_back(outputRect.top_left().to_vec3(glyphOrigin.z));
					iVertexArrays.vertices().push_back(outputRect.top_right().to_vec3(glyphOrigin.z));
					iVertexArrays.vertices().push_back(outputRect.bottom_right().to_vec3(glyphOrigin.z));
					iVertexArrays.vertices().push_back(outputRect.bottom_left().to_vec3(glyphOrigin.z));
					iVertexArrays.colours().insert(iVertexArrays.colours().end(), 4,  drawOp.appearance.ink().is<colour>() ?
						std::array <uint8_t, 4>{{
							static_variant_cast<const colour&>(drawOp.appearance.ink()).red(),
							static_variant_cast<const colour&>(drawOp.appearance.ink()).green(),
							static_variant_cast<const colour&>(drawOp.appearance.ink()).blue(),
							static_variant_cast<const colour&>(drawOp.appearance.ink()).alpha()}} :
						std::array <uint8_t, 4>{});
					iVertexArrays.texture_coords().insert(iVertexArrays.texture_coords().end(), iTempTextureCoords.begin(), iTempTextureCoords.end());
				}
			}
		}

		if (iVertexArrays.vertices().empty())
			return;

		glCheck(glActiveTexture(GL_TEXTURE1));
		glCheck(glClientActiveTexture(GL_TEXTURE1));
		glCheck(glEnable(GL_TEXTURE_2D));
		glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousTexture));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		if (firstOp.glyph.subpixel())
		{
			glCheck(glActiveTexture(GL_TEXTURE2));
			glCheck(glClientActiveTexture(GL_TEXTURE2));
			glCheck(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, reinterpret_cast<GLuint>(iSurface.rendering_target_texture_handle())));
			glCheck(glActiveTexture(GL_TEXTURE1));
			glCheck(glClientActiveTexture(GL_TEXTURE1));
		}

		if (firstOp.appearance.ink().is<gradient>())
			gradient_on(static_variant_cast<const gradient&>(firstOp.appearance.ink()), rect{ point{ iVertexArrays.vertices()[0][0], iVertexArrays.vertices()[0][1] }, point{ iVertexArrays.vertices()[2][0], iVertexArrays.vertices()[2][1] } });

		const i_glyph_texture& firstGlyphTexture = firstOp.glyph.glyph_texture();
		glCheck(glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLuint>(firstGlyphTexture.texture().native_texture()->handle())));

		glCheck(glEnable(GL_BLEND));
		glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		disable_anti_alias daa(*this);

		use_shader_program usp{ *this, iRenderingEngine, iRenderingEngine.glyph_shader_program(firstOp.glyph.subpixel())};

		std::size_t index = 0;
		for (uint32_t pass = 1; pass <= 2; ++pass)
		{
			auto& shader = iRenderingEngine.active_shader_program();

			iVertexArrays.instantiate(*this, shader);

			bool guiCoordinates = (logical_coordinates().first.y > logical_coordinates().second.y);
			shader.set_uniform_variable("guiCoordinates", guiCoordinates);
			shader.set_uniform_variable("outputExtents", static_cast<float>(iSurface.surface_size().cx), static_cast<float>(iSurface.surface_size().cy));
			
			shader.set_uniform_variable("glyphTexture", 1);

			if (firstOp.glyph.subpixel())
				shader.set_uniform_variable("outputTexture", 2);

			if (pass == 2 && firstOp.glyph.subpixel())
			{
				glCheck(glTextureBarrier());
			}

			if (pass == 1)
			{
				for (const auto& op : aDrawGlyphOps)
				{
					auto& drawOp = static_variant_cast<const graphics_operation::draw_glyph&>(op);

					if (drawOp.appearance.has_effect())
					{
						shader.set_uniform_variable("effect", static_cast<int>(drawOp.appearance.effect().type()));
						if (drawOp.appearance.effect().type() == text_effect::Outline)
						{
							GLsizei n = static_cast<GLsizei>(std::pow(drawOp.appearance.effect().width() * 2.0 + 1.0, 2.0));
							glCheck(glDrawArrays(GL_QUADS, index, n * 4));
							index += (n * 4);
						}
						else if (drawOp.appearance.effect().type() == text_effect::Glow || drawOp.appearance.effect().type() == text_effect::Shadow)
						{
							const i_glyph_texture& glyphTexture = drawOp.glyph.glyph_texture();
							shader.set_uniform_variable("glyphOrigin",
								static_cast<float>(iVertexArrays.texture_coords()[index][0] * glyphTexture.texture().atlas_texture().storage_extents().cx),
								static_cast<float>(iVertexArrays.texture_coords()[index][1] * glyphTexture.texture().atlas_texture().storage_extents().cy));

							if (guiCoordinates)
							{
								shader.set_uniform_variable("effectRect", vec4f{ vec4{ iVertexArrays.vertices()[index][0], iVertexArrays.vertices()[index + 2][1] - 1.0, iVertexArrays.vertices()[index + 2][0], iVertexArrays.vertices()[index][1] - 1.0 } });
								shader.set_uniform_variable("glyphRect", vec4f{ vec4{ iVertexArrays.vertices()[index + 4][0], iVertexArrays.vertices()[index + 4 + 2][1] - 1.0, iVertexArrays.vertices()[index + 4 + 2][0], iVertexArrays.vertices()[index + 4][1] - 1.0 } });
							}
							else
							{
								shader.set_uniform_variable("effectRect", vec4f{ vec4{ iVertexArrays.vertices()[index][0], iVertexArrays.vertices()[index][1], iVertexArrays.vertices()[index + 2][0], iVertexArrays.vertices()[index + 2][1] } });
								shader.set_uniform_variable("glyphRect", vec4f{ vec4{ iVertexArrays.vertices()[index + 4][0], iVertexArrays.vertices()[index + 4][1], iVertexArrays.vertices()[index + 4 + 2][0], iVertexArrays.vertices()[index + 4 + 2][1] } });
							}
							shader.set_uniform_variable("effectWidth", static_cast<int>(drawOp.appearance.effect().width()));
							glCheck(glDrawArrays(GL_QUADS, index, 4));
							index += 4;
						}
					}
				}
			}
			else if (pass == 2)
			{
				auto count = iVertexArrays.vertices().size() - index;
				if (count > 0)
				{
					shader.set_uniform_variable("effect", 0);
					glCheck(glDrawArrays(GL_QUADS, index, count));
				}
			}
		}

		glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(iPreviousTexture)));

		if (firstOp.appearance.ink().is<gradient>())
			gradient_off();
	}

	void opengl_graphics_context::draw_textures(const i_mesh& aMesh, const optional_colour& aColour, shader_effect aShaderEffect)
	{
		auto face_cmp = [&aMesh](const face& aLhs, const face& aRhs) { return (*aMesh.textures())[aLhs.texture].first->native_texture()->handle() < (*aMesh.textures())[aRhs.texture].first->native_texture()->handle(); };
		if (!std::is_sorted(aMesh.faces().begin(), aMesh.faces().end(), face_cmp))
			std::sort(aMesh.faces().begin(), aMesh.faces().end(), face_cmp);

		colour colourizationColour{ 0xFF, 0xFF, 0xFF, 0xFF };
		if (aColour != boost::none)
			colourizationColour = *aColour;

		auto transformedVertices = aMesh.transformed_vertices(); // todo: have vertex shader do this transformation

		use_shader_program usp{ *this, iRenderingEngine, iRenderingEngine.texture_shader_program() };
		iRenderingEngine.active_shader_program().set_uniform_variable("effect", static_cast<int>(aShaderEffect));

		glCheck(glActiveTexture(GL_TEXTURE1));
		glCheck(glClientActiveTexture(GL_TEXTURE1));
		glCheck(glEnable(GL_TEXTURE_2D));
		glCheck(glEnable(GL_BLEND));
		glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLint previousTexture;
		glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture));
		glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		GLuint textureHandle = 0;
		bool first = true;
		bool newTexture = false;
		for (auto const& f : aMesh.faces())
		{
			auto const& texture = *(*aMesh.textures())[f.texture].first;

			if (first || textureHandle != reinterpret_cast<GLuint>(texture.native_texture()->handle()))
			{
				newTexture = true;
				textureHandle = reinterpret_cast<GLuint>(texture.native_texture()->handle());
				glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.sampling() == texture_sampling::NormalMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
				glCheck(glBindTexture(GL_TEXTURE_2D, textureHandle));
				if (!texture.native_texture()->is_resident())
					throw texture_not_resident();
				if (first)
					iRenderingEngine.active_shader_program().set_uniform_variable("tex", 1);
			}

			auto textureRect = (*aMesh.textures())[f.texture].second ? *(*aMesh.textures())[f.texture].second : rect{ point{ 0.0, 0.0 }, texture.extents() };

			if (texture.type() == i_texture::SubTexture)
				textureRect.position() += texture.as_sub_texture().atlas_location().top_left();
			iTempTextureCoords.clear();
			texture_vertices(texture.storage_extents(), textureRect + point{ 1.0, 1.0 }, logical_coordinates(), iTempTextureCoords);

			if (newTexture)
			{
				newTexture = false;
				if (!first)
				{
					iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());
					glCheck(glDrawArrays(GL_TRIANGLES, 0, iVertexArrays.vertices().size()));
				}
				iVertexArrays.vertices().clear();
				iVertexArrays.texture_coords().clear();
				iVertexArrays.colours().clear();
			}

			for (auto vi : f.vertices)
			{
				auto const& v = transformedVertices[vi];
				iVertexArrays.vertices().push_back(xyz{ v.coordinates.x, v.coordinates.y, v.coordinates.z });
				iVertexArrays.texture_coords().push_back((iTempTextureCoords[0] + (iTempTextureCoords[2] - iTempTextureCoords[0]) * ~v.textureCoordinates.xy).v);
				iVertexArrays.colours().push_back(std::array<uint8_t, 4>{ {colourizationColour.red(), colourizationColour.green(), colourizationColour.blue(), colourizationColour.alpha()}});
			}

			first = false;
		}

		iVertexArrays.instantiate(*this, iRenderingEngine.active_shader_program());
		glCheck(glDrawArrays(GL_TRIANGLES, 0, iVertexArrays.vertices().size()));

		glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture)));
	}

	xyz opengl_graphics_context::to_shader_vertex(const point& aPoint, coordinate aZ) const
	{
		return xyz{{ aPoint.x, aPoint.y, aZ }};
	}

}