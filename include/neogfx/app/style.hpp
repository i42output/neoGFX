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
#include "i_style.hpp"
#include "palette.hpp"

namespace neogfx
{
    class style : public i_style
    {
    public:
        define_declared_event(Changed, changed, style_aspect)
    public:
        style(const std::string& aName);
        style(const std::string& aName, const i_style& aOther);
        style(const i_style& aOther);
        style(const style& aOther);
        virtual ~style();
    public:
        style& operator=(const i_style& aOther);
    public:
        bool operator==(const i_style& aOther) const;
        bool operator!=(const i_style& aOther) const;
    public:
        const std::string& name() const override;
        const neogfx::margins& margins() const override;
        void set_margins(const neogfx::margins& aMargins) override;
        const size& spacing() const override;
        void set_spacing(const size& aSpacing) override;
        const i_palette& palette() const override;
        i_palette& palette() override;
        void set_palette(const i_palette& aPalette) override;
        const neogfx::font_info& font_info() const override;
        void set_font_info(const neogfx::font_info& aFontInfo) override;
        const neogfx::font& font() const override;
    private:
        void handle_change(style_aspect aAspect);
    private:
        std::string iName;
        neogfx::margins iMargins;
        size iSpacing;
        neogfx::palette iPalette;
        neogfx::font_info iFontInfo;
        mutable optional_font iFont;
    };
}