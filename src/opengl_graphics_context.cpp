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

#include "neogfx.hpp"
#include "neogfx.hpp"
#include "text.hpp"
#include "i_rendering_engine.hpp"
#include "opengl_graphics_context.hpp"
#include "text_direction_map.hpp"
#include "i_native_font_face.hpp"
#include "native_font_face.hpp"
#include "i_font_texture.hpp"

namespace neogfx
{
	opengl_graphics_context::opengl_graphics_context(i_rendering_engine& aRenderingEngine) :
		iRenderingEngine(aRenderingEngine), iSmoothingMode(SmoothingModeNone), iClipCounter(0)
	{
		set_smoothing_mode(SmoothingModeAntiAlias);
	}
	opengl_graphics_context::opengl_graphics_context(const opengl_graphics_context& aOther) :
		iRenderingEngine(aOther.iRenderingEngine), iSmoothingMode(aOther.iSmoothingMode), iClipCounter(0)
	{
		set_smoothing_mode(iSmoothingMode);
	}

	opengl_graphics_context::~opengl_graphics_context()
	{
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
		glCheck(glStencilMask(-1));
		glCheck(glStencilFunc(GL_NEVER, 0, -1));
		fill_solid_rect(rendering_area(), colour::White);
		glCheck(glStencilFunc(GL_NEVER, 1, -1));
		fill_solid_rect(aRect, colour::White);
		glCheck(glStencilFunc(GL_NEVER, 1, -1));
		glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		glCheck(glDepthMask(GL_TRUE));
		glCheck(glStencilMask(0x00));
		// draw only where stencil's value is 1
		glCheck(glStencilFunc(GL_EQUAL, 1, -1));
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
		glCheck(glStencilMask(-1));
		glCheck(glStencilFunc(GL_NEVER, 0, -1));
		fill_solid_rect(rendering_area(), colour::White);
		glCheck(glStencilFunc(GL_EQUAL, 1, -1));
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
				glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
			}
		}
		if (aPathOutline != 0)
		{
			glCheck(glStencilFunc(GL_NEVER, 0, -1));
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
					glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
				}
			}
		} 
		glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		glCheck(glDepthMask(GL_TRUE));
		glCheck(glStencilMask(0x00));
		// draw only where stencil's value is 1
		glCheck(glStencilFunc(GL_EQUAL, 1, -1));
	}

	void opengl_graphics_context::reset_clip()
	{
		if (--iClipCounter == 0)
		{
			glCheck(glDisable(GL_STENCIL_TEST));
		}
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
			glCheck(glEnable(GL_MULTISAMPLE));
		}
		else
		{
			glCheck(glDisable(GL_LINE_SMOOTH));
			glCheck(glDisable(GL_POLYGON_SMOOTH));
			glCheck(glDisable(GL_MULTISAMPLE));
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
			glCheck(glDisable(GL_LOGIC_OP));
		}
		else
		{
			glCheck(glEnable(GL_LOGIC_OP));
			switch (iLogicalOperationStack.back())
			{
			case LogicalXor:
				glCheck(glLogicOp(GL_XOR));
				break;
			}
		}	
	}

	void opengl_graphics_context::line_stipple_on(uint32_t aFactor, uint16_t aPattern)
	{
		glCheck(glEnable(GL_LINE_STIPPLE));
		glCheck(glLineStipple(static_cast<GLint>(aFactor), static_cast<GLushort>(aPattern)));
	}

	void opengl_graphics_context::line_stipple_off()
	{
		glCheck(glDisable(GL_LINE_STIPPLE));
	}

	void opengl_graphics_context::clear(const colour& aColour)
	{
		disable_anti_alias daa(*this);
		fill_solid_rect(rendering_area(), aColour);
	}

	void opengl_graphics_context::set_pixel(const point& aPoint, const colour& aColour)
	{
	}

	void opengl_graphics_context::draw_pixel(const point& aPoint, const colour& aColour)
	{
	}

	void opengl_graphics_context::draw_line(const point& aFrom, const point& aTo, const pen& aPen)
	{
		std::vector<double> vertices{aFrom.x, aFrom.y, aTo.x, aTo.y};
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
		path rectPath(aRect);
		auto vertices = rectPath.to_vertices(rectPath.paths()[0], path::LineLoop);
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
		glCheck(glLineWidth(static_cast<GLfloat>(aPen.width())));
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_LINE_LOOP, 0, vertices.size() / 2));
		glCheck(glLineWidth(1.0f));
	}

	void opengl_graphics_context::draw_path(const path& aPath, const pen& aPen)
	{
		for (std::size_t i = 0; i < aPath.paths().size(); ++i)
		{
			if (aPath.paths()[i].size() > 2)
			{
				clip_to(aPath, aPen.width());
				auto vertices = aPath.to_vertices(aPath.paths()[i]);
				std::vector<double> texCoords(vertices.size(), 0.0);
				std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{aPen.colour().red(), aPen.colour().green(), aPen.colour().blue(), aPen.colour().alpha()}});
				glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
				glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
				glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
				glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
				reset_clip();
			}
		}
	}

	void opengl_graphics_context::fill_solid_rect(const rect& aRect, const colour& aColour)
	{
		path rectPath(aRect);
		auto vertices = rectPath.to_vertices(rectPath.paths()[0]);
		std::vector<double> texCoords(vertices.size(), 0.0);
		std::vector<std::array<uint8_t, 4>> colours(vertices.size() / 2, std::array <uint8_t, 4>{{aColour.red(), aColour.green(), aColour.blue(), aColour.alpha()}});
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
	}

	void opengl_graphics_context::fill_gradient_rect(const rect& aRect, const gradient& aGradient)
	{
		if (aRect.empty())
			return;
		path rectPath(aRect);
		auto vertices = rectPath.to_vertices(rectPath.paths()[0]);
		std::vector<std::array<uint8_t, 4>> colours;
		if (aGradient.direction() == gradient::Vertical)
		{
			for (auto& v : vertices)
			{
				if ((&v - &vertices[0]) % 2 == 1)
				{
					colour c = aGradient.at(v, aRect.top(), aRect.bottom());
					colours.push_back(std::array < uint8_t, 4 > {{c.red(), c.green(), c.blue(), c.alpha()}});
				}
			}
		}
		else if (aGradient.direction() == gradient::Horizontal)
		{
			for (auto& v : vertices)
			{
				if ((&v - &vertices[0]) % 2 == 0)
				{
					colour c = aGradient.at(v, aRect.left(), aRect.right());
					colours.push_back(std::array < uint8_t, 4 > {{c.red(), c.green(), c.blue(), c.alpha()}});
				}
			}
		}
		std::vector<double> texCoords(vertices.size(), 0.0);
		glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours[0]));
		glCheck(glVertexPointer(2, GL_DOUBLE, 0, &vertices[0]));
		glCheck(glTexCoordPointer(2, GL_DOUBLE, 0, &texCoords[0]));
		glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
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
				glCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 2));
				reset_clip();
			}
		}
		if (aPen.width() != 0.0)
			draw_path(aPath, aPen);
	}

	glyph_text opengl_graphics_context::to_glyph_text(text::const_iterator aTextBegin, text::const_iterator aTextEnd, const font& aFont) const
	{
		bool fallbackNeeded = false;
		glyph_text::container result = to_glyph_text_impl(aTextBegin, aTextEnd, aFont, fallbackNeeded);
		if (fallbackNeeded)
		{
			glyph_text::container fallbackResult = to_glyph_text_impl(aTextBegin, aTextEnd, aFont.fallback(), fallbackNeeded);
			for (auto i = result.begin(), j = fallbackResult.begin(); i != result.end(); ++i, ++j)
			{
				if (i->use_fallback())
				{
					*i = *j;
					i->set_use_fallback(true);
				}
			}
		}
		return glyph_text(aFont, std::move(result));
	}

	void opengl_graphics_context::begin_drawing_glyphs()
	{
		iRenderingEngine.activate_shader_program(iRenderingEngine.subpixel_shader_program());

		glCheck(glActiveTexture(GL_TEXTURE1));
		glCheck(glClientActiveTexture(GL_TEXTURE1));
		glCheck(glEnable(GL_TEXTURE_2D));
		glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousTexture));
		iActiveGlyphTexture = static_cast<GLuint>(iPreviousTexture);
	}

	void opengl_graphics_context::draw_glyph(const point& aPoint, const glyph& aGlyph, const font& aFont, const colour& aColour)
	{
		if (aGlyph.is_whitespace())
			return;

		const i_glyph_texture& glyphTexture = !aGlyph.use_fallback() ? aFont.native_font_face().glyph_texture(aGlyph) : aFont.fallback().native_font_face().glyph_texture(aGlyph);

		auto& vertices = iVertices;
		auto& colours = iColours;
		auto& textureCoords = iTextureCoords;
		textureCoords.resize(8);
		auto& shifts = iShifts;
		shifts.clear();
		shifts.resize(4, std::array<GLdouble, 2>{{aPoint.x - std::floor(aPoint.x), aPoint.y - std::floor(aPoint.y)}});

		point glyphOrigin(aPoint.x + glyphTexture.placement().x, aPoint.y + glyphTexture.placement().y);
		vertices.clear();
		vertices.insert(vertices.begin(),
		{
			to_shader_vertex(glyphOrigin),
			to_shader_vertex(glyphOrigin + point(0.0, glyphTexture.extents().cy)),
			to_shader_vertex(glyphOrigin + point(glyphTexture.extents().cx, glyphTexture.extents().cy)),
			to_shader_vertex(glyphOrigin + point(glyphTexture.extents().cx, 0.0))
		});

		colours.clear();
		colours.resize(vertices.size(), std::array<double, 4>{{aColour.red<double>(), aColour.green<double>(), aColour.blue<double>(), aColour.alpha<double>()}});

		bool lcdMode = iRenderingEngine.screen_metrics().subpixel_format() == i_screen_metrics::SubpixelFormatRGBHorizontal ||
			iRenderingEngine.screen_metrics().subpixel_format() == i_screen_metrics::SubpixelFormatBGRHorizontal;

		textureCoords[0] = glyphTexture.font_texture_location().x / glyphTexture.font_texture().extents().cx;
		textureCoords[1] = glyphTexture.font_texture_location().y / glyphTexture.font_texture().extents().cy;
		textureCoords[2] = glyphTexture.font_texture_location().x / glyphTexture.font_texture().extents().cx;
		textureCoords[3] = (glyphTexture.font_texture_location().y + glyphTexture.extents().cy) / glyphTexture.font_texture().extents().cy;
		textureCoords[4] = (glyphTexture.font_texture_location().x + glyphTexture.extents().cx * (lcdMode ? 3.0 : 1.0)) / glyphTexture.font_texture().extents().cx;
		textureCoords[5] = (glyphTexture.font_texture_location().y + glyphTexture.extents().cy) / glyphTexture.font_texture().extents().cy;
		textureCoords[6] = (glyphTexture.font_texture_location().x + glyphTexture.extents().cx * (lcdMode ? 3.0 : 1.0)) / glyphTexture.font_texture().extents().cx;
		textureCoords[7] = glyphTexture.font_texture_location().y / glyphTexture.font_texture().extents().cy;

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

		GLuint vertexPositionAttribArrayIndex = reinterpret_cast<GLuint>(iRenderingEngine.subpixel_shader_program().variable("VertexPosition"));
		glCheck(glEnableVertexAttribArray(vertexPositionAttribArrayIndex));
		GLuint vertexColorAttribArrayIndex = reinterpret_cast<GLuint>(iRenderingEngine.subpixel_shader_program().variable("VertexColor"));
		glCheck(glEnableVertexAttribArray(vertexColorAttribArrayIndex));
		GLuint vertexTextureCoordAttribArrayIndex = reinterpret_cast<GLuint>(iRenderingEngine.subpixel_shader_program().variable("VertexTextureCoord"));
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
		
		iRenderingEngine.subpixel_shader_program().set_uniform_variable("glyphTexture", 1);
		iRenderingEngine.subpixel_shader_program().set_uniform_variable("glyphTextureExtents", glyphTexture.font_texture().extents().cx, glyphTexture.font_texture().extents().cy);

		glCheck(glEnable(GL_BLEND));
		glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		disable_anti_alias daa(*this);
		glCheck(glDrawArrays(GL_QUADS, 0, vertices.size()));

		glCheck(glBindVertexArray(previousVertexArrayBinding));

		glCheck(glBindBuffer(GL_ARRAY_BUFFER, previousArrayBufferBinding));

		glCheck(glDeleteVertexArrays(1, &vaoHandle));

		glCheck(glDeleteBuffers(3, boHandles));
	}

	void opengl_graphics_context::end_drawing_glyphs()
	{
		glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(iPreviousTexture)));

		iRenderingEngine.deactivate_shader_program();
	}

	opengl_graphics_context::vertex opengl_graphics_context::to_shader_vertex(const point& aPoint) const
	{
		return vertex{{aPoint.x, aPoint.y, 0.0}};
	}

	glyph_text::container opengl_graphics_context::to_glyph_text_impl(text::const_iterator aTextBegin, text::const_iterator aTextEnd, const font& aFont, bool& aFallbackFontNeeded) const
	{
		glyph_text::container result;
		aFallbackFontNeeded = false;
		if (aTextEnd - aTextBegin == 0)
			return result;

		auto& clusterMap = iClusterMap;
		clusterMap.clear();
		auto& textDirections = iTextDirections;
		textDirections.clear();

		std::u32string codePoints = neolib::utf8_to_utf32(aTextBegin, aTextEnd, [&clusterMap](std::string::size_type aFrom, std::u32string::size_type aTo)
		{
			clusterMap.push_back(std::make_pair(aFrom, aTo));
		});
		if (codePoints.empty())
			return result;
		
		hb_font_t* hbFont = static_cast<native_font_face::hb_handle*>(aFont.native_font_face().aux_handle())->font;
		hb_buffer_t* buf = static_cast<native_font_face::hb_handle*>(aFont.native_font_face().aux_handle())->buf;
		hb_unicode_funcs_t* unicodeFuncs = static_cast<native_font_face::hb_handle*>(aFont.native_font_face().aux_handle())->unicodeFuncs;

		auto& runs = iRuns;
		runs.clear();
		text_direction previousDirection = get_text_direction(codePoints[0]);
		hb_script_t previousScript = hb_unicode_script(unicodeFuncs, codePoints[0]);
		uint32_t* runStart = &codePoints[0];
		std::size_t lastCodePointIndex = codePoints.size() - 1;
		for (std::size_t i = 0; i <= lastCodePointIndex; ++i)
		{
			text_direction currentDirection = get_text_direction(codePoints[i]);
			hb_script_t currentScript = hb_unicode_script(unicodeFuncs, codePoints[i]);
			textDirections.push_back(currentDirection);
			bool newRun = (previousDirection == text_direction::LTR && currentDirection == text_direction::RTL) ||
				(previousDirection == text_direction::RTL && currentDirection == text_direction::LTR) ||
				(previousScript != currentScript && (currentDirection == text_direction::LTR || currentDirection == text_direction::RTL)) ||
				i == lastCodePointIndex;
			if (!newRun && currentDirection == text_direction::Whitespace && previousDirection == text_direction::RTL)
			{
				for (std::size_t j = i + 1; j <= lastCodePointIndex; ++j)
				{
					text_direction nextDirection = get_text_direction(codePoints[j]);
					if (nextDirection == text_direction::RTL)
						break;
					else if (nextDirection == text_direction::LTR)
					{
						newRun = true;
						currentDirection = text_direction::LTR;
						break;
					}
				}
			}
			if (newRun)
			{
				runs.push_back(std::make_tuple(runStart, &codePoints[i != lastCodePointIndex ? i : i+1], previousDirection, previousScript));
				runStart = &codePoints[i];
			}
			if (currentDirection == text_direction::LTR || currentDirection == text_direction::RTL)
			{
				previousDirection = currentDirection;
				previousScript = currentScript;
			}
		}

		for (std::size_t i = 0; i < runs.size(); ++i)
		{
			hb_buffer_set_direction(buf, std::get<2>(runs[i]) == text_direction::RTL ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
			hb_buffer_set_script(buf, std::get<3>(runs[i]));
			hb_buffer_add_utf32(buf, std::get<0>(runs[i]), std::get<1>(runs[i]) - std::get<0>(runs[i]), 0, std::get<1>(runs[i]) - std::get<0>(runs[i]));
			hb_shape(hbFont, buf, NULL, 0);
			unsigned int glyphCount;
			hb_glyph_info_t* glyphInfo = hb_buffer_get_glyph_infos(buf, &glyphCount);
			hb_glyph_position_t* glyphPos = hb_buffer_get_glyph_positions(buf, &glyphCount);
			for (unsigned int j = 0; j < glyphCount; ++j)
			{
				if (glyphInfo[j].codepoint == 0)
					aFallbackFontNeeded = true;
				std::u32string::size_type cluster = glyphInfo[j].cluster + (std::get<0>(runs[i]) - &codePoints[0]);
				std::string::size_type sourceClusterStart, sourceClusterEnd;
				auto c = std::lower_bound(clusterMap.begin(), clusterMap.end(), cluster_map_t::value_type(0, cluster), [](const cluster_map_t::value_type& lhs, const cluster_map_t::value_type& rhs) { return lhs.second < rhs.second; });
				sourceClusterStart = c->first;
				if (c + 1 != clusterMap.end())
					sourceClusterEnd = (c + 1)->first;
				else
					sourceClusterEnd = aTextEnd - aTextBegin;
				result.push_back(glyph(textDirections[cluster], glyphInfo[j].codepoint, glyph::source_type(sourceClusterStart, sourceClusterEnd), size(glyphPos[j].x_advance / 64.0, glyphPos[j].y_advance / 64.0)));
				if (result.back().direction() == text_direction::Whitespace)
					result.back().set_value(aTextBegin[sourceClusterStart]);
				if (glyphInfo[j].codepoint == 0)
					result.back().set_use_fallback(true);
			}
			hb_buffer_clear_contents(buf);
		}

		return result;
	}
}