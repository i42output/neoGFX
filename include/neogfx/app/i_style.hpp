// i_style.hpp
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
#include <neogfx/core/event.hpp>
#include <neogfx/core/primitives.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/app/i_palette.hpp>

namespace neogfx
{
    enum class style_aspect : uint32_t
    {
        None        = 0x0000,
        Geometry    = 0x0001,
        Font        = 0x0002,
        Color       = 0x0004,
        Style       = Geometry | Font | Color
    };

    enum class padding_role : uint32_t
    {
        Layout  = 0x0000,
        Widget  = 0x0001,
        Window  = 0x0002,
        Dialog  = 0x0003,
        Menu    = 0x0004,

        COUNT
    };

    inline constexpr style_aspect operator|(style_aspect aLhs, style_aspect aRhs)
    {
        return static_cast<style_aspect>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr style_aspect operator&(style_aspect aLhs, style_aspect aRhs)
    {
        return static_cast<style_aspect>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    enum class font_role : uint32_t
    {
        Caption,
        Menu,
        Toolbar,
        StatusBar,
        Widget
    };

    typedef optional<font_role> optional_font_role;

    class i_style
    {
    public:
        struct no_font_for_role : std::runtime_error { no_font_for_role() : std::runtime_error{ "neogfx::i_style::no_font_for_role" } {} };
    public:
        declare_event(changed, style_aspect)
    public:
        typedef std::array<neogfx::padding, static_cast<std::size_t>(padding_role::COUNT)> padding_list;
    public:
        virtual ~i_style() = default;
    public:
        virtual std::string const& name() const = 0;
        virtual const padding_list& all_padding() const = 0;
        virtual const neogfx::padding& padding(padding_role aPaddingRole) const = 0;
        virtual void set_padding(padding_role aPaddingRole, const neogfx::padding& aPadding) = 0;
        virtual const size& spacing() const = 0;
        virtual void set_spacing(const size& aSpacing) = 0;
        virtual const i_palette& palette() const = 0;
        virtual i_palette& palette() = 0;
        virtual void set_palette(const i_palette& aPalette) = 0;
        virtual void set_palette_color(color_role aRole, const optional_color& aColor) = 0;
        virtual bool font_available(font_role aRole) const = 0;
        virtual const neogfx::font_info& font_info(font_role aRole) const = 0;
        virtual const i_optional<neogfx::font_info>& maybe_font_info(font_role aRole) const = 0;
        virtual void set_font_info(font_role aRole, const neogfx::font_info& aFontInfo) = 0;
        virtual const neogfx::font& font(font_role aRole) const = 0;
    public:
        const neogfx::font_info& font_info() const
        {
            return font_info(font_role::Widget);
        }
        void set_font_info(const neogfx::font_info& aFontInfo)
        {
            set_font_info(font_role::Widget, aFontInfo);
        }
        const neogfx::font& font() const
        {
            return font(font_role::Widget);
        }
    };
}