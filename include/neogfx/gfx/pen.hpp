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
            iWidth{ 0.0 }, iAntiAliased{ true }
        {}
        pen(color const& aColor, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iAntiAliased{ aAntiAliased }
        {}
        pen(color const& aColor, dimension aWidth, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iAntiAliased{ aAntiAliased }
        {}
        pen(gradient const& aColor, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iAntiAliased{ aAntiAliased }
        {}
        pen(gradient const& aColor, dimension aWidth, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iAntiAliased{ aAntiAliased }
        {}
        pen(color_or_gradient const& aColor, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ 1.0 }, iAntiAliased{ aAntiAliased }
        {}
        pen(color_or_gradient const& aColor, dimension aWidth, bool aAntiAliased = true) :
            iColor{ aColor }, iWidth{ aWidth }, iAntiAliased{ aAntiAliased }
        {}
        // operations
    public:
        color_or_gradient const& color() const 
        { 
            return iColor; 
        }
        dimension width() const
        { 
            return iWidth; 
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
    private:
        color_or_gradient iColor;
        dimension iWidth;
        std::optional<neogfx::color> iSecondaryColor;
        bool iAntiAliased;
    };

    typedef std::optional<pen> optional_pen;
}