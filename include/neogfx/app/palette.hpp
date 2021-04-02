// palette.hpp
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
#include <neogfx/app/i_palette.hpp>

namespace neogfx
{
    struct palette_proxy_t {};
    inline const palette_proxy_t palette_proxy() { return palette_proxy_t{}; }
    struct current_style_palette_proxy_t {};
    inline const current_style_palette_proxy_t current_style_palette_proxy() { return current_style_palette_proxy_t{}; }

    class palette : public i_palette
    {
    public:
        typedef i_palette abstract_type;
    public:
        define_declared_event(Changed, changed)
    public:
        palette();
        palette(current_style_palette_proxy_t);
        palette(const i_palette& aOther);
        palette(const palette& aOther);
        palette(const i_palette& aOther, palette_proxy_t);
        palette(const palette& aOther, palette_proxy_t);
    public:
        palette& operator=(const i_palette& aOther);
        palette& operator=(const palette& aOther);
    public:
        bool operator==(const palette& aOther) const;
        bool operator!=(const palette& aOther) const;
        bool operator==(const i_palette& aOther) const override;
        bool operator!=(const i_palette& aOther) const override;
    public:
        bool has_color(color_role aRole) const override;
        neogfx::color color(color_role aRole) const override;
        const optional_color& maybe_color(color_role aRole) const override;
        void set_color(color_role aRole, const optional_color& aColor) override;
    public:
        neogfx::color default_text_color_for_widget(const i_widget& aWidget) const override;
    public:
        bool has_proxy() const override;
        const i_palette& proxy() const override;
    protected:
        virtual const i_palette* proxy_ptr() const override;
    private:
        std::optional<const i_palette*> iProxy;
        optional_color iThemeColor;
        optional_color iBackgroundColor;
        optional_color iForegroundColor;
        optional_color iBaseColor;
        optional_color iAlternateBaseColor;
        optional_color iTextColor;
        optional_color iSelectionColor;
        optional_color iSelectedTextColor;
        optional_color iHoverColor;
        optional_color iPrimaryAccentColor;
        optional_color iSecondaryAccentColor;
        optional_color iVoidColor;
    };
}