// pen.hpp
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

#include <neogfx/gfx/line.hpp>
#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/gradient.hpp>

namespace neogfx
{
    class pen
    {
    public:
        // construction
    public:
        pen() :
            iWidth{ 0.0 }, iStyle{ line_style::None }, iCustomDash{}, iAntiAliased { true }
        {}
        pen(color const& aColor, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iStyle{ line_style::Solid }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(color const& aColor, line_style aStyle, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iStyle{ aStyle }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(color const& aColor, line_dash const& aCustomDash, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iStyle{ line_style::CustomDash }, iCustomDash{ aCustomDash }, iAntiAliased{ aAntiAliased }
        {}
        pen(color const& aColor, dimension aWidth, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iStyle{ line_style::Solid }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(color const& aColor, dimension aWidth, line_style aStyle, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iStyle{ aStyle }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(color const& aColor, dimension aWidth, line_dash const& aCustomDash, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iStyle{ line_style::CustomDash }, iCustomDash{ aCustomDash }, iAntiAliased{ aAntiAliased }
        {}
        pen(gradient const& aColor, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iStyle{ line_style::Solid }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(gradient const& aColor, line_style aStyle, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iStyle{ aStyle }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(gradient const& aColor, line_dash const& aCustomDash, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iStyle{ line_style::CustomDash }, iCustomDash{ aCustomDash }, iAntiAliased{ aAntiAliased }
        {}
        pen(gradient const& aColor, dimension aWidth, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iStyle{ line_style::Solid }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(gradient const& aColor, line_style aStyle, dimension aWidth, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iStyle{ aStyle }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(gradient const& aColor, line_dash const& aCustomDash, dimension aWidth, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iStyle{ line_style::CustomDash }, iCustomDash{ aCustomDash }, iAntiAliased{ aAntiAliased }
        {}
        pen(color_or_gradient const& aColor, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iStyle{ line_style::Solid }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(color_or_gradient const& aColor, line_style aStyle, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iStyle{ aStyle }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(color_or_gradient const& aColor, line_dash const& aCustomDash, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iStyle{ line_style::CustomDash }, iCustomDash{ aCustomDash }, iAntiAliased{ aAntiAliased }
        {}
        pen(color_or_gradient const& aColor, dimension aWidth, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iStyle{ line_style::Solid }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(color_or_gradient const& aColor, line_style aStyle, dimension aWidth, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iStyle{ aStyle }, iCustomDash{}, iAntiAliased{ aAntiAliased }
        {}
        pen(color_or_gradient const& aColor, line_dash const& aCustomDash, dimension aWidth, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iStyle{ line_style::CustomDash }, iCustomDash{ aCustomDash }, iAntiAliased{ aAntiAliased }
        {}
        // operations
    public:
        color_or_gradient const& color() const 
        { 
            return iColor; 
        }
        pen& set_color(color_or_gradient const& aColor)
        {
            iColor = aColor;
            return *this;
        }
        dimension width() const
        { 
            return iWidth; 
        }
        pen& set_width(dimension aWidth)
        {
            iWidth = aWidth;
            return *this;
        }
        line_style style() const
        {
            return iStyle;
        }
        pen& set_style(line_style aStyle)
        {
            iStyle = aStyle;
            return *this;
        }
        line_dash const& custom_dash() const
        {
            return iCustomDash;
        }
        pen& set_custom_dash(line_dash const& aCustomDash)
        {
            iCustomDash = aCustomDash;
            return *this;
        }
        std::optional<neogfx::color> const& secondary_color() const
        {
            return iSecondaryColor;
        }
        pen& set_secondary_color(std::optional<neogfx::color> const& aColor)
        {
            iSecondaryColor = aColor;
            return *this;
        }
        bool anti_aliased() const 
        { 
            return iAntiAliased; 
        }
        pen& set_anti_aliased(bool aAntiAliased)
        {
            iAntiAliased = aAntiAliased;
            return *this;
        }
    private:
        color_or_gradient iColor;
        dimension iWidth;
        line_style iStyle;
        line_dash iCustomDash;
        std::optional<neogfx::color> iSecondaryColor;
        bool iAntiAliased;
    };

    inline bool has_stipple(pen const& aPen)
    {
        if (aPen.width() == 0.0)
            return false;
        switch (aPen.style())
        {
        case line_style::None:
            return false;
        case line_style::Solid:
            return false;
        case line_style::Dash:
        case line_style::Dot:
        case line_style::DashDot:
        case line_style::DashDotDot:
        case line_style::CustomDash:
            return true;
        default:
            return false;
        }
    }

    inline stipple to_stipple(pen const& aPen)
    {
        switch (aPen.style())
        {
        case line_style::None:
            return stipple{ 0u, 0.0 };
        case line_style::Solid:
            return stipple{};
        case line_style::Dash:
            return stipple{ { 2.0, 1.0 }, aPen.width()};
        case line_style::Dot:
            return stipple{ 0x5555u, aPen.width() };
        case line_style::DashDot:
            return stipple{ { 2.0, 1.0, 1.0, 1.0 }, aPen.width() };
        case line_style::DashDotDot:
            return stipple{ { 2.0, 1.0, 1.0, 1.0, 1.0, 1.0 }, aPen.width() };
        case line_style::CustomDash:
            return aPen.custom_dash();
        default:
            return stipple{ 0u, 0.0 };
        }
    }

    typedef std::optional<pen> optional_pen;
}