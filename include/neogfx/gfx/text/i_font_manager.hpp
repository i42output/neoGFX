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
#include <neolib/jar.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/device_metrics.hpp>
#include "font.hpp"

namespace neogfx
{
    class i_native_font;
    class i_native_font_face;

    class i_texture_atlas;
    class i_emoji_atlas;

    class i_fallback_font_info
    {
    public:
        struct no_fallback : std::logic_error { no_fallback() : std::logic_error("neogfx::i_fallback_font_info::no_fallback") {} };
    public:
        virtual bool has_fallback_for(const std::string& aFontFamilyName) const = 0;
        virtual const std::string& fallback_for(const std::string& aFontFamilyName) const = 0;
    };

    class i_font_manager : public neolib::i_small_cookie_consumer
    {
        friend class native_font;
    public:
        struct bad_font_family_index : std::logic_error { bad_font_family_index() : std::logic_error("neogfx::i_font_manager::bad_font_family_index") {} };
        struct bad_font_style_index : std::logic_error { bad_font_style_index() : std::logic_error("neogfx::i_font_manager::bad_font_style_index") {} };
        struct no_fallback_font : std::logic_error { no_fallback_font() : std::logic_error("neogfx::i_font_manager::no_fallback_font") {} };
    public:
        virtual void* font_library_handle() const = 0;
        virtual const font_info& default_system_font_info() const = 0;
        virtual const i_fallback_font_info& default_fallback_font_info() const = 0;
        virtual std::shared_ptr<i_native_font_face> create_default_font(const i_device_resolution& aDevice) = 0;
        virtual bool has_fallback_font(const i_native_font_face& aExistingFont) const = 0;
        virtual std::shared_ptr<i_native_font_face> create_fallback_font(const i_native_font_face& aExistingFont) = 0;
        virtual std::shared_ptr<i_native_font_face> create_font(const std::string& aFamilyName, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual std::shared_ptr<i_native_font_face> create_font(const std::string& aFamilyName, const std::string& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual std::shared_ptr<i_native_font_face> create_font(const font_info& aInfo, const i_device_resolution& aDevice) = 0;
        virtual std::shared_ptr<i_native_font_face> create_font(i_native_font& aFont, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual std::shared_ptr<i_native_font_face> create_font(i_native_font& aFont, const std::string& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual bool is_font_file(const std::string& aFileName) const = 0;
        virtual std::shared_ptr<i_native_font_face> load_font_from_file(const std::string& aFileName, const i_device_resolution& aDevice) = 0;
        virtual std::shared_ptr<i_native_font_face> load_font_from_file(const std::string& aFileName, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual std::shared_ptr<i_native_font_face> load_font_from_file(const std::string& aFileName, const std::string& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual std::shared_ptr<i_native_font_face> load_font_from_memory(const void* aData, std::size_t aSizeInBytes, const i_device_resolution& aDevice) = 0;
        virtual std::shared_ptr<i_native_font_face> load_font_from_memory(const void* aData, std::size_t aSizeInBytes, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice) = 0;
        virtual std::shared_ptr<i_native_font_face> load_font_from_memory(const void* aData, std::size_t aSizeInBytes, const std::string& aStyleName, font::point_size aSize, const i_device_resolution& aDevice) = 0;
    public:
        virtual uint32_t font_family_count() const = 0;
        virtual std::string font_family(uint32_t aFamilyIndex) const = 0;
        virtual uint32_t font_style_count(uint32_t aFamilyIndex) const = 0;
        virtual std::string font_style(uint32_t aFamilyIndex, uint32_t aStyleIndex) const = 0;
    private:
        virtual font_id allocate_font_id() = 0;
    public:
        virtual const font& font_from_id(font_id aId) const = 0;
    public:
        virtual const i_texture_atlas& glyph_atlas() const = 0;
        virtual i_texture_atlas& glyph_atlas() = 0;
        virtual const i_emoji_atlas& emoji_atlas() const = 0;
        virtual i_emoji_atlas& emoji_atlas() = 0;
    };
}