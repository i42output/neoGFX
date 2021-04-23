// native_font_face.cpp
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
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_LCD_FILTER_H
#include FT_ADVANCES_H
#include "../../native/opengl.hpp"
#include "../../native/i_native_texture.hpp"
#include "native_font_face.hpp"
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>
#include <neogfx/gfx/i_texture_atlas.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>

namespace neogfx
{
    namespace
    {
        typedef std::unordered_map<std::pair<FT_UInt, FT_Int32>, FT_Fixed, boost::hash<std::pair<FT_UInt, FT_Int32>>> get_advance_cache_face;
        typedef std::unordered_map<FT_Face, get_advance_cache_face> get_advance_cache;
        get_advance_cache sGetAdvanceCache;
    }

    extern "C" FT_EXPORT(FT_Error)
        neoGFX_Get_Advance(FT_Face face, FT_UInt gindex, FT_Int32 load_flags, FT_Fixed* padvance)
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
            auto result = FT_Get_Advance(face, gindex, load_flags, padvance);
            if (result == FT_Err_Ok)
                cachedFace->second[std::make_pair(gindex, load_flags)] = *padvance;
            return result;
        }
        auto result = FT_Get_Advance(face, gindex, load_flags, padvance);
        return result;
    }

    thread_local bool tKerningEnabled = false;

    bool kerning_enabled()
    {
        return tKerningEnabled;
    }

    void enable_kerning()
    {
        tKerningEnabled = true;
    }

    void disable_kerning()
    {
        tKerningEnabled = false;
    }

    extern "C" FT_EXPORT(FT_Error)
        neoGFX_Get_Kerning(FT_Face     face,
            FT_UInt     left_glyph,
            FT_UInt     right_glyph,
            FT_UInt     kern_mode,
            FT_Vector* akerning)
    {
        // Not currently used by Harfbuzz as we have disabled OT kerning in Harfbuzz.
        auto result = FT_Get_Kerning(face, left_glyph, right_glyph, kern_mode, akerning);
        if (!kerning_enabled())
            *akerning = FT_Vector{};
        return result;
    }

    native_font_face::native_font_face(font_id aId, i_native_font& aFont, font_style aStyle, font::point_size aSize, neogfx::size aDpiResolution, FT_Face aHandle) :
        iId{ aId }, iFont{ aFont }, iStyle{ aStyle }, iStyleName{ aHandle->style_name }, iSize{ aSize }, iPixelDensityDpi{ aDpiResolution }, iHandle{ aHandle }, iHasKerning{ !!FT_HAS_KERNING(iHandle) }
    {
        set_metrics();
        sGetAdvanceCache[iHandle] = get_advance_cache_face{};
    }

    native_font_face::~native_font_face()
    {
        if (iHandle != nullptr)
            sGetAdvanceCache.erase(sGetAdvanceCache.find(iHandle));
        iAuxHandle = nullptr;
        FT_Done_Face(iHandle);
        if (iFallbackFont != nullptr)
            iFallbackFont->release();
    }

    font_id native_font_face::id() const
    {
        return iId;
    }

    i_native_font& native_font_face::native_font()
    {
        return iFont;
    }

    i_string const& native_font_face::family_name() const
    {
        return iFont.family_name();
    }

    font_style native_font_face::style() const
    {
        return iStyle;
    }

    i_string const& native_font_face::style_name() const
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
        return iMetrics->height / 64.0;
    }

    dimension native_font_face::ascender() const
    {
        return iMetrics->ascender / 64.0;
    }

    dimension native_font_face::descender() const
    {
        return iMetrics->descender / 64.0;
    }

    dimension native_font_face::underline_position() const
    {
        auto result = 0.0;
#if 0 // todo: get this to work properly
        if (FT_IS_SCALABLE(iHandle))
            result = iHandle->underline_position / 64.0;
        else
#endif
            result = -1.0 - underline_thickness();
        return std::floor(result);
    }

    dimension native_font_face::underline_thickness() const
    {
        auto result = 1.0;
        if (FT_IS_SCALABLE(iHandle))
            result = iHandle->underline_thickness / 64.0;
        else
            result = static_cast<dimension>(font_info::weight_from_style_name(iStyleName)) / static_cast<dimension>(font_weight::Normal);
        if (result < 1.0 || (result > 1.0 && iSize < 20))
            result = 1.0;
        return std::floor(result);
    }

    dimension native_font_face::line_spacing() const
    {
        /* todo */
        return 0;
    }

    dimension native_font_face::kerning(glyph_index_t aLeftGlyphIndex, glyph_index_t aRightGlyphIndex) const
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

    bool native_font_face::is_bitmap_font() const
    {
        return !FT_IS_SCALABLE(iHandle);
    }

    uint32_t native_font_face::num_fixed_sizes() const
    {
        return iHandle->num_fixed_sizes;
    }

    font::point_size native_font_face::fixed_size(uint32_t aFixedSizeIndex) const
    {
        if (aFixedSizeIndex < num_fixed_sizes())
            return iHandle->available_sizes[aFixedSizeIndex].size / 64.0;
        throw bad_fixed_size_index();
    }

    bool native_font_face::has_fallback() const
    {
        if (iHasFallback == std::nullopt)
            iHasFallback = service<i_font_manager>().has_fallback_font(*this);
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
            iFallbackFont = service<i_font_manager>().create_fallback_font(*this);
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
            iAuxHandle = std::make_unique<hb_handle>(*this);
        return &*iAuxHandle;
    }

    native_font_face::glyph_index_t native_font_face::glyph_index(char32_t aCodePoint) const
    {
        return FT_Get_Char_Index(iHandle, aCodePoint);
    }

    inline glyph_pixel_mode to_glyph_pixel_mode(unsigned char aFreeTypePixelMode)
    {
        switch (aFreeTypePixelMode)
        {
        default:
        case FT_PIXEL_MODE_NONE:
            return glyph_pixel_mode::None;
        case FT_PIXEL_MODE_MONO:
            return glyph_pixel_mode::Mono;
        case FT_PIXEL_MODE_GRAY:
            return glyph_pixel_mode::Gray;
        case FT_PIXEL_MODE_GRAY2:
            return glyph_pixel_mode::Gray2Bit;
        case FT_PIXEL_MODE_GRAY4:
            return glyph_pixel_mode::Gray4Bit;
        case FT_PIXEL_MODE_LCD:
            return glyph_pixel_mode::LCD;
        case FT_PIXEL_MODE_LCD_V:
            return glyph_pixel_mode::LCD_V;
        case FT_PIXEL_MODE_BGRA:
            return glyph_pixel_mode::BGRA;
        }
    }
         
    i_glyph_texture& native_font_face::glyph_texture(const glyph& aGlyph) const
    {
        auto existingGlyph = iGlyphs.find(aGlyph.value);
        if (existingGlyph != iGlyphs.end())
            return existingGlyph->second;
        try
        {
            try
            {
                freetypeCheck(FT_Load_Glyph(iHandle, aGlyph.value, FT_LOAD_TARGET_LCD | FT_LOAD_NO_BITMAP));
            }
            catch (freetype_error fe)
            {
                service<debug::logger>() << "neogfx: warning: Cannot load font glyph" << endl;
                throw freetype_load_glyph_error(fe.what());
            }
            try
            {
                freetypeCheck(FT_Render_Glyph(iHandle->glyph, FT_RENDER_MODE_LCD));
            }
            catch (freetype_error fe)
            {
                service<debug::logger>() << "neogfx: warning: Cannot render font glyph" << endl;
                throw freetype_render_glyph_error(fe.what());
            }
        }
        catch (...)
        {
            thread_local bool inHere = false;
            if (!inHere)
            {
                neolib::scoped_flag sf{ inHere };
                glyph invalid = aGlyph;
                auto const replacementGlyph = FT_Get_Char_Index(iHandle, 0xFFFD);
                if (replacementGlyph != 0)
                {
                    invalid.value = replacementGlyph;
                    return glyph_texture(aGlyph);
                }
            }
            if (iInvalidGlyph == std::nullopt)
            {
                auto& subTexture = service<i_font_manager>().glyph_atlas().create_sub_texture(
                    neogfx::size{ height(), height() }.ceil(),
                    1.0, texture_sampling::Normal, texture_data_format::SubPixel);
                iInvalidGlyph.emplace(
                    subTexture,
                    true,
                    point{},
                    glyph_pixel_mode::LCD);
                // todo: render an invalid glyph symbol
            }
            return *iInvalidGlyph;
        }

        bool useSubpixelFiltering = true;

        FT_Bitmap& bitmap = iHandle->glyph->bitmap;

        auto pixelMode = to_glyph_pixel_mode(bitmap.pixel_mode);

        if (pixelMode != glyph_pixel_mode::LCD)
            useSubpixelFiltering = false;

        auto subTextureWidth = bitmap.width / (useSubpixelFiltering ? 3 : 1);
        auto& subTexture = service<i_font_manager>().glyph_atlas().create_sub_texture(
            neogfx::size{ static_cast<dimension>(subTextureWidth), static_cast<dimension>(bitmap.rows) }.ceil(),
            1.0, texture_sampling::Normal, pixelMode != glyph_pixel_mode::Mono ? texture_data_format::SubPixel : texture_data_format::Red);

        rect glyphRect{ subTexture.atlas_location() };
        i_glyph_texture& glyphTexture = iGlyphs.insert(std::make_pair(aGlyph.value,
            neogfx::glyph_texture{
                subTexture,
                useSubpixelFiltering,
                point{
                    iHandle->glyph->metrics.horiBearingX / 64.0,
                    (iHandle->glyph->metrics.horiBearingY - iHandle->glyph->metrics.height) / 64.0 },
                pixelMode })).first->second;

        thread_local std::vector<GLubyte> glyphTextureData;
        thread_local std::vector<std::array<GLubyte, 4>> subpixelGlyphTextureData;
        glyphTextureData.clear();
        subpixelGlyphTextureData.clear();

        const GLubyte* textureData = 0;
        
        if (useSubpixelFiltering)
        {
            subpixelGlyphTextureData.resize(static_cast<std::size_t>(glyphRect.cx * glyphRect.cy));
            // sub-pixel FIR filter.
            static double coefficients[] = { 1.5 / 16.0, 3.5 / 16.0, 6.0 / 16.0, 3.5 / 16.0, 1.5 / 16.0 };
            for (uint32_t y = 0; y < bitmap.rows; y++)
            {
                for (uint32_t x = 0; x < bitmap.width; x++)
                {
                    uint8_t alpha = 0;
                    for (int32_t z = -2; z <= 2; ++z)
                    {
                        int32_t const s = x + z;
                        if (s >= 0 && s <= static_cast<int32_t>(bitmap.width) - 1)
                            alpha += static_cast<uint8_t>(bitmap.buffer[s + bitmap.pitch * y] * coefficients[z + 2]);
                    }
                    subpixelGlyphTextureData[(x / 3) + (bitmap.rows - 1 - y) * static_cast<std::size_t>(glyphRect.cx)][x % 3] = alpha;
                }
            }
            textureData = &subpixelGlyphTextureData[0][0];
        }
        else
        {
            glyphTextureData.resize(static_cast<std::size_t>(glyphRect.cx * glyphRect.cy));
            for (uint32_t y = 0; y < bitmap.rows; y++)
                switch (bitmap.pixel_mode)
                {
                case FT_PIXEL_MODE_MONO: // 1 bit per pixel monochrome
                    for (uint32_t x = 0; x < bitmap.width; x += 8)
                        for (uint32_t b = 0; b < std::min(bitmap.width - x, 8u); ++b)
                            glyphTextureData[(x + b) + (bitmap.rows - 1 - y) * static_cast<std::size_t>(glyphRect.cx)] =
                                (x >= bitmap.width || y >= bitmap.rows) ? 0x00 : ((bitmap.buffer[x / 8 + bitmap.pitch * y] & (1 << (7 - b))) != 0 ? 0xFF : 0x00);
                    break;
                case FT_PIXEL_MODE_GRAY:
                default:
                    for (uint32_t x = 0; x < bitmap.width; x++)
                        glyphTextureData[x + (bitmap.rows - 1 - y) * static_cast<std::size_t>(glyphRect.cx)] =
                            (x >= bitmap.width || y >= bitmap.rows) ? 0x00 : bitmap.buffer[x + bitmap.pitch * y];
                    break;
                }
            textureData = &glyphTextureData[0];
        }
        
        static_cast<i_native_texture&>(glyphTexture.texture().native_texture()).set_pixels(glyphRect, &textureData[0], 1u);

        return glyphTexture;
    }

    void native_font_face::set_metrics()
    {
        auto const size = ((style() & (font_style::Superscript | font_style::Subscript)) == font_style::Invalid) ? iSize : iSize * 0.58;
        if (!is_bitmap_font())
        {
            freetypeCheck(FT_Set_Char_Size(iHandle, 0, static_cast<FT_F26Dot6>(size * 64), static_cast<FT_UInt>(iPixelDensityDpi.cx), static_cast<FT_UInt>(iPixelDensityDpi.cy)));
        }
        else
        {
            auto requestedSize = size * iPixelDensityDpi.cy / 72.0;
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
        if (iMetrics == std::nullopt)
            iMetrics.emplace(iHandle->size->metrics);
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