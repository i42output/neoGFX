// palette.hpp
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
#include "i_palette.hpp"

namespace neogfx
{
    class palette : public i_palette
    {
    public:
        define_declared_event(Changed, changed)
    public:
        palette();
        palette(const i_palette& aOther);
        palette(const palette& aOther);
    public:
        palette& operator=(const i_palette& aOther);
    public:
        bool operator==(const i_palette& aOther) const;
        bool operator!=(const i_palette& aOther) const;
    public:
        bool has_color(color_role aRole) const override;
        neogfx::color color(color_role aRole) const override;
        const optional_color& maybe_color(color_role aRole) const override;
        void set_color(color_role aRole, const optional_color& aColor) override;
    private:
        optional_color iThemeColor;
        optional_color iBackgroundColor;
        optional_color iForegroundColor;
        optional_color iTextColor;
        optional_color iSelectionColor;
        optional_color iHoverColor;
        optional_color iWidgetDetailPrimaryColor;
        optional_color iWidgetDetailSecondaryColor;
    };
}