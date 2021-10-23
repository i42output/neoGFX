// native_font_face.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef u8
#undef u8
#include <harfbuzz\hb.h>
#include <harfbuzz\hb-ot.h>
#include <harfbuzz\hb-ucdn\ucdn.h>
#define u8
#else
#include <harfbuzz\hb.h>
#include <harfbuzz\hb-ot.h>
#endif
#include "..\..\native\opengl.hpp"
#include <neolib/core/reference_counted.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/gfx/text/font.hpp>
#include "glyph_texture.hpp"
#include "i_native_font.hpp"
#include "i_native_font_face.hpp"

namespace neogfx
{
    struct freetype_error : std::runtime_error { freetype_error(std::string const& aError) : std::runtime_error(aError) {} };
    inline std::string getFreeTypeErrorMessage(FT_Error err)
    {
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
        return "(Unknown error)";
    }
}

#define freetypeCheck(x) \
{ \
    FT_Error err = x; \
    if (err != FT_Err_Ok) \
        throw freetype_error("neoGFX FreeType error: " + getFreeTypeErrorMessage(err)); \
}

namespace neogfx
{
    class i_rendering_engine;

    bool kerning_enabled();
    void enable_kerning();
    void disable_kerning();

    class scoped_kerning
    {
    public:
        scoped_kerning(bool aEnableKerning) : iPrevious{ kerning_enabled() }
        {
            if (aEnableKerning)
                enable_kerning();
            else
                disable_kerning();
        }
        ~scoped_kerning()
        {
            if (iPrevious)
                enable_kerning();
            else
                disable_kerning();
        }
    private:
        bool iPrevious;
    };

    struct font_face_handle
    {
        FT_Face freetypeFace;
        hb_face_t* harfbuzzFace;
        hb_font_t* harfbuzzFont;
        hb_buffer_t* harfbuzzBuf;
        hb_unicode_funcs_t* harfbuzzUnicodeFuncs;
        font_face_handle(FT_Face aFreetypeFace, hb_face_t* aHarfbuzzFace) :
            freetypeFace{ aFreetypeFace },
            harfbuzzFace{ aHarfbuzzFace },
            harfbuzzFont{ hb_font_create(aHarfbuzzFace) },
            harfbuzzBuf{ hb_buffer_create() },
            harfbuzzUnicodeFuncs{ hb_buffer_get_unicode_funcs(harfbuzzBuf) }
        {
        }
        ~font_face_handle()
        {
            hb_font_destroy(harfbuzzFont);
            hb_buffer_destroy(harfbuzzBuf);
        }
    };

    class native_font_face : public neolib::reference_counted<i_native_font_face>
    {
    private:
        typedef std::unordered_map<glyph_index_t, neogfx::glyph_texture> glyph_map;
        typedef std::pair<glyph_index_t, glyph_index_t> kerning_pair;
        typedef std::unordered_map<kerning_pair, dimension, boost::hash<kerning_pair>, std::equal_to<kerning_pair>,
            boost::fast_pool_allocator<std::pair<const kerning_pair, dimension>>> kerning_table;
    public:
        struct freetype_load_glyph_error : freetype_error { freetype_load_glyph_error(std::string const& aError) : freetype_error(aError) {} };
        struct freetype_render_glyph_error : freetype_error { freetype_render_glyph_error(std::string const& aError) : freetype_error(aError) {} };
    public:
        native_font_face(FT_Library aFontLib, font_id aId, i_native_font& aFont, font_style aStyle, font::point_size aSize, neogfx::size aDpiResolution, FT_Face aFreetypeFace, hb_face_t* aHarfbuzzFace);
        ~native_font_face();
    public:
        font_id id() const override;
        i_native_font& native_font() override;
        i_string const& family_name() const override;
        font_style style() const override;
        font::point_size size() const override;
        i_string const& style_name() const override;
        dimension horizontal_dpi() const override;
        dimension vertical_dpi() const override;
        dimension height() const override;
        dimension ascender() const override;
        dimension descender() const override;
        dimension underline_position() const override;
        dimension underline_thickness() const override;
        dimension line_spacing() const override;
        neogfx::kerning_method kerning_method() const override;
        void set_kerning_method(neogfx::kerning_method aKerningMethod) override;
        dimension kerning(glyph_index_t aLeftGlyphIndex, glyph_index_t aRightGlyphIndex) const override;
        bool is_bitmap_font() const override;
        uint32_t num_fixed_sizes() const override;
        font::point_size fixed_size(uint32_t aFixedSizeIndex) const override;
        bool has_fallback() const override;
        bool fallback_cached() const override;
        i_native_font_face& fallback() const override;
        void* handle() const override;
        glyph_index_t glyph_index(char32_t aCodePoint) const override;
        i_glyph_texture& glyph_texture(const glyph& aGlyph) const override;
    private:
        i_glyph_texture& invalid_glyph() const;
        void set_metrics();
    private:
        FT_Library iFontLib;
        font_id iId;
        i_native_font& iFont;
        font_style iStyle;
        string iStyleName;
        font::point_size iSize;
        neogfx::size iPixelDensityDpi;
        mutable font_face_handle iHandle;
        std::optional<FT_Size_Metrics> iMetrics;
        mutable ref_ptr<i_native_font_face> iFallbackFont;
        mutable glyph_map iGlyphs;
        bool iHasKerning = false;
        neogfx::kerning_method iKerningMethod = neogfx::kerning_method::Harfbuzz;
        mutable kerning_table iKerningTable;
        mutable std::optional<bool> iHasFallback;
        mutable std::optional<neogfx::glyph_texture> iInvalidGlyph;
    };
}