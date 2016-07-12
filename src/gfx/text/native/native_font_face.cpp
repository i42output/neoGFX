// native_font_face.cpp
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
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_LCD_FILTER_H
#include "../../native/opengl.hpp"
#include "native_font_face.hpp"
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>

namespace neogfx
{
	native_font_face::native_font_face(i_rendering_engine& aRenderingEngine, i_native_font& aFont, font::style_e aStyle, font::point_size aSize, neogfx::size aDpiResolution, FT_Face aHandle) :
		iRenderingEngine(aRenderingEngine), iFont(aFont), iStyle(aStyle), iStyleName(aHandle->style_name), iSize(aSize), iPixelDensityDpi(aDpiResolution), iHandle(aHandle), iHasKerning(!!FT_HAS_KERNING(iHandle))
	{
		FT_Set_Char_Size(iHandle, 0, static_cast<FT_F26Dot6>(aSize * 64), static_cast<FT_UInt>(iPixelDensityDpi.cx), static_cast<FT_UInt>(iPixelDensityDpi.cy));
		FT_Select_Charmap(iHandle, FT_ENCODING_UNICODE);
	}

	native_font_face::~native_font_face()
	{
		FT_Done_Face(iHandle);
	}

	i_native_font& native_font_face::native_font()
	{
		return iFont;
	}

	const std::string& native_font_face::family_name() const
	{
		return iFont.family_name();
	}

	font::style_e native_font_face::style() const
	{
		return iStyle;
	}

	const std::string& native_font_face::style_name() const
	{
		return iStyleName;
	}

	font::point_size native_font_face::size() const
	{
		return iSize;
	}

	dimension native_font_face::horizontal_dpi() const
	{
		return iPixelDensityDpi.cx;
	}

	dimension native_font_face::vertical_dpi() const
	{
		return iPixelDensityDpi.cy;
	}

	dimension native_font_face::height() const
	{
		return iHandle->size->metrics.height / 64.0;
	}

	dimension native_font_face::descender() const
	{
		return iHandle->size->metrics.descender / 64.0;
	}

	dimension native_font_face::underline_position() const
	{
		if (FT_IS_SCALABLE(iHandle))
			return iHandle->underline_position / 64.0;
		else
			return underline_thickness() / 2.0;
	}

	dimension native_font_face::underline_thickness() const
	{
		if (FT_IS_SCALABLE(iHandle))
			return iHandle->underline_thickness / 64.0;
		else
			return static_cast<dimension>(font_info::weight_from_style_name(iStyleName)) / static_cast<dimension>(font_info::WeightNormal);
	}

	dimension native_font_face::line_spacing() const
	{
		/* todo */
		return 0;
	}

	dimension native_font_face::kerning(uint32_t aLeftGlyphIndex, uint32_t aRightGlyphIndex) const
	{
		if (!iHasKerning)
			return 0.0;
		auto existing = iKerningTable.find(std::make_pair(aLeftGlyphIndex, aRightGlyphIndex));
		if (existing != iKerningTable.end())
			return existing->second;
		FT_Vector delta;
		FT_Get_Kerning(iHandle, aLeftGlyphIndex, aRightGlyphIndex, FT_KERNING_DEFAULT, &delta);
		return (iKerningTable[std::make_pair(aLeftGlyphIndex, aRightGlyphIndex)] = delta.x / 64.0);
	}

	i_native_font_face& native_font_face::fallback() const
	{
		if (iFallbackFont == 0)
			iFallbackFont = iRenderingEngine.font_manager().create_fallback_font(*this);
		return *iFallbackFont;
	}
	
	void* native_font_face::handle() const
	{
		return iHandle;
	}

	void* native_font_face::aux_handle() const
	{
		if (iAuxHandle == nullptr)
			iAuxHandle = std::make_unique<hb_handle>(iHandle);
		return &*iAuxHandle;
	}

	uint32_t native_font_face::glyph_index(char32_t aCodePoint) const
	{
		return FT_Get_Char_Index(iHandle, aCodePoint);
	}

	i_glyph_texture& native_font_face::glyph_texture(const glyph& aGlyph) const
	{
		auto existingGlyph = iGlyphs.find(aGlyph.value());
		if (existingGlyph != iGlyphs.end())
			return existingGlyph->second;
		bool lcdMode = iRenderingEngine.screen_metrics().subpixel_format() == i_screen_metrics::SubpixelFormatRGBHorizontal ||
			iRenderingEngine.screen_metrics().subpixel_format() == i_screen_metrics::SubpixelFormatBGRHorizontal;

		FT_Load_Glyph(iHandle, aGlyph.value(), (lcdMode ? FT_LOAD_TARGET_LCD : FT_LOAD_TARGET_NORMAL) | FT_LOAD_NO_AUTOHINT);
		FT_Render_Glyph(iHandle->glyph, lcdMode ? FT_RENDER_MODE_LCD : FT_RENDER_MODE_NORMAL);
		FT_Bitmap& bitmap = iHandle->glyph->bitmap;

		rect glyphRect;
		i_font_texture& fontTexture = iRenderingEngine.font_manager().allocate_glyph_space(neogfx::size{ static_cast<dimension>(bitmap.width / (lcdMode ? 3.0 : 1.0)), static_cast<dimension>(bitmap.rows) }, glyphRect);
		i_glyph_texture& glyphTexture = iGlyphs.insert(std::make_pair(aGlyph.value(),
			neogfx::glyph_texture(
				fontTexture,
				glyphRect + point{ 1.0, 1.0 } -delta{ 2.0, 2.0 },
				neogfx::size{ static_cast<dimension>(bitmap.width / (lcdMode ? 3.0 : 1.0)), static_cast<dimension>(bitmap.rows) },
				neogfx::point{
					iHandle->glyph->metrics.horiBearingX / 64.0 / (lcdMode ? 3.0 : 1.0),
					(iHandle->glyph->metrics.horiBearingY - iHandle->glyph->metrics.height) / 64.0 }))).first->second;

		iGlyphTextureData.clear();
		iGlyphTextureData.resize(static_cast<std::size_t>(glyphRect.cx * glyphRect.cy));
		iSubpixelGlyphTextureData.clear();
		iSubpixelGlyphTextureData.resize(static_cast<std::size_t>(glyphRect.cx * glyphRect.cy));

		const GLubyte* textureData = 0;

		if (lcdMode)
		{
			/* todo */
			for (uint32_t y = 0; y < bitmap.rows; y++)
			{
				for (uint32_t x = 0; x < bitmap.width; x++)
				{
					auto mid = &bitmap.buffer[x + bitmap.pitch * y];
					GLubyte alpha = (mid[x > 0 ? -1 : 0] + mid[0] + mid[x < bitmap.width - 1 ? 1 : 0]) / 3;
					iSubpixelGlyphTextureData[(x / 3 + 1) + (y + 1) * static_cast<std::size_t>(glyphRect.cx)][x % 3] = alpha;
				}
			}
			textureData = &iSubpixelGlyphTextureData[0][0];
		}
		else
		{
			for (uint32_t y = 0; y < bitmap.rows; y++)
				for (uint32_t x = 0; x < bitmap.width; x++)
					iGlyphTextureData[(x + 1) + (y + 1) * static_cast<std::size_t>(glyphRect.cx)] =
						(x >= bitmap.width || y >= bitmap.rows) ? 0 : bitmap.buffer[x + bitmap.pitch * y];
			textureData = &iGlyphTextureData[0];
		}

		GLint previousTexture;
		glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture));
		glCheck(glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLuint>(glyphTexture.font_texture().handle())));

		glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0,
			static_cast<GLint>(glyphRect.x), static_cast<GLint>(glyphRect.y), static_cast<GLsizei>(glyphRect.cx), static_cast<GLsizei>(glyphRect.cy), 
			lcdMode ? GL_RGBA : GL_ALPHA, GL_UNSIGNED_BYTE, &textureData[0]));

		glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture)));

		return glyphTexture;
	}
}