// font_manager.hpp
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
#include <set>
#include <neolib/core/jar.hpp>
#include <neolib/core/string_ci.hpp>
#include <neogfx/gfx/texture_atlas.hpp>
#include <neogfx/gfx/text/emoji_atlas.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>

typedef struct FT_LibraryRec_* FT_Library;

namespace neogfx
{
    class native_font;

    class fallback_font_info : public i_fallback_font_info
    {
    public:
        fallback_font_info(std::vector<string> aFallbackFontFamilies);
    public:
        bool has_fallback_for(i_string const& aFontFamilyName) const final;
        i_string const& fallback_for(i_string const& aFontFamilyName) const final;
    private:
        std::vector<string> iFallbackFontFamilies;
    };

    class font_manager : public i_font_manager
    {
        friend class native_font_face;
    private:
        using native_font_list = std::list<native_font>;
        using font_family_list = std::map<string, std::vector<native_font_list::iterator>, neolib::ci_less>;
        using id_cache_entry = font ;
        using id_cache = neolib::small_jar<id_cache_entry>;
    public:
        struct error_initializing_font_library : std::runtime_error { error_initializing_font_library() : std::runtime_error("neogfx::font_manager::error_initializing_font_library") {} };
        struct no_matching_font_found : std::runtime_error { no_matching_font_found() : std::runtime_error("neogfx::font_manager::no_matching_font_found") {} };
        struct failed_to_allocate_glyph_space : std::runtime_error { failed_to_allocate_glyph_space() : std::runtime_error("neogfx::font_manager::failed_to_allocate_glyph_space") {} };
    public:
        font_manager();
        ~font_manager();
    public:
        void* font_library_handle() const final;
        i_optional<font_info> const& default_system_font_info(system_font_role aRole) const final;
        const i_fallback_font_info& default_fallback_font_info() const final;
        i_native_font_face& create_default_font(const i_device_resolution& aDevice) final;
        bool has_fallback_font(const i_native_font_face& aExistingFont) const final;
        i_native_font_face& create_fallback_font(const i_native_font_face& aExistingFont) final;
        i_native_font_face& create_font(i_string const& aFamilyName, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) final;
        i_native_font_face& create_font(i_string const& aFamilyName, neogfx::font_style aStyle, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) final;
        i_native_font_face& create_font(const font_info& aInfo, const i_device_resolution& aDevice) final;
        i_native_font_face& create_font(i_native_font& aFont, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) final;
        i_native_font_face& create_font(i_native_font& aFont, neogfx::font_style aStyle, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) final;
        i_native_font_face& create_font(i_native_font& aFont, const font_info& aInfo, const i_device_resolution& aDevice) final;
        bool is_font_file(i_string const& aFileName) const final;
        i_native_font_face& load_font_from_file(i_string const& aFileName, const i_device_resolution& aDevice) final;
        i_native_font_face& load_font_from_file(i_string const& aFileName, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) final;
        i_native_font_face& load_font_from_file(i_string const& aFileName, neogfx::font_style aStyle, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) final;
        i_native_font_face& load_font_from_file(i_string const& aFileName, const font_info& aInfo, const i_device_resolution& aDevice) final;
        i_native_font_face& load_font_from_memory(const void* aData, std::size_t aSizeInBytes, const i_device_resolution& aDevice) final;
        i_native_font_face& load_font_from_memory(const void* aData, std::size_t aSizeInBytes, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) final;
        i_native_font_face& load_font_from_memory(const void* aData, std::size_t aSizeInBytes, neogfx::font_style aStyle, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) final;
        i_native_font_face& load_font_from_memory(const void* aData, std::size_t aSizeInBytes, const font_info& aInfo, const i_device_resolution& aDevice) final;
    public:
        uint32_t font_family_count() const final;
        i_string const& font_family(uint32_t aFamilyIndex) const final;
        uint32_t font_style_count(uint32_t aFamilyIndex) const final;
        neogfx::font_style font_style(uint32_t aFamilyIndex, uint32_t aStyleIndex) const final;
        i_string const& font_style_name(uint32_t aFamilyIndex, uint32_t aStyleIndex) const final;
    private:
        font_id allocate_font_id() final;
    public:
        const font& font_from_id(font_id aId) const final;
    public:
        i_glyph_text_factory& glyph_text_factory() const final;
    public:
        const i_texture_atlas& glyph_atlas() const final;
        i_texture_atlas& glyph_atlas() final;
        const i_emoji_atlas& emoji_atlas() const final;
        i_emoji_atlas& emoji_atlas() final;
    protected:
        void add_ref(font_id aId) final;
        void release(font_id aId) final;
        long use_count(font_id aId) const final;
    private:
        i_native_font& find_font(i_string const& aFamilyName, i_string const& aStyleName, font::point_size aSize);
        i_native_font& find_font(font_info const& aFontInfo);
        i_native_font& find_best_font(i_string const& aFamilyName, neogfx::font_style aStyle, font::point_size aSize);
    private:
        i_native_font_face& add_font(const ref_ptr<i_native_font_face>& aNewFont);
        void cleanup();
    private:
        mutable std::unordered_map<system_font_role, optional<font_info>> iDefaultSystemFontInfo;
        mutable std::optional<fallback_font_info> iDefaultFallbackFontInfo;
        FT_Library iFontLib;
        native_font_list iNativeFonts;
        font_family_list iFontFamilies;
    private:
        id_cache iIdCache;
        std::unique_ptr<i_glyph_text_factory> iGlyphTextFactory;
        texture_atlas iGlyphAtlas;
        neogfx::emoji_atlas iEmojiAtlas;
    };
}