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
#include <neogfx/gfx/text/text_direction_map.hpp>
#include <neogfx/gfx/text/i_font_texture.hpp>
#include "i_native_texture.hpp"
#include "../text/native/i_native_font_face.hpp"
#include "../text/native/native_font_face.hpp"
#include "opengl_graphics_context.hpp"

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

		inline std::vector<GLdouble> rect_vertices(const rect& aRect, dimension aPixelAdjust, bool aIncludeCentre)
		{
			std::vector<GLdouble> result;
			result.reserve(16);
			if (aIncludeCentre) // fill
			{
				result.push_back(aRect.centre().x);
				result.push_back(aRect.centre().y);
				result.push_back(aRect.top_left().x);
				result.push_back(aRect.top_left().y);
				result.push_back(aRect.top_right().x);
				result.push_back(aRect.top_right().y);
				result.push_back(aRect.bottom_right().x);
				result.push_back(aRect.bottom_right().y);
				result.push_back(aRect.bottom_left().x);
				result.push_back(aRect.bottom_left().y);
				result.push_back(aRect.top_left().x);
				result.push_back(aRect.top_left().y);
			}
			else // draw (outline)
			{
				result.push_back(aRect.top_left().x);
				result.push_back(aRect.top_left().y + aPixelAdjust);
				result.push_back(aRect.top_right().x);
				result.push_back(aRect.top_right().y + aPixelAdjust);
				result.push_back(aRect.top_right().x - aPixelAdjust);
				result.push_back(aRect.top_right().y);
				result.push_back(aRect.bottom_right().x - aPixelAdjust);
				result.push_back(aRect.bottom_right().y);
				result.push_back(aRect.bottom_right().x);
				result.push_back(aRect.bottom_right().y - aPixelAdjust);
				result.push_back(aRect.bottom_left().x);
				result.push_back(aRect.bottom_left().y - aPixelAdjust);
				result.push_back(aRect.bottom_left().x + aPixelAdjust);
				result.push_back(aRect.bottom_left().y);
				result.push_back(aRect.top_left().x + aPixelAdjust);
				result.push_back(aRect.top_left().y);
			}
			return result;
		};

		inline std::vector<GLdouble> arc_vertices(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, bool aIncludeCentre)
		{
			std::vector<GLdouble> result;
			uint32_t segments = static_cast<uint32_t>(20 * std::sqrt(aRadius));
			result.reserve((segments + (aIncludeCentre ? 2 : 1)) * 2);
			if (aIncludeCentre)
			{
				result.push_back(aCentre.x);
				result.push_back(aCentre.y);
			}
			coordinate theta = (aEndAngle - aStartAngle) / static_cast<coordinate>(segments);
			coordinate c = std::cos(theta);
			coordinate s = std::sin(theta);
			auto startCoordinate = mat22{ { std::cos(aStartAngle), std::sin(aStartAngle) },{ -std::sin(aStartAngle), std::cos(aStartAngle) } } *
				vec2{ aRadius, 0.0 };
			coordinate x = startCoordinate.x;
			coordinate y = startCoordinate.y;
			for (uint32_t i = 0; i < segments; ++i)
			{
				result.push_back(x + aCentre.x);
				result.push_back(y + aCentre.y);
				coordinate t = x;
				x = c * x - s * y;
				y = s * t + c * y;
			}
			return result;
		}

		inline std::vector<GLdouble> circle_vertices(const point& aCentre, dimension aRadius, bool aIncludeCentre)
		{
			auto result = arc_vertices(aCentre, aRadius, 0, boost::math::constants::two_pi<coordinate>(), aIncludeCentre);
			result.push_back(result[aIncludeCentre ? 2 : 0]);
			result.push_back(result[aIncludeCentre ? 3 : 1]);
			return result;
		}

		inline std::vector<GLdouble> rounded_rect_vertices(const rect& aRect, dimension aRadius, bool aIncludeCentre)
		{
			std::vector<GLdouble> result;
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
			result.reserve(topLeft.size() + topRight.size() + bottomRight.size() + bottomLeft.size() + (aIncludeCentre ? 2 : 1));
			if (aIncludeCentre)
			{
				result.push_back(aRect.centre().x);
				result.push_back(aRect.centre().y);
			}
			result.insert(result.end(), topLeft.begin(), topLeft.end());
			result.insert(result.end(), topRight.begin(), topRight.end());
			result.insert(result.end(), bottomRight.begin(), bottomRight.end());
			result.insert(result.end(), bottomLeft.begin(), bottomLeft.end());
			result.push_back(result[aIncludeCentre ? 2 : 0]);
			result.push_back(result[aIncludeCentre ? 3 : 1]);
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

		inline std::vector<GLdouble> line_loop_to_lines(const std::vector<GLdouble>& aLineLoop)
		{
			std::vector<GLdouble> result;
			result.reserve(aLineLoop.size() * 2);
			for (auto v = aLineLoop.begin(); v != aLineLoop.end(); v += 2)
			{
				result.push_back(*v);
				result.push_back(*(v + 1));
				if (v != aLineLoop.begin() && v != aLineLoop.end() - 2)
				{
					result.push_back(*v);
					result.push_back(*(v + 1));
				}
			}
			return result;
		}
	}

	opengl_graphics_context::opengl_graphics_context(i_rendering_engine& aRenderingEngine, const i_native_surface& aSurface) :
		iRenderingEngine(aRenderingEngine), 
		iSurface(aSurface), 
		iSavedCoordinateSystem(aSurface.logical_coordinate_system()), 
		iLogicalCoordinateSystem(iSavedCoordinateSystem), 
		iLogicalCoordinates(aSurface.logical_coordinates()), 
		iSmoothingMode(SmoothingModeNone), 
		iMonochrome(false), 
		iClipCounter(0),
		iLineStippleActive(false),
		iSubpixelRendering(aRenderingEngine.is_subpixel_rendering_on())
	{
		iSurface.activate_context();
		set_smoothing_mode(SmoothingModeAntiAlias);
	}

	opengl_graphics_context::opengl_graphics_context(i_rendering_engine& aRenderingEngine, const i_native_surface& aSurface, const i_widget& aWidget) :
		iRenderingEngine(aRenderingEngine), 
		iSurface(aSurface), 
		iSavedCoordinateSystem(aWidget.logical_coordinate_system()),
		iLogicalCoordinateSystem(iSavedCoordinateSystem),
		iLogicalCoordinates(aSurface.logical_coordinates()),
		iSmoothingMode(SmoothingModeNone), 
		iMonochrome(false),
		iClipCounter(0), 
		iLineStippleActive(false),
		iSubpixelRendering(aRenderingEngine.is_subpixel_rendering_on())
	{
		iSurface.activate_context();
		set_smoothing_mode(SmoothingModeAntiAlias);
	}

	opengl_graphics_context::opengl_graphics_context(const opengl_graphics_context& aOther) :
		iRenderingEngine(aOther.iRenderingEngine), 
		iSurface(aOther.iSurface), 
		iSavedCoordinateSystem(aOther.iSavedCoordinateSystem),
		iLogicalCoordinateSystem(aOther.iLogicalCoordinateSystem),
		iLogicalCoordinates(aOther.iLogicalCoordinates),
		iSmoothingMode(aOther.iSmoothingMode), 
		iMonochrome(false),
		iClipCounter(0),
		iLineStippleActive(false),
		iSubpixelRendering(false)
	{
		iSurface.activate_context();
		set_smoothing_mode(iSmoothingMode);
	}

	opengl_graphics_context::~opengl_graphics_context()
	{
		set_logical_coordinate_system(iSavedCoordinateSystem);
		iSurface.deactivate_context();
		if (iGradientTextures != boost::none)
		{
			glCheck(glDeleteTextures(1, &iGradientTextures->first));
			glCheck(glDeleteTextures(1, &iGradientTextures->second));
		}
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
		if (iLogicalCoordinateSystem != aSystem)
		{
			iLogicalCoordinateSystem = aSystem;
			const auto& logicalCoordinates = logical_coordinates();
			glCheck(glLoadIdentity());
			glCheck(glOrtho(logicalCoordinates[0], logicalCoordinates[2], logicalCoordinates[1], logicalCoordinates[3], -1.0, 1.0));
		}
	}

	const vector4& opengl_graphics_context::logical_coordinates() const
	{
		switch (iLogicalCoordinateSystem)
		{
		case neogfx::logical_coordinate_system::Specified:
			return iLogicalCoordinates;
		case neogfx::logical_coordinate_system::AutomaticGui:
			return iLogicalCoordinates = vector4{ 0.0, surface().surface_size().cy, surface().surface_size().cx, 0.0 };
		case neogfx::logical_coordinate_system::AutomaticGame:
			return iLogicalCoordinates = vector4{ 0.0, 0.0, surface().surface_size().cx, surface().surface_size().cy };
		}
		return iLogicalCoordinates;
	}

	void opengl_graphics_context::set_logical_coordinates(const vector4& aCoordinates) const
	{
		if (iLogicalCoordinates != aCoordinates)
		{
			iLogicalCoordinates = aCoordinates;
			const auto& logicalCoordinates = logical_coordinates();
			glCheck(glLoadIdentity());
			glCheck(glOrtho(logicalCoordinates[0], logicalCoordinates[2], logicalCoordinates[1], logicalCoordinates[3], -1.0, 1.0));
		}
	}

	void opengl_graphics_context::flush()
	{
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
		GLint x = static_cast<GLint>(std::ceil(scissor_rect()->x));
		GLint y = static_cast<GLint>(std::ceil(rendering_area(false).cy - scissor_rect()->cy - scissor_rect()->y));
		GLsizei cx = static_cast<GLsizei>(std::ceil(scissor_rect()->cx));
		GLsizei cy = static_cast<GLsizei>(std::ceil(scissor_rect()->cy));
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
				std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{0xFF, 0xFF, 0xFF, 0xFF}});
				glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
				glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
				std::vector<double> texCoords(vertices.size(), 0.0);
				glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
				glCheck(glDrawArrays(path_shape_to_gl_mode(aPath), 0, vertices.size() / 2));
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
					std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{0xFF, 0xFF, 0xFF, 0xFF}});
					glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
					glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
					std::vector<double> texCoords(vertices.size(), 0.0);
					glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
					glCheck(glDrawArrays(path_shape_to_gl_mode(innerPath), 0, vertices.size() / 2));
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

	bool opengl_graphics_context::monochrome() const
	{
		return iMonochrome;
	}

	void opengl_graphics_context::set_monochrome(bool aMonochrome)
	{
		iMonochrome = aMonochrome;
	}

	smoothing_mode_e opengl_graphics_context::smoothing_mode() const
	{
		return iSmoothingMode;
	}

	smoothing_mode_e opengl_graphics_context::set_smoothing_mode(smoothing_mode_e aSmoothingMode)
	{
		glCheck((void)0);
		smoothing_mode_e oldSmoothingMode = iSmoothingMode;
		iSmoothingMode = aSmoothingMode;
		if (iSmoothingMode == SmoothingModeAntiAlias)
		{
			glCheck(glEnable(GL_LINE_SMOOTH));
			glCheck(glEnable(GL_POLYGON_SMOOTH));
		}
		else
		{
			glCheck(glDisable(GL_LINE_SMOOTH));
			glCheck(glDisable(GL_POLYGON_SMOOTH));
		}
		return oldSmoothingMode;
	}

	void opengl_graphics_context::push_logical_operation(logical_operation_e aLogicalOperation)
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
		if (iLogicalOperationStack.empty() || iLogicalOperationStack.back() == LogicalNone)
		{
			glCheck(glDisable(GL_COLOR_LOGIC_OP));
		}
		else
		{
			glCheck(glEnable(GL_COLOR_LOGIC_OP));
			switch (iLogicalOperationStack.back())
			{
			case LogicalXor:
				glCheck(glLogicOp(GL_XOR));
				break;
			}
		}	
	}

	void opengl_graphics_context::gradient_on(const gradient& aGradient, const rect& aBoundingBox)
	{
		basic_rect<float> boundingBox{ aBoundingBox };
		iRenderingEngine.activate_shader_program(iRenderingEngine.gradient_shader_program());
		iRenderingEngine.gradient_shader_program().set_uniform_variable("posTopLeft", boundingBox.top_left().x, boundingBox.top_left().y);
		iRenderingEngine.gradient_shader_program().set_uniform_variable("posBottomRight", boundingBox.bottom_right().x, boundingBox.bottom_right().y);
		iRenderingEngine.gradient_shader_program().set_uniform_variable("nGradientDirection", static_cast<int>(aGradient.direction()));
		auto combinedStops = aGradient.combined_stops();
		iGradientStopPositions.reserve(combinedStops.size());
		iGradientStopColours.reserve(combinedStops.size());
		iGradientStopPositions.clear();
		iGradientStopColours.clear();
		for (const auto& stop : combinedStops)
		{
			iGradientStopPositions.push_back(static_cast<float>(stop.first));
			iGradientStopColours.push_back(std::array<uint8_t, 4>{ {stop.second.red(), stop.second.green(), stop.second.blue(), stop.second.alpha()}});
		}
		iRenderingEngine.gradient_shader_program().set_uniform_variable("nStopCount", static_cast<int>(iGradientStopPositions.size()));
		glCheck(glEnable(GL_TEXTURE_RECTANGLE));
		if (iGradientTextures == boost::none)
		{
			iGradientTextures.emplace(0, 0);
			glCheck(glGenTextures(1, &iGradientTextures->first));
			glCheck(glGenTextures(1, &iGradientTextures->second));
			GLint previousTexture;
			glCheck(glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &previousTexture));
			glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, iGradientTextures->first));
			glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			static const std::array<float, gradient::MaxStops> sZeroStopPositions = {};
			glCheck(glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, static_cast<GLsizei>(gradient::MaxStops), 1, 0, GL_RED, GL_FLOAT, &sZeroStopPositions[0]));
			glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, iGradientTextures->second));
			glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			glCheck(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			static const std::array<std::array<uint8_t, 4>, gradient::MaxStops> sZeroStopColours = {};
			glCheck(glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, static_cast<GLsizei>(gradient::MaxStops), 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &sZeroStopColours[0]));
			glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, previousTexture));
		}
		glCheck(glActiveTexture(GL_TEXTURE2));
		glCheck(glClientActiveTexture(GL_TEXTURE2));
		glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, iGradientTextures->first));
		glCheck(glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, iGradientStopPositions.size(), 1, GL_RED, GL_FLOAT, &iGradientStopPositions[0]));
		glCheck(glActiveTexture(GL_TEXTURE3));
		glCheck(glClientActiveTexture(GL_TEXTURE3));
		glCheck(glBindTexture(GL_TEXTURE_RECTANGLE, iGradientTextures->second));
		glCheck(glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, iGradientStopColours.size(), 1, GL_RGBA, GL_UNSIGNED_BYTE, &iGradientStopColours[0]));
		iRenderingEngine.gradient_shader_program().set_uniform_variable("texStopPositions", 2);
		iRenderingEngine.gradient_shader_program().set_uniform_variable("texStopColours", 3);
		glCheck(glActiveTexture(GL_TEXTURE1));
		glCheck(glClientActiveTexture(GL_TEXTURE1));
	}

	void opengl_graphics_context::gradient_off()
	{
		iRenderingEngine.deactivate_shader_program();
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
		/* todo */
		(void)aPoint;
		(void)aColour;
	}

	void opengl_graphics_context::draw_pixel(const point& aPoint, const colour& aColour)
	{
		/* todo */
		(void)aPoint;
		(void)aColour;
	}

	void opengl_graphics_context::draw_line(const point& aFrom, const point& aTo, const pen& aPen)
	{
		double pixelAdjust = pixel_adjust(aPen);
		std::vector<double> vertices{aFrom.x + pixelAdjust, aFrom.y + pixelAdjust, aTo.x + pixelAdjust, aTo.y + pixelAdjust };
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_LINES, 0, vertices.size() / 2));
		glCheck(glLineWidth(1.0f));
	}

	void opengl_graphics_context::draw_rect(const rect& aRect, const pen& aPen)
	{
		auto vertices = rect_vertices(aRect, pixel_adjust(aPen), false);
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{ {aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glDrawArrays(GL_LINES, 0, vertices.size() / 2));
		glCheck(glLineWidth(1.0f));
	}

	void opengl_graphics_context::draw_rounded_rect(const rect& aRect, dimension aRadius, const pen& aPen)
	{
		double pixelAdjust = pixel_adjust(aPen);
		auto vertices = rounded_rect_vertices(aRect + point{ pixelAdjust, pixelAdjust }, aRadius, false);
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{ {aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glDrawArrays(GL_LINE_LOOP, 0, vertices.size() / 2));
		glCheck(glLineWidth(1.0f));
	}

	void opengl_graphics_context::draw_circle(const point& aCentre, dimension aRadius, const pen& aPen)
	{
		auto vertices = circle_vertices(aCentre, aRadius, false);
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_LINE_LOOP, 0, vertices.size() / 2));
		glCheck(glLineWidth(1.0f));
	}

	void opengl_graphics_context::draw_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const pen& aPen)
	{
		auto vertices = line_loop_to_lines(arc_vertices(aCentre, aRadius, aStartAngle, aEndAngle, false));
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{ {aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_LINES, 0, vertices.size() / 2));
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
				auto vertices = aPath.to_vertices(aPath.paths()[i]);
				std::vector<double> texCoords(vertices.size(), 0.0);
				std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
				glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
				glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
				glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
				glCheck(glDrawArrays(path_shape_to_gl_mode(aPath.shape()), 0, vertices.size() / 2));
				if (aPath.shape() == path::ConvexPolygon)
					reset_clip();
			}
		}
	}

	void opengl_graphics_context::fill_rect(const rect& aRect, const colour& aColour)
	{
		if (aRect.empty())
			return;
		auto vertices = rect_vertices(aRect, 0.0, true);
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{aColour.red(), aColour.green(), aColour.blue(), aColour.alpha()}});
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
	}

	void opengl_graphics_context::fill_rect(const rect& aRect, const gradient& aGradient)
	{
		if (aRect.empty())
			return;
		gradient_on(aGradient, aRect);
		auto vertices = rect_vertices(aRect, 0.0, true);
		std::vector<std::array<uint8_t, 4>> colours{ vertices.size() / 2, { { 0, 0, 0, 0 } } };
		std::vector<double> texCoords(vertices.size(), 0.0);
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
		gradient_off();
	}

	void opengl_graphics_context::fill_rounded_rect(const rect& aRect, dimension aRadius, const colour& aColour)
	{
		if (aRect.empty())
			return;
		auto vertices = rounded_rect_vertices(aRect, aRadius, true);
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{ {aColour.red(), aColour.green(), aColour.blue(), aColour.alpha()}});
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
	}

	void opengl_graphics_context::fill_rounded_rect(const rect& aRect, dimension aRadius, const gradient& aGradient)
	{
		if (aRect.empty())
			return;
		gradient_on(aGradient, aRect);
		auto vertices = rounded_rect_vertices(aRect, aRadius, true);
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours{ vertices.size() / 2,{ { 0, 0, 0, 0 } } };
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
		gradient_off();
	}

	void opengl_graphics_context::fill_circle(const point& aCentre, dimension aRadius, const colour& aColour)
	{
		auto vertices = circle_vertices(aCentre, aRadius, true);
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{aColour.red(), aColour.green(), aColour.blue(), aColour.alpha()}});
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
	}

	void opengl_graphics_context::fill_arc(const point& aCentre, dimension aRadius, angle aStartAngle, angle aEndAngle, const colour& aColour)
	{
		auto vertices = arc_vertices(aCentre, aRadius, aStartAngle, aEndAngle, true);
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{ {aColour.red(), aColour.green(), aColour.blue(), aColour.alpha()}});
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
	}

	void opengl_graphics_context::fill_shape(const point& aCentre, const vertex_list2& aVertices, const colour& aColour)
	{
		vertex_list2 vertices;
		vertices.reserve(aVertices.size() + 2);
		vertices.push_back(aCentre.to_vector());
		vertices.insert(vertices.end(), aVertices.begin(), aVertices.end());
		vertices.push_back(vertices[1]);
		std::vector<double> texCoords(vertices.size() * 2, 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size(), std::array <uint8_t, 4>{ {aColour.red(), aColour.green(), aColour.blue(), aColour.alpha()}});
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size()));
	}

	void opengl_graphics_context::fill_and_draw_path(const path& aPath, const colour& aFillColour, const pen& aPen)
	{
		for (std::size_t i = 0; i < aPath.paths().size(); ++i)
		{
			if (aPath.paths()[i].size() > 2)
			{
				clip_to(aPath, 0.0);
				auto vertices = aPath.to_vertices(aPath.paths()[i]);
				std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{aFillColour.red(), aFillColour.green(), aFillColour.blue(), aFillColour.alpha()}});
				glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
				glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
				std::vector<double> texCoords(vertices.size(), 0.0);
				glCheck(glDrawArrays(path_shape_to_gl_mode(aPath.shape()), 0, vertices.size() / 2));
				reset_clip();
			}
		}
		if (aPen.width() != 0.0)
			draw_path(aPath, aPen);
	}

	glyph_text opengl_graphics_context::to_glyph_text(string::const_iterator aTextBegin, string::const_iterator aTextEnd, const font& aFont) const
	{
		return to_glyph_text(aTextBegin, aTextEnd, [&aFont](std::string::size_type) { return aFont; });
	}

	glyph_text opengl_graphics_context::to_glyph_text(string::const_iterator aTextBegin, string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const
	{
		return glyph_text(aFontSelector(0), to_glyph_text_impl(aTextBegin, aTextEnd, aFontSelector));
	}

	void opengl_graphics_context::set_mnemonic(bool aShowMnemonics, char aMnemonicPrefix)
	{
		iMnemonic = std::make_pair(aShowMnemonics, aMnemonicPrefix);
	}

	void opengl_graphics_context::unset_mnemonic()
	{
		iMnemonic = boost::none;
	}

	bool opengl_graphics_context::mnemonics_shown() const
	{
		return iMnemonic != boost::none && iMnemonic->first;
	}

	void opengl_graphics_context::begin_drawing_glyphs()
	{
		glCheck(glTextureBarrierNV());
		glCheck(glActiveTexture(GL_TEXTURE1));
		glCheck(glClientActiveTexture(GL_TEXTURE1));
		glCheck(glEnable(GL_TEXTURE_2D));
		glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousTexture));
		iActiveGlyphTexture = static_cast<GLuint>(iPreviousTexture);
		iRenderingEngine.activate_shader_program(iRenderingEngine.glyph_shader_program(is_subpixel_rendering_on()));
	}

	namespace
	{
		std::vector<double> texture_vertices(const size& aTextureStorageSize, const rect& aTextureRect, const vec4& aLogicalCoordinates)
		{
			std::vector<double> result;
			rect normalizedRect = aTextureRect / aTextureStorageSize;
			result.push_back(normalizedRect.top_left().x);
			result.push_back(normalizedRect.top_left().y);
			result.push_back(normalizedRect.top_right().x);
			result.push_back(normalizedRect.top_right().y);
			result.push_back(normalizedRect.bottom_right().x);
			result.push_back(normalizedRect.bottom_right().y);
			result.push_back(normalizedRect.bottom_left().x);
			result.push_back(normalizedRect.bottom_left().y);
			if (aLogicalCoordinates[1] < aLogicalCoordinates[3])
			{
				std::swap(result[1], result[5]);
				std::swap(result[3], result[7]);
			}
			return result;
		}
	}

	size opengl_graphics_context::draw_glyph(const point& aPoint, const glyph& aGlyph, const font& aFont, const colour& aColour)
	{
		if (aGlyph.is_whitespace())
			return size{};

		if (&iRenderingEngine.active_shader_program() != &iRenderingEngine.glyph_shader_program(aGlyph.subpixel()))
			iRenderingEngine.activate_shader_program(iRenderingEngine.glyph_shader_program(aGlyph.subpixel()));

		const i_glyph_texture& glyphTexture = !aGlyph.use_fallback() ? aFont.native_font_face().glyph_texture(aGlyph) : aFont.fallback().native_font_face().glyph_texture(aGlyph);

		auto& vertices = iGlyphVertices;
		auto& colours = iGlyphColours;
		auto& textureCoords = iGlyphTextureCoords;

		point glyphOrigin(aPoint.x + glyphTexture.placement().x, 
			logical_coordinates()[1] < logical_coordinates()[3] ? 
				aPoint.y + (glyphTexture.placement().y + -aFont.descender()) :
				aPoint.y + aFont.height() - (glyphTexture.placement().y + -aFont.descender()) - glyphTexture.extents().cy);

		vertices.clear();
		vertices.insert(vertices.begin(),
		{
			to_shader_vertex(glyphOrigin),
			to_shader_vertex(glyphOrigin + point{ glyphTexture.extents().cx, 0.0 }),
			to_shader_vertex(glyphOrigin + point{ glyphTexture.extents().cx, glyphTexture.extents().cy }),
			to_shader_vertex(glyphOrigin + point{ 0.0, glyphTexture.extents().cy })
		});

		colours.clear();
		colours.resize(vertices.size(), std::array<double, 4>{{aColour.red<double>(), aColour.green<double>(), aColour.blue<double>(), aColour.alpha<double>()}});

		textureCoords = texture_vertices(glyphTexture.font_texture().extents(), rect{ glyphTexture.font_texture_location(), glyphTexture.extents() }, logical_coordinates());

		/* todo: cache VBOs and use glBufferSubData(). */

		GLuint boHandles[3];
		glCheck(glGenBuffers(3, boHandles));

		GLuint positionBufferHandle = boHandles[0];
		GLuint colourBufferHandle = boHandles[1];
		GLuint textureCoordBufferHandle = boHandles[2];

		GLint previousVertexArrayBinding;
		glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVertexArrayBinding));

		GLint previousArrayBufferBinding;
		glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousArrayBufferBinding));

		glCheck(glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle));
		glCheck(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW));
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, colourBufferHandle));
		glCheck(glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(colours[0]), &colours[0], GL_STATIC_DRAW));
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, textureCoordBufferHandle));
		glCheck(glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(textureCoords[0]), &textureCoords[0], GL_STATIC_DRAW));

		GLuint vaoHandle;
		glCheck(glGenVertexArrays(1, &vaoHandle));
		glCheck(glBindVertexArray(vaoHandle));

		GLuint vertexPositionAttribArrayIndex = reinterpret_cast<GLuint>(iRenderingEngine.glyph_shader_program(aGlyph.subpixel()).variable("VertexPosition"));
		glCheck(glEnableVertexAttribArray(vertexPositionAttribArrayIndex));
		GLuint vertexColorAttribArrayIndex = reinterpret_cast<GLuint>(iRenderingEngine.glyph_shader_program(aGlyph.subpixel()).variable("VertexColor"));
		glCheck(glEnableVertexAttribArray(vertexColorAttribArrayIndex));
		GLuint vertexTextureCoordAttribArrayIndex = reinterpret_cast<GLuint>(iRenderingEngine.glyph_shader_program(aGlyph.subpixel()).variable("VertexTextureCoord"));
		glCheck(glEnableVertexAttribArray(vertexTextureCoordAttribArrayIndex));

		glCheck(glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle));
		glCheck(glVertexAttribPointer(vertexPositionAttribArrayIndex, 3, GL_DOUBLE, GL_FALSE, 0, 0));
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, colourBufferHandle));
		glCheck(glVertexAttribPointer(vertexColorAttribArrayIndex, 4, GL_DOUBLE, GL_FALSE, 0, 0));
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, textureCoordBufferHandle));
		glCheck(glVertexAttribPointer(vertexTextureCoordAttribArrayIndex, 2, GL_DOUBLE, GL_FALSE, 0, 0));

		glCheck(glBindVertexArray(vaoHandle));

		if (iActiveGlyphTexture != reinterpret_cast<GLuint>(glyphTexture.font_texture().handle()))
		{
			iActiveGlyphTexture = reinterpret_cast<GLuint>(glyphTexture.font_texture().handle());
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
			iRenderingEngine.glyph_shader_program(aGlyph.subpixel()).set_uniform_variable("guiCoordinates", logical_coordinates()[1] > logical_coordinates()[3]);
			iRenderingEngine.glyph_shader_program(aGlyph.subpixel()).set_uniform_variable("outputExtents", static_cast<float>(iSurface.surface_size().cx), static_cast<float>(iSurface.surface_size().cy));
			iRenderingEngine.glyph_shader_program(aGlyph.subpixel()).set_uniform_variable("outputTexture", 2);
		}

		glCheck(glEnable(GL_BLEND));
		glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		disable_anti_alias daa(*this);
		glCheck(glDrawArrays(GL_QUADS, 0, vertices.size()));

		glCheck(glBindVertexArray(previousVertexArrayBinding));

		glCheck(glBindBuffer(GL_ARRAY_BUFFER, previousArrayBufferBinding));

		glCheck(glDeleteVertexArrays(1, &vaoHandle));

		glCheck(glDeleteBuffers(3, boHandles));

		return glyphTexture.extents();
	}

	void opengl_graphics_context::end_drawing_glyphs()
	{
		glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(iPreviousTexture)));
		iRenderingEngine.deactivate_shader_program();
	}

	void opengl_graphics_context::draw_texture(const texture_map& aTextureMap, const i_texture& aTexture, const rect& aTextureRect, const optional_colour& aColour)
	{
		if (aTexture.is_empty())
			return;
		glCheck(glActiveTexture(GL_TEXTURE1));
		glCheck(glClientActiveTexture(GL_TEXTURE1));
		glCheck(glEnable(GL_TEXTURE_2D));
		glCheck(glEnable(GL_BLEND));
		glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLint previousTexture;
		glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture));
		glCheck(glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLuint>(aTexture.native_texture()->handle())));
		if (!aTexture.native_texture()->is_resident())
			throw texture_not_resident();
		auto texCoords = texture_vertices(aTexture.storage_extents(), aTextureRect + point{1.0, 1.0}, logical_coordinates());
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &aTextureMap[0][0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		colour c{0xFF, 0xFF, 0xFF, 0xFF};
		if (aColour != boost::none)
			c = *aColour;
		std::vector<std::array<uint8_t, 4>> colours(4, std::array<uint8_t, 4>{{c.red(), c.green(), c.blue(), c.alpha()}});
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		if (iMonochrome)
		{
			iRenderingEngine.activate_shader_program(iRenderingEngine.monochrome_shader_program());
			iRenderingEngine.monochrome_shader_program().set_uniform_variable("tex", 1);
		}
		glCheck(glDrawArrays(GL_QUADS, 0, 4));
		if (iMonochrome)
			iRenderingEngine.deactivate_shader_program();
		glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture)));
	}

	opengl_graphics_context::vertex opengl_graphics_context::to_shader_vertex(const point& aPoint) const
	{
		return vertex{{aPoint.x, aPoint.y, 0.0}};
	}

	glyph_text::container opengl_graphics_context::to_glyph_text_impl(string::const_iterator aTextBegin, string::const_iterator aTextEnd, std::function<font(std::string::size_type)> aFontSelector) const
	{
		glyph_text::container result;
		if (aTextEnd - aTextBegin == 0)
			return result;

		auto& clusterMap = iClusterMap;
		clusterMap.clear();
		auto& textDirections = iTextDirections;
		textDirections.clear();

		iCodePointsBuffer.clear();
		std::u32string& codePoints = iCodePointsBuffer;

		codePoints = neolib::utf8_to_utf32(aTextBegin, aTextEnd, [&clusterMap](std::string::size_type aFrom, std::u32string::size_type)
		{
			clusterMap.push_back(cluster{aFrom});
		});
		
		if (iMnemonic != boost::none)
		{
			for (auto i = codePoints.begin(); i != codePoints.end();)
			{
				if (*i == static_cast<char32_t>(iMnemonic->second))
				{
					clusterMap.erase(clusterMap.begin() + (i - codePoints.begin()));
					i = codePoints.erase(i);
					if (i != codePoints.end())
					{
						auto& cluster = *(clusterMap.begin() + (i - codePoints.begin()));
						if (*i != static_cast<char32_t>(iMnemonic->second))
							cluster.flags = glyph::Mnemonic;
					}
				}
				else
					++i;
			}
		}

		if (codePoints.empty())
			return result;
		
		auto& runs = iRuns;
		runs.clear();
		text_direction previousDirection = get_text_direction(codePoints[0]);
		char32_t* runStart = &codePoints[0];
		std::size_t lastCodePointIndex = codePoints.size() - 1;
		font previousFont = aFontSelector(clusterMap[0].from);
		hb_script_t previousScript = hb_unicode_script(static_cast<native_font_face::hb_handle*>(previousFont.native_font_face().aux_handle())->unicodeFuncs, codePoints[0]);

		std::deque<std::pair<text_direction, bool>> directionStack;
		const char32_t LRE = U'\u202A';
		const char32_t RLE = U'\u202B';
		const char32_t LRO = U'\u202D';
		const char32_t RLO = U'\u202E';
		const char32_t PDF = U'\u202C';

		bool currentLineHasLTR = false;

		for (std::size_t i = 0; i <= lastCodePointIndex; ++i)
		{
			font currentFont = aFontSelector(clusterMap[i].from);
			if (currentFont.password())
				codePoints[i] = neolib::utf8_to_utf32(currentFont.password_mask())[0];
			if (codePoints[i] == '\r' || codePoints[i] == '\n')
				currentLineHasLTR = false;
			switch (codePoints[i])
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
			text_direction currentDirection = get_text_direction(codePoints[i]);
			textDirections.push_back(currentDirection);
			auto bidi_check = [&directionStack](text_direction aDirection)
			{
				if (!directionStack.empty())
				{
					switch (aDirection)
					{
					case text_direction::LTR:
					case text_direction::RTL:
						if (directionStack.back().second == true)
							return directionStack.back().first;
						break;
					case text_direction::None:
					case text_direction::Whitespace:
						return directionStack.back().first;
						break;
					default:
						break;
					}
				}
				return aDirection;
			};
			currentDirection = bidi_check(currentDirection);
			if (currentDirection == text_direction::LTR)
				currentLineHasLTR = true;
			hb_script_t currentScript = hb_unicode_script(unicodeFuncs, codePoints[i]);
			bool newRun = 
				previousFont != currentFont || 
				(previousDirection == text_direction::LTR && currentDirection == text_direction::RTL) ||
				(previousDirection == text_direction::RTL && currentDirection == text_direction::LTR) ||
				(previousScript != currentScript && (previousScript != HB_SCRIPT_COMMON && currentScript != HB_SCRIPT_COMMON));
			if (!newRun)
			{
				if ((currentDirection == text_direction::Whitespace || currentDirection == text_direction::None) && previousDirection == text_direction::RTL)
				{
					for (std::size_t j = i + 1; j <= lastCodePointIndex; ++j)
					{
						text_direction nextDirection = bidi_check(get_text_direction(codePoints[j]));
						if (nextDirection == text_direction::RTL)
							break;
						else if (nextDirection == text_direction::LTR || (j == lastCodePointIndex - 1 && currentLineHasLTR))
						{
							newRun = true;
							currentDirection = text_direction::LTR;
							break;
						}
					}
				}
			}
			if (newRun)
			{
				runs.push_back(std::make_tuple(runStart, &codePoints[i], previousDirection, previousScript));
				runStart = &codePoints[i];
			}
			if (i == lastCodePointIndex)
				runs.push_back(std::make_tuple(runStart, &codePoints[i + 1], currentDirection, currentScript));
			if (currentDirection == text_direction::LTR || currentDirection == text_direction::RTL)
			{
				previousDirection = currentDirection;
				previousScript = currentScript;
			}
			previousFont = currentFont;
		}

		for (std::size_t i = 0; i < runs.size(); ++i)
		{
			std::string::size_type sourceClusterRunStart = (clusterMap.begin() + (std::get<0>(runs[i]) - &codePoints[0]))->from;
			font tryFont = aFontSelector(sourceClusterRunStart);
			bool usingFallback = false;
			bool glyphNotFound = false;
			do
			{
				glyphNotFound = false;
				hb_font_t* hbFont = static_cast<native_font_face::hb_handle*>(tryFont.native_font_face().aux_handle())->font;
				hb_ft_font_set_load_flags(hbFont, is_subpixel_rendering_on() ? FT_LOAD_TARGET_LCD : FT_LOAD_TARGET_NORMAL);
				hb_buffer_t* buf = static_cast<native_font_face::hb_handle*>(tryFont.native_font_face().aux_handle())->buf;
				hb_buffer_set_direction(buf, std::get<2>(runs[i]) == text_direction::RTL ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
				hb_buffer_set_script(buf, std::get<3>(runs[i]));
				hb_buffer_add_utf32(buf, reinterpret_cast<const uint32_t*>(std::get<0>(runs[i])), std::get<1>(runs[i]) - std::get<0>(runs[i]), 0, std::get<1>(runs[i]) - std::get<0>(runs[i]));
				hb_shape(hbFont, buf, NULL, 0);
				unsigned int glyphCount;
				hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(buf, &glyphCount);
				hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(buf, &glyphCount);
				for (unsigned int j = 0; j < glyphCount; ++j)
				{
					std::u32string::size_type cluster = glyphInfo[j].cluster + (std::get<0>(runs[i]) - &codePoints[0]);
					if (glyphInfo[j].codepoint == 0)
						glyphInfo[j].codepoint = tryFont.native_font_face().glyph_index(codePoints[cluster]);
					if (glyphInfo[j].codepoint == 0)
					{
						// todo: improve on this naive font selection method (OS may have multiple fallback fonts)
						auto currentFont = tryFont;
						tryFont = tryFont.fallback();
						if (tryFont != currentFont)
						{
							usingFallback = true;
							glyphNotFound = true;
							break;
						}
					}
				}
				if (!glyphNotFound)
				{
					for (unsigned int j = 0; j < glyphCount; ++j)
					{
						std::u32string::size_type cluster = glyphInfo[j].cluster + (std::get<0>(runs[i]) - &codePoints[0]);
						std::string::size_type sourceClusterStart, sourceClusterEnd;
						auto c = clusterMap.begin() + cluster;
						sourceClusterStart = c->from;
						if (c + 1 != clusterMap.end())
							sourceClusterEnd = (c + 1)->from;
						else
							sourceClusterEnd = aTextEnd - aTextBegin;
						if (j > 0)
							result.back().kerning_adjust(static_cast<float>(aFontSelector(sourceClusterStart).kerning(glyphInfo[j - 1].codepoint, glyphInfo[j].codepoint)));
						size advance{ glyphPos[j].x_advance / 64.0, glyphPos[j].y_advance / 64.0 };
						result.push_back(glyph(textDirections[cluster], glyphInfo[j].codepoint, glyph::source_type(sourceClusterStart, sourceClusterEnd), advance, size(glyphPos[j].x_offset / 64.0, glyphPos[j].y_offset / 64.0)));
						if (result.back().direction() == text_direction::Whitespace)
							result.back().set_value(aTextBegin[sourceClusterStart]);
						if ((aFontSelector(sourceClusterStart).style() & font::Underline) == font::Underline)
							result.back().set_underline(true);
						if (is_subpixel_rendering_on())
							result.back().set_subpixel(true);
						if ((c->flags & glyph::Mnemonic) == glyph::Mnemonic)
							result.back().set_mnemonic(true);
						if (usingFallback)
							result.back().set_use_fallback(true);
						if (result.back().direction() != text_direction::Whitespace)
						{
							auto& glyph = result.back();
							if (glyph.advance() != advance.ceil())
							{
								const i_glyph_texture& glyphTexture = aFontSelector(sourceClusterStart).native_font_face().glyph_texture(glyph);
								auto visibleAdvance = std::ceil(glyph.offset().cx + glyphTexture.placement().x + glyphTexture.extents().cx);
								if (visibleAdvance > advance.cx)
								{
									advance.cx = visibleAdvance;
									glyph.set_advance(advance);
								}
							}
						}
					}
				}
				hb_buffer_clear_contents(buf);
			} while (glyphNotFound);
		}

		return result;
	}
}