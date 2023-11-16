// native_font.hpp
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
#include <tuple>
#include <neolib/core/variant.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <harfbuzz/hb.h>
#include "i_native_font.hpp"
#include "i_native_font_face.hpp"

namespace neogfx
{
    class native_font : public reference_counted<i_native_font>
    {
    public:
        typedef std::string filename_type;
        typedef std::pair<const void*, std::size_t> memory_block_type;
    private:
        typedef std::variant<std::monostate, filename_type, memory_block_type> source_type;
        typedef std::map<std::pair<font_style, string>, FT_Long> style_map;
        typedef std::map<std::tuple<FT_Long, font_style, font::point_size, stroke, size>, ref_ptr<i_native_font_face>> face_map;
    public:
        struct failed_to_load_font : std::runtime_error { failed_to_load_font() : std::runtime_error("neogfx::native_font::failed_to_load_font") {} };
        struct no_matching_style_found : std::runtime_error { no_matching_style_found() : std::runtime_error("neogfx::native_font::no_matching_style_found") {} };
    public:
        native_font(FT_Library aFontLib, const std::string aFileName);
        native_font(FT_Library aFontLib, const void* aData, std::size_t aSizeInBytes);
        ~native_font();
    public:
        i_string const& family_name() const final;
        bool has_style(font_style aStyle) const final;
        uint32_t style_count() const final;
        font_style style(uint32_t aStyle) const final;
        i_string const& style_name(uint32_t aStyleIndex) const final;
        void remove_style(font_style aStyleIndex) final;
        void remove_style(uint32_t aStyleIndex) final;
        void create_face(font_style aStyle, font::point_size aSize, stroke aOutline, i_device_resolution const& aDevice, i_ref_ptr<i_native_font_face>& aResult) final;
        void create_face(font_style aStyle, i_string const& aStyleName, font::point_size aSize, stroke aOutline, i_device_resolution const& aDevice, i_ref_ptr<i_native_font_face>& aResult) final;
        void create_face(font_info const& aFontIinfo, i_device_resolution const& aDevice, i_ref_ptr<i_native_font_face>& aResult) final;
    private:
        style_map::const_iterator find_style(font_style aStyle) const;
        void register_faces();
        void register_face(FT_Long aFaceIndex);
        std::pair<FT_Face, hb_face_t*> open_face(FT_Long aFaceIndex);
        void close_face(FT_Face aFace);
        ref_ptr<i_native_font_face> create_face(FT_Long aFaceIndex, font_style aStyle, font::point_size aSize, stroke aOutline, i_device_resolution const& aDevice);
    private:
        FT_Library iFontLib;
        source_type iSource;
        std::vector<unsigned char> iCache;
        hb_blob_t* iHarfbuzzBlob = nullptr;
        string iFamilyName;
        FT_Long iFaceCount;
        style_map iStyleMap;
        face_map iFaces;
    };
}