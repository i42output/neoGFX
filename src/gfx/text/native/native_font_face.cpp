// native_font_face.cpp
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
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_LCD_FILTER_H
#include "../../native/opengl.hpp"
#include "../../native/i_native_texture.hpp"
#include "native_font_face.hpp"
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>

namespace neogfx
{
	namespace
	{
		typedef std::unordered_map<std::pair<FT_UInt, FT_Int32>, FT_Fixed, boost::hash<std::pair<FT_UInt, FT_Int32>>> get_advance_cache_face;
		typedef std::unordered_map<FT_Face, get_advance_cache_face> get_advance_cache;
		get_advance_cache sGetAdvanceCache;

		extern "C"
		{
			FT_EXPORT(FT_Error) orig_FT_Get_Advance(FT_Face face, FT_UInt gindex, FT_Int32 load_flags, FT_Fixed* padvance);
		}

		FT_Error neogfx_FT_Get_Advance(FT_Face face, FT_UInt gindex, FT_Int32 load_flags, FT_Fixed* padvance)
		{
			auto cachedFace = sGetAdvanceCache.find(face);
			if (cachedFace != sGetAdvanceCache.end())
			{
				auto entry = cachedFace->second.find(std::make_pair(gindex, load_flags));
				if (entry != cachedFace->second.end())
				{
					*padvance = entry->second;
					return FT_Err_Ok;
				}
				auto result = orig_FT_Get_Advance(face, gindex, load_flags, padvance);
				if (result == FT_Err_Ok)
					cachedFace->second[std::make_pair(gindex, load_flags)] = *padvance;
				return result;
			}
			auto result = orig_FT_Get_Advance(face, gindex, load_flags, padvance);
			freetypeCheck(result);
			return result;
		}

		extern "C"
		{
			FT_EXPORT(FT_Error) FT_Get_Advance(FT_Face face, FT_UInt gindex, FT_Int32 load_flags, FT_Fixed* padvance)
			{
				return neogfx_FT_Get_Advance(face, gindex, load_flags, padvance);
			}
		}
	}

	native_font_face::native_font_face(i_rendering_engine& aRenderingEngine, i_native_font& aFont, font::style_e aStyle, font::point_size aSize, neogfx::size aDpiResolution, FT_Face aHandle) :
		iRenderingEngine(aRenderingEngine), iFont(aFont), iStyle(aStyle), iStyleName(aHandle->style_name), iSize(aSize), iPixelDensityDpi(aDpiResolution), iHandle(aHandle), iHasKerning(!!FT_HAS_KERNING(iHandle))
	{
		set_metrics();
		sGetAdvanceCache[iHandle] = get_advance_cache_face{};
	}

	native_font_face::~native_font_face()
	{
		if (iHandle != nullptr)
			sGetAdvanceCache.erase(sGetAdvanceCache.find(iHandle));
		FT_Done_Face(iHandle);
		if (iFallbackFont != nullptr)
			iFallbackFont->release();
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
		freetypeCheck(FT_Get_Kerning(iHandle, aLeftGlyphIndex, aRightGlyphIndex, FT_KERNING_DEFAULT, &delta));
		return (iKerningTable[std::make_pair(aLeftGlyphIndex, aRightGlyphIndex)] = delta.x / 64.0);
	}

	bool native_font_face::has_fallback() const
	{
		if (iHasFallback == boost::none)
			iHasFallback = iRenderingEngine.font_manager().has_fallback_font(*this);
		return *iHasFallback;
	}

	bool native_font_face::fallback_cached() const
	{
		return iFallbackFont != nullptr;
	}

	i_native_font_face& native_font_face::fallback() const
	{
		if (!has_fallback())
			throw no_fallback_font();
		if (iFallbackFont == nullptr)
			iFallbackFont = iRenderingEngine.font_manager().create_fallback_font(*this);
		return *iFallbackFont;
	}
	
	void* native_font_face::handle() const
	{
		return iHandle;
	}

	void native_font_face::update_handle(void* aHandle) 
	{ 
		if (iHandle != nullptr)
			sGetAdvanceCache.erase(sGetAdvanceCache.find(iHandle));
		iHandle = static_cast<FT_Face>(aHandle);
		if (iHandle != nullptr)
			sGetAdvanceCache[iHandle] = get_advance_cache_face{};
		iAuxHandle.reset();
		if (iHandle != nullptr)
			set_metrics();
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
		auto existingGlyph = iGlyphs.find(std::make_pair(aGlyph.value(), aGlyph.subpixel()));
		if (existingGlyph != iGlyphs.end())
			return existingGlyph->second;

		try
		{
			freetypeCheck(FT_Load_Glyph(iHandle, aGlyph.value(), aGlyph.subpixel() ? FT_LOAD_TARGET_LCD : FT_LOAD_TARGET_NORMAL));
		}
		catch (freetype_error fe)
		{
			throw freetype_load_glyph_error(fe.what());
		}
		try
		{
			freetypeCheck(FT_Render_Glyph(iHandle->glyph, aGlyph.subpixel() ? FT_RENDER_MODE_LCD : FT_RENDER_MODE_NORMAL));
		}
		catch (freetype_error fe)
		{
			throw freetype_render_glyph_error(fe.what());
		}

		FT_Bitmap& bitmap = iHandle->glyph->bitmap;

		auto& subTexture = iRenderingEngine.font_manager().glyph_atlas().create_sub_texture(
			neogfx::size{ static_cast<dimension>(bitmap.width / (aGlyph.subpixel() ? 3.0 : 1.0)), static_cast<dimension>(bitmap.rows) },
			1.0, texture_sampling::Normal);
		rect glyphRect{ subTexture.atlas_location() };
		i_glyph_texture& glyphTexture = iGlyphs.insert(std::make_pair(std::make_pair(aGlyph.value(), aGlyph.subpixel()),
			neogfx::glyph_texture{
				subTexture,
				point{
					iHandle->glyph->metrics.horiBearingX / 64.0,
					(iHandle->glyph->metrics.horiBearingY - iHandle->glyph->metrics.height) / 64.0 } })).first->second;

		iGlyphTextureData.clear();
		iGlyphTextureData.resize(static_cast<std::size_t>(glyphRect.cx * glyphRect.cy));
		iSubpixelGlyphTextureData.clear();
		iSubpixelGlyphTextureData.resize(static_cast<std::size_t>(glyphRect.cx * glyphRect.cy));

		const GLubyte* textureData = 0;
		
		if (aGlyph.subpixel())
		{
			// sub-pixel FIR filter.
			static double coefficients[] = { 1.5/16.0, 3.0/16.0, 7.0/16.0, 3.0/16.0, 1.5/16.0 };
			for (uint32_t y = 0; y < bitmap.rows; y++)
			{
				for (uint32_t x = 0; x < bitmap.width; x++)
				{
					uint8_t alpha = 0;
					for (int32_t z = 0; z < 5; ++z)
						alpha += static_cast<uint8_t>(bitmap.buffer[std::max<int32_t>(0, x - z + 2) + bitmap.pitch * y] * coefficients[z]);
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
		glCheck(glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLuint>(glyphTexture.texture().native_texture()->handle())));

		glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0,
			static_cast<GLint>(glyphRect.x), static_cast<GLint>(glyphRect.y), static_cast<GLsizei>(glyphRect.cx), static_cast<GLsizei>(glyphRect.cy), 
			aGlyph.subpixel() ? GL_RGBA : GL_ALPHA, GL_UNSIGNED_BYTE, &textureData[0]));

		glCheck(glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture)));

		return glyphTexture;
	}

	void native_font_face::add_ref()
	{
		native_font().add_ref(*this);
	}

	void native_font_face::release()
	{
		native_font().release(*this);
	}

	void native_font_face::set_metrics()
	{
		if (iHandle->num_fixed_sizes == 0)
		{
			freetypeCheck(FT_Set_Char_Size(iHandle, 0, static_cast<FT_F26Dot6>(iSize * 64), static_cast<FT_UInt>(iPixelDensityDpi.cx), static_cast<FT_UInt>(iPixelDensityDpi.cy)));
		}
		else
		{
			auto requestedSize = iSize * iPixelDensityDpi.cy / 72.0;
			auto availableSize = iHandle->available_sizes[0].size / 64.0;
			FT_Int strikeIndex = 0;
			for (FT_Int si = 0; si < iHandle->num_fixed_sizes; ++si)
			{
				auto nextAvailableSize = iHandle->available_sizes[si].size / 64.0;
				if (abs(requestedSize - nextAvailableSize) < abs(requestedSize - availableSize))
				{
					availableSize = nextAvailableSize;
					strikeIndex = si;
				}
			}
			freetypeCheck(FT_Select_Size(iHandle, strikeIndex));
		}
		for (const FT_CharMap* cm = iHandle->charmaps; cm != iHandle->charmaps + iHandle->num_charmaps; ++cm)
		{
			if ((**cm).encoding == FT_ENCODING_UNICODE)
			{
				freetypeCheck(FT_Select_Charmap(iHandle, FT_ENCODING_UNICODE));
				break;
			}
		}
	}
}