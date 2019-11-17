// font_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <set>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <neolib/cookie_jar.hpp>
#include <neolib/string_utils.hpp>
#include <neogfx/gfx/texture_atlas.hpp>
#include <neogfx/gfx/text/emoji_atlas.hpp>
#include "i_font_manager.hpp"

namespace neogfx
{
    class native_font;

    class fallback_font_info : public i_fallback_font_info
    {
    public:
        fallback_font_info(std::vector<std::string> aFallbackFontFamilies);
    public:
        bool has_fallback_for(const std::string& aFontFamilyName) const override;
        const std::string& fallback_for(const std::string& aFontFamilyName) const override;
    private:
        std::vector<std::string> iFallbackFontFamilies;
    };

    class font_manager : public i_font_manager
    {
        friend class native_font_face;
    private:
        typedef std::list<native_font> native_font_list;
        typedef std::map<neolib::ci_string, std::vector<native_font_list::iterator>> font_family_list;
        typedef std::pair<font, uint32_t> id_cache_entry;
        typedef neolib::small_cookie_jar<id_cache_entry> id_cache;
        friend neolib::small_cookie item_cookie(const id_cache_entry&);
    public:
        struct error_initializing_font_library : std::runtime_error { error_initializing_font_library() : std::runtime_error("neogfx::font_manager::error_initializing_font_library") {} };
        struct no_matching_font_found : std::runtime_error { no_matching_font_found() : std::runtime_error("neogfx::font_manager::no_matching_font_found") {} };
        struct failed_to_allocate_glyph_space : std::runtime_error { failed_to_allocate_glyph_space() : std::runtime_error("neogfx::font_manager::failed_to_allocate_glyph_space") {} };
    public:
        font_manager();
        ~font_manager();
    public:
        void* font_library_handle() const override;
        const font_info& default_system_font_info() const override;
        const i_fallback_font_info& default_fallback_font_info() const override;
        std::shared_ptr<i_native_font_face> create_default_font(const i_device_resolution& aDevice) override;
        bool has_fallback_font(const i_native_font_face& aExistingFont) const override;
        std::shared_ptr<i_native_font_face> create_fallback_font(const i_native_font_face& aExistingFont) override;
        std::shared_ptr<i_native_font_face> create_font(const std::string& aFamilyName, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) override;
        std::shared_ptr<i_native_font_face> create_font(const std::string& aFamilyName, const std::string& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) override;
        std::shared_ptr<i_native_font_face> create_font(const font_info& aInfo, const i_device_resolution& aDevice) override;
        std::shared_ptr<i_native_font_face> create_font(i_native_font& aFont, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) override;
        std::shared_ptr<i_native_font_face> create_font(i_native_font& aFont, const std::string& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) override;
        bool is_font_file(const std::string& aFileName) const override;
        std::shared_ptr<i_native_font_face> load_font_from_file(const std::string& aFileName, const i_device_resolution& aDevice) override;
        std::shared_ptr<i_native_font_face> load_font_from_file(const std::string& aFileName, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) override;
        std::shared_ptr<i_native_font_face> load_font_from_file(const std::string& aFileName, const std::string& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) override;
        std::shared_ptr<i_native_font_face> load_font_from_memory(const void* aData, std::size_t aSizeInBytes, const i_device_resolution& aDevice) override;
        std::shared_ptr<i_native_font_face> load_font_from_memory(const void* aData, std::size_t aSizeInBytes, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) override;
        std::shared_ptr<i_native_font_face> load_font_from_memory(const void* aData, std::size_t aSizeInBytes, const std::string& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) override;
    public:
        uint32_t font_family_count() const override;
        std::string font_family(uint32_t aFamilyIndex) const override;
        uint32_t font_style_count(uint32_t aFamilyIndex) const override;
        std::string font_style(uint32_t aFamilyIndex, uint32_t aStyleIndex) const override;
    private:
        font_id allocate_font_id() override;
    public:
        const font& font_from_id(font_id aId) const override;
    public:
        const i_texture_atlas& glyph_atlas() const override;
        i_texture_atlas& glyph_atlas() override;
        const i_emoji_atlas& emoji_atlas() const override;
        i_emoji_atlas& emoji_atlas() override;
    private:
        i_native_font& find_font(const std::string& aFamilyName, const std::string& aStyleName, font::point_size aSize);
        i_native_font& find_best_font(const std::string& aFamilyName, neogfx::font_style aStyle, font::point_size aSize);
    private:
        void add_ref(font_id aId) override;
        void release(font_id aId) override;
        long use_count(font_id aId) const override;
    private:
        std::shared_ptr<i_native_font_face> add_font(i_native_font_face& aNewFont);
        void cleanup();
    private:
        font_info iDefaultSystemFontInfo;
        fallback_font_info iDefaultFallbackFontInfo;
        FT_Library iFontLib;
        native_font_list iNativeFonts;
        font_family_list iFontFamilies;
    private:
        id_cache iIdCache;
        texture_atlas iGlyphAtlas;
        neogfx::emoji_atlas iEmojiAtlas;
    };
}