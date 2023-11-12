// i_font_manager.hpp
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
#include <neolib/core/jar.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/device_metrics.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gfx/text/glyph_text.hpp>

namespace neogfx
{
    class i_native_font;
    class i_native_font_face;

    class i_texture_atlas;
    class i_emoji_atlas;

    enum class system_font_role : uint32_t
    {
        Caption,
        Menu,
        Toolbar,
        StatusBar,
        Widget
    };

    class i_fallback_font_info
    {
    public:
        struct no_fallback : std::logic_error { no_fallback() : std::logic_error("neogfx::i_fallback_font_info::no_fallback") {} };
    public:
        virtual bool has_fallback_for(i_string const& aFontFamilyName) const = 0;
        virtual i_string const& fallback_for(i_string const& aFontFamilyName) const = 0;
    };

    class i_font_manager : public neolib::i_small_cookie_consumer, public i_service
    {
        friend class native_font;
    public:
        struct bad_font_family_index : std::logic_error { bad_font_family_index() : std::logic_error("neogfx::i_font_manager::bad_font_family_index") {} };
        struct bad_font_style_index : std::logic_error { bad_font_style_index() : std::logic_error("neogfx::i_font_manager::bad_font_style_index") {} };
        struct no_fallback_font : std::logic_error { no_fallback_font() : std::logic_error("neogfx::i_font_manager::no_fallback_font") {} };
    public:
        virtual ~i_font_manager() = default;
    public:
        virtual void* font_library_handle() const = 0;
        virtual i_optional<font_info> const& default_system_font_info(system_font_role aRole) const = 0;
        virtual const i_fallback_font_info& default_fallback_font_info() const = 0;
        virtual i_native_font_face& create_default_font(const i_device_resolution& aDevice) = 0;
        virtual bool has_fallback_font(i_native_font_face const& aExistingFont) const = 0;
        virtual i_native_font_face& create_fallback_font(i_native_font_face const& aExistingFont) = 0;
        virtual i_native_font_face& create_font(i_string const& aFamilyName, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual i_native_font_face& create_font(i_string const& aFamilyName, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual i_native_font_face& create_font(const font_info& aInfo, const i_device_resolution& aDevice) = 0;
        virtual i_native_font_face& create_font(i_native_font& aFont, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual i_native_font_face& create_font(i_native_font& aFont, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual i_native_font_face& create_font(i_native_font& aFont, const font_info& aInfo, const i_device_resolution& aDevice) = 0;
        virtual bool is_font_file(i_string const& aFileName) const = 0;
        virtual i_native_font_face& load_font_from_file(i_string const& aFileName, i_device_resolution const& aDevice) = 0;
        virtual i_native_font_face& load_font_from_file(i_string const& aFileName, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual i_native_font_face& load_font_from_file(i_string const& aFileName, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual i_native_font_face& load_font_from_file(i_string const& aFileName, const font_info& aInfo, const i_device_resolution& aDevice) = 0;
        virtual i_native_font_face& load_font_from_memory(const void* aData, std::size_t aSizeInBytes, i_device_resolution const& aDevice) = 0;
        virtual i_native_font_face& load_font_from_memory(const void* aData, std::size_t aSizeInBytes, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual i_native_font_face& load_font_from_memory(const void* aData, std::size_t aSizeInBytes, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual i_native_font_face& load_font_from_memory(const void* aData, std::size_t aSizeInBytes, const font_info& aInfo, const i_device_resolution& aDevice) = 0;
    public:
        virtual uint32_t font_family_count() const = 0;
        virtual i_string const& font_family(uint32_t aFamilyIndex) const = 0;
        virtual uint32_t font_style_count(uint32_t aFamilyIndex) const = 0;
        virtual neogfx::font_style font_style(uint32_t aFamilyIndex, uint32_t aStyleIndex) const = 0;
        virtual i_string const& font_style_name(uint32_t aFamilyIndex, uint32_t aStyleIndex) const = 0;
    private:
        virtual font_id allocate_font_id() = 0;
    public:
        virtual const font& font_from_id(font_id aId) const = 0;
    public:
        virtual i_glyph_text_factory& glyph_text_factory() const = 0;
    public:
        virtual const i_texture_atlas& glyph_atlas() const = 0;
        virtual i_texture_atlas& glyph_atlas() = 0;
        virtual const i_emoji_atlas& emoji_atlas() const = 0;
        virtual i_emoji_atlas& emoji_atlas() = 0;
    public:
        bool has_font(std::string const& aFamily, std::string const& aStyle) const
        {
            for (uint32_t familyIndex = 0; familyIndex < font_family_count(); ++familyIndex)
                if (font_family(familyIndex).to_std_string_view() == aFamily)
                {
                    for (uint32_t styleIndex = 0; styleIndex < font_style_count(familyIndex); ++styleIndex)
                        if (font_style_name(familyIndex, styleIndex).to_std_string_view() == aStyle)
                            return true;
                    return false;
                }
            return false;
        }
    public:
        static uuid const& iid() { static uuid const sIid{ 0x83bbaf78, 0x66a8, 0x4862, 0x9221, { 0x4c, 0xfd, 0x93, 0xfb, 0xf3, 0xe7 } }; return sIid; }
    };
}