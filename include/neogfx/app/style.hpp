// style.hpp
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
#include "i_style.hpp"
#include "palette.hpp"

namespace neogfx
{
    class style : public i_style
    {
    public:
        define_declared_event(Changed, changed, style_aspect)
    public:
        style(std::string const& aName);
        style(std::string const& aName, const i_style& aOther);
        style(const i_style& aOther);
        style(const style& aOther);
        virtual ~style();
    public:
        style& operator=(const i_style& aOther);
    public:
        bool operator==(const i_style& aOther) const;
        bool operator!=(const i_style& aOther) const;
    public:
        std::string const& name() const final;
        const margin_list& all_margin() const final;
        const neogfx::margin& margin(margin_role aMarginRole) const final;
        void set_margin(margin_role aMarginRole, const neogfx::margin& aMargin) final;
        const border_list& all_border() const final;
        const neogfx::border& border(border_role aBorderRole) const final;
        void set_border(border_role aBorderRole, const neogfx::border& aBorder) final;
        const padding_list& all_padding() const final;
        const neogfx::padding& padding(padding_role aPaddingRole) const final;
        void set_padding(padding_role aPaddingRole, const neogfx::padding& aPadding) final;
        const size& spacing() const final;
        void set_spacing(const size& aSpacing) final;
        const i_palette& palette() const final;
        i_palette& palette() final;
        void set_palette(const i_palette& aPalette) final;
        void set_palette_color(color_role aRole, const optional_color& aColor) final;
        bool font_available(font_role aRole) const final;
        const neogfx::font_info& font_info(font_role aRole) const final;
        const i_optional<neogfx::font_info>& maybe_font_info(font_role aRole) const final;
        void set_font_info(font_role aRole, const neogfx::font_info& aFontInfo) final;
        const neogfx::font& font(font_role aRole) const final;
    private:
        void handle_change(style_aspect aAspect);
    private:
        std::string iName;
        margin_list iMargin;
        border_list iBorder;
        padding_list iPadding;
        size iSpacing;
        neogfx::palette iPalette;
        mutable std::unordered_map<font_role, optional_font_info> iFontInfo;
        mutable std::unordered_map<font_role, optional_font> iFont;
    };
}