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
#include "../../native/i_native_texture.hpp"
#include "native_font_face.hpp"
#include <neogfx/gfx/text/i_font_manager.hpp>
#include <neogfx/gfx/i_texture_atlas.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/graphics_context.hpp>

namespace neogfx
{
    namespace
    {
        typedef std::unordered_map<std::pair<FT_UInt, FT_Int32>, FT_Fixed, boost::hash<std::pair<FT_UInt, FT_Int32>>> get_advance_cache_face;
        typedef std::unordered_map<FT_Face, get_advance_cache_face> get_advance_cache;
        get_advance_cache sGetAdvanceCache;
    }

    bool& kerning_enabled_flag()
    {
        thread_local bool tKerningEnabled = true;
        return tKerningEnabled;
    }

    bool kerning_enabled()
    {
        return kerning_enabled_flag();
    }

    void enable_kerning(bool aEnableKerning)
    {
        kerning_enabled_flag() = aEnableKerning;
    }

    hb_position_t hb_kerning_func(hb_font_t* font, void* font_data, hb_codepoint_t first_glyph, hb_codepoint_t second_glyph, void* user_data)
    {
        return static_cast<hb_position_t>(static_cast<font_face_handle*>(user_data)->owner.kerning(first_glyph, second_glyph));
    }

    native_font_face::native_font_face(
        FT_Library aFontLib, font_id aId, i_native_font& aFont, font_style aStyle, font::point_size aSize, stroke aOutline, 
        neogfx::size aDpiResolution, FT_Face aFreetypeFace, hb_face_t* aHarfbuzzFace) :
        iFontLib{ aFontLib }, iId{ aId }, iFont{ aFont }, iStyle{ aStyle }, iStyleName{ aFreetypeFace->style_name }, iSize{ aSize }, 
        iOutline{ aOutline }, iPixelDensityDpi {aDpiResolution }, iHandle{ *this, aFreetypeFace, aHarfbuzzFace }, 
        iHasKerning{ !!FT_HAS_KERNING(iHandle.freetypeFace) }
    {
        switch (aStyle)
        {
        case font_style::EmulatedBold:
            iStyleName = "Bold (Emulated)";
            break;
        case font_style::EmulatedItalic:
            iStyleName = "Italic (Emulated)";
            break;
        case font_style::EmulatedBoldItalic:
            iStyleName = "Bold Italic (Emulated)";
            break;
        }
        set_metrics();
        sGetAdvanceCache[aFreetypeFace] = get_advance_cache_face{};
    }

    native_font_face::~native_font_face()
    {
        if (iHandle.freetypeFace != nullptr)
            sGetAdvanceCache.erase(sGetAdvanceCache.find(iHandle.freetypeFace));
        FT_Done_Face(iHandle.freetypeFace);
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
        if (iSize >= 0.0)
            return iSize;
        else
            return 72.0 / iPixelDensityDpi.cy * -iSize;
    }

    stroke native_font_face::outline() const
    {
        return iOutline;
    }

    dimension native_font_face::horizontal_dpi() const
    {
        return iPixelDensityDpi.cx;
    }

    dimension native_font_face::vertical_dpi() const
    {
        return iPixelDensityDpi.cy;
    }

    size native_font_face::em_size() const
    {
        return basic_size<FT_UShort>{ iMetrics->x_ppem, iMetrics->y_ppem };
    }

    dimension native_font_face::height() const
    {
        return iMetrics->height / 64.0;
    }

    dimension native_font_face::max_advance() const
    {
        return iMetrics->max_advance / 64.0;
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
        if (FT_IS_SCALABLE(iHandle.freetypeFace))
            result = iHandle.freetypeFace->underline_thickness / 64.0;
        else
            result = static_cast<dimension>(font_info::weight_from_style_name(iStyleName)) / static_cast<dimension>(font_weight::Normal);
        if (result < 1.0 || (result > 1.0 && size() < 20.0))
            result = 1.0;
        return std::floor(result);
    }

    dimension native_font_face::line_spacing() const
    {
        /* todo */
        return 0;
    }

    kerning_method native_font_face::kerning_method() const
    {
        return iKerningMethod;
    }

    void native_font_face::set_kerning_method(neogfx::kerning_method aKerningMethod)
    {
        iKerningMethod = aKerningMethod;
        iKerningTable.clear();
    }

    dimension native_font_face::kerning(glyph_index_t aLeftGlyphIndex, glyph_index_t aRightGlyphIndex) const
    {
        if (!iHasKerning)
            return 0.0;
        auto existing = iKerningTable.find(std::make_pair(aLeftGlyphIndex, aRightGlyphIndex));
        if (existing != iKerningTable.end())
            return existing->second;
        switch (kerning_method())
        {
        case neogfx::kerning_method::Freetype:
        case neogfx::kerning_method::Harfbuzz:
            {
                FT_Vector delta;
                freetypeCheck(FT_Get_Kerning(iHandle.freetypeFace, aLeftGlyphIndex, aRightGlyphIndex, FT_KERNING_UNFITTED, &delta));
                return (iKerningTable[std::make_pair(aLeftGlyphIndex, aRightGlyphIndex)] = delta.x / 64.0);
            }
        case neogfx::kerning_method::Disabled:
        default:
            return 0.0;
        }
    }

    bool native_font_face::is_bitmap_font() const
    {
        return !FT_IS_SCALABLE(iHandle.freetypeFace);
    }

    uint32_t native_font_face::num_fixed_sizes() const
    {
        return iHandle.freetypeFace->num_fixed_sizes;
    }

    font::point_size native_font_face::fixed_size(uint32_t aFixedSizeIndex) const
    {
        if (aFixedSizeIndex < num_fixed_sizes())
            return iHandle.freetypeFace->available_sizes[aFixedSizeIndex].size / 64.0;
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
        return &iHandle;
    }

    native_font_face::glyph_index_t native_font_face::glyph_index(char32_t aCodePoint) const
    {
        return FT_Get_Char_Index(iHandle.freetypeFace, aCodePoint);
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
         
    i_glyph& native_font_face::glyph(const glyph_char& aGlyphChar) const
    {
        // todo: investigate why turning off sub-pixel doesn't produce same grayscale bitmap as Windows with ClearType disabled
        bool useSubpixelFiltering = true;

        auto existingGlyph = iGlyphs.find(aGlyphChar.value);
        if (existingGlyph != iGlyphs.end())
            return existingGlyph->second;
        try
        {
            try
            {
                if (useSubpixelFiltering)
                {
                    freetypeCheck(FT_Load_Glyph(iHandle.freetypeFace, aGlyphChar.value, FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LCD | FT_LOAD_NO_BITMAP));
                }
                else
                {
                    freetypeCheck(FT_Load_Glyph(iHandle.freetypeFace, aGlyphChar.value, FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_BITMAP));
                }
            }
            catch (freetype_error fe)
            {
                service<debug::logger>() << neolib::logger::severity::Debug << "neogfx: warning: Cannot load font glyph" << endl;
                throw freetype_load_glyph_error(fe.what());
            }
            try
            {
                if (useSubpixelFiltering)
                {
                    freetypeCheck(FT_Render_Glyph(iHandle.freetypeFace->glyph, FT_RENDER_MODE_LCD));
                }
                else
                {
                    freetypeCheck(FT_Render_Glyph(iHandle.freetypeFace->glyph, FT_RENDER_MODE_NORMAL));
                }
            }
            catch (freetype_error fe)
            {
                service<debug::logger>() << neolib::logger::severity::Debug << "neogfx: warning: Cannot render font glyph" << endl;
                throw freetype_render_glyph_error(fe.what());
            }
        }
        catch (...)
        {
            thread_local bool inHere = false;
            if (!inHere)
            {
                neolib::scoped_flag sf{ inHere };
                glyph_char invalid = aGlyphChar;
                auto const replacementGlyph = FT_Get_Char_Index(iHandle.freetypeFace, 0xFFFD);
                if (replacementGlyph != 0)
                {
                    invalid.value = replacementGlyph;
                    return glyph(invalid);
                }
            }
            return invalid_glyph();
        }

        FT_Bitmap& bitmap = iHandle.freetypeFace->glyph->bitmap;

        if ((style() & (font_style::EmulatedBold)) == font_style::EmulatedBold)
            FT_Bitmap_Embolden(iFontLib, &bitmap, static_cast<FT_F26Dot6>(xn_dpi_scale_factor(iPixelDensityDpi.cx) * 64), 0);

        auto pixelMode = to_glyph_pixel_mode(bitmap.pixel_mode);

        if (pixelMode != glyph_pixel_mode::LCD)
            useSubpixelFiltering = false;

        auto subTextureWidth = bitmap.width / (useSubpixelFiltering ? 3 : 1);

        auto& subTexture = service<i_font_manager>().glyph_atlas().create_sub_texture(
            neogfx::size{ static_cast<dimension>(subTextureWidth), static_cast<dimension>(bitmap.rows) }.ceil(),
            1.0, texture_sampling::Normal, pixelMode == glyph_pixel_mode::LCD ? texture_data_format::SubPixel : texture_data_format::Red);

        rect glyphRect{ subTexture.atlas_location() };
        i_glyph& glyphTexture = iGlyphs.insert(std::make_pair(aGlyphChar.value,
            neogfx::glyph{
                subTexture,
                useSubpixelFiltering,
                glyph_metrics{
                    vec2{ iHandle.freetypeFace->glyph->metrics.width / 64.0, iHandle.freetypeFace->glyph->metrics.height / 64.0 }.round(),
                    vec2{ iHandle.freetypeFace->glyph->metrics.horiBearingX / 64.0, iHandle.freetypeFace->glyph->metrics.horiBearingY / 64.0 }.round() },
                pixelMode })).first->second;

        thread_local std::vector<std::uint8_t> glyphTextureData;
        thread_local std::vector<std::array<std::uint8_t, 4>> subpixelGlyphData;
        glyphTextureData.clear();
        subpixelGlyphData.clear();

        const std::uint8_t* textureData = 0;
        
        if (subTextureWidth != 0)
        {
            if (useSubpixelFiltering)
            {
                subpixelGlyphData.resize(static_cast<std::size_t>(glyphRect.cx * glyphRect.cy));
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
                        subpixelGlyphData[(x / 3) + (bitmap.rows - 1 - y) * static_cast<std::size_t>(glyphRect.cx)][x % 3] = alpha;
                    }
                }
                textureData = &subpixelGlyphData[0][0];
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
        }

        return glyphTexture;
    }

    i_glyph& native_font_face::invalid_glyph() const
    {
        if (iInvalidGlyph == std::nullopt)
        {
            auto& subTexture = service<i_font_manager>().glyph_atlas().create_sub_texture(
                neogfx::size{ em_size().cx, ascender() }.ceil(),
                1.0, texture_sampling::Normal, texture_data_format::SubPixel);
            iInvalidGlyph.emplace(
                subTexture,
                true,
                glyph_metrics{
                    vec2{ em_size().cx, ascender() }.ceil(),
                    vec2{} },
                glyph_pixel_mode::LCD);
            graphics_context gc{ iInvalidGlyph->texture() };
            auto r = iInvalidGlyph->texture().atlas_location().deflated(neogfx::size{ 1.0, 1.0 });
            gc.draw_rect(r, color::White);
            --r.cx;
            --r.cy;
            gc.draw_line(r.top_left(), r.bottom_right(), color::White);
            gc.draw_line(r.top_right(), r.bottom_left(), color::White);
        }
        return *iInvalidGlyph;
    }

    void native_font_face::set_metrics()
    {
        auto const desiredSize = ((style() & (font_style::Superscript | font_style::Subscript)) == font_style::Invalid) ? iSize : iSize * 0.58;
        bool const sizeSpecified = (iSize >= 0.0);
        bool const heightSpecified = !sizeSpecified;
        auto const requestedSize = sizeSpecified ? desiredSize : -desiredSize / iPixelDensityDpi.cy * 72.0;
        auto const requestedHeight = heightSpecified ? -desiredSize : desiredSize * iPixelDensityDpi.cy / 72.0;
        double correction = 1.0;
        if (!is_bitmap_font())
        {
            freetypeCheck(FT_Set_Char_Size(iHandle.freetypeFace, 0, static_cast<FT_F26Dot6>(requestedSize * 64), static_cast<FT_UInt>(iPixelDensityDpi.cx), static_cast<FT_UInt>(iPixelDensityDpi.cy)));
            if (heightSpecified)
            {
                double const gotHeight = iHandle.freetypeFace->size->metrics.height / 64.0;
                if (gotHeight != requestedHeight)
                {
                    correction = requestedHeight / gotHeight;
                    auto const corrected = static_cast<FT_F26Dot6>(requestedSize * correction * 64);
                    freetypeCheck(FT_Set_Char_Size(iHandle.freetypeFace, 0, corrected, static_cast<FT_UInt>(iPixelDensityDpi.cx), static_cast<FT_UInt>(iPixelDensityDpi.cy)));
                }
            }
        }
        else
        {
            auto availableHeight = iHandle.freetypeFace->available_sizes[0].size / 64.0;
            FT_Int strikeIndex = 0;
            for (FT_Int si = 0; si < iHandle.freetypeFace->num_fixed_sizes; ++si)
            {
                auto nextAvailableHeight = iHandle.freetypeFace->available_sizes[si].size / 64.0;
                if (std::abs(requestedHeight - nextAvailableHeight) < std::abs(requestedHeight - availableHeight))
                {
                    availableHeight = nextAvailableHeight;
                    strikeIndex = si;
                }
            }
            freetypeCheck(FT_Select_Size(iHandle.freetypeFace, strikeIndex));
        }
        if (iMetrics == std::nullopt)
            iMetrics.emplace(iHandle.freetypeFace->size->metrics);
        for (const FT_CharMap* cm = iHandle.freetypeFace->charmaps; cm != iHandle.freetypeFace->charmaps + iHandle.freetypeFace->num_charmaps; ++cm)
        {
            if ((**cm).encoding == FT_ENCODING_UNICODE)
            {
                freetypeCheck(FT_Select_Charmap(iHandle.freetypeFace, FT_ENCODING_UNICODE));
                break;
            }
        }
        hb_font_set_scale(
            iHandle.harfbuzzFont,
            static_cast<int>(requestedSize * correction * iPixelDensityDpi.cx / 72.0 * 64),
            static_cast<int>(requestedSize * correction * iPixelDensityDpi.cy / 72.0 * 64));
    }
}